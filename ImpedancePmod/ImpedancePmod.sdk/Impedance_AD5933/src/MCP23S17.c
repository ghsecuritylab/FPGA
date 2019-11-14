#include <stdio.h>
#include <math.h>
#include "MCP23S17.h"

char aPortSEL, bPortSEL = 0x00;
int rcalval = 0;
uint8_t RCAL, RFB =  1;

/*
* Rcal & RFB selects
 */
int RCal_RFB_Select(int RCAL, int RFB){
	rcalval = 0;
	aPortSEL=0x00;
	bPortSEL=0x00;

	switch(RCAL)
		{
			case 1:     print("Calibrating AD5933 using RCAL1..");
						aPortSEL = (GPA6 | GPA7);   //Rcal 1
						rcalval = 5000;
						break;
			case 2:     print("Calibrating AD5933 using RCAL2..");
						aPortSEL = (GPA7);   //Rcal 1
						rcalval = 10000;
						break;
			case 3:     print("Calibrating AD5933 using RCAL3..");
						aPortSEL = (GPA6);   //Rcal 1
						rcalval = 40000;
						break;
		}

		switch(RFB)
		{
			case 1:     print("&RFB1..Zoveel ohm..\n\r");
						break;
			case 2:     print("&RFB2..Zoveel ohm..\n\r");
						aPortSEL |= (GPA5);   //RFB 2
						break;
			case 3:     print("&RFB3..\n\r");
						aPortSEL |= (GPA4);   //Rcal 1
						break;
			case 4:     print("&RFB4..\n\r");
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

			default: break;
		}
	}
	if(probeCurrentCycle == 2)
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

			default: break;
		}
	}


	//Transfer stage
	portTransfer();

	// Prepare next cycle
	if (probeVoltCycle > 5)
		{
			probeVoltCycle = 1;
			if (probeCurrentCycle >= 8)
			{
				probeCurrentCycle = 0;
			}
			else probeCurrentCycle++;
		}
	else probeVoltCycle++;
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

	XSpi_Start(spiPTR);
	XSpi_IntrGlobalDisable(spiPTR);

	return XST_SUCCESS;
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
	arrayGPA[2] = aPortSEL;

	arrayGPB[1] = GPIOB_ADR;
	arrayGPB[2] = bPortSEL;

	//Transfer SPI buffers COMMENTED FOR DEBUGGING
	//XSpi_Transfer(&SpiInstance, arrayGPB, readBuffer, BUFFER_SIZE);
	//XSpi_Transfer(&SpiInstance, arrayGPA, readBuffer, BUFFER_SIZE);
}
