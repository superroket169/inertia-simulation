#include "motor.h"
#include "config.h"

void motor_init(void)
{
    // TODO: direction pins as outputs, driven low.
    // TODO: Timer1 fast PWM mode 14 (WGM13:10 = 1110), ICR1 = PWM_TOP,
    //       non-inverting on OC1A/OC1B, prescaler 1. D9/D10 as outputs.
}

void motor_set(uint8_t axis, int16_t duty)
{
    (void)axis;
    (void)duty;
    // TODO: clamp to +-MOTOR_DUTY_MAX, set DIR1/DIR2 from sign,
    //       write |duty| to OCR1A (axis 0) or OCR1B (axis 1).
}

void motor_off_all(void)
{
    // TODO: OCR1A = OCR1B = 0, direction pins low.
}
