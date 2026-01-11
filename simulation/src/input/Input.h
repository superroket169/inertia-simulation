#ifndef INPUT_H
#define INPUT_H

#include<unordered_map>
#include<vector>
/**
 * ilk olarak bizim butonlardan aldığımız değerler olacak
 * bu değerleri normal butondan alıyormuşuz gibi alacağız
 * yani event muamelesi yapacağız
 * 
 * diğer bir girdi de direksiyonun yönü pedallardan gelen % lik değerler olacak bir de
 * bu değerler'in herbiri için get...Value gibi fonksiyon yazacağız, bunlar için başka bir hash tutabilirmiyiz?
 * zaten bunlar için static bir hash tutmamıza gerek yok? çünkü fix şeyler
 * 
 * Input namespace inin ismi ne olacak? bilmiyorum... : HardwareInput
 * 
 * şimdi sırada class mı yazacaz c tarzı mı olacak?
 * tabii class yazacaz!
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
     */
    // std::unordered_map<Action, std::vector</** Irrlich girdi türü */> keyMap =
    // {
    //     {Action::HandBrake, {...}},
    //     {Action::Pause,     {...}},
    //     {Action::GearAuto,  {...}},
    //     {Action::GearUp,    {...}},
    //     {Action::GearDown,  {...}}
    // };

    std::unordered_map<Action, bool> curr;
    std::unordered_map<Action, bool> prev;

    class Input
    {
    private:
        float wheelSteer = 0.0f;
        float brakePedal = 0.0f;
        float throttlePedal = 0.0f;
    public:
        float getWheelSteer();
        float getBrakePedal();
        float getThrottlePedal();
    };
}

#endif