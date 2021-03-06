/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "timers.h"
#include "xil_printf.h"

/* App includes. */
//#include "xgpio.h"
#include "xgpiops.h"
#define GPIO_DEVICE_ID	XPAR_XGPIOPS_0_DEVICE_ID

#define OUTPUT_PIN_1		52	/* Pin connected to LED/Output */
#define OUTPUT_PIN_2		53	/* Pin connected to LED/Output */

#define BLINK_PERIOD	100 /* Delay duration before next state transition */

/*-------TIMERS-----*/
#define TIMER_ID1	1
#define TIMER_ID2	2
#define DELAY_3_SECONDS	3000UL
#define DELAY_5_SECONDS	5000UL
#define TIMER_CHECK_THRESHOLD	9
static void vTimerCallback( TimerHandle_t pxTimer );
static TimerHandle_t xTimer1, xTimer2= NULL;
int TimerFlag, TimerFlag2 = 0;

/* Priorities at which the tasks are created. */
#define mainLED_ON_TASK_PRIORITY		( tskIDLE_PRIORITY + 1 )
#define	mainLED_OFF_TASK_PRIORITY		( tskIDLE_PRIORITY + 1 )

/*-----------------------------------------------------------*/
static void prvLed_ON( void *pvParameters );
static void prvLed_OFF( void *pvParameters );
void prvSetGpioHardware(void);
void prvLed_Toggle (int Mode);

XGpioPs Gpio;	/* The driver instance for GPIO Device. */
XGpioPs_Config *ConfigPtr; /* The driver config instance for GPIO Device. */

xSemaphoreHandle xMutex_Led = NULL;

static void vTimerCallback( TimerHandle_t pxTimer )
{
	long lTimerId;
	configASSERT( pxTimer );

	lTimerId = ( long ) pvTimerGetTimerID( pxTimer );

	if (lTimerId == TIMER_ID1)
	{
	xil_printf("Turning on 5sec LED... PASSED");
	TimerFlag = 1;
	}
	else if (lTimerId == TIMER_ID2)
	{
		xil_printf("Toggling 3sec LED... PASSED");
		TimerFlag2 = !TimerFlag2;
		xTimerStart( xTimer2, 0 );
	}
	else xil_printf("Error in Intr timer... FAILED");
}


int main( void )
{
	xil_printf( "Hello from Gilles.. Oefening 11 TIMERS. \r\n" );
	xMutex_Led = xSemaphoreCreateMutex();

	configASSERT( xMutex_Led );

	/* Setup the GPIO Hardware. */
	prvSetGpioHardware();

	const TickType_t x3seconds = pdMS_TO_TICKS( DELAY_3_SECONDS );
	const TickType_t x5seconds = pdMS_TO_TICKS( DELAY_5_SECONDS );

	/* Start the two tasks */
	xTaskCreate( prvLed_ON, ( signed char * ) "LED_ON",
				configMINIMAL_STACK_SIZE, NULL,
				mainLED_ON_TASK_PRIORITY, NULL );
	/*
	xTaskCreate( prvLed_OFF, ( signed char * ) "LED_OFF",
				configMINIMAL_STACK_SIZE, NULL,
				mainLED_OFF_TASK_PRIORITY, NULL );*/

	/*----Timer Configs---*/
	xTimer1 = xTimerCreate( (const char *) "Timer1",
							x5seconds,
							pdFALSE,
							(void *) TIMER_ID1,
							vTimerCallback);
	xTimer2 = xTimerCreate( (const char *) "Timer2",
							x3seconds,
							pdFALSE,
							(void *) TIMER_ID2,
							vTimerCallback);

	/* Start the tasks and timer running. */
	xTimerStart( xTimer1, 0 );
	xTimerStart( xTimer2, 0 );
	vTaskStartScheduler();

	/* If all is well, the scheduler will now be running, and the following line
	will never be reached.  If the following line does execute, then there was
	insufficient FreeRTOS heap memory available for the idle and/or timer tasks
	to be created.  See the memory management section on the FreeRTOS web site
	for more details. */
	for( ;; );
}

/*-----------------------------------------------------------*/
static void prvLed_ON( void *pvParameters )
{
	for( ;; )
	{

		prvLed_Toggle(TimerFlag2);
		if (TimerFlag)
		{
			//xTimerStart( xTimer1, 0 );
			XGpioPs_WritePin(&Gpio, OUTPUT_PIN_2, 1);
			TimerFlag=0;
		}
		taskYIELD();
	}
}

/*-----------------------------------------------------------*/
/*
static void prvLed_OFF( void *pvParameters )
{
	for( ;; )
	{
		prvLed_Toggle(0x0);
		taskYIELD();
	}
} */

void prvLed_Toggle (int Mode)
{
	portTickType xNextWakeTime;
	int Data;

	xSemaphoreTake(xMutex_Led, ( portTickType ) portMAX_DELAY);

	XGpioPs_WritePin(&Gpio, OUTPUT_PIN_1, Mode);
	Data = XGpioPs_ReadPin(&Gpio, OUTPUT_PIN_1);
	if (Data != Mode ) {
		xil_printf("Pin value read is not as expected\n");
	}

	if (Mode == 0x1)
		xil_printf("LED2 ON\r\n");
	else
		xil_printf("LED2 OFF\r\n");

	/* Initialize xNextWakeTime - this only needs to be done once. */
	xNextWakeTime = xTaskGetTickCount();

	/* Place this task in the blocked state until it is time to run again.
	The block time is specified in ticks, the constant used converts ticks
	to ms.  While in the Blocked state this task will not consume any CPU
	time. */
	vTaskDelayUntil(&xNextWakeTime, BLINK_PERIOD );

	xSemaphoreGive(xMutex_Led);
}

/*-----------------------------------------------------------*/
void prvSetGpioHardware(void)
{
	int Status;
	/*
	 * Initialize the GPIO driver.
	 */
	ConfigPtr = XGpioPs_LookupConfig(GPIO_DEVICE_ID);
	Status = XGpioPs_CfgInitialize(&Gpio, ConfigPtr,
					ConfigPtr->BaseAddr);
	if (Status != XST_SUCCESS) {
		xil_printf("GPIO Initialize failed\n");
	}

	XGpioPs_SetDirectionPin(&Gpio, OUTPUT_PIN_1, 1);
	XGpioPs_SetOutputEnablePin(&Gpio, OUTPUT_PIN_1, 1);

	XGpioPs_SetDirectionPin(&Gpio, OUTPUT_PIN_2, 1);
	XGpioPs_SetOutputEnablePin(&Gpio, OUTPUT_PIN_2, 1);

	XGpioPs_WritePin(&Gpio, OUTPUT_PIN_1, 0x0);
	XGpioPs_WritePin(&Gpio, OUTPUT_PIN_2, 0x0);
}


/*-----------------------------------------------------------*/
void vApplicationMallocFailedHook( void )
{
	/* vApplicationMallocFailedHook() will only be called if
	configUSE_MALLOC_FAILED_HOOK is set to 1 in FreeRTOSConfig.h.  It is a hook
	function that will get called if a call to pvPortMalloc() fails.
	pvPortMalloc() is called internally by the kernel whenever a task, queue or
	semaphore is created.  It is also called by various parts of the demo
	application.  If heap_1.c or heap_2.c are used, then the size of the heap
	available to pvPortMalloc() is defined by configTOTAL_HEAP_SIZE in
	FreeRTOSConfig.h, and the xPortGetFreeHeapSize() API function can be used
	to query the size of free heap space that remains (although it does not
	provide information on how the remaining heap might be fragmented). */
	taskDISABLE_INTERRUPTS();
	for( ;; );
}

/*-----------------------------------------------------------*/
void vApplicationStackOverflowHook( xTaskHandle *pxTask, signed char *pcTaskName )
{
	( void ) pcTaskName;
	( void ) pxTask;

	/* vApplicationStackOverflowHook() will only be called if
	configCHECK_FOR_STACK_OVERFLOW is set to either 1 or 2.  The handle and name
	of the offending task will be passed into the hook function via its
	parameters.  However, when a stack has overflowed, it is possible that the
	parameters will have been corrupted, in which case the pxCurrentTCB variable
	can be inspected directly. */
	taskDISABLE_INTERRUPTS();
	for( ;; );
}

void vApplicationSetupHardware( void )
{
	/* Do nothing */
}
