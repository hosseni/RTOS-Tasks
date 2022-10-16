
/*
	try different priority levels
	preemption enable from config file
	understand the effect of the load
*/
 /*************************************************** Task 2 *************************************************************/

/* Standard includes. */
#include <stdlib.h>
#include <stdio.h>

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"
#include "lpc21xx.h"

/* Peripheral includes. */
#include "serial.h"
#include "GPIO.h"




/*-----------------------------------------------------------*/

/* Constants to setup I/O and processor. */
#define mainBUS_CLK_FULL	( ( unsigned char ) 0x01 )

/* Constants for the ComTest demo application tasks. */
#define mainCOM_TEST_BAUD_RATE	( ( unsigned long ) 115200 )


/*-----------------------------------------------------------*/

/*
 * Configure the processor for use with the Keil demo board.  This is very
 * minimal as most of the setup is managed by the settings in the project
 * file.
 */

static void prvSetupHardware( void );


/*-----------------------------------------------------------*/


TaskHandle_t task1_Handle =NULL;
TaskHandle_t task2_Handle =NULL;



TaskTimingParameters task1Time,task2Time;
uint8_t CPU_load =0;

/*-----------------------------------------------------------*/
void task1(void * pvParameters) 
{
	TickType_t xLastWakeTime1;
	uint32_t i=0;
	//TickType_t StartTime,EndTime;
  xLastWakeTime1 = xTaskGetTickCount();
	
   vTaskSetApplicationTaskTag( task1_Handle, (TaskHookFunction_t ) 1 );
	
	for( ; ; ) 
	{
		
		vTaskDelayUntil( &xLastWakeTime1, pdMS_TO_TICKS(5) ); 
		
		/* IDLE task */
		GPIO_write(PORT_0,PIN2,PIN_IS_LOW);
		
		for(i=0;i<18000;i++)
		{
			i = i;
		}
	}
}


void task2(void * pvParameters)
{
	TickType_t xLastWakeTime2;
	uint32_t i=0;
	xLastWakeTime2 = xTaskGetTickCount();
	
  vTaskSetApplicationTaskTag( task2_Handle, (TaskHookFunction_t ) 2 );
	
	for( ; ; ) 
	{
		vTaskDelayUntil( &xLastWakeTime2, pdMS_TO_TICKS(15) );
		
		/* IDLE task */
		GPIO_write(PORT_0,PIN2,PIN_IS_LOW);
		
		for(i=0;i<20000;i++) 
		{
			i = i;
		
		}
	}
}

/* Implement Tick Hook */
void vApplicationTickHook( void )
{	
	/* Tick */
	GPIO_write(PORT_0,PIN1,PIN_IS_HIGH);
	GPIO_write(PORT_0,PIN1,PIN_IS_LOW);
	
	/* CPU load */
	CPU_load=((float)(task1Time.TotalExecution + task2Time.TotalExecution)/(T1TC))*100;
	
}

void vApplicationIdleHook( void )
{
	/* IDLE task*/
	GPIO_write(PORT_0,PIN2,PIN_IS_HIGH);
}
/*-----------------------------------------------------------*/

 
int main( void )
{
	/* variables declaration */
	
	
	/* Setup the hardware for use with the Keil demo board. */
	prvSetupHardware();

	
  /* Create Tasks here */
  xTaskCreatePeriodic(task1,"Task1",100,NULL,1,&task1_Handle,5);
	xTaskCreatePeriodic(task2,"Task2",100,NULL,1,&task2_Handle,15);
	
	/* Initialize tasks Exection time to zero */
	task1Time.TotalExecution =0;
  task2Time.TotalExecution =0;
	
  vTaskStartScheduler();

	/* Should never reach here!  If you do then there was not enough heap
	available for the idle task to be created. */
	for( ;; );
}
/*-----------------------------------------------------------*/

/*-----------------------------------------------------------*/

/* Timer 1 */

void ResetTimer1()
{
	T1TCR |=0x02;
	T1PR &= ~(0x02);
	
}

static void configTimer1()
{
	T1PR =1000; /* 20 khz  */
	T1TCR |=0x01;
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
	
	/* Configure Trace Timer 1 and read T1TC to get the current tick */
	configTimer1();

	/* Setup the peripheral bus to be the same as the PLL output. */
	VPBDIV = mainBUS_CLK_FULL;
}


/**********************************************************************************************************************/




