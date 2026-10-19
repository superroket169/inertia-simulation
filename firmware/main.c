#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <util/atomic.h>
#include <stdbool.h>
#include <stdint.h>

#include "config.h"
#include "motor.h"

_Static_assert(F_CPU / 64 / CONTROL_HZ - 1 <= 255, "CONTROL_HZ too low for Timer2 /64");

// ---- Encoders ----

static volatile int32_t enc_count[AXIS_COUNT];

// x2 decoding: interrupt on every edge of channel A, channel B gives direction.
// TODO: confirm sign on the rig (swap B wires or flip here).
ISR(INT0_vect)
{
    uint8_t p = PIND;
    bool a = p & _BV(ENC0_A_BIT);
    bool b = p & _BV(ENC0_B_BIT);
    enc_count[0] += (a == b) ? 1 : -1;
}

ISR(INT1_vect)
{
    uint8_t p = PIND;
    bool a = p & _BV(ENC1_A_BIT);
    bool b = p & _BV(ENC1_B_BIT);
    enc_count[1] += (a == b) ? 1 : -1;
}

static void encoder_init(void)
{
    uint8_t mask = _BV(ENC0_A_BIT) | _BV(ENC1_A_BIT) | _BV(ENC0_B_BIT) | _BV(ENC1_B_BIT);
    DDRD &= ~mask;
    PORTD |= mask; // pull-ups for open-collector encoders

    EICRA = _BV(ISC00) | _BV(ISC10); // any edge
    EIMSK = _BV(INT0) | _BV(INT1);
}

static int32_t encoder_read(uint8_t axis)
{
    int32_t v;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) { v = enc_count[axis]; }
    return v;
}

static void encoder_zero(uint8_t axis)
{
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) { enc_count[axis] = 0; }
}

// ---- Serial: SMC3 frames from FlyPT Mover ----
//
// Frame: '[' axis hi lo ']'  with axis 'A' or 'B', value big-endian.
// Mover template: [A<R0.A1.B>][B<R0.A2.B>]

static volatile uint16_t target_raw[AXIS_COUNT];
static volatile bool frame_received;

ISR(USART_RX_vect)
{
    static uint8_t state;
    static uint8_t axis;
    static uint16_t value;

    uint8_t c = UDR0;

    switch (state) {
    case 0:
        if (c == '[') state = 1;
        break;
    case 1:
        if (c == 'A' || c == 'B') {
            axis = c - 'A';
            state = 2;
        } else {
            state = 0;
        }
        break;
    case 2:
        value = (uint16_t)c << 8;
        state = 3;
        break;
    case 3:
        value |= c;
        state = 4;
        break;
    case 4:
        if (c == ']') {
            target_raw[axis] = value;
            frame_received = true;
        }
        state = 0;
        break;
    }
}

static void uart_init(void)
{
    UCSR0A = _BV(U2X0);
    UBRR0 = F_CPU / 8 / SERIAL_BAUD - 1;
    UCSR0B = _BV(RXEN0) | _BV(RXCIE0);
    UCSR0C = _BV(UCSZ01) | _BV(UCSZ00); // 8N1
}

static uint16_t target_raw_read(uint8_t axis)
{
    uint16_t v;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) { v = target_raw[axis]; }
    return v;
}

// 0..TARGET_RAW_MAX -> -AXIS_HALF_TRAVEL_COUNTS..+AXIS_HALF_TRAVEL_COUNTS
static int32_t raw_to_counts(uint16_t raw)
{
    return (int32_t)((int64_t)raw * 2 * AXIS_HALF_TRAVEL_COUNTS / TARGET_RAW_MAX)
        - AXIS_HALF_TRAVEL_COUNTS;
}

// ---- Control tick (Timer2, CONTROL_HZ) ----

static volatile bool tick;

ISR(TIMER2_COMPA_vect)
{
    tick = true;
}

static void tick_init(void)
{
    TCCR2A = _BV(WGM21); // CTC
    TCCR2B = _BV(CS22);  // /64
    OCR2A = F_CPU / 64 / CONTROL_HZ - 1;
    TIMSK2 = _BV(OCIE2A);
}

// ---- PID ----

typedef struct {
    float integral;
    int32_t prev_error;
} Pid;

static int16_t pid_step(Pid *pid, int32_t error)
{
    const float dt = 1.0f / CONTROL_HZ;

    pid->integral += (float)error * dt;
    if (pid->integral > PID_I_LIMIT) pid->integral = PID_I_LIMIT;
    if (pid->integral < -PID_I_LIMIT) pid->integral = -PID_I_LIMIT;

    float derivative = (float)(error - pid->prev_error) / dt;
    pid->prev_error = error;

    float out = PID_KP * error + PID_KI * pid->integral + PID_KD * derivative;
    if (out > MOTOR_DUTY_MAX) out = MOTOR_DUTY_MAX;
    if (out < -MOTOR_DUTY_MAX) out = -MOTOR_DUTY_MAX;
    return (int16_t)out;
}

// ---- Safety inputs ----

static void inputs_init(void)
{
    DDRB &= ~_BV(HOA_AUTO_BIT);
    HOA_AUTO_PORT |= _BV(HOA_AUTO_BIT);

    DDRC &= ~(_BV(HOME0_BIT) | _BV(HOME1_BIT));
    PORTC |= _BV(HOME0_BIT) | _BV(HOME1_BIT);
}

static bool hoa_is_auto(void)
{
    return !(HOA_AUTO_PIN & _BV(HOA_AUTO_BIT));
}

// ---- Main ----

typedef enum {
    STATE_DISABLED, // HOA not in auto, motors off
    STATE_HOMING,
    STATE_RUNNING,
    STATE_FAULT,    // latched until HOA leaves auto
} RigState;

int main(void)
{
    MCUSR = 0;
    wdt_disable();

    for (uint8_t i = 0; i < AXIS_COUNT; i++) target_raw[i] = TARGET_RAW_MAX / 2;

    motor_init();
    motor_off_all();
    encoder_init();
    uart_init();
    tick_init();
    inputs_init();

    wdt_enable(WDTO_60MS);
    sei();

    RigState state = STATE_DISABLED;
    bool homed = false;
    uint16_t frame_age_ticks = 0;
    Pid pid[AXIS_COUNT] = {0};

    for (;;) {
        if (!tick) continue;
        tick = false;
        wdt_reset();

        if (frame_received) {
            frame_received = false;
            frame_age_ticks = 0;
        } else if (frame_age_ticks < UINT16_MAX) {
            frame_age_ticks++;
        }

        if (!hoa_is_auto()) {
            motor_off_all();
            state = STATE_DISABLED;
            continue;
        }

        switch (state) {
        case STATE_DISABLED:
            for (uint8_t i = 0; i < AXIS_COUNT; i++) pid[i] = (Pid){0};
            state = homed ? STATE_RUNNING : STATE_HOMING;
            break;

        case STATE_HOMING:
            // TODO: drive each axis slowly to its home switch, zero there,
            //       then move to center. Until switches exist, power-on
            //       position is taken as center.
            for (uint8_t i = 0; i < AXIS_COUNT; i++) encoder_zero(i);
            homed = true;
            state = STATE_RUNNING;
            break;

        case STATE_RUNNING:
            if (frame_age_ticks > SERIAL_TIMEOUT_MS * CONTROL_HZ / 1000) {
                // TODO: ramp slowly to center instead of holding the last target.
            }
            for (uint8_t i = 0; i < AXIS_COUNT; i++) {
                int32_t pos = encoder_read(i);
                int32_t target = raw_to_counts(target_raw_read(i));
                int32_t error = target - pos;

                bool overtravel = pos > AXIS_HALF_TRAVEL_COUNTS + OVERTRAVEL_FAULT_COUNTS
                    || pos < -AXIS_HALF_TRAVEL_COUNTS - OVERTRAVEL_FAULT_COUNTS;
                bool lagging = error > FOLLOW_ERROR_FAULT_COUNTS
                    || error < -FOLLOW_ERROR_FAULT_COUNTS;
                if (overtravel || lagging) {
                    state = STATE_FAULT;
                    break;
                }

                motor_set(i, pid_step(&pid[i], error));
            }
            if (state == STATE_FAULT) motor_off_all();
            break;

        case STATE_FAULT:
            motor_off_all();
            break;
        }
    }
}
