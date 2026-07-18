/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include "main.h"
#include "spi.h"
#include "usart.h"
#include "gpio.h"
#include "fsmc.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "lcd.h"
#include "key_led.h"
#include "norflash.h"
#include "malloc.h"
#include "usbd_core.h"
#include "usbd_desc.h"
#include "usbd_msc.h"
#include "usbd_storage.h"
#include "usbd_conf.h"

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
void SystemClock_Config(void);
uint8_t sys_stm32_clock_init(uint32_t plln, uint32_t pllm, uint32_t pllp, uint32_t pllq);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
USBD_HandleTypeDef USBD_Device;             /* USB Device处理结构体 */
extern volatile uint8_t g_usb_state_reg;    /* USB状态 */
extern volatile uint8_t g_device_state;     /* USB连接 情况 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */
  uint8_t offline_cnt = 0;
  uint8_t tct = 0;
  uint8_t usb_sta;
  uint8_t device_sta;
  uint16_t id;
  /* USER CODE END Init */

  /* Configure the system clock */
  // SystemClock_Config();
  // n = 336, m = 8, p = 2, q = 7
  sys_stm32_clock_init(336, 8, 2, 7); /* 设置时钟,168Mhz */

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_FSMC_Init();
  MX_SPI2_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  DWT_Init(); // 初始化DWT

  Key_Init();
  Led_Init();
  lcd_init();
  norflash_init();
  my_mem_init(SRAMIN);                /* 初始化内部SRAM内存池 */
  my_mem_init(SRAMCCM);               /* 初始化内部SRAMCCM内存池 */
  
  lcd_show_string(10, 50, 300, 32, 32, "FFFER_TEST", RED);
  lcd_show_string(10, 85, 450, 24, 24, "KEY TEST", BLUE);

  id = norflash_read_id();                /* 读取FLASH ID */    
  if ((id == 0) || (id == 0xFFFF))
  {
      lcd_show_string(30, 130, 200, 16, 16, "NorFlash Error!", RED);          /* 检测NorFlash错误 */
  }
  else   /* SPI FLASH 正常 */
  {
      lcd_show_string(30, 150, 200, 16, 16, "SPI FLASH Size:12MB", RED);
  }
  lcd_show_string(30, 190, 200, 16, 16, "USB Connecting...", RED);            /* 提示正在建立连接 */
  
  USBD_Init(&USBD_Device, &FS_Desc, DEVICE_FS);       /* 初始化USB */
  USBD_RegisterClass(&USBD_Device, &USBD_MSC);        /* 添加类 */
  USBD_MSC_RegisterStorage(&USBD_Device, &USBD_Storage_Interface_fops_FS);    /* 为MSC类添加回调函数 */
  USBD_Start(&USBD_Device);                           /* 开启USB */
  HAL_Delay(1800);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
    KeyPressedID key_id = KEY_None;
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    #if 0
    key_id = Key_Scan();
    if(key_id == KEY0_Pressed){
        lcd_show_string(10, 115, 200, 24, 24, "key 0 pressed.", BLUE);
        Led_Toggle(LED1);
    }
    else if(key_id == KEY1_Pressed){
        lcd_show_string(10, 115, 200, 24, 24, "key 1 pressed.", BLUE);
        Led_Toggle(LED2);
    }
    else if(key_id == KEY2_Pressed){
        lcd_show_string(10, 115, 200, 24, 24, "key 2 pressed.", BLUE);
        Led_Toggle(LED1);
        Led_Toggle(LED2);
    }  
    #endif
            HAL_Delay(1);

        if (usb_sta != g_usb_state_reg)                 /* 状态改变了 */
        {
            lcd_fill(30, 210, 240, 210 + 16, WHITE);    /* 清除显示 */

            if (g_usb_state_reg & 0x01)                 /* 正在写 */
            {
                Led_On(LED2);
                lcd_show_string(30, 210, 200, 16, 16, "USB Writing...", RED);   /* 提示USB正在写入数据 */
            }

            if (g_usb_state_reg & 0x02)                 /* 正在读 */
            {
                Led_On(LED2);
                lcd_show_string(30, 210, 200, 16, 16, "USB Reading...", RED);   /* 提示USB正在读出数据 */
            }

            if (g_usb_state_reg & 0x04)
            {
                lcd_show_string(30, 230, 200, 16, 16, "USB Write Err ", RED);   /* 提示写入错误 */
            }
            else
            {
                lcd_fill(30, 230, 240, 230 + 16, WHITE);                        /* 清除显示 */
            }
            
            if (g_usb_state_reg & 0x08)
            {
                lcd_show_string(30, 250, 200, 16, 16, "USB Read  Err ", RED);   /* 提示读出错误 */
            }
            else
            {
                lcd_fill(30, 250, 240, 250 + 16, WHITE);    /* 清除显示 */
            }
            
            usb_sta = g_usb_state_reg;  /* 记录最后的状态 */
        }

        if (device_sta != g_device_state)
        {
            if (g_device_state == 1)
            {
                lcd_show_string(30, 190, 200, 16, 16, "USB Connected    ", RED);    /* 提示USB连接已经建立 */
            }
            else
            {
                lcd_show_string(30, 190, 200, 16, 16, "USB DisConnected ", RED);    /* 提示USB被拔出了 */
            }
            
            device_sta = g_device_state;
        }

        tct++;

        if (tct == 200)
        {
            tct = 0;
            Led_Off(LED2);        /* 关闭 LED2 */
            Led_Toggle(LED1);  /* LED1 闪烁 */

            if (g_usb_state_reg & 0x10)
            {
                offline_cnt = 0;    /* USB连接了,则清除offline计数器 */
                g_device_state = 1;
            }
            else                    /* 没有得到轮询 */
            {
                offline_cnt++;

                if (offline_cnt > 10)
                {
                    g_device_state = 0;     /* 2s内没收到在线标记,代表USB被拔出了 */
                }
            }

            g_usb_state_reg = 0;
        }
  }
  /* USER CODE END 3 */
}

/**
 * @brief       时钟设置函数
 * @param       plln: PLL1倍频系数(PLL倍频), 取值范围: 64~432.
 * @param       pllm: PLL1预分频系数(进PLL之前的分频), 取值范围: 2~63.
 * @param       pllp: PLL1的p分频系数(PLL之后的分频), 分频后作为系统时钟, 取值范围: 2,4,6,8.(仅限这4个值!)
 * @param       pllq: PLL1的q分频系数(PLL之后的分频), 取值范围: 2~15.
 * @note
 *
 *              Fvco: VCO频率
 *              Fsys: 系统时钟频率, 也是PLL1的p分频输出时钟频率
 *              Fq:   PLL1的q分频输出时钟频率
 *              Fs:   PLL输入时钟频率, 可以是HSI, CSI, HSE等.
 *              Fvco = Fs * (plln / pllm);
 *              Fsys = Fvco / pllp = Fs * (plln / (pllm * pllp));
 *              Fq   = Fvco / pllq = Fs * (plln / (pllm * pllq));
 *
 *              外部晶振为8M的时候, 推荐值: plln = 336, pllm = 8, pllp = 2, pllq = 7.
 *              得到:Fvco = 8 * (336 / 8) = 336Mhz
 *                   Fsys = pll1_p_ck = 336 / 2 = 168Mhz
 *                   Fq   = pll1_q_ck = 336 / 7 = 48
 *
 * @retval      错误代码: 0, 成功; 1, 错误;
 */
uint8_t sys_stm32_clock_init(uint32_t plln, uint32_t pllm, uint32_t pllp, uint32_t pllq)
{
    HAL_StatusTypeDef ret = HAL_OK;
    RCC_ClkInitTypeDef rcc_clk_init_handle;
    RCC_OscInitTypeDef rcc_osc_init_handle;
    
    __HAL_RCC_PWR_CLK_ENABLE();                                         /* 使能PWR时钟 */
    
    /* 下面这个设置用来设置调压器输出电压级别，以便在器件以最大频率工作时使性能与功耗实现平衡 */

    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);      /* VOS = 1, Scale1, 1.2V内核电压,FLASH访问可以得到最高性能 */

    /* 使能HSE，并选择HSE作为PLL时钟源，配置PLL1，开启USB时钟 */
    rcc_osc_init_handle.OscillatorType = RCC_OSCILLATORTYPE_HSE;        /* 时钟源为HSE */
    rcc_osc_init_handle.HSEState = RCC_HSE_ON;                          /* 打开HSE */
    rcc_osc_init_handle.PLL.PLLState = RCC_PLL_ON;                      /* 打开PLL */
    rcc_osc_init_handle.PLL.PLLSource = RCC_PLLSOURCE_HSE;              /* PLL时钟源选择HSE */
    rcc_osc_init_handle.PLL.PLLN = plln;//336
    rcc_osc_init_handle.PLL.PLLM = pllm;//8
    rcc_osc_init_handle.PLL.PLLP = pllp;//2
    rcc_osc_init_handle.PLL.PLLQ = pllq;//7

    ret = HAL_RCC_OscConfig(&rcc_osc_init_handle);                      /* 初始化RCC */
    if(ret != HAL_OK)
    {
        return 1;                                                       /* 时钟初始化失败，可以在这里加入自己的处理 */
    }

    /* 选中PLL作为系统时钟源并且配置HCLK,PCLK1和PCLK2*/
    rcc_clk_init_handle.ClockType = ( RCC_CLOCKTYPE_SYSCLK \
                                    | RCC_CLOCKTYPE_HCLK \
                                    | RCC_CLOCKTYPE_PCLK1 \
                                    | RCC_CLOCKTYPE_PCLK2);

    rcc_clk_init_handle.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;       /* 设置系统时钟时钟源为PLL */
    rcc_clk_init_handle.AHBCLKDivider  = RCC_SYSCLK_DIV1;               /* AHB分频系数为1 */
    rcc_clk_init_handle.APB1CLKDivider = RCC_HCLK_DIV4;                 /* APB1分频系数为4 */
    rcc_clk_init_handle.APB2CLKDivider = RCC_HCLK_DIV2;                 /* APB2分频系数为2 */

    ret = HAL_RCC_ClockConfig(&rcc_clk_init_handle, FLASH_LATENCY_5);   /* 同时设置FLASH延时周期为5WS，也就是6个CPU周期 */
    if(ret != HAL_OK)
    {
        return 1;                                                       /* 时钟初始化失败 */
    }
    
    /* STM32F405x/407x/415x/417x Z版本的器件支持预取功能 */
    if (HAL_GetREVID() == 0x1001)
    {
        __HAL_FLASH_PREFETCH_BUFFER_ENABLE();                           /* 使能flash预取 */
    }
    return 0;
}


/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
