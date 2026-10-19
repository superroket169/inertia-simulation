#ifndef CONFIG_H
#define CONFIG_H

#include <avr/io.h>

// All values below are placeholders until the real rig is assembled.

#define AXIS_COUNT 2

// ---- Serial (must match FlyPT Mover serial output settings) ----

// 16 MHz / (8 * 8) = 250000 exactly with U2X, so no baud error.
// TODO: confirm Mover accepts 250000; 115200 has ~2% error on 16 MHz.
#define SERIAL_BAUD 250000UL

// Mover actuator bit depth: 16 bit -> 65535, 10 bit -> 1023.
#define TARGET_RAW_MAX 65535UL

// No valid frame for this long -> stop following new targets.
#define SERIAL_TIMEOUT_MS 100

// ---- Control loop ----

#define CONTROL_HZ 1000

// TODO: tune on the real rig. Units: PWM duty per encoder count.
#define PID_KP 1.0f
#define PID_KI 0.0f
#define PID_KD 0.0f
#define PID_I_LIMIT 1000.0f

// ---- Mechanics ----

// Counts per motor revolution after decoding (x2: both edges of channel A).
// TODO: real encoder CPR.
#define ENCODER_COUNTS_PER_REV (600L * 2)

// Motor revolutions per output revolution (gearbox * belt).
// TODO: real gearbox and belt pulley ratio.
#define GEAR_RATIO 10L

// Travel from center to each end, in output degrees.
// TODO: from the mechanical team's measurements.
#define AXIS_HALF_RANGE_DEG 15L

#define AXIS_HALF_TRAVEL_COUNTS \
    (ENCODER_COUNTS_PER_REV * GEAR_RATIO * AXIS_HALF_RANGE_DEG / 360L)

// Position beyond the travel range by this much -> fault.
#define OVERTRAVEL_FAULT_COUNTS (AXIS_HALF_TRAVEL_COUNTS / 10)

// |target - position| above this -> fault (stall, broken belt, wrong sign).
#define FOLLOW_ERROR_FAULT_COUNTS (AXIS_HALF_TRAVEL_COUNTS / 2)

// ---- Motor output ----

// Timer1 fast PWM with ICR1 as TOP: 16 MHz / (TOP + 1) = 20 kHz (inaudible).
#define PWM_TOP 799

// Cap duty while prototyping.
#define MOTOR_DUTY_MAX (PWM_TOP * 6 / 10)

// ---- Pins (Arduino Nano, ATmega328P) ----
//
//  D0/D1   UART (USB serial)
//  D2      encoder 0 channel A (INT0, fixed)
//  D3      encoder 1 channel A (INT1, fixed)
//  D4      encoder 0 channel B
//  D7      encoder 1 channel B
//  D9      motor 0 PWM (OC1A, fixed)
//  D10     motor 1 PWM (OC1B, fixed)
//  D5/D6   motor 0 direction
//  D11/D12 motor 1 direction
//  D8      HOA "auto" input, active low
//  A0/A1   home switches axis 0/1, active low
//
// Encoder channel B pins must stay on PORTD (read together with PIND).

#define ENC0_A_BIT PD2
#define ENC1_A_BIT PD3
#define ENC0_B_BIT PD4
#define ENC1_B_BIT PD7

#define HOA_AUTO_PIN  PINB
#define HOA_AUTO_PORT PORTB
#define HOA_AUTO_BIT  PB0

#define HOME0_BIT PC0
#define HOME1_BIT PC1

#define MOTOR0_DIR1_BIT PD5
#define MOTOR0_DIR2_BIT PD6
#define MOTOR1_DIR1_BIT PB3
#define MOTOR1_DIR2_BIT PB4

#endif
