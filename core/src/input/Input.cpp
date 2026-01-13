#include "Input.h"

void HardwareInput::InputManager::calibrateRawValue()
{
    calibratedInput.wheelSteer = calibrateX.getNormalizedValue(rawInput.wheelSteer);
    calibratedInput.gasPedal   = calibrateY.getNormalizedValue(rawInput.gasPedal);
    calibratedInput.brakePedal = calibrateZ.getNormalizedValue(rawInput.brakePedal);
}

void HardwareInput::InputManager::updateInputs()
{
    rawInput.wheelSteer = GetGamepadAxisMovement(0, 0);
    rawInput.gasPedal = GetGamepadAxisMovement(0, 4);
    rawInput.brakePedal = GetGamepadAxisMovement(0, 5);

    calibrateRawValue();
}

void HardwareInput::InputManager::updateFeedBacks()
{
    if (!isFFActive) return;

    float currentSteer = getWheelSteer();
    float finalForce = 0.0f;
    float wallStiffness = 9.0f; // sınır kuvveti

    // sınır algılaması / kontrolü
    if (currentSteer > wheelRightLimit) // sağ
    {
        float overlap = currentSteer - wheelRightLimit;
        finalForce = overlap * wallStiffness; 
    }
    else if (currentSteer < wheelLeftLimit) // sol
    {
        float overlap = wheelLeftLimit - currentSteer;
        finalForce = -overlap * wallStiffness;
    }

    // manual kuvvet (force feedback!!)
    else
    {
        if (isSettedAngle) { finalForce = (settedAngle - currentSteer) * 2.0f; } // direk angle setting
        else // asıl feedback kısmı
        {
            finalForce = wheelConstantForce;
            
            // self centring
            if (selfCentring && wheelConstantForce == 0)
            {
                finalForce = currentSteer * 0.6f; 
            }
        }

        // dahiyane fikir : sinus dalgalı titreşim!
        if (wheelVibrationForce > 0)
        {
            float time = (float)GetTime(); 
            float vibration = sin(time * wheelVibrationFreq * 6.28318f) * wheelVibrationForce;
            finalForce += vibration;
        }
    }
    
    // güvenlik
    if (finalForce > 1.0f) finalForce = 1.0f;
    if (finalForce < -1.0f) finalForce = -1.0f;

    // Son hesaplanan gücü Linux çekirdeğine gönder
    ff.SetForce(finalForce, 30); // not burada ne kadar süreyle olacağı da var ama şimdilik sabit bir değer
}