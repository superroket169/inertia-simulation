#include "motor.h"
#include "config.h"

// Pin ports are fixed by the Nano pinout in config.h:
//   motor 0 PWM D9  = PB1 (OC1A), direction D5/D6   = PD5/PD6
//   motor 1 PWM D10 = PB2 (OC1B), direction D11/D12 = PB3/PB4

#define MOTOR0_DIR_MASK (_BV(MOTOR0_DIR1_BIT) | _BV(MOTOR0_DIR2_BIT))
#define MOTOR1_DIR_MASK (_BV(MOTOR1_DIR1_BIT) | _BV(MOTOR1_DIR2_BIT))

_Static_assert(MOTOR_DUTY_MAX <= PWM_TOP, "MOTOR_DUTY_MAX must not exceed PWM_TOP");

// Fast PWM with OCR1x = 0 still emits a one-clock spike every period, so a
// zero duty also disconnects OC1x from the pin and holds the pin low.
// Timer1 16-bit registers are only touched from the main loop (no ISR uses
// Timer1), so the shared TEMP register needs no atomic block here.
static void pwm_write(uint8_t axis, uint16_t duty)
{
    if (axis == 0) {
        if (duty == 0) {
            TCCR1A &= (uint8_t)~_BV(COM1A1);
            PORTB  &= (uint8_t)~_BV(PB1);
            OCR1A = 0;
        } else {
            OCR1A = duty;
            TCCR1A |= _BV(COM1A1);
        }
    } else {
        if (duty == 0) {
            TCCR1A &= (uint8_t)~_BV(COM1B1);
            PORTB  &= (uint8_t)~_BV(PB2);
            OCR1B = 0;
        } else {
            OCR1B = duty;
            TCCR1A |= _BV(COM1B1);
        }
    }
}

// dir: +1 -> DIR1 high, -1 -> DIR2 high, 0 -> both low.
// The pin being released is driven low before the other is driven high, so
// both inputs are never high at the same time during a reversal.
static void dir_write(uint8_t axis, int8_t dir)
{
    if (axis == 0) {
        if (dir > 0) {
            PORTD &= (uint8_t)~_BV(MOTOR0_DIR2_BIT);
            PORTD |= _BV(MOTOR0_DIR1_BIT);
        } else if (dir < 0) {
            PORTD &= (uint8_t)~_BV(MOTOR0_DIR1_BIT);
            PORTD |= _BV(MOTOR0_DIR2_BIT);
        } else {
            PORTD &= (uint8_t)~MOTOR0_DIR_MASK;
        }
    } else {
        if (dir > 0) {
            PORTB &= (uint8_t)~_BV(MOTOR1_DIR2_BIT);
            PORTB |= _BV(MOTOR1_DIR1_BIT);
        } else if (dir < 0) {
            PORTB &= (uint8_t)~_BV(MOTOR1_DIR1_BIT);
            PORTB |= _BV(MOTOR1_DIR2_BIT);
        } else {
            PORTB &= (uint8_t)~MOTOR1_DIR_MASK;
        }
    }
}

void motor_init(void)
{
    // Outputs driven low before they are enabled, so nothing moves at boot.
    PORTD &= (uint8_t)~MOTOR0_DIR_MASK;
    PORTB &= (uint8_t)~(MOTOR1_DIR_MASK | _BV(PB1) | _BV(PB2));
    DDRD  |= MOTOR0_DIR_MASK;
    DDRB  |= MOTOR1_DIR_MASK | _BV(PB1) | _BV(PB2);

    // Timer1 stopped while configuring. ICR1 is not double-buffered in
    // mode 14, so it is set before the clock starts.
    TCCR1B = 0;
    TCCR1A = _BV(WGM11);                   // mode 14: WGM13:10 = 1110
    ICR1   = PWM_TOP;                      // 16 MHz / (799 + 1) = 20 kHz
    OCR1A  = 0;
    OCR1B  = 0;
    TCNT1  = 0;
    // COM1A1/COM1B1 (non-inverting) are set per axis in pwm_write().
    TCCR1B = _BV(WGM13) | _BV(WGM12) | _BV(CS10);   // prescaler 1
}

void motor_set(uint8_t axis, int16_t duty)
{
    // Widen first: -INT16_MIN does not fit in int16_t.
    int32_t d = duty;
    uint16_t mag;

    if (axis >= AXIS_COUNT) return;

    if (d >  MOTOR_DUTY_MAX) d =  MOTOR_DUTY_MAX;
    if (d < -MOTOR_DUTY_MAX) d = -MOTOR_DUTY_MAX;

    if (d == 0) {
        pwm_write(axis, 0);
        dir_write(axis, 0);
        return;
    }

    mag = (uint16_t)(d < 0 ? -d : d);
    dir_write(axis, d > 0 ? 1 : -1);
    pwm_write(axis, mag);
}

void motor_off_all(void)
{
    for (uint8_t i = 0; i < AXIS_COUNT; i++) {
        pwm_write(i, 0);
        dir_write(i, 0);
    }
}
