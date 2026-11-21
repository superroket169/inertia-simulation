#ifndef MOTOR_H
#define MOTOR_H

#include <stdint.h>

// BTS7960 per motor: RPWM and LPWM both on PWM pins (Timer0 + Timer1, see
// docs/PINOUT.md). Positive duty drives one PWM input, negative the other;
// the idle one stays 0.

void motor_init(void);

// duty in [-MOTOR_DUTY_MAX, MOTOR_DUTY_MAX], sign selects direction.
void motor_set(uint8_t axis, int16_t duty);

// Both PWM inputs 0 on all axes.
void motor_off_all(void);

#endif
