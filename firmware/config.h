#ifndef CONFIG_H
#define CONFIG_H

#include <avr/io.h>

// All values below are placeholders until the real rig is assembled.

// Axis 0 = left (panel "1"), axis 1 = right (panel "2").
// SMC3 'A' drives axis 0, 'B' drives axis 1.
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

#define CONTROL_HZ 1000L

// TODO: tune on the real rig. Units: PWM duty per encoder count.
#define PID_KP 1.0f
#define PID_KI 0.0f
#define PID_KD 0.0f
#define PID_I_LIMIT 1000.0f

// ---- Mechanics ----

// Counts per motor revolution after decoding.
// TODO: real encoder resolution (see issue #6).
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

// Both motor PWM pins sit on Timer0 and Timer1, both run 8 bit (TOP = 255)
// so every pin gets the same frequency.
//   PWM_7812HZ   fast PWM, /8          within the BTS7960 25 kHz limit, audible
//   PWM_31372HZ  phase correct PWM, /1 inaudible, above the BTS7960 limit
//   PWM_3921HZ   phase correct PWM, /8
#define PWM_7812HZ  0
#define PWM_31372HZ 1
#define PWM_3921HZ  2

#define PWM_FREQ PWM_7812HZ

#define PWM_TOP 255

// Cap duty while prototyping.
#define MOTOR_DUTY_MAX (PWM_TOP * 6 / 10)

// ---- Pins (Arduino Nano, ATmega328P), see docs/PINOUT.md ----

// Panel HOA selector, per axis. Pull-up, active low: 0 = position selected.
#define AXIS0_AUTO_PIN  PIND
#define AXIS0_AUTO_PORT PORTD
#define AXIS0_AUTO_DDR  DDRD
#define AXIS0_AUTO_BIT  PD4 // D4

#define AXIS0_HAND_PIN  PIND
#define AXIS0_HAND_PORT PORTD
#define AXIS0_HAND_DDR  DDRD
#define AXIS0_HAND_BIT  PD7 // D7

#define AXIS1_AUTO_PIN  PINB
#define AXIS1_AUTO_PORT PORTB
#define AXIS1_AUTO_DDR  DDRB
#define AXIS1_AUTO_BIT  PB4 // D12

#define AXIS1_HAND_PIN  PINB
#define AXIS1_HAND_PORT PORTB
#define AXIS1_HAND_DDR  DDRB
#define AXIS1_HAND_BIT  PB0 // D8

// Motor driver power present; the e-stop cuts it. Pull-up input.
// TODO: confirm active low (0 = power present), as on the panel inputs.
#define DRIVER_POWER_PIN  PINB
#define DRIVER_POWER_PORT PORTB
#define DRIVER_POWER_DDR  DDRB
#define DRIVER_POWER_BIT  PB3 // D11

// BTS7960 PWM inputs. Which one is RPWM/LPWM decides the sign.
// TODO: confirm the a/b = driver pin, 1/2 = motor reading of the pinout.
//   axis 0: D6 (OC0A) + D9 (OC1A)
//   axis 1: D5 (OC0B) + D10 (OC1B)

// BTS7960 current sense (R_IS/L_IS), ADC channels.
#define AXIS0_IS_A_ADC 2 // A2
#define AXIS0_IS_B_ADC 6 // A6, analog only
#define AXIS1_IS_A_ADC 3 // A3
#define AXIS1_IS_B_ADC 7 // A7, analog only

// Encoders, not connected yet.
// TODO: assignment depends on issue #6.
//   D2 (INT0), D3 (INT1), A0, A1, plus I2C on A4 (SDA) / A5 (SCL)
#define ENC0_A_BIT PD2 // D2
#define ENC1_A_BIT PD3 // D3
#define ENC0_B_BIT PC0 // A0
#define ENC1_B_BIT PC1 // A1

#define LED_PORT PORTB
#define LED_DDR  DDRB
#define LED_BIT  PB5 // D13

#endif
