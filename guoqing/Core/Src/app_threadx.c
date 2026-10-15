/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    app_threadx.c
  * @author  MCD Application Team
  * @brief   ThreadX applicative file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "app_threadx.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
/* threads.hpp 是 C++ 头，这里只声明那个 C 入口 */
extern UINT Threads_CreateAll(void);
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/**
  * @brief  Application ThreadX Initialization.
  * @param memory_ptr: memory pointer
  * @retval int
  */
UINT App_ThreadX_Init(VOID *memory_ptr)
{
  UINT ret = TX_SUCCESS;
  TX_BYTE_POOL *byte_pool = (TX_BYTE_POOL*)memory_ptr;
  //内核给一个内存池指针
  //把内核给的原始指针，强制转换成一个 ThreadX 专用的“字节内存池”结构体

  /* USER CODE BEGIN App_ThreadX_Init */
  (void)byte_pool;

  /* 三个线程的栈是静态分配的，所以这里不用从 byte_pool 里申请内存。
   * 失败时返回错误码，由 tx_application_define 里的分支处理。 */
  ret = Threads_CreateAll();
  /* USER CODE END App_ThreadX_Init */

  return ret;
}

/**
  * @brief  MX_ThreadX_Init
  * @param  None
  * @retval None
  */
void MX_ThreadX_Init(void)//启动操作系统内核
{
  /* USER CODE BEGIN  Before_Kernel_Start */

  /* USER CODE END  Before_Kernel_Start */

  tx_kernel_enter();
  //一旦执行，永不返回。控制权交给了ThreadX内核，内核会去调度多线程任务

  /* USER CODE BEGIN  Kernel_Start_Error */

  /* USER CODE END  Kernel_Start_Error */
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
