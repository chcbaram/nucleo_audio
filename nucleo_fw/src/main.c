/*
 * main.c
 *
 *  Created on: 2017. 3. 19.
 *      Author: baram
 */
#include "main.h"



void mainInit(void);




osThreadId thread_led_handle;

static void thread_led(void const *argument);



int main(void)
{
  uint32_t t_time;
  uint32_t cnt = 0;


  mainInit();


  osThreadDef(THREAD_LED, thread_led, osPriorityNormal, 0, configMINIMAL_STACK_SIZE);
  thread_led_handle = osThreadCreate(osThread(THREAD_LED), NULL);


  osKernelStart();

  for (;;);



  t_time    = millis();
  while(1)
  {
    if (millis()-t_time >= 1000)
    {
      t_time = millis();
      uartPrintf(_DEF_UART1, "uartPrint : %d\r\n", cnt++);
    }

    if (buttonGetPressed(0) == true)
    {
      if (buttonGetPressedTime(0) > 3000)
      {
        ledOn(0);
        delay(200);
        ledOff(0);
        cmdifLoop();
      }
    }
  }

  return 0;
}

void mainInit(void)
{
  bspInit();
  hwInit();
  apInit();


  cmdifBegin(_DEF_UART1, 115200);
}


static void thread_led(void const *argument)
{
  (void) argument;


  for (;;)
  {
    ledToggle(0);
    osDelay(250);
  }
}
