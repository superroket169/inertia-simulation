#ifndef INPUT_H
#define INPUT_H

#include <unordered_map>
#include <vector>
#include "../calibration/calibration.h"
/**
 * evet planda bir değişiklik var
 * InputManager diye bir class lazım. bu class da olması gereken fonksiyonlar : 
 * update();
 * getWheerSteer(); [-1, 1]
 * get...() [-1, 1]
 * 
 * 
 */

namespace HardwareInput
{
    /**
     * butonlardan gelen veri için
     */
    enum Action
    {
        HandBrake, // elfreni buton mu pedal mı? idk
        Pause,
        GearAuto,
        GearUp,
        GearDown
    };

    /**
     * yine butonlardan gelen veri için map
     * şimdilik buton kullanılmayacak
     */
    // std::unordered_map<Action, std::vector</** Irrlich girdi türü */> keyMap =
    // {
    //     {Action::HandBrake, {...}},
    //     {Action::Pause,     {...}},
    //     {Action::GearAuto,  {...}},
    //     {Action::GearUp,    {...}},
    //     {Action::GearDown,  {...}}
    // };

    // std::unordered_map<Action, bool> curr;
    // std::unordered_map<Action, bool> prev;

    class Input
    {
    public:
        float wheelSteer = 0.0f;
        float brakePedal = 0.0f;
        float GasPedal = 0.0f;

        float getWheelSteer() { return wheelSteer; }
        float getBrakePedal() { return brakePedal; }
        float getGasPedal()   { return GasPedal; }

        void setInput(float wheelSteer, float brakePedal, float GasPedal)
        {
            this->wheelSteer = wheelSteer;
            this->brakePedal = brakePedal;
            this->GasPedal = GasPedal;
        }
    };

    class InputManager
    {
    private:
        clb::InputAxis calibrateValue;
    public:
        InputManager() = default;
        InputManager(clb::InputAxis ia) : calibrateValue(ia) {}

        void update();
        float getWheelSteer();
        float getBrakePedal();
        float getGasPedal();
    };
}

#endif