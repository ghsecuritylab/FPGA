#include <stdio.h>
#include <math.h>
#include "MCP23S17.h"
#include "AD5933.h"

char aPortSEL, bPortSEL = 0x00;
int rcalval = 0;
uint8_t RCAL, RFB;

/*
* Rcal & RFB selects
 */
int RCal_RFB_Select(int RCAL, int RFB){
	rcalval = 0;
	aPortSEL=0x00;
	bPortSEL=0x00;
    //portTransfer();

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
			case 1:     print(" & RFB1 20K.\n");
						break;
			case 2:     print(" & RFB1 10K.\n");
						aPortSEL |= (GPA5);   //RFB 2
						break;
			case 3:     print(" & RFB1 47K.\n");
						aPortSEL |= (GPA4);   //Rcal 1
						break;
			case 4:     print(" & RFB1 66K.\n");
						aPortSEL |= (GPA4 | GPA5);   //Rcal 1
						break;
		}
		portTransfer();
		return rcalval;
}
/*
* Measurement main cycle program/function
 */
void probeMeasureSelect(){

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
	portTransfer();

  }


/*
* sorts input array on GPIO adresses.
 */
void portSelection(char RegisterADR, char Port){

	//arrayGPB[3], arrayGPA[3] = {0x00, 0x00, 0x00};
	//int size = (int)(sizeof(arraySelection) / sizeof(arraySelection[0]));

	if (RegisterADR == GPIOA_ADR) // GPIOA Register adress
	{
		arrayGPA[1] = GPIOA_ADR;
		arrayGPA[2] = Port;
	}
	else if(RegisterADR == GPIOB_ADR) // GPIOB Register adress
	{
		arrayGPB[1] = GPIOB_ADR;
		arrayGPB[2] = Port;
	}

	arrayGPB[0] = MCP23S17_SPI_ADDR;
	arrayGPA[0] = MCP23S17_SPI_ADDR;

	return;
}

/*
* Prepare buffers & SPI Transfer.
 */
void portTransfer(){
	//Select ports
	arrayGPB[0] = MCP23S17_SPI_ADDR;
	arrayGPA[0] = MCP23S17_SPI_ADDR;

	arrayGPA[1] = GPIOA_ADR;
	arrayGPB[1] = GPIOB_ADR;

	arrayGPA[2] = 0x00;
	arrayGPB[2] = 0x00;

	XSpi_SetSlaveSelect(&SpiInstance, 0x01);
	XSpi_Transfer(&SpiInstance, arrayGPB, readBuffer, BUFFER_SIZE);
	XSpi_SetSlaveSelect(&SpiInstance, 0x00);

	XSpi_SetSlaveSelect(&SpiInstance, 0x01);
	XSpi_Transfer(&SpiInstance, arrayGPA, readBuffer, BUFFER_SIZE);
	XSpi_SetSlaveSelect(&SpiInstance, 0x00);

	arrayGPA[2] = aPortSEL;
	arrayGPB[2] = bPortSEL;

	XSpi_SetSlaveSelect(&SpiInstance, 0x01);
	XSpi_Transfer(&SpiInstance, arrayGPB, readBuffer, BUFFER_SIZE);
	XSpi_SetSlaveSelect(&SpiInstance, 0x00);

	XSpi_SetSlaveSelect(&SpiInstance, 0x01);
	XSpi_Transfer(&SpiInstance, arrayGPA, readBuffer, BUFFER_SIZE);
	XSpi_SetSlaveSelect(&SpiInstance, 0x00);
}


/*
* Have to add Registers standard values, HAEN_ON, IODIR, BANK_ON
 */
int SPIStart(XSpi* spiPTR, u16 spiID){
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

/*
* initialze MCP23S17 for measurement
 */
void initMCSP(XSpi* spiPTR)
{
    //Init MCPS GPIO EXAPNDER WITH CORRECT CONFIG
    setConfig[0] = MCP23S17_SPI_ADDR;
    setConfig[1] = IOCON;
    setConfig[2] = BANK_OFF | INT_MIRROR_OFF | SEQOP_OFF | DISSLW_OFF | HAEN_OFF |  ODR_OFF |  INTPOL_LOW;
    XSpi_Transfer(spiPTR, setConfig, readBuffer, BUFFER_SIZE);

    //Set IO Directions
    setDirections[0] = MCP23S17_SPI_ADDR;
    setDirections[1] = IODIRA;
    setDirections[2] = 0x00;
    XSpi_Transfer(spiPTR, setDirections, readBuffer, BUFFER_SIZE);
    setDirections[1] = IODIRB;
    XSpi_Transfer(spiPTR, setDirections, readBuffer, BUFFER_SIZE);

    //Set Pullups
    setPullupsB[0] = MCP23S17_SPI_ADDR;
    setPullupsB[1] = GPPUA;
    setPullupsB[2] = 0x00;
    XSpi_Transfer(spiPTR, setPullupsB, readBuffer, BUFFER_SIZE);
    setPullupsB[1] = GPPUB;
    XSpi_Transfer(spiPTR, setPullupsB, readBuffer, BUFFER_SIZE);

    //Set Pullups
    setPullupsB[0] = MCP23S17_SPI_ADDR;
    setPullupsB[1] = IPOLA;
    setPullupsB[2] = 0x00;
    XSpi_Transfer(spiPTR, setPullupsB, readBuffer, BUFFER_SIZE);
    setPullupsB[1] = IPOLB;
    XSpi_Transfer(spiPTR, setPullupsB, readBuffer, BUFFER_SIZE);

	//Select ports
	arrayGPB[0] = MCP23S17_SPI_ADDR;
	arrayGPA[0] = MCP23S17_SPI_ADDR;

	arrayGPA[1] = GPIOA_ADR;
	arrayGPB[1] = GPIOB_ADR;

	arrayGPA[2] = 0x00;
	arrayGPB[2] = 0x00;

	XSpi_Transfer(spiPTR, arrayGPB, readBuffer, BUFFER_SIZE);
	XSpi_Transfer(spiPTR, arrayGPA, readBuffer, BUFFER_SIZE);
}
