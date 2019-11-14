/******************************************************************************
*
* Copyright (C) 2009 - 2014 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

/*
 * helloworld.c: simple test application
 *
 * This application configures UART 16550 to baud rate 9600.
 * PS7 UART (Zynq) is not initialized by this application, since
 * bootrom/bsp configures it to baud rate 115200
 *
 * ------------------------------------------------
 * | UART TYPE   BAUD RATE                        |
 * ------------------------------------------------
 *   uartns550   9600
 *   uartlite    Configurable only in HW design
 *   ps7_uart    115200 (configured by bootrom/bsp)
 */

#include <stdio.h>
#include "platform.h"
#include "xil_printf.h"
#include "xil_types.h"
#include "xil_exception.h"
#include "AD5933.h"
#include "MCP23S17.h"
#include "platform.h"
#include "xparameters.h"
#include "sleep.h"

#include "xuartps_hw.h"
#include "xgpiops.h"

//GPIOPS
static XGpioPs Gpio; /* The Instance of the GPIO Driver */
#define GPIO_BANK	XGPIOPS_BANK0
u32 Input_Pin;

void startGPIOPS();

int main()
{
	init_platform();
	int rcalChoice;
    int Status;
    startGPIOPS();
    Status = SPIStart(&SpiInstance, SPI_DEVICE_ID);
    if(Status != XST_SUCCESS) {
    	return XST_FAILURE;
    }

    print("EIT: Impedance measurement program by Gilles Lenaerts.\n\r");

    //Calibrate AD5933 with x resistor (41K now)

    while(1){

    	if (XGpioPs_ReadPin(&Gpio, Input_Pin))
    	{
    		rcalChoice=0;
    		probeCurrentCycle++;
    		print("Choose Rcal for measurement..1,2 or 3. //Debugging\n\r");  //Printf print niet
    		while (rcalChoice == 0)
    		{
    			rcalChoice = inbyte(); //krijgt een Decimal 49 bij 1, 50 bij 2 -> -48 bij transfer naar func.
    		}
    		calibration(RCal_RFB_Select(rcalChoice-48,1)); // Loopt vast als er geen hw connected is bij AD5933GetTemp(), logisch :)
        	//Measurement Cycle
        	while (probeCurrentCycle < 8){
            	probeMeasureSelect();
            	measureImpedance();
        		if (probeVoltCycle >= 5)
        			{
        				probeVoltCycle = 1;
        				if (probeCurrentCycle >= 8)
        				{
        					probeCurrentCycle = 0;
        					break;
        				}
        				else probeCurrentCycle++;
        			}
        		else probeVoltCycle++;
        	}
    	}
    }
    cleanup_platform();
    return 0;
}

void startGPIOPS()
{
	volatile int Delay;

	int status;
	XGpioPs_Config *GPIOConfigPtr;

	GPIOConfigPtr = XGpioPs_LookupConfig(XPAR_PS7_GPIO_0_DEVICE_ID);
	status = XGpioPs_CfgInitialize(&Gpio, GPIOConfigPtr,GPIOConfigPtr->BaseAddr);

	if (status != XST_SUCCESS) {
		printf("status error \n\r");
		return XST_FAILURE;
	}

    printf("Starting GPIO PS..\n\r");
	// Set Input pin
    Input_Pin = 0;
	XGpioPs_SetDirectionPin(&Gpio,Input_Pin,0);
}
