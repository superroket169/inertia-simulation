#ifndef INPUT_H
#define INPUT_H

#include "raylib.h"
#include "linux_ff.h"
#include <iostream>
#include <vector>
#include "../calibration/calibration.h"

namespace HardwareInput
{
    enum Action
    {
        HandBrake,
        Pause,
        GearAuto,
        GearUp,
        GearDown
    };

    class InputPack
    {
    public:
        float wheelSteer = 0.0f;
        float brakePedal = 0.0f;
        float gasPedal = 0.0f;

        float getWheelSteer() { return wheelSteer; }
        float getBrakePedal() { return brakePedal; }
        float getGasPedal()   { return gasPedal; }

        void setInput(float wheelSteer, float brakePedal, float GasPedal)
        {
            this->wheelSteer = wheelSteer;
            this->brakePedal = brakePedal;
            this->gasPedal = GasPedal;
        }
    };

    class InputManager
    {
    private:

        clb::InputAxis  calibrateX;
        clb::InputAxis  calibrateY;
        clb::InputAxis  calibrateZ;

        InputPack       rawInput;
        InputPack       calibratedInput;

        RaylibLinuxFF   ff; 
        bool            isFFActive = false;

        bool selfCentring = false;

        float wheelConstantForce = 0;
        float wheelConstantForceLen = 0;

        float wheelVibrationForce = 0;
        float wheelVibrationFreq = 0;

        float wheelLeftLimit = -1;
        float wheelRightLimit = 1;

        bool isSettedAngle = 0;
        float settedAngle = 0;

        void calibrateRawValue();

    public:
        InputManager() = default;
        InputManager(clb::InputAxis x, clb::InputAxis y, clb::InputAxis z)
        : calibrateX(x), calibrateY(y), calibrateZ(z) 
        {
            isFFActive = ff.Init("/dev/input/by-id/usb-046d_Driving_Force_GT-event-joystick"); // ff activation
        }

        void updateInputs();
        void updateFeedBacks();

        float getWheelSteer() { return calibratedInput.getWheelSteer(); };                  // [-1, 0]
        float getBrakePedal() { return 1 - (calibratedInput.getBrakePedal() + 1.0f) / 2.0f; };  // [0, 1]
        float getGasPedal()   { return 1 - (calibratedInput.getGasPedal() + 1.0f) / 2.0f; };    // [0, 1]

        void setCalibrationValueX(clb::InputAxis calibrateValue) { this->calibrateX = calibrateValue; }
        void setCalibrationValueY(clb::InputAxis calibrateValue) { this->calibrateY = calibrateValue; }
        void setCalibrationValueZ(clb::InputAxis calibrateValue) { this->calibrateZ = calibrateValue; }

        void setWheelAngle(float angle) { this->settedAngle = angle; this->isSettedAngle = true; }
        void setWheelLimits(float left, float right) { if(left < -1 || right > 1) return; wheelLeftLimit = left; wheelRightLimit = right; }
        void setWheelForce(float force, float len) { wheelConstantForce = force; wheelConstantForceLen = len; }
        void setWheelVibration(float force, float len) { wheelVibrationForce = force; wheelVibrationFreq = len; }

        void activateSelfCenter() { selfCentring = true; };
        void deactivateSelfCenter() { selfCentring = false; };
    };
}

#endif