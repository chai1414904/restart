#include "pid.hpp"

PID::PID(float kp, float ki, float kd, float maxOut, float maxIOut, int mode)
    : mode(mode), kp(kp), ki(ki), kd(kd), maxOut(maxOut), maxIOut(maxIOut)
{
    Clear();
}

void PID::Tuning(float tuning_kp, float tuning_ki, float tuning_kd)
{
    kp = tuning_kp;
    ki = tuning_ki;
    kd = tuning_kd;
}

void PID::UpdateResult()
{
    err[2] = err[1];
    err[1] = err[0];
    err[0] = ref - fdb;//当前误差
    //记录历史误差

    if ( mode == PID_POSITION)
    {
        //位置式： 直接算出本次应有的输出值
        pResult = kp * err[0];
        iResult += ki * err[0];
        dResult = kd * (err[0] - err[1]);

        iResult = Numeric::LimitABS(iResult, maxIOut);                  // 积分限幅，防止积分饱和
        result = Numeric::LimitABS(pResult + iResult + dResult, maxOut); // 输出限幅，保护执行器
        // pResult	对当前误差的响应 ✓
        // iResult	对历史误差累积的补偿 ✓
        // dResult	对误差变化趋势的阻尼 ✓
    }
    else
    {
        pResult = kp * (err[0] - err[1]);
        iResult = ki * err[0];
        dResult = kd *(err[0] - 2.0f * err[1] + err[2]);

        result = Numeric::LimitABS(result + pResult + iResult + dResult, maxOut);
    }
}

void PID::Clear()
{
    ref = fdb = 0.0f;
    err[0] = err[1] = err[2] = 0.0f;
    pResult = iResult = dResult = result = 0.0f;
}