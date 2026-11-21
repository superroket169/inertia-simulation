# inertia-simulation

2DOF motion simulator rig driven by DC motors.

- **PC side:** [FlyPT Mover](https://www.flyptmover.com) reads game telemetry and sends SMC3 frames (`[A hi lo][B hi lo]`) over serial.
- **`firmware/`:** Arduino Nano (ATmega328P) controller in plain C: SMC3 parser, PID position loop, HOA/fault handling. Build with `make`, flash with `make flash`, run parser tests with `make test`. Rig-specific values are in `firmware/config.h`.
- **`hardware/`, `mafsal/`, `Bağlantı_Ekipmanları/`:** CAD and electronics files.
- **`core/` (legacy):** C++/raylib wheel input and Linux force-feedback experiments. Not part of the motion rig; kept for reference.

Open work is tracked in GitHub issues.
