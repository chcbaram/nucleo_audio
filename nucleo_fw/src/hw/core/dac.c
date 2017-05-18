/*
 *  dac.c
 *
 *  Created on: 2016. 7. 13.
 *      Author: Baram
 */
#include "hw.h"
#include "dac.h"




DAC_HandleTypeDef       DacHandle;


typedef struct
{
  DAC_ChannelConfTypeDef  sConfig;
  uint32_t                channel;
  uint8_t                 resolution;
} dac_t;



//const uint8_t aEscalator8bit[6] = {0x0, 0x33, 0x66, 0x99, 0xCC, 0xFF};
uint8_t dac_buffer[256];


static dac_t dac_tbl[DAC_CH_MAX];



int dacCmdif(int argc, char **argv);
void dacInitTimer(uint32_t hz);



void dacInit(void)
{
  uint32_t i;


  cmdifAdd("dac", dacCmdif);


  for (i=0; i<256; i++)
  {
    dac_buffer[i] = i;
  }




  DacHandle.Instance = DAC1;

  HAL_DAC_Init(&DacHandle);

  dac_tbl[0].channel    = DAC_CHANNEL_1;
  dac_tbl[0].resolution = 8;

  dac_tbl[0].sConfig.DAC_Trigger      = DAC_TRIGGER_T6_TRGO;
  dac_tbl[0].sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;

  HAL_DAC_ConfigChannel(&DacHandle, &dac_tbl[0].sConfig, dac_tbl[0].channel);

  HAL_DAC_Start_DMA(&DacHandle, dac_tbl[0].channel, (uint32_t *)dac_buffer, 100, DAC_ALIGN_8B_R);



  dac_tbl[1].channel    = DAC_CHANNEL_2;
  dac_tbl[1].resolution = 8;

  dac_tbl[1].sConfig.DAC_Trigger      = DAC_TRIGGER_T6_TRGO;
  dac_tbl[1].sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;

  HAL_DAC_ConfigChannel(&DacHandle, &dac_tbl[1].sConfig, dac_tbl[1].channel);

  HAL_DAC_Start_DMA(&DacHandle, dac_tbl[1].channel, (uint32_t *)dac_buffer, 100, DAC_ALIGN_8B_R);


  dacInitTimer(100000);
}


void dacInitTimer(uint32_t hz)
{
  static TIM_HandleTypeDef  htim;
  TIM_MasterConfigTypeDef sMasterConfig;

  htim.Instance = TIM6;


  htim.Init.Period            = 10-1;
  htim.Init.Prescaler         = (uint32_t)((SystemCoreClock / 1) / (hz*10)) - 1;
  htim.Init.ClockDivision     = 0;
  htim.Init.CounterMode       = TIM_COUNTERMODE_UP;
  htim.Init.RepetitionCounter = 0;
  HAL_TIM_Base_Init(&htim);

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
  sMasterConfig.MasterSlaveMode     = TIM_MASTERSLAVEMODE_DISABLE;

  HAL_TIMEx_MasterConfigSynchronization(&htim, &sMasterConfig);

  HAL_TIM_Base_Start(&htim);
}


//-- dacCmdif
//
int dacCmdif(int argc, char **argv)
{
  bool ret = true;
  //uint8_t number;


  if (argc == 3)
  {
    //number = (uint8_t) strtoul((const char * ) argv[2], (char **)NULL, (int) 0);

    if(strcmp("on", argv[1]) == 0)
    {
    }
    else if(strcmp("off", argv[1])==0)
    {
    }
    else if(strcmp("toggle", argv[1])==0)
    {
    }
    else if(strcmp("demo", argv[1])==0)
    {
      while(cmdifRxAvailable() == 0)
      {
      }
    }
    else
    {
      ret = false;
    }
  }
  else
  {
    ret = false;
  }

  if (ret == false)
  {
    cmdifPrintf( "dac on/off/toggle/demo number ...\n");
  }

  return 0;
}



void DMA1_Channel3_IRQHandler(void)
{
  HAL_DMA_IRQHandler(DacHandle.DMA_Handle1);
}

void DMA1_Channel4_IRQHandler(void)
{
  HAL_DMA_IRQHandler(DacHandle.DMA_Handle2);
}


void HAL_DAC_MspInit(DAC_HandleTypeDef *hdac)
{
  GPIO_InitTypeDef          GPIO_InitStruct;
  static DMA_HandleTypeDef  hdma_dac1;
  static DMA_HandleTypeDef  hdma_dac2;


  /*##-1- Enable peripherals and GPIO Clocks #################################*/
  /* Enable GPIO clock ****************************************/
  __HAL_RCC_GPIOA_CLK_ENABLE();
  /* DAC Periph clock enable */
  __HAL_RCC_DAC1_CLK_ENABLE();
  /* DMA1 clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /*##-2- Configure peripheral GPIO ##########################################*/
  /* DAC Channel1 GPIO pin configuration */
  GPIO_InitStruct.Pin   = GPIO_PIN_4;
  GPIO_InitStruct.Mode  = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull  = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*##-3- Configure the DMA ##########################################*/
  /* Set the parameters to be configured for DACx_DMA1_CHANNEL3 */
  hdma_dac1.Instance                  = DMA1_Channel3;

  hdma_dac1.Init.Request              = DMA_REQUEST_6;

  hdma_dac1.Init.Direction            = DMA_MEMORY_TO_PERIPH;
  hdma_dac1.Init.PeriphInc            = DMA_PINC_DISABLE;
  hdma_dac1.Init.MemInc               = DMA_MINC_ENABLE;
  hdma_dac1.Init.PeriphDataAlignment  = DMA_PDATAALIGN_BYTE;
  hdma_dac1.Init.MemDataAlignment     = DMA_MDATAALIGN_BYTE;
  hdma_dac1.Init.Mode                 = DMA_CIRCULAR;
  hdma_dac1.Init.Priority             = DMA_PRIORITY_HIGH;

  HAL_DMA_Init(&hdma_dac1);

  /* Associate the initialized DMA handle to the DAC handle */
  __HAL_LINKDMA(hdac, DMA_Handle1, hdma_dac1);

  /*##-4- Configure the NVIC for DMA #########################################*/
  /* Enable the DMA1_Channel3 IRQ Channel */
  HAL_NVIC_SetPriority(DMA1_Channel3_IRQn, 2, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel3_IRQn);






  /*##-2- Configure peripheral GPIO ##########################################*/
  /* DAC Channel1 GPIO pin configuration */
  GPIO_InitStruct.Pin   = GPIO_PIN_5;
  GPIO_InitStruct.Mode  = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull  = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*##-3- Configure the DMA ##########################################*/
  /* Set the parameters to be configured for DACx_DMA1_CHANNEL3 */
  hdma_dac2.Instance                  = DMA1_Channel4;

  hdma_dac2.Init.Request              = DMA_REQUEST_5;

  hdma_dac2.Init.Direction            = DMA_MEMORY_TO_PERIPH;
  hdma_dac2.Init.PeriphInc            = DMA_PINC_DISABLE;
  hdma_dac2.Init.MemInc               = DMA_MINC_ENABLE;
  hdma_dac2.Init.PeriphDataAlignment  = DMA_PDATAALIGN_BYTE;
  hdma_dac2.Init.MemDataAlignment     = DMA_MDATAALIGN_BYTE;
  hdma_dac2.Init.Mode                 = DMA_CIRCULAR;
  hdma_dac2.Init.Priority             = DMA_PRIORITY_HIGH;

  HAL_DMA_Init(&hdma_dac2);

  /* Associate the initialized DMA handle to the DAC handle */
  __HAL_LINKDMA(hdac, DMA_Handle2, hdma_dac2);

  /*##-4- Configure the NVIC for DMA #########################################*/
  /* Enable the DMA1_Channel3 IRQ Channel */
  HAL_NVIC_SetPriority(DMA1_Channel4_IRQn, 2, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel4_IRQn);

}

/**
  * @brief  DeInitializes the DAC MSP.
  * @param  hdac: pointer to a DAC_HandleTypeDef structure that contains
  *         the configuration information for the specified DAC.
  * @retval None
  */
void HAL_DAC_MspDeInit(DAC_HandleTypeDef *hdac)
{
  __HAL_RCC_DAC1_FORCE_RESET();
  __HAL_RCC_DAC1_RELEASE_RESET();

  HAL_GPIO_DeInit(GPIOA, GPIO_PIN_4);
  HAL_GPIO_DeInit(GPIOA, GPIO_PIN_5);

  HAL_DMA_DeInit(hdac->DMA_Handle1);
  HAL_NVIC_DisableIRQ(DMA1_Channel3_IRQn);

  HAL_DMA_DeInit(hdac->DMA_Handle2);
  HAL_NVIC_DisableIRQ(DMA1_Channel5_IRQn);

}


