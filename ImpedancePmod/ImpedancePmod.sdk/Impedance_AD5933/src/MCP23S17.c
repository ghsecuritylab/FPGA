#include <stdio.h>
#include <math.h>
#include "MCP23S17.h"
#include "AD5933.h"

char rcalSEL = 0x00;

/*
* Rcal & RFB selects
 */
int RCal_RFB_Select(int RCAL, int RFB){

	static char aPortSEL=0x00;
	static char bPortSEL=0x00;
	static int rcalval;

	rcalval = 0;

	switch(RCAL)
		{
			case 1:     print("Calibrate using RCAL1 10K.");
						aPortSEL = (GPA6 | GPA7);   //Rcal 1
						rcalval = 10000;
						break;
			case 2:     print("Calibrate using RCAL2 22K.");
						aPortSEL = (GPA7);   //Rcal 1
						rcalval = 22000;
						break;
			case 3:     print("Calibrate using RCAL3 47K.");
						aPortSEL = (GPA6);   //Rcal 1
						rcalval = 47000;
						break;
		}

		switch(RFB)
		{
			case 1:     print(" & RFB1 10K.\n");
						break;
			case 2:     print(" & RFB1 22K.\n");
						aPortSEL |= (GPA5);   //RFB 2
						break;
			case 3:     print(" & RFB1 1K.\n");
						aPortSEL |= (GPA4);   //Rcal 1
						break;
			case 4:     print(" & RFB1 66K.\n");
						aPortSEL |= (GPA4 | GPA5);   //Rcal 1
						break;
		}
		rcalSEL = aPortSEL;

		//Transfer stage
		sendMCSP(GPIOA_ADR, rcalSEL);
		sendMCSP(GPIOB_ADR, bPortSEL);

		return rcalval; 	//used in gainfactor
}
/*
* Measurement main cycle program/function
 */
void probeMeasureSelect(){

	static char aPortSEL;
	static char bPortSEL;

	aPortSEL=0x00;
	bPortSEL=0x00;

	// Prepare next cycle
	if (probeVoltCycle >= 5)
		{
			probeVoltCycle = 0;
			probeCurrentCycle++;
		}
	probeVoltCycle++;
	if (probeCurrentCycle > 8) return;

	xil_printf("VoltCycle:%d, CurrentCycle:%d\r", probeVoltCycle, probeCurrentCycle);

	if(probeCurrentCycle == 1)
	{
		switch (probeVoltCycle)
		{
			case 1: aPortSEL = GPA1 | GPA2;
					bPortSEL = GPB0 | GPB7;
					break;

			case 2: aPortSEL = GPA3;
					bPortSEL = GPB0 | GPB6 | GPB7;
					break;

			case 3: aPortSEL = GPA0 | GPA1 | GPA3;
					bPortSEL = GPB0;
					break;

			case 4: aPortSEL = GPA0 | GPA2 | GPA3;
					bPortSEL=  GPB0 | GPB6;
					break;

			case 5:	aPortSEL = GPA0 | GPA1 | GPA2 | GPA3;
					bPortSEL=  GPB0 | GPB7;
					break;

			default: aPortSEL = 0x00;
					 bPortSEL = 0x00;
					 break;
		}
	}
	else if(probeCurrentCycle == 2)
	{
		switch (probeVoltCycle)
		{
			case 1: aPortSEL = GPA3;
					bPortSEL = GPB1 | GPB3 | GPB6 | GPB7;
					break;

			case 2: aPortSEL = GPA0 | GPA1 | GPA3;
					bPortSEL = GPB1 | GPB3 ;
					break;

			case 3: aPortSEL = GPA0 | GPA2 | GPA3;
					bPortSEL = GPB1 | GPB3 | GPB6;
					break;

			case 4: aPortSEL = GPA0 | GPA1 | GPA2 | GPA3;
					bPortSEL=  GPB1 | GPB3 | GPB7;
					break;

			case 5:	aPortSEL = GPA0;
					bPortSEL=  GPB1 | GPB3 | GPB6 | GPB7;
					break;

			default: aPortSEL = 0x00;
			 	 	 bPortSEL = 0x00;
			 	 	 break;
		}
	}
	else if(probeCurrentCycle == 3)
	{

		switch (probeVoltCycle)
		{
			case 1: aPortSEL = GPA1;
					bPortSEL = GPB1 | GPB0 | GPB4;
					break;

			case 2: aPortSEL = GPA0 | GPA1 | GPA3;
					bPortSEL = GPB0 | GPB1 | GPB4;
					break;

			case 3: aPortSEL = GPA0 | GPA2 | GPA3;
					bPortSEL = GPB0 | GPB1 | GPB4 | GPB6;
					break;

			case 4: aPortSEL = GPA0 | GPA1 | GPA2 | GPA3;
					bPortSEL=  GPB0 | GPB1 | GPB4 | GPB7;
					break;

			case 5:	aPortSEL = GPA0;
					bPortSEL=  GPB0 | GPB1 | GPB4 | GPB6 | GPB7;
					break;

			default: aPortSEL = 0x00;
					 bPortSEL = 0x00;
					 break;
		}
	}
	else if(probeCurrentCycle == 4)
	{
		switch (probeVoltCycle)
		{
			case 1: aPortSEL = GPA1;
					bPortSEL = GPB2 | GPB3 | GPB4;
					break;

			case 2: aPortSEL = GPA2;
					bPortSEL = GPB2 | GPB3 | GPB4 | GPB6;
					break;

			case 3: aPortSEL = GPA0 | GPA2 | GPA3;
					bPortSEL = GPB2 | GPB3 | GPB4  | GPB6;
					break;

			case 4: aPortSEL = GPA0 | GPA1 | GPA2 | GPA3;
					bPortSEL=  GPB2 | GPB3 | GPB4 |  GPB7;
					break;

			case 5:	aPortSEL = GPA0;
					bPortSEL=  GPB2 | GPB3 | GPB4 | GPB6 | GPB7;
					break;

			default: aPortSEL = 0x00;
					 bPortSEL = 0x00;
					 break;
		}
	}
	else if(probeCurrentCycle == 5)
	{
		switch (probeVoltCycle)
		{
			case 1: aPortSEL = GPA1;
					bPortSEL = GPB0 | GPB2 | GPB5;
					break;

			case 2: aPortSEL = GPA2;
					bPortSEL = GPB0 | GPB2 | GPB5 | GPB6;
					break;

			case 3: aPortSEL = GPA1 | GPA2;
					bPortSEL = GPB0 | GPB2 | GPB5  | GPB7;
					break;

			case 4: aPortSEL = GPA0 | GPA1 | GPA2 | GPA3;
					bPortSEL=  GPB2 | GPB3 | GPB4 |  GPB7;
					break;

			case 5:	aPortSEL = GPA0;
					bPortSEL=  GPB0 | GPB2 | GPB5 | GPB6 | GPB7;
					break;

			default: aPortSEL = 0x00;
			 	 	 bPortSEL = 0x00;
			 	 	 break;
		}
	}
	else if(probeCurrentCycle == 6)
	{
		switch (probeVoltCycle)
		{
			case 1: aPortSEL = GPA1;
					bPortSEL = GPB1 | GPB2 | GPB3 | GPB5;
					break;

			case 2: aPortSEL = GPA2;
					bPortSEL = GPB1 | GPB2 | GPB3 | GPB5 | GPB6;
					break;

			case 3: aPortSEL = GPA1 | GPA2;
					bPortSEL = GPB1 | GPB2 | GPB3  | GPB5 | GPB7;
					break;

			case 4: aPortSEL = GPA3;
					bPortSEL=  GPB1 | GPB2 | GPB3  | GPB5 | GPB6 | GPB7;
					break;

			case 5:	aPortSEL = GPA0;
					bPortSEL=  GPB1 | GPB2 | GPB3 | GPB5 | GPB6 | GPB7;
					break;

			default: aPortSEL = 0x00;
					 bPortSEL = 0x00;
					 break;
		}
	}
	else if(probeCurrentCycle == 7)
	{
		switch (probeVoltCycle)
		{
			case 1: aPortSEL = GPA1;
					bPortSEL = GPB0 | GPB1 | GPB2 | GPB4 | GPB5;
					break;

			case 2: aPortSEL = GPA2;
					bPortSEL = GPB0 | GPB1 | GPB2 | GPB4 | GPB5 | GPB6;
					break;

			case 3: aPortSEL = GPA1 | GPA2;
					bPortSEL = GPB0 | GPB1 | GPB2  | GPB4 | GPB5 | GPB7;
					break;

			case 4: aPortSEL = GPA3;
					bPortSEL=  GPB0 | GPB1 | GPB2  | GPB4 | GPB5 | GPB6 | GPB7;
					break;

			case 5:	aPortSEL = GPA0 | GPA1 | GPA3;
					bPortSEL=  GPB0 | GPB1 | GPB2 | GPB4 | GPB5;
					break;

			default: aPortSEL = 0x00;
			 	 	 bPortSEL = 0x00;
			 	 	 break;
		}
	}
	else if(probeCurrentCycle == 8)
	{
		switch (probeVoltCycle)
		{
			case 1: aPortSEL = GPA2;
					bPortSEL = GPB0 | GPB1 | GPB2 | GPB6;
					break;

			case 2: aPortSEL = GPA1 | GPA2;
					bPortSEL = GPB0 | GPB1 | GPB2 | GPB7;
					break;

			case 3: aPortSEL = GPA3;
					bPortSEL = GPB0 | GPB1 | GPB2  | GPB6 | GPB7;
					break;

			case 4: aPortSEL = GPA0 | GPA1 | GPA3;
					bPortSEL=  GPB0 | GPB1 | GPB2;
					break;

			case 5:	aPortSEL = GPA0 | GPA2 | GPA3;
					bPortSEL=  GPB0 | GPB1 | GPB2 | GPB6;
					break;

			default: aPortSEL = 0x00;
					 bPortSEL = 0x00;
					 break;
		}
	}
	else
	{
		aPortSEL = 0x00;
		bPortSEL = 0x00;
	}


	//Transfer stage
	sendMCSP(GPIOA_ADR, aPortSEL | rcalSEL);
	sendMCSP(GPIOB_ADR, bPortSEL);

  }

/*
* Have to add Registers standard values, HAEN_ON, IODIR, BANK_ON
 */
int SPIStart(XSpi* spiPTR, u16 spiID)
{
	int Status;
		XSpi_Config *confPTR;

		confPTR = XSpi_LookupConfig(spiID);
		if(confPTR == NULL) {
			return XST_DEVICE_NOT_FOUND;
		}

		Status = XSpi_CfgInitialize(spiPTR, confPTR, confPTR->BaseAddress);
		if(Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		Status = XSpi_SetOptions(spiPTR, XSP_MASTER_OPTION |
				XSP_MANUAL_SSELECT_OPTION );
		if(Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		XSpi_SetSlaveSelect(spiPTR, 0x01);

		XSpi_Start(spiPTR);
		XSpi_IntrGlobalDisable(spiPTR);

		return XST_SUCCESS;
}

/* mscpRegister : Register adress to be written to
 * mcspRegValue : Value written to register
 */
void sendMCSP(char mcspRegister, char mcspRegValue)
{
	static char setConfigo[3];
	setConfigo[0] = MCP23S17_SPI_ADDR;
	setConfigo[1] = mcspRegister;
	setConfigo[2] = mcspRegValue;
	//XSpi_SetSlaveSelect(&SpiInstance, 0x01);
	XSpi_Transfer(&SpiInstance,setConfigo, readBuffer, 3);
	//XSpi_SetSlaveSelect(&SpiInstance, 0x00);
}

/*
* initialze MCP23S17 for measurement
 */
void initMCSP()
{
	//Init MCPS GPIO EXAPNDER WITH CORRECT CONFIG
	sendMCSP(IOCON, (BANK_OFF | INT_MIRROR_OFF | SEQOP_OFF | DISSLW_OFF | HAEN_OFF |  ODR_OFF |  INTPOL_LOW) );

    //Set IO Directions
    sendMCSP(IODIRA, 0x00);
    sendMCSP(IODIRB, 0x00);

    //Set Pullups
    sendMCSP(GPPUA, 0x00);
    sendMCSP(GPPUB, 0x00);

    //Set Pullups
    sendMCSP(IPOLA, 0x00);
    sendMCSP(IPOLB, 0x00);

	//Select ports
    sendMCSP(GPIOA_ADR, 0x00);
    sendMCSP(GPIOB_ADR, 0x00);
}
