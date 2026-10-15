#include "threads.hpp"

#include "LED.hpp"
#include "DJIMotorHandler.hpp"  

namespace Threads
{
    /*线程对象与栈都放静态储存，不占用ThreadX的字节内存池*/
static TX_THREAD s_alive_thread;
static UCHAR s_alive_stack[kAliveStackSize];//内部链接

TX_SEMAPHORE sem_init_done;
TX_SEMAPHORE sem_imu_beat;
TX_SEMAPHORE sem_control_beat;
volatile bool g_imu_init_ok = false;


/**
 * @brief 把信号量的计数全部取走，返回一共取到几次
 * @note  信号量是计数语义：被监控者每 put 一次计数加一，且 put 是累加的、不会丢。
 *        每次监控先排空，就得到"上次检查到现在发生过几次"，
 *        于是"这期间有没有发生"和"总共发生过几次"就被区分开了。
 *        这正是单靠一个计数器做不到、必须靠信号量计数来做的事。
 */
static UINT Drain(TX_SEMAPHORE *sem)
{
    UINT beats = 0U;
    while (tx_semaphore_get(sem, TX_NO_WAIT) == TX_SUCCESS)
    {
        ++beats;
    }
    return beats;
}
//“排空”：疯狂往外掏信号量，掏空为止
//ex.Drain 把计数取走后，下次再取，如果返回 0，说明这 10 毫秒内 IMU 线程卡死了
//如果返回 10，说明跑了 10 次。这就是用信号量做心跳监控的终极奥义

void CreateAliveThread(void)
{
    (void)tx_thread_create(&s_alive_thread, (CHAR *)"alive", AliveThreadEntry, 0,
                           s_alive_stack, sizeof(s_alive_stack), 
                           kAlivePriority, kAlivePriority, TX_NO_TIME_SLICE, TX_AUTO_START);
}
// tx_thread_create(&s_alive_thread,        // 1  线程控制块
//                  (CHAR *)"alive",         // 2  名字（调试器里能看到 ✓）
//                  AliveThreadEntry,        // 3  入口函数
//                  0,                       // 4  传给入口的参数
//                  s_alive_stack,           // 5  栈起始地址
//                  sizeof(s_alive_stack),   // 6  栈字节数
//                  kAlivePriority,          // 7  优先级
//                  kAlivePriority,          // 8  抢占阈值（= 优先级 ⇒ 同级不抢占）
//                  TX_NO_TIME_SLICE,        // 9  关闭时间片轮转
//                  TX_AUTO_START);          // 10 创建后立即启动

void AliveThreadEntry(ULONG input)
{
    (void)input;

    LED::Init();

    bool init_reported = false; // 是否已经拿到初始化结果（或已判超时）
    bool init_ok = false;
    ULONG waited_ticks = 0U;

    while (true)
    {
        // 1. 排空信号量，获取这 100ms 周期内发生了几次事件
        const UINT init_beats = Drain(&sem_init_done);
        const UINT imu_beats = Drain(&sem_imu_beat);
        const UINT control_beats = Drain(&sem_control_beat);
        

        if (!init_reported)
        {
            if (init_beats > 0U)
            {
                // 拿到初始化完成信号了！
                init_reported = true;   // 标记已上报
                init_ok = g_imu_init_ok;// 去读全局变量看看IMU成功没
            }
            else
            {
                waited_ticks += kAlivePeriodTicks;
                if (waited_ticks >= kInitWaitTicks)// kInitWaitTicks = 20秒
                {
                    // 等到上限还没等到，按初始化失败处理，之后进入常规监控
                    init_reported = true;
                    init_ok = false;
                }
            }
        }

        if (!init_reported)
        {
            // 初始化进行中（BMI088 标定本身要几秒）：红灯闪，另外两盏灭。
            // 这段时间不能因为收不到 imu 心跳就判它掉线——它正在长时间初始化。
            LED::Toggle(LED::RED);
            LED::Off(LED::GREEN);
            LED::Off(LED::BLUE);
        }
        else if (!init_ok)
        {
            // 初始化失败：红灯常亮，另外两盏灭
            LED::On(LED::RED);
            LED::Off(LED::GREEN);
            LED::Off(LED::BLUE);
        }
        else
        {
            // 初始化成功：红灯灭。
            // 绿灯跟 imu 心跳、蓝灯跟 control 心跳：有心跳就翻转，没心跳就熄灭。
            // 于是"哪一盏不闪/灭着"直接指出是谁掉了。
            LED::Off(LED::RED);

            if (imu_beats > 0U)
            {
                LED::Toggle(LED::GREEN);
            }
            else
            {
                LED::Off(LED::GREEN);
            }

            if (control_beats > 0U)
            {
                LED::Toggle(LED::BLUE);
            }
            else
            {
                LED::Off(LED::BLUE);
            }
        }

        tx_thread_sleep(kAlivePeriodTicks);
    }

} 
}// namespace Threads



UINT Threads_CreateAll(void)
{
    UINT status;

    //initial_count 取0：事件由put产生， 不用预先置数
    status = tx_semaphore_create(&Threads::sem_init_done, (CHAR *)"init done", 0U);
    if(status != TX_SUCCESS)
    {
        return status;//失败则返回错误码
    }

    status = tx_semaphore_create(&Threads::sem_imu_beat, (CHAR *)"imu beat", 0U);
    if (status != TX_SUCCESS)
    {
        return status;
    }

    status = tx_semaphore_create(&Threads::sem_control_beat, (CHAR *)"ctl beat", 0U);
    if (status != TX_SUCCESS)
    {
        return status;
    }

    DJIMotorHandler_Init();  
    Threads::CreateAliveThread();//先建好信号量
    Threads::CreateImuThread();//再开启线程
    // Threads::CreateControlThread();
        /* TODO(D 系列): modules 做完后补上——
       Threads::CreateControlThread();   需要 M2006 */

    return TX_SUCCESS;

}