/*
 * FreeRTOS Kernel V10.2.0
 * Copyright (C) 2019 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://www.FreeRTOS.org
 * http://aws.amazon.com/freertos
 *
 * 1 tab == 4 spaces!
 */

/* 
	NOTE : Tasks run in system mode and the scheduler runs in Supervisor mode.
	The processor MUST be in supervisor mode when vTaskStartScheduler is 
	called.  The demo applications included in the FreeRTOS.org download switch
	to supervisor mode prior to main being called.  If you are not using one of
	these demo application projects then ensure Supervisor mode is used.
*/


/*
 * Creates all the demo application tasks, then starts the scheduler.  The WEB
 * documentation provides more details of the demo application tasks.
 * 
 * Main.c also creates a task called "Check".  This only executes every three 
 * seconds but has the highest priority so is guaranteed to get processor time.  
 * Its main function is to check that all the other tasks are still operational.
 * Each task (other than the "flash" tasks) maintains a unique count that is 
 * incremented each time the task successfully completes its function.  Should 
 * any error occur within such a task the count is permanently halted.  The 
 * check task inspects the count of each task to ensure it has changed since
 * the last time the check task executed.  If all the count variables have 
 * changed all the tasks are still executing error free, and the check task
 * toggles the onboard LED.  Should any task contain an error at any time 
 * the LED toggle rate will change from 3 seconds to 500ms.
 *
 */

/* Standard includes. */
#include <stdlib.h>
#include <stdio.h>

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "lpc21xx.h"
#include "semphr.h"
#include "queue.h"

/* Peripheral includes. */
#include "serial.h"
#include "GPIO.h"


/* Handler init */
TaskHandle_t Button1_TriggerTaskHandler =NULL;
TaskHandle_t Button2_TriggerTaskHandler =NULL;
TaskHandle_t SendStr100msTaskHandler =NULL;
TaskHandle_t UART_WriteTaskHandler =NULL;



QueueHandle_t xQueue; 

typedef enum{
	RisingEdge,
  FallingEdge,
	Level_Low,
	Level_High
}ButtonStatus;

typedef struct{
	uint8_t* string;
}message;



/*-----------------------------------------------------------*/

/*-----------------------------------------------------------*/


/*-----------------------------------------------------------*/

/* Constants to setup I/O and processor. */
#define mainBUS_CLK_FULL	( ( unsigned char ) 0x01 )

/* Constants for the ComTest demo application tasks. */
#define mainCOM_TEST_BAUD_RATE	( ( unsigned long ) 115200 )


/*
 * Configure the processor for use with the Keil demo board.  This is very
 * minimal as most of the setup is managed by the settings in the project
 * file.
 */
static void prvSetupHardware( void );
/*-----------------------------------------------------------*/

 uint32_t i=0;
 uint32_t j=0;
 uint32_t load ,delay;
 
/*___________________________________________________________________________________________________________*/

ButtonStatus B1_status=Level_High;
ButtonStatus B2_status=Level_High;

message button1_message={(uint8_t *)"---BUTTON 1---\n"};
message button2_message={(uint8_t *)"---BUTTON 2---\n"};
message SendStr_message={(uint8_t *)"Hello World !!\n"};

message BufferdMessage;

uint8_t Button1,Button2;

/*-----------------------------------------------------------*/
void Button1_TriggerTask(void * pvParameters)
{
	for( ; ; ) 
	{
		Button1=GPIO_read(PORT_0,PIN0);
		
		if(Button1==0)
		{
			if(B1_status==Level_High)
			{
				/*falling edge */
				B1_status = FallingEdge;
				button1_message.string=(uint8_t *)"B1_FallingEdge\n";
			}
			else
			{
				/*Low Level*/
				B1_status = Level_Low;
				button1_message.string=(uint8_t *)"Button1 NoEdge\n";
				
			}
		}
		else /*B1==1*/
		{
			if(B1_status == Level_Low)
			{
				/*rising edge*/
				B1_status = RisingEdge;
				button1_message.string = (uint8_t *)"B1_RisingEdge \n";
			}
			else
			{
				/*High Level*/
				B1_status = Level_High;
				button1_message.string = (uint8_t *)"Button1 NoEdge\n";
			}
			
		}
		
		if(B1_status < 2)
			{
			if( xQueue != NULL )
				xQueueSend( xQueue,( void * ) &button1_message,( TickType_t ) 10 );
			}
     
		vTaskDelay(20);  /*periodicity : 20 ms */
	}
}
/*___________________________________________________________________________________________________________*/

void Button2_TriggerTask(void * pvParameters)
{
	for( ; ; ) 
	{
		Button2=GPIO_read(PORT_0,PIN1);
		
		if(Button2 == 0)
		{
			if(B2_status == Level_High)
			{
				/*falling edge */
				B2_status = FallingEdge;
				button2_message.string = (uint8_t *)"B2_FallingEdge\n";
			}
			else
			{
				/*Low Level*/
				B2_status = Level_Low;
				button2_message.string = (uint8_t *)"Button2 NoEdge\n";
				
			}
		}
		else /*B2==1*/
		{
			if(B2_status == Level_Low)
			{
				/*rising edge*/
				B2_status = RisingEdge;
				button2_message.string = (uint8_t *)"B2_RisingEdge \n";
			}
			else
			{
				/*High Level*/
				B2_status = Level_High;
				button2_message.string = (uint8_t *)"Button2 NoEdge\n";
			}
			
		}
		
		if(B2_status<2)
			{
			if( xQueue != 0 )
			xQueueSend( xQueue,( void * ) &button2_message,( TickType_t ) 10 );
			}
		
	
		vTaskDelay(30);  /*periodicity : 30 ms */
	}
}


/*___________________________________________________________________________________________________________*/

void SendStr100msTask(void * pvParameters)
{
	for( ; ; ) 
	{
		if( xQueue != 0 )
		{
		xQueueSend( xQueue,( void * ) &SendStr_message,( TickType_t ) 10 );
		}
		
		vTaskDelay(100);  /* periodicity : 100 ms */
	}
		
		
}
/*___________________________________________________________________________________________________________*/

void UART_Write(void * pvParameters)
{
	for( ; ; ) 
	{
		 if( xQueueReceive( xQueue,&(BufferdMessage ),( TickType_t ) 10 ) == pdPASS )
      {
         /* BufferdMessage now contains a copy of xMessage. */
				vSerialPutString((const signed char *)(BufferdMessage.string),15);
      }
		
		
		vTaskDelay(40);  /* periodicity : 40 ms */
	}
}


/*___________________________________________________________________________________________________________*/


/*
 * Application entry point:
 * Starts all the other tasks, then starts the scheduler. 
 */
int main( void )
{
	/* Setup the hardware for use with the Keil demo board. */
	prvSetupHardware();

	/*___________________________________________________________________________________________________________*/

    /* Create Tasks here */
	/*___________________________________________________________________________________________________________*/

	 xQueue = xQueueCreate(3, sizeof(message) );
	
	xTaskCreate(Button1_TriggerTask,"Button1 Trigger Task",  100,NULL,2,&Button1_TriggerTaskHandler);
	xTaskCreate(Button2_TriggerTask,"Button2 Trigger Task",  100,NULL,2,&Button2_TriggerTaskHandler);
	xTaskCreate(SendStr100msTask,		"Send String 100ms Task",100,NULL,1,&SendStr100msTaskHandler);
	xTaskCreate(UART_Write,			 		"UART Write Task",			 100,NULL,1,&UART_WriteTaskHandler);

		
										
/*___________________________________________________________________________________________________________*/

		
	/* Now all the tasks have been started - start the scheduler.
	NOTE : Tasks run in system mode and the scheduler runs in Supervisor mode.
	The processor MUST be in supervisor mode when vTaskStartScheduler is 
	called.  The demo applications included in the FreeRTOS.org download switch
	to supervisor mode prior to main being called.  If you are not using one of
	these demo application projects then ensure Supervisor mode is used here. */
	vTaskStartScheduler();

	/* Should never reach here!  If you do then there was not enough heap
	available for the idle task to be created. */
	for( ;; );
}
/*-----------------------------------------------------------*/

static void prvSetupHardware( void )
{
	/* Perform the hardware setup required.  This is minimal as most of the
	setup is managed by the settings in the project file. */

	/* Configure UART */
	xSerialPortInitMinimal(mainCOM_TEST_BAUD_RATE);

	/* Configure GPIO */
	GPIO_init();

	/* Setup the peripheral bus to be the same as the PLL output. */
	VPBDIV = mainBUS_CLK_FULL;
}
/*-----------------------------------------------*/