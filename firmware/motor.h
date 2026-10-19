#ifndef MOTOR_H
#define MOTOR_H

#include <stdint.h>

// One PWM + two direction pins per motor. Fits both:
//   L298N:   EN = PWM, IN1/IN2 = direction
//   BTS7960: R_EN + L_EN tied = PWM, RPWM/LPWM = direction

void motor_init(void);

// duty in [-MOTOR_DUTY_MAX, MOTOR_DUTY_MAX], sign selects direction.
void motor_set(uint8_t axis, int16_t duty);

// PWM off on all axes, motors coast.
void motor_off_all(void);

#endif
