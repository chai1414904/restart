//
// Created by cosmosmount on 2025/8/29.
//

#ifndef RM26_PID_HPP
#define RM26_PID_HPP

#include "math.hpp"
#include <cstdint>

/**
 * @brief PID模式
 */
enum PidModeType
{
    PID_POSITION = 0X01, // 位置式PID
    PID_DELTA = 0X02, // 增量式PID
};


/**
 * @brief PID类
 */
class PID
{
    public:
    uint8_t mode;

    float kp;//比例
    float ki;//积分
    float kd;//微分

    float ref;
    float fdb;
    float err[3]{};//关键的记忆数组。err[0]是这次的误差，err[1]是上一次的，err[2]是上上次的。存起来是为了算D（变化率）

    float pResult;
    float iResult;
    float dResult;
    float result;

    float maxOut;//总输出的限制（防止打满）
    float maxIOut;//积分项限制

    PID(float kp, float ki, float kd, float maxOut, float maxIOut, int mode = PID_POSITION);
    void Tuning(float tuning_kp, float tuning_ki, float tuning_kd);
    void UpdateResult();
    void Clear();
};

#endif //RM26_PID_HPP