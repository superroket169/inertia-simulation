#ifndef MOTOR_H
#define MOTOR_H

#include <stdint.h>

// BTS7960 per motor: RPWM and LPWM both on PWM pins (Timer0 + Timer1, see
// config.h). R_EN/L_EN of both drivers share the enable line driven by main.c.
// Positive duty drives one PWM input, negative the other; the idle one stays 0.

void motor_init(void);

// duty in [-MOTOR_DUTY_MAX, MOTOR_DUTY_MAX], sign selects direction.
void motor_set(uint8_t axis, int16_t duty);

// Both PWM inputs 0 on all axes. The BTS7960 brakes while the enable line
// is high and lets the motor coast while it is low.
void motor_off_all(void);

#endif
