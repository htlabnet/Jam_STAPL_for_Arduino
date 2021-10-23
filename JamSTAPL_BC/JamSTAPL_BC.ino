//
// #################################################
//
//    Jam STAPL Byte-Code Player for Arduino
//
//    Copyright (C) 2021
//      Hideto Kikuchi / PJ (@pcjpnet) - http://pc-jp.net/
//
//
//    !!!!! NOT ALLOWED COMMERCIAL USE !!!!!
//
// #################################################
//


#if defined(STM32_CORE_VERSION)
  // STM32duino with STM32SD / FatFs Library
  #include <STM32SD.h>
#else
  // Raspberry Pi Pico
  // ESP32
  // Arduino Board [8.3 File Name]
  #include <SPI.h>
  #include <SD.h> // ONLY [8.3 File Name]
#endif

#include "jbiarduino.h"
#include "stm32f7xx_hal_sdram.h"


#define SDRAM_START_ADDR                         (0xc0000000)
#define SDRAM_SIZE                               (0x00800000) // 8MB

#define SDRAM_MEMORY_WIDTH                       FMC_SDRAM_MEM_BUS_WIDTH_16
#define SDCLOCK_PERIOD                           FMC_SDRAM_CLOCK_PERIOD_2
#define SDRAM_TIMEOUT                            ((uint32_t)0xFFFF)
#define SDRAM_MODEREG_BURST_LENGTH_1             ((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_LENGTH_2             ((uint16_t)0x0001)
#define SDRAM_MODEREG_BURST_LENGTH_4             ((uint16_t)0x0002)
#define SDRAM_MODEREG_BURST_LENGTH_8             ((uint16_t)0x0004)
#define SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL      ((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_TYPE_INTERLEAVED     ((uint16_t)0x0008)
#define SDRAM_MODEREG_CAS_LATENCY_2              ((uint16_t)0x0020)
#define SDRAM_MODEREG_CAS_LATENCY_3              ((uint16_t)0x0030)
#define SDRAM_MODEREG_OPERATING_MODE_STANDARD    ((uint16_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_PROGRAMMED ((uint16_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_SINGLE     ((uint16_t)0x0200)


/**
  * @brief SDRAM MSP Initialization
  *        This function configures the hardware resources used in this example: 
  *           - Peripheral's clock enable
  *           - Peripheral's GPIO Configuration  
  * @param hsram: SDRAM handle pointer
  * @retval None
  */
void HAL_SDRAM_MspInit(SDRAM_HandleTypeDef *hsdram)
{
  GPIO_InitTypeDef  GPIO_Init_Structure;

  /*##-1- Enable peripherals and GPIO Clocks #################################*/
  /* Enable GPIO clocks */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();

  /* Enable FMC clock */
  __HAL_RCC_FMC_CLK_ENABLE();

  /*##-2- Configure peripheral GPIO ##########################################*/
  GPIO_Init_Structure.Mode      = GPIO_MODE_AF_PP;
  GPIO_Init_Structure.Pull      = GPIO_PULLUP;
  GPIO_Init_Structure.Speed     = GPIO_SPEED_FAST;
  GPIO_Init_Structure.Alternate = GPIO_AF12_FMC;


  /* GPIOC configuration */
  GPIO_Init_Structure.Pin   = GPIO_PIN_3;
  HAL_GPIO_Init(GPIOC, &GPIO_Init_Structure);
  
  /* GPIOD configuration */
  GPIO_Init_Structure.Pin   = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_8 | GPIO_PIN_9 |
                              GPIO_PIN_10 | GPIO_PIN_14 | GPIO_PIN_15;
  HAL_GPIO_Init(GPIOD, &GPIO_Init_Structure);

  /* GPIOE configuration */
  GPIO_Init_Structure.Pin   = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_7| GPIO_PIN_8 | GPIO_PIN_9       |\
                              GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 |\
                              GPIO_PIN_15;
  HAL_GPIO_Init(GPIOE, &GPIO_Init_Structure);
  
  /* GPIOF configuration */
  GPIO_Init_Structure.Pin   = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2| GPIO_PIN_3 | GPIO_PIN_4      |\
                              GPIO_PIN_5 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 |\
                              GPIO_PIN_15;
  HAL_GPIO_Init(GPIOF, &GPIO_Init_Structure);
  
  /* GPIOG configuration */
  GPIO_Init_Structure.Pin   = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_4| GPIO_PIN_5 | GPIO_PIN_8 |\
                              GPIO_PIN_15;
  HAL_GPIO_Init(GPIOG, &GPIO_Init_Structure);

  /* GPIOH configuration */
  GPIO_Init_Structure.Pin   = GPIO_PIN_3 | GPIO_PIN_5;
  HAL_GPIO_Init(GPIOH, &GPIO_Init_Structure);
}

/**
  * @brief  Perform the SDRAM external memory initialization sequence
  * @param  hsdram: SDRAM handle
  * @param  Command: Pointer to SDRAM command structure
  * @retval None
  */
static void BSP_SDRAM_Initialization_Sequence(SDRAM_HandleTypeDef *hsdram, FMC_SDRAM_CommandTypeDef *Command)
{
  __IO uint32_t tmpmrd =0;
  /* Step 3:  Configure a clock configuration enable command */
  Command->CommandMode = FMC_SDRAM_CMD_CLK_ENABLE;
  Command->CommandTarget = FMC_SDRAM_CMD_TARGET_BANK1;
  Command->AutoRefreshNumber = 1;
  Command->ModeRegisterDefinition = 0;

  /* Send the command */
  HAL_SDRAM_SendCommand(hsdram, Command, SDRAM_TIMEOUT);

  /* Step 4: Insert 100 us minimum delay */
  /* Inserted delay is equal to 1 ms due to systick time base unit (ms) */
  HAL_Delay(1);

  /* Step 5: Configure a PALL (precharge all) command */
  Command->CommandMode = FMC_SDRAM_CMD_PALL;
  Command->CommandTarget = FMC_SDRAM_CMD_TARGET_BANK1;
  Command->AutoRefreshNumber = 1;
  Command->ModeRegisterDefinition = 0;

  /* Send the command */
  HAL_SDRAM_SendCommand(hsdram, Command, SDRAM_TIMEOUT);

  /* Step 6 : Configure a Auto-Refresh command */
  Command->CommandMode = FMC_SDRAM_CMD_AUTOREFRESH_MODE;
  Command->CommandTarget = FMC_SDRAM_CMD_TARGET_BANK1;
  Command->AutoRefreshNumber = 8;
  Command->ModeRegisterDefinition = 0;

  /* Send the command */
  HAL_SDRAM_SendCommand(hsdram, Command, SDRAM_TIMEOUT);

  /* Step 7: Program the external memory mode register */
  tmpmrd = (uint32_t)SDRAM_MODEREG_BURST_LENGTH_1          |
                     SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL   |
                     SDRAM_MODEREG_CAS_LATENCY_2           |
                     SDRAM_MODEREG_OPERATING_MODE_STANDARD |
                     SDRAM_MODEREG_WRITEBURST_MODE_SINGLE;

  Command->CommandMode = FMC_SDRAM_CMD_LOAD_MODE;
  Command->CommandTarget = FMC_SDRAM_CMD_TARGET_BANK1;
  Command->AutoRefreshNumber = 1;
  Command->ModeRegisterDefinition = tmpmrd;

  /* Send the command */
  HAL_SDRAM_SendCommand(hsdram, Command, SDRAM_TIMEOUT);

  /* Step 8: Set the refresh rate counter */
  /* (15.62 us x Freq) - 20 */
  /* Set the device refresh counter */
  hsdram->Instance->SDRTR |= ((uint32_t)((1292)<< 1));
}


void setup()
{
  SDRAM_HandleTypeDef      hsdram;
  FMC_SDRAM_TimingTypeDef  SDRAM_Timing;
  FMC_SDRAM_CommandTypeDef command;

  uint32_t * sdram_ptr = (uint32_t *)SDRAM_START_ADDR;

  Serial.begin(115200);

  while (!Serial) {
    ; // Wait For Connect
  }
  delay(1000);
 

  /*##-1- Configure the SDRAM device #########################################*/
  /* SDRAM device configuration */
  hsdram.Instance = FMC_SDRAM_DEVICE;
  SDRAM_Timing.LoadToActiveDelay    = 2;
  SDRAM_Timing.ExitSelfRefreshDelay = 6;
  SDRAM_Timing.SelfRefreshTime      = 4;
  SDRAM_Timing.RowCycleDelay        = 6;
  SDRAM_Timing.WriteRecoveryTime    = 2;
  SDRAM_Timing.RPDelay              = 2;
  SDRAM_Timing.RCDDelay             = 2;
  hsdram.Init.SDBank             = FMC_SDRAM_BANK1;
  hsdram.Init.ColumnBitsNumber   = FMC_SDRAM_COLUMN_BITS_NUM_8;
  hsdram.Init.RowBitsNumber      = FMC_SDRAM_ROW_BITS_NUM_12;
  hsdram.Init.MemoryDataWidth    = SDRAM_MEMORY_WIDTH;
  hsdram.Init.InternalBankNumber = FMC_SDRAM_INTERN_BANKS_NUM_4;
  hsdram.Init.CASLatency         = FMC_SDRAM_CAS_LATENCY_2;
  hsdram.Init.WriteProtection    = FMC_SDRAM_WRITE_PROTECTION_DISABLE;
  hsdram.Init.SDClockPeriod      = SDCLOCK_PERIOD;
  hsdram.Init.ReadBurst          = FMC_SDRAM_RBURST_ENABLE;
  hsdram.Init.ReadPipeDelay      = FMC_SDRAM_RPIPE_DELAY_0;

  /* FMC GPIO Settings */
  HAL_SDRAM_MspInit(&hsdram);

  /* Initialize the SDRAM controller */
  if(HAL_SDRAM_Init(&hsdram, &SDRAM_Timing) != HAL_OK)
  {
    /* Initialization Error */
    Serial.println("SDRAM Initialization Error");
    while (1);
  }

  /* Program the SDRAM external device */
  BSP_SDRAM_Initialization_Sequence(&hsdram, &command);


  /* SDRAM R/W Test */
  Serial.println("------------------------ SDRAM TEST ------------------------");
  Serial.print("Address ");
  Serial.println((uint32_t)sdram_ptr, HEX);
  *sdram_ptr = 0x12345678;
  Serial.print("Write data = ");
  Serial.println(0x12345678, HEX);
  Serial.print("Read data = ");
  Serial.println(*sdram_ptr, HEX);

  uint32_t sum1 = 0;
  for (uint32_t i = 0; i < SDRAM_SIZE / 4; i++)
  {
    *(sdram_ptr + i) = i;
    sum1 += i;
  }
  Serial.print("SUM1:");
  Serial.println(sum1, HEX);

  uint32_t sum2 = 0;
  for (uint32_t i = 0; i < SDRAM_SIZE / 4; i++)
  {
    sum2 += *(sdram_ptr + i);
  }
  Serial.print("SUM2:");
  Serial.println(sum2, HEX);

  // zero fill
  for (uint32_t i = 0; i < SDRAM_SIZE / 4; i++)
  {
    *(sdram_ptr + i) = 0;
  }
  Serial.println("------------------------ SDRAM END ------------------------");


  Serial.println("############ Update Start ############");

  // Set SD CS Pin Number
  if (!SD.begin()) {
    Serial.println("SD Initialization Failed!");
    while (1);
  } else {
    Serial.println("SD Initialization Done.");
  }

  // Set Output Stream
  jbi_set_stream(Serial);

  // Set JTAG Pin(TCK, TDI, TMS, TDO)
  jbi_begin(5, 6, 7, 8);
  //jbi_begin(16, 17, 18, 19);


  //File jbc_file = SD.open("/sfp.jbc");
  File jbc_file = SD.open("/led.jbc");

  // Execute Program (Normal Operation)
  jbi_exec_program(jbc_file);

  // Execute Configure (MAX10 SRAM Write)
  //jbi_exec_configure(jbc_file);

}



void loop() {
  // main loop
  
}
