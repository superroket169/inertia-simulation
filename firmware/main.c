#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <util/atomic.h>
#include <stdbool.h>
#include <stdint.h>

#include "config.h"
#include "motor.h"
#include "smc3.h"

_Static_assert(F_CPU / 64 / CONTROL_HZ - 1 <= 255, "CONTROL_HZ too low for Timer2 /64");

// ---- Encoders ----

static volatile int32_t enc_count[AXIS_COUNT];

// x2 decoding: interrupt on every edge of channel A, channel B gives direction.
// Channel A on PORTD (INT0/INT1), channel B on PORTC.
// TODO: confirm sign on the rig (swap B wires or flip here).
ISR(INT0_vect)
{
    bool a = PIND & _BV(ENC0_A_BIT);
    bool b = PINC & _BV(ENC0_B_BIT);
    enc_count[0] += (a == b) ? 1 : -1;
}

ISR(INT1_vect)
{
    bool a = PIND & _BV(ENC1_A_BIT);
    bool b = PINC & _BV(ENC1_B_BIT);
    enc_count[1] += (a == b) ? 1 : -1;
}

static void encoder_init(void)
{
    // Pull-ups for open-collector encoders.
    uint8_t mask_a = _BV(ENC0_A_BIT) | _BV(ENC1_A_BIT);
    DDRD &= ~mask_a;
    PORTD |= mask_a;

    uint8_t mask_b = _BV(ENC0_B_BIT) | _BV(ENC1_B_BIT);
    DDRC &= ~mask_b;
    PORTC |= mask_b;

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

static volatile uint16_t target_raw[AXIS_COUNT];
static volatile bool frame_received;

ISR(USART_RX_vect)
{
    static Smc3Parser parser;

    if (smc3_feed(&parser, UDR0)) {
        target_raw[parser.axis] = parser.value;
        frame_received = true;
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

// ---- Panel: HOA selector per axis, driver power ----

static void panel_init(void)
{
    AXIS0_AUTO_DDR &= ~_BV(AXIS0_AUTO_BIT);
    AXIS0_AUTO_PORT |= _BV(AXIS0_AUTO_BIT);
    AXIS0_HAND_DDR &= ~_BV(AXIS0_HAND_BIT);
    AXIS0_HAND_PORT |= _BV(AXIS0_HAND_BIT);
    AXIS1_AUTO_DDR &= ~_BV(AXIS1_AUTO_BIT);
    AXIS1_AUTO_PORT |= _BV(AXIS1_AUTO_BIT);
    AXIS1_HAND_DDR &= ~_BV(AXIS1_HAND_BIT);
    AXIS1_HAND_PORT |= _BV(AXIS1_HAND_BIT);

    DRIVER_POWER_DDR &= ~_BV(DRIVER_POWER_BIT);
    DRIVER_POWER_PORT |= _BV(DRIVER_POWER_BIT);
}

// Auto and Hand both active is not a valid selector position, so it does
// not count as Auto.
static bool axis_is_auto(uint8_t axis)
{
    bool is_auto, is_hand;
    if (axis == 0) {
        is_auto = !(AXIS0_AUTO_PIN & _BV(AXIS0_AUTO_BIT));
        is_hand = !(AXIS0_HAND_PIN & _BV(AXIS0_HAND_BIT));
    } else {
        is_auto = !(AXIS1_AUTO_PIN & _BV(AXIS1_AUTO_BIT));
        is_hand = !(AXIS1_HAND_PIN & _BV(AXIS1_HAND_BIT));
    }
    return is_auto && !is_hand;
}

static bool driver_powered(void)
{
    return !(DRIVER_POWER_PIN & _BV(DRIVER_POWER_BIT));
}

// ---- Main ----

typedef enum {
    AXIS_DISABLED, // not in Auto or no driver power: PWM 0
    AXIS_HOMING,
    AXIS_RUNNING,
    AXIS_FAULT,    // latched until the axis leaves Auto
} AxisState;

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
    panel_init();

    wdt_enable(WDTO_60MS);
    sei();

    AxisState state[AXIS_COUNT] = {AXIS_DISABLED, AXIS_DISABLED};
    bool homed[AXIS_COUNT] = {false, false};
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

        if (frame_age_ticks > SERIAL_TIMEOUT_MS * CONTROL_HZ / 1000) {
            // TODO: ramp slowly to center instead of holding the last target (issue #12).
        }

        bool powered = driver_powered();

        for (uint8_t i = 0; i < AXIS_COUNT; i++) {
            if (!powered || !axis_is_auto(i)) {
                state[i] = AXIS_DISABLED;
                motor_set(i, 0);
                continue;
            }

            switch (state[i]) {
            case AXIS_DISABLED:
                pid[i] = (Pid){0};
                state[i] = homed[i] ? AXIS_RUNNING : AXIS_HOMING;
                break;

            case AXIS_HOMING:
                // TODO: the pinout has no home switches, so this needs an
                //       absolute encoder (issue #6). Until then the position
                //       at the first Auto is taken as center.
                encoder_zero(i);
                homed[i] = true;
                state[i] = AXIS_RUNNING;
                break;

            case AXIS_RUNNING: {
                int32_t pos = encoder_read(i);
                int32_t target = raw_to_counts(target_raw_read(i));
                int32_t error = target - pos;

                bool overtravel = pos > AXIS_HALF_TRAVEL_COUNTS + OVERTRAVEL_FAULT_COUNTS
                    || pos < -AXIS_HALF_TRAVEL_COUNTS - OVERTRAVEL_FAULT_COUNTS;
                bool lagging = error > FOLLOW_ERROR_FAULT_COUNTS
                    || error < -FOLLOW_ERROR_FAULT_COUNTS;
                if (overtravel || lagging) {
                    state[i] = AXIS_FAULT;
                    motor_set(i, 0);
                    break;
                }

                motor_set(i, pid_step(&pid[i], error));
                break;
            }

            case AXIS_FAULT:
                motor_set(i, 0);
                break;
            }
        }
    }
}
