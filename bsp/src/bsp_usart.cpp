#include "bsp_usart.hpp"

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart6;
//extern了什么
//可以用#include "usart.h" 取代

/// 阻塞发送的超时时间：留出余量即可，不能用 HAL_MAX_DELAY，否则串口异常时会永久卡死
#define USART_BLOCK_TIMEOUT_MS 100U
//u是什么单位:无符号整数100,在底层编程里，习惯加个U防止类型隐式转换出bug

void USART_Init(void)
{
    USART1_Init();
    USART6_Init();
    
}
//为什么有的需要加void 有的不需要:加void,这个函数明确不接受任何参数

void USART6_Init()
{
    // 清掉上电过程中可能残留的空闲标志与溢出标志，避免刚使能中断就误触发一次
    __HAL_UART_CLEAR_IDLEFLAG(&huart6);
    __HAL_UART_CLEAR_OREFLAG(&huart6);
    //DMA 通道与引脚复用由 CubeMX 生成的 HAL_UART_MspInit 配好；
    // 空闲中断无需在此手动打开：HAL_UARTEx_ReceiveToIdle_DMA 会自行使能 IDLEIE。
    // 接收由 USART_Receive() 启动，缓冲区由调用方提供。
    //空闲标志位（IDLE）;溢出错误标志位（ORE)
}

void USART1_Init()
{
    __HAL_UART_CLEAR_IDLEFLAG(&huart1);
    __HAL_UART_CLEAR_OREFLAG(&huart1);
}

void USART_Transmit(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size, enum USART_Mode mode)
{
    //分别为串口句柄，数据指针，数据长度，发送模式
    if (huart == nullptr || pData == nullptr || Size == 0U)
    {
        return;
    }
     switch (mode)
     {
    case USART_MODE_BLOCK:
        (void)HAL_UART_Transmit(huart, pData, Size, USART_BLOCK_TIMEOUT_MS);
        break;//阻塞发射，调用HAL_UART_Transmit超过100ms
    case USART_MODE_DMA:
        (void)HAL_UART_Transmit_DMA(huart, pData, Size);
        break;//DMA发射
    case USART_MODE_IT:
        (void)HAL_UART_Transmit_IT(huart, pData, Size);
        break;//中断发射
    default:
        break;
    }
    //
    //USART与UART的区别在哪里：
    //-->UART是“异步”收发器（没有时钟线，全靠波特率约定）
    //-->USART是“同步/异步”收发器（多了一根时钟线，可以接同步设备）
    //USART与UART的函数有需要那些变量
    //-->串口句柄，数据指针，数据长度，发送模式
    //这几个模块是干什么的？
    //HUART又是什么，这几个函数前面为什么要加VOID
    //-->handle UART(一个存有这个串口所有信息的struct)
}
void USART_Receive(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size)
{
    if (huart == nullptr || pData == nullptr || Size == 0U)
    {
        return;
    }
    (void)HAL_UARTEx_ReceiveToIdle_DMA(huart, pData, Size);
    // 收到 IDLE（一帧结束）或缓冲区填满时，HAL 调用 HAL_UARTEx_RxEventCallback(huart, Size)，
    // 长度由 HAL 按 DMA 剩余计数算出；本次接收到此结束，调用方处理完需再次调用本函数重新武装。
    //用于接收不定长数据，，其中空闲是指一次发送接收彻底结束
    //[帧头 0xAA] [数据长度 0x05] [数据1] [数据2] [数据3] [校验和] [帧尾 0xBB]
    //那么上述演示中数据1与数据2的中间只是一bit(高低电平转换的时间)
    //而完整帧的核验往往超过10个bit
    //那么如果遇到连续发送数据就需要”手动“断帧
    //ex.固定长度或者环形缓冲区（Ring Buffer）+ 半满/全满中断
    //靠协议断帧：固定每个包的长度（比如每次固定收20个字节），收到20个就处理一次，然后重新挂载DMA接收。或者靠帧头帧尾（比如检测到 0xAA 开头，0xBB 结尾）。
    //靠“缓冲区满”断帧：就是利用 HAL_UARTEx_ReceiveToIdle_DMA 里的半满中断（HT）和全满中断（TC）。
}
