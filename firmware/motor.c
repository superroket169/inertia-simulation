#include <avr/io.h>

#include "motor.h"
#include "config.h"

_Static_assert(MOTOR_DUTY_MAX <= PWM_TOP, "MOTOR_DUTY_MAX above PWM_TOP");

// PWM a on Timer0, PWM b on Timer1 (see docs/PINOUT.md):
//   axis 0: a = OC0A (D6, PD6), b = OC1A (D9, PB1)
//   axis 1: a = OC0B (D5, PD5), b = OC1B (D10, PB2)
// TODO: confirm which of a/b is RPWM; positive duty drives a.

#if PWM_FREQ == PWM_7812HZ
// Fast PWM, 8 bit, /8: 16 MHz / (8 * 256)
#define TIMER0_WGM_A (_BV(WGM01) | _BV(WGM00))
#define TIMER0_CS    _BV(CS01)
#define TIMER1_WGM_A _BV(WGM10)
#define TIMER1_WGM_B _BV(WGM12)
#define TIMER1_CS    _BV(CS11)
#elif PWM_FREQ == PWM_31372HZ
// Phase correct PWM, 8 bit, /1: 16 MHz / 510
#define TIMER0_WGM_A _BV(WGM00)
#define TIMER0_CS    _BV(CS00)
#define TIMER1_WGM_A _BV(WGM10)
#define TIMER1_WGM_B 0
#define TIMER1_CS    _BV(CS10)
#elif PWM_FREQ == PWM_3921HZ
// Phase correct PWM, 8 bit, /8: 16 MHz / (8 * 510)
#define TIMER0_WGM_A _BV(WGM00)
#define TIMER0_CS    _BV(CS01)
#define TIMER1_WGM_A _BV(WGM10)
#define TIMER1_WGM_B 0
#define TIMER1_CS    _BV(CS11)
#else
#error "unknown PWM_FREQ"
#endif

typedef enum { PWM_0A, PWM_0B, PWM_1A, PWM_1B } PwmChannel;

// In fast PWM a compare value of 0 still gives a one-tick spike every
// period, so 0 disconnects the pin from the timer and the pin falls back
// to its PORT bit, which stays low.
static void pwm_write(PwmChannel ch, uint8_t value)
{
    switch (ch) {
    case PWM_0A:
        if (value) { OCR0A = value; TCCR0A |= _BV(COM0A1); }
        else TCCR0A &= ~_BV(COM0A1);
        break;
    case PWM_0B:
        if (value) { OCR0B = value; TCCR0A |= _BV(COM0B1); }
        else TCCR0A &= ~_BV(COM0B1);
        break;
    case PWM_1A:
        if (value) { OCR1A = value; TCCR1A |= _BV(COM1A1); }
        else TCCR1A &= ~_BV(COM1A1);
        break;
    case PWM_1B:
        if (value) { OCR1B = value; TCCR1A |= _BV(COM1B1); }
        else TCCR1A &= ~_BV(COM1B1);
        break;
    }
}

void motor_init(void)
{
    // PORT low before DDR so the pins never glitch high.
    PORTD &= ~(_BV(PD5) | _BV(PD6));
    PORTB &= ~(_BV(PB1) | _BV(PB2));
    DDRD |= _BV(PD5) | _BV(PD6);
    DDRB |= _BV(PB1) | _BV(PB2);

    OCR0A = 0;
    OCR0B = 0;
    OCR1A = 0;
    OCR1B = 0;

    // Compare outputs stay disconnected until pwm_write gets a nonzero value.
    TCCR0A = TIMER0_WGM_A;
    TCCR0B = TIMER0_CS;
    TCCR1A = TIMER1_WGM_A;
    TCCR1B = TIMER1_WGM_B | TIMER1_CS;
}

void motor_set(uint8_t axis, int16_t duty)
{
    if (duty > MOTOR_DUTY_MAX) duty = MOTOR_DUTY_MAX;
    if (duty < -MOTOR_DUTY_MAX) duty = -MOTOR_DUTY_MAX;

    PwmChannel a = axis == 0 ? PWM_0A : PWM_0B;
    PwmChannel b = axis == 0 ? PWM_1A : PWM_1B;

    // Idle input first, so both inputs are never driven at the same time.
    if (duty >= 0) {
        pwm_write(b, 0);
        pwm_write(a, (uint8_t)duty);
    } else {
        pwm_write(a, 0);
        pwm_write(b, (uint8_t)-duty);
    }
}

void motor_off_all(void)
{
    pwm_write(PWM_0A, 0);
    pwm_write(PWM_0B, 0);
    pwm_write(PWM_1A, 0);
    pwm_write(PWM_1B, 0);
}
