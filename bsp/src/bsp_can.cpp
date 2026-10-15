#include "bsp_can.hpp"
#include "can.h"

namespace {
    CAN_RxCallback_t s_rxCallback = nullptr;          // 文件内静态 ✓
}
/**
 * @brief 初始化CAN滤波器配置。
 * 设置CAN硬件的滤波器，用于优化接收数据的处理。
 * 更多信息，请参考原文，链接：https://blog.csdn.net/weixin_54448108/article/details/128570593
 */
void CAN_Init(void)
{
    CAN_FilterTypeDef can_filter_st;                   ///< 定义过滤器结构体:定义一个“门禁规则”的结构体
    can_filter_st.FilterActivation = ENABLE;           ///< ENABLE使能过滤器：门禁生效，开始过滤
    can_filter_st.FilterMode = CAN_FILTERMODE_IDMASK;  ///< 设置过滤器模式--标识符屏蔽位模式:过滤模式选“掩码模式”
    can_filter_st.FilterScale = CAN_FILTERSCALE_32BIT; ///< 过滤器的位宽 32 :位用32位规则来过滤
    can_filter_st.FilterIdHigh = 0x0000;               ///< ID高位：掩码全为0,所有ID的消息我都收(在调试阶段或者单节点模式下经常这么干）
    can_filter_st.FilterIdLow = 0x0000;                ///< ID低位
    can_filter_st.FilterMaskIdHigh = 0x0000;           ///< 过滤器掩码高位
    can_filter_st.FilterMaskIdLow = 0x0000;            ///< 过滤器掩码低位

    can_filter_st.FilterBank = 0;                                      ///< 过滤器组-双CAN可指定0~27：使用0号过滤器组
    can_filter_st.FilterFIFOAssignment = CAN_RX_FIFO0;                 ///< 与过滤器组管理的 FIFO：收到的消息放到“FIFO0”这个收纳盒里
    HAL_CAN_ConfigFilter(&hcan1, &can_filter_st);                      ///< HAL库配置过滤器函数：+\/  \/  \/
    HAL_CAN_Start(&hcan1);                                             ///< 使能CAN1控制器：把上面定的规矩写入硬件，然后启动CAN1
    HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING); ///< 使能CAN的各种中断：开启中断通知。意思是：“FIFO0收纳盒里一有新消息，立刻打断CPU，让CPU去处理！”

    can_filter_st.SlaveStartFilterBank = 14;                           ///< 双CAN模式下规定CAN的主从模式的过滤器分配，从过滤器为14
    can_filter_st.FilterBank = 14;                                     ///< 过滤器组-双CAN可指定0~27
    HAL_CAN_ConfigFilter(&hcan2, &can_filter_st);                      ///< HAL库配置过滤器函数
    HAL_CAN_Start(&hcan2);                                             ///< 使能CAN2控制器
    HAL_CAN_ActivateNotification(&hcan2, CAN_IT_RX_FIFO0_MSG_PENDING); ///< 使能CAN的各种中断
}

void CAN_Transmit(CAN_HandleTypeDef *hcan, uint32_t Id, uint8_t *msg, uint16_t len)
{
    if (HAL_CAN_GetTxMailboxesFreeLevel(hcan) == 0)
    {
        // 如果邮箱已满，可以选择等待或者采取其他操作
        // 例如，可以添加一个超时机制或者直接返回错误状态
        // Monitor::instance()->Log_Messages(Monitor::WARNING, (uint8_t *)"CAN Tx Mailbox is full\r\n");
        return;
    }
    // T 校验标准帧 ID 和 DLC (0~8)，构造帧头并调用 HAL_CAN_AddTxMessage。
    if( Id > 0x7FFU || len > 8U)
    {
        return;
    }
    //标准CAN帧的ID最大是 11位（也就是0x7FF，十进制的2047），数据长度（DLC）最大是 8字节。超出这个范围，说明调用这个函数的人瞎传数据，直接扔掉（return）

    CAN_TxHeaderTypeDef txHeader = {};
    txHeader.StdId = Id;//填目标ID
    txHeader.ExtId = 0;//
    txHeader.IDE = CAN_ID_STD;//声明标准帧
    txHeader.RTR = CAN_RTR_DATA;//声明发数据对象
    txHeader.DLC = len;//数据长度

    uint32_t txMailbox = 0;
    (void)HAL_CAN_AddTxMessage(hcan, &txHeader, msg, &txMailbox);//数据输入自动发出
}

void CAN_SetRxCallback(CAN_RxCallback_t cb)
{
    s_rxCallback = cb;
}

// 实现 CAN 接收回调，根据总线和标准帧 ID 分发电机反馈。
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
    CAN_RxHeaderTypeDef rxHeader;//一次发送
    uint8_t rxData[8];//数据容器

    if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &rxHeader, rxData)!= HAL_OK)
    {
        return;
    }
// 只有 DJI 电机反馈帧（0x201~0x208）交给电机管理器，其余帧本层不处理
    if(rxHeader.IDE != CAN_ID_STD || rxHeader.StdId < 0x201U || rxHeader.StdId > 0x208U)
    //过滤非标准帧，限制ID范围
    {
        return;
    }
     if (s_rxCallback != nullptr)
    {
        s_rxCallback(hcan, rxHeader.StdId, rxData);   // ★ 只转发，不解析 ✓
    }
    //BSP层只管收数据，不管数据怎么解析，解析交给专门的业务层（）
}