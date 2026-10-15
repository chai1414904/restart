#include "math.hpp"

namespace Numeric 
{
    float LimitABS(float input, float maxvalue)
    {
        if (input > maxvalue)
            return maxvalue;
        if (input < -maxvalue)
            return -maxvalue;
        return input;
    }
}
//限幅防止极端电流值