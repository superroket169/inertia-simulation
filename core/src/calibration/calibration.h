#ifndef CALIBRATION_H
#define CALIBRATION_H
/**
 * plan : 
 * ilk önce dummy değerlerimiz olacak : calibrationValue dakiler
 * eğer kalibrasyon yapılmazsa bu değerler kabul edilecek
 * kalibrasyon nasıl yapılacak : 
 * 
 * - class calibratedValue 
 * diye birşeyimiz olabilir
 * bu init yaptıktan sonra .calibrateTop..Bottom..Mid(float second);
 * yaptığımızda second saniye kadar değerlere göre dummy değerleri değiştirecek& kalibre edecek
 * 
 * bundan sonra input classında :
 * class InputChannel 
 * gibi birşey olacak. bu sayede bu class ile 
 * InputChannel ch; ch.setCallib(class callibratedValue); 
 * şeklinde initilation yapabiliriz
 * bundan sonra bu channel dan şu şekilde değer alacağız : 
 * ch.getWheelValue();
 * ch.getGasPedalValue();
 * cg.getBrakePedalValue();
 * şeklinde.
 * bunlar valibration değerine uygun bir şekilde
 * [-1, 1] aralığında bir değer gönderecek. 
 */

/**
 * 
 */

#include<algorithm>

namespace clb
{
    // her axis için Input class i
    class InputAxis
    {
    private:
        float rawMin;
        float rawMax; 
        float center;
        bool isCalibrated;

    public:

        InputAxis() : rawMax(-1), rawMin(1), center(0), isCalibrated(false) {}
        InputAxis(float rawMax, float rawMin)
        {
            this->rawMax = rawMax;
            this->rawMin = rawMin;
            this->center = (rawMin - rawMax) / 2;
        }

        // basit calibrate fonksiyonu
        float calibrateValues(float currentRawValue)
        {
            float debugValue = 0.f;
            if (currentRawValue < rawMin)
            {
                debugValue = rawMin - currentRawValue;
                rawMin = currentRawValue;
            }

            if (currentRawValue > rawMax)
            {
                debugValue = rawMax - currentRawValue;
                rawMax = currentRawValue;
            }

            return debugValue;
        }

        float getNormalizedValue(float currentRawValue)
        {
            if (currentRawValue < rawMin) currentRawValue = rawMin;
            if (currentRawValue > rawMax) currentRawValue = rawMax;

            float temp = (float)(currentRawValue - rawMin) / (rawMax - rawMin);
            
            return (temp * 2.0f) - 1.0f;
        }

    };
}

#endif