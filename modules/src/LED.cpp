#include "LED.hpp"
#include "main.h"

namespace LED
{
namespace
{
struct Pin
{
    GPIO_TypeDef *port;//在 STM32 HAL 库中
    //GPIO_TypeDef 是一个结构体类型，代表一个 GPIO 端口
    //使用指针 *port 是因为在 STM32 中，硬件寄存器是映射到内存地址的
    //我们不需要拷贝整个端口的数据，只需要保存这个端口寄存器的内存地址即可
    //（例如保存 GPIOH 的地址）
    uint16_t pin;//16位无符号整数
};

constexpr Pin kPins[3] = {
    {LED_R_GPIO_Port, LED_R_Pin},
    {LED_G_GPIO_Port, LED_G_Pin},
    {LED_B_GPIO_Port, LED_B_Pin},
};
constexpr GPIO_PinState kLevelOn = GPIO_PIN_SET;
constexpr GPIO_PinState kLevelOff = GPIO_PIN_RESET;
}
//对namespace的理解为声明，定义了一个命名空间实体（Namespace Entity），并引入了一个作用域
//而里面的内容是具体的实体,ex.
//类型实体（Type Entity）LED::Pin（struct Pin { ... };）
//变量定义（Variable Definition）（constexpr Pin kPins[3] = {...};）
//函数定义（Function Definition)LED::Init（void Init(void) { ... }）
void Init(void)
{
    for (const Pin &p :kPins)
    {
        HAL_GPIO_WritePin(p.port, p.pin, kLevelOff);
        //对所有灯初始化，并熄灭
    }
}

void On(Color color)
{
    HAL_GPIO_WritePin(kPins[color].port, kPins[color].pin, kLevelOn);
}
void Off(Color color)
{
    HAL_GPIO_WritePin(kPins[color].port, kPins[color].pin, kLevelOff);
}
//差点被kLevelOn kLevelOff中的LO阴了
void Toggle(Color color)
{
    HAL_GPIO_TogglePin(kPins[color].port, kPins[color].pin);
}
}