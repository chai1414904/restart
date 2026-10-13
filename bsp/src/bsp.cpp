#include "bsp.hpp"

#include "bsp_pwm.hpp"
#include "bsp_usart.hpp"

void bsp_Init(){
    PWM_Init();
    USART_Init();
}