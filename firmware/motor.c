#include "motor.h"
#include "config.h"

void motor_init(void)
{
    // TODO: D5, D6, D9, D10 as outputs, compare registers 0.
    // TODO: Timer0 and Timer1 in 8-bit mode per PWM_FREQ (fast PWM /8, or
    //       phase correct /1 or /8), non-inverting on OC0A/OC0B/OC1A/OC1B.
}

void motor_set(uint8_t axis, int16_t duty)
{
    (void)axis;
    (void)duty;
    // TODO: clamp to +-MOTOR_DUTY_MAX. Write |duty| to one PWM input of the
    //       axis by sign and 0 to the other:
    //         axis 0: OCR0A (D6) / OCR1A (D9)
    //         axis 1: OCR0B (D5) / OCR1B (D10)
}

void motor_off_all(void)
{
    // TODO: OCR0A = OCR0B = OCR1A = OCR1B = 0.
}
