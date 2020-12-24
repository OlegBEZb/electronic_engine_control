#include "control.h"
#include "motor_speed.h"

int16_t control_run( int16_t desiredSpeed ) {
    const int16_t proportionalCoefficent = 10;
    const int16_t integralCoefficent     = 0;
    const int16_t differenceCoefficent   = 1;
    
    static int16_t previousError = 0;
    static int16_t integral      = 0;
    
    int16_t controlSignal = 0;
    int16_t difference    = 0;
    int16_t currentError  = 0;
    
    currentError  = desiredSpeed - motor_speed_getSpeed();
    integral     += currentError;
    difference    = currentError - previousError;
    
    controlSignal  = difference * differenceCoefficent;
    controlSignal += integral * integralCoefficent;
    controlSignal += currentError;
    controlSignal *= proportionalCoefficent;
    
    previousError = currentError;
    
    return controlSignal;
}