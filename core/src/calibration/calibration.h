#ifndef CALIBRATION_H
#define CALIBRATION_H

#include <algorithm>
#include <cmath>

namespace clb
{
    class InputAxis
    {
    private:
        float rawMin;
        float rawMax; 
        float center;
        bool isCalibrated;

    public:
        InputAxis() : rawMax(1.0f), rawMin(-1.0f), center(0.0f), isCalibrated(false) {}

        InputAxis(float rawMax, float rawMin)
        {
            this->rawMax = rawMax;
            this->rawMin = rawMin;
            this->center = (rawMin + rawMax) / 2.0f;
        }
        
        void resetForCalibration()
        {
            rawMin = 10000.0f;  // tam ters şekilde reset için
            rawMax = -10000.0f; // tam ters şekilde reset için
            isCalibrated = false;
        }

        float calibrateValues(float currentRawValue)
        {
            if (currentRawValue < rawMin) rawMin = currentRawValue;
            if (currentRawValue > rawMax) rawMax = currentRawValue;
            
            center = (rawMin + rawMax) / 2.0f;
            isCalibrated = true;
            
            return currentRawValue;
        }

        float getNormalizedValue(float currentRawValue)
        {
            // max ve min eşitse basit koruma 
            if (std::abs(rawMax - rawMin) < 0.0001f) return 0.0f;

            // değerleri sınırlar içerisinde tutmak
            if (currentRawValue < rawMin) currentRawValue = rawMin;
            if (currentRawValue > rawMax) currentRawValue = rawMax;

            // normalizasyon
            float t = (currentRawValue - rawMin) / (rawMax - rawMin);
            return (t * 2.0f) - 1.0f;
        }
    };
}

#endif