# Pinout (Arduino Nano, ATmega328P)

Axis 1 = left, axis 2 = right.

## Panel (HOA selector, per axis)

| Pin | Name  | Dir | Notes |
|-----|-------|-----|-------|
| D4  | auto1 | in, pull-up | Left axis in Auto |
| D7  | hand1 | in, pull-up | Left axis in Hand |
| D12 | auto2 | in, pull-up | Right axis in Auto |
| D8  | hand2 | in, pull-up | Right axis in Hand |
| D11 | enable | out | Power enable for both motor drivers; the e-stop also cuts it |

## Motor drivers (BTS7960)

| Pin | Name | Dir | Notes |
|-----|------|-----|-------|
| D6  | ma1  | out | Motor A, PWM (Timer0, OC0A) |
| D5  | ma2  | out | Motor A, PWM (Timer0, OC0B) |
| D9  | mb1  | out | Motor B, PWM (Timer1, OC1A) |
| D10 | mb2  | out | Motor B, PWM (Timer1, OC1B) |
| A2  | maı1 | in, analog | Motor A current sense (IS) |
| A3  | maı2 | in, analog | Motor A current sense (IS) |
| A6  | mbı1 | in, analog only | Motor B current sense (IS) |
| A7  | mbı2 | in, analog only | Motor B current sense (IS) |

## Encoders (not connected yet)

| Pin | Notes |
|-----|-------|
| D2  | INT0 |
| D3  | INT1 |
| A0  | |
| A1  | |
| A4  | I2C SDA |
| A5  | I2C SCL |

## Other

| Pin | Notes |
|-----|-------|
| D0/D1 | USB serial (FlyPT Mover) |
| D13 | Onboard LED |

All 22 pins are assigned.
