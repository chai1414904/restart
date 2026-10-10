#ifndef LED_HPP
#define LED_HPP

/**
 * @brief C板板载三色LED的简易封装
 * @note 只负责“点亮/熄灭/反转”的三个动作
 *      闪烁节奏状态编码这类逻辑放在使用他的线程中
 *      因为那属于“怎样用灯”，而不属于怎样点灯“怎样用灯”
 */
namespace LED
{
/**
 * @enum Color
 * @brief C板上的三个颜色
 */
enum Color
{
    RED = 0, ///< PH12
    GREEN = 1, ///< PH11
    BLUE = 2, ///< PH10
};
/**
 * @brief 初始化：把三颗灯熄灭，给出确定的起始状态
 */
void Init(void);

/**
 * @brief 点亮指定颜色的灯
 */
void On(Color color);


/**
 * @brief 熄灭指定颜色的灯
 */
void Off(Color color);


/**
 * @brief 反转指定颜色的灯
 */
void Toggle(Color color);
}
#endif // LED_HPP

//还有头文件保护宏防止重复定义
//文档注释（Doxygen 风格）@brief 是简介，@note 是备注
//命名空间与枚举类型（对外公开的数据类型）
//enum Color：定义了一个名为 Color 的枚举类型，包含 RED、GREEN、BLUE
//注意：外部使用时，必须写 LED::RED 或 LED::On(LED::RED)
//函数声明（暴露给外界的接口）