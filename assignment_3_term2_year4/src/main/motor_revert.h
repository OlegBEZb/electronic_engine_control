#include <stdint.h>

void transf_ABC_to_moving_XY( int32_t currentA, int32_t currentB, int32_t currentC, uint16_t tempPosition );
void transf_XY_to_static_ABC( int32_t curD, int32_t curQ, uint16_t tempPosition);
void motor_limit_ABC( int32_t currentA, int32_t currentB, int32_t currentC );

int32_t getCurrentFromADC(uint8_t numberOfValue);
int32_t getCurrentX(void);
int32_t getCurrentY(void);

int32_t getVoltageA(void);
int32_t getVoltageB(void);
int32_t getVoltageC(void);

int32_t motor_current_getLimitCurrentA(void);
int32_t motor_current_getLimitCurrentB(void);
int32_t motor_current_getLimitCurrentC(void);


float fastInvSqrt(float x);
float fastSqrt(float x);
