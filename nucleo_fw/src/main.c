/*
 * main.c
 *
 *  Created on: 2017. 3. 19.
 *      Author: baram
 */
#include "main.h"
#include <math.h>


void mainInit(void);




osThreadId thread_led_handle;
osThreadId thread_dac_handle;

static void thread_led(void const *argument);
static void thread_dac(void const *argument);



int main(void)
{
  uint32_t t_time;
  uint32_t cnt = 0;


  mainInit();


  //osThreadDef(THREAD_LED, thread_led, osPriorityNormal, 0, configMINIMAL_STACK_SIZE);
  //osThreadDef(THREAD_LED, thread_led, osPriorityNormal, 0, 1024);
  osThreadDef(THREAD_DAC, thread_dac, osPriorityNormal, 0, 1024);

  //thread_led_handle = osThreadCreate(osThread(THREAD_LED), NULL);
  thread_dac_handle = osThreadCreate(osThread(THREAD_DAC), NULL);


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
  uint16_t color = _BLUE;
  uint16_t x = 0;
  uint32_t pre_time;
  uint8_t toggle = 0;
  uint8_t index = 0;
  uint32_t length;
  uint32_t i;

  lcdFillScreen(_RED);
  lcdDrawFrame();

  //dacSetup(44100);
  //dacStart();
  //dacStop();

  /*
  length = dacAvailable();
  for (i=0; i<length; i++)
  {
    dacPutch(index++);
  }

  dacStart();
  */

  pre_time = millis();
  for (;;)
  {
    ledToggle(0);
    osDelay(1);
    //uartPrintf(_DEF_UART1, "test\n");

    lcdFillScreen(_BLACK);
    lcdFillRect(x, 0, 50, 50, color);
    lcdDrawFrame();
    //color += 50;

    x++;
    x %= 50;

    /*
    length = dacAvailable();
    for (i=0; i<length-1; i++)
    {
      dacPutch(index++);
      index %= 100;
    }
    */

    //uartPrintf(_DEF_UART1, "length %d\r\n", dacAvailable());

    if (millis()-pre_time > 1000)
    {
      pre_time = millis();

      /*
      if (toggle)
        dacStart();
      else
        dacStop();
      */
      toggle ^= 1;
    }
  }
}

extern uint32_t dac_isr_count;

static void thread_dac(void const *argument)
{
  (void) argument;
  uint32_t index = 0;
  uint32_t length;
  uint32_t i;
  uint32_t pre_count;
  uint32_t pre_time;

  float wave_out = 0;
  int16_t out = 0;


  //dacSetup(44100);
  dacSetup(8000);
  dacStart();
  dacStop();


  length = dacAvailable();
  for (i=0; i<length; i++)
  {
    out  = (wave_data[index] - 128) * 2;
    out += 128;

    if (out > 255) out = 255;
    if (out < 0  ) out = 0;
    dacPutch(out);
    index++;
  }

  uartPrintf(_DEF_UART1, "length %d %d %d\r\n", length, dacAvailable(), dacGetDebug());

  dacStart();


  for (;;)
  {
    osDelay(1);

    length = dacAvailable();
    //uartPrintf(_DEF_UART1, "len in %d %d\r\n", length, dacGetDebug());
    for (i=0; i<length-1; i++)
    {
      out  = (wave_data[index] - 128) * 2;
      out += 128;

      if (out > 255) out = 255;
      if (out < 0  ) out = 0;

      dacPutch(out);

      if (index < NUM_ELEMENTS)
      {
        index++;
      }
    }
    //uartPrintf(_DEF_UART1, "len out %d %d\r\n", dacAvailable(), dacGetDebug());

    uartPrintf(_DEF_UART1, "cnt %d %d %d %d\r\n", millis()-pre_time, ((pre_count + 2048) - dacGetDebug())%2048, dacGetDebug(), dacAvailable());

    pre_count = dacGetDebug();
    pre_time  = millis();
  }
}
