#include <stdio.h>
#include <math.h>
#include "AD5933.h"
#include "xiic_l.h"

//0x005CFF
#define startFrequentie 		0x051EB8	//0x051EB8 = 10k, 0x0F5C28 = 30K //Adres, High bits, Lowbits
#define stepCount 				0x00B4		//0x00B4 = 180 steps
#define frequentiePerStep  		0x004189	//0x008312 = 1KHz, 0x004189 = 500Hz //0x000500 -> 200Hz per stap
#define recordFreqSteps			500			// aantal freq steps per meting
#define sleepTime				5			// in microSec.

volatile int   rxData;
int            temperature     = 0;
unsigned long  impedanceKohms  = 0;
unsigned long  impedanceOhms   = 0;
float          impedance       = 0.0f;
float          gainFactor      = 0.0f;

void AD5933_ConfigSweepCycle()
{


	// Configure starting frequency
	AD5933_SetRegisterValue(AD5933_START_FREQ_REG_HB,
							startFrequentie,
							3);

	// Configure number of steps
	AD5933_SetRegisterValue(AD5933_NR_INCR_REG_HB,
							stepCount,
							2);

	// Configure frequency increment step
	AD5933_SetRegisterValue(AD5933_FREQ_INCR_REG_HB,
							frequentiePerStep,
							3);


	// Place AD5933 in standby
	AD5933_SetRegisterValue(AD5933_CONTROL_REG_HB,
							AD5933_CONTROL_FUNCTION(AD5933_STANDBY),
							1);

	// Select internal system clock
	AD5933_SetRegisterValue(AD5933_CONTROL_REG_LB,
							0x00,
							1);

	// Initialize starting frequency
	AD5933_SetRegisterValue(AD5933_CONTROL_REG_HB,
							AD5933_CONTROL_FUNCTION(AD5933_INIT_START_FREQ),
							1);

}


/******************************************************************************
* @brief Set an AD5933 internal register value.
*
* @param registerAddress - Address of AD5933 register.
*
* @param registerValue - Value of data to be written in the register.
*
* @param numberOfBytes - Number of bytes to be written in the register
*
* @return None.
******************************************************************************/

void AD5933_SetRegisterValue(int registerAddress,
							 int registerValue,
							 char numberOfBytes)
{
	char 			byte         = 0;
	unsigned char 	writeData[4] = {0, 0, 0, 0};
	// Write numberOfBytes from starting address to starting address + numberOfBytes
	for(byte = 0; byte < numberOfBytes; byte++)
	{
		writeData[0] = registerAddress + numberOfBytes - byte - 1;
		writeData[1] = ((registerValue >> (8*byte)) & 0xFF);
		XIic_Send(XPAR_AXI_IIC_0_BASEADDR, AD5933_I2C_ADDR,
				writeData, 2, XIIC_STOP);
	}
}

/******************************************************************************
* @brief Read an AD5933 internal register value.
*
* @param registerAddress - Address of AD5933 register.
*
* @param numberOfBytes - Number of bytes to be read from the register.
*
* @return Register value.
******************************************************************************/
int AD5933_GetRegisterValue(int registerAddress,
							char numberOfBytes)
{
	int  registerValue 	= 0;
	char byte 			= 0;
	unsigned char  writeData[4] 	= {0, 0, 0, 0};
	unsigned char  readData[4] 	    = {0, 0, 0, 0};

	for(byte = 0; byte < numberOfBytes; byte++)
	{
		// Set pointer to desired register
		//writeData[0] = AD5933_ADDR_POINTER;
		//writeData[1] = registerAddress + byte;
		writeData[0] = registerAddress + byte;
		XIic_Send(XPAR_AXI_IIC_0_BASEADDR, AD5933_I2C_ADDR,
				writeData, 2, XIIC_STOP);

		// Read register data
		XIic_Recv(XPAR_AXI_IIC_0_BASEADDR, AD5933_I2C_ADDR,
				readData, 1, XIIC_STOP);
		registerValue |= readData[0] << (8 * (numberOfBytes - byte - 1));

		//xil_printf("Reg val: %d", readData[0]);
		//xil_printf("Reg val: %d", registerValue);
	}
	return(registerValue);
}

/******************************************************************************
* @brief Read the AD5933 temperature.
*
* @param None.
*
* @return Temperature value.
******************************************************************************/
int AD5933_GetTemperature(void)
{
	int temperature = 0;
	int status 		= 0;

	// Enable temperature measurement
	AD5933_SetRegisterValue(AD5933_CONTROL_REG_HB,
							AD5933_CONTROL_FUNCTION(AD5933_MEAS_TEMP),
							1);
	// Wait for read temperature to be valid
	while((status & 1) == 0)
	{
		status = AD5933_GetRegisterValue(AD5933_STATUS_REG, 1);
	}

	// Read correct temperature value
	temperature = AD5933_GetRegisterValue(AD5933_TEMP_REG_HB, 2);

	// Calculate temperature according to datasheet specifications
	if(temperature < 8192)
	{
		temperature /= 32;
	}
	else
	{
		temperature -= 16384;
		temperature /= 32;
	}

	return(temperature);
}

/******************************************************************************
* @brief Configure the AD5933 frequency sweep parameters.
*
* @param startFreq - Starting frequency value.
*
* @param incSteps - Number of increment steps.
*
* @param incFreq - Frequency step value.
*
* @return None.
******************************************************************************/
void AD5933_ConfigSweep(int startFreq,
						int incSteps,
						int incFreq)
{

	// Configure starting frequency
	AD5933_SetRegisterValue(AD5933_START_FREQ_REG_HB,
							startFreq,
							3);
	// Configure number of steps
	AD5933_SetRegisterValue(AD5933_NR_INCR_REG_HB,
							incSteps,
							2);
	// Configure frequency increment step
	AD5933_SetRegisterValue(AD5933_FREQ_INCR_REG_HB,
							incFreq,
							3);
}

/******************************************************************************
* @brief Start AD5933 frequency sweep.
*
* @param None.
*
* @return None.
******************************************************************************/
void AD5933_StartSweep(void)
{
	int realData, imgData = 0;
	int status = 0;

	// Place AD5933 in standby
	AD5933_SetRegisterValue(AD5933_CONTROL_REG_HB,
							AD5933_CONTROL_FUNCTION(AD5933_STANDBY),
							1);

	// Select internal system clock
	AD5933_SetRegisterValue(AD5933_CONTROL_REG_LB,
							0x00,
							1);

	// Initialize starting frequency
	AD5933_SetRegisterValue(AD5933_CONTROL_REG_HB,
							AD5933_CONTROL_FUNCTION(AD5933_INIT_START_FREQ),
							1);

	// Start frequency sweep
	AD5933_SetRegisterValue(AD5933_CONTROL_REG_HB,
							AD5933_CONTROL_FUNCTION(AD5933_START_FREQ_SWEEP),
							1);


	// Wait for data to be valid
	while((status & AD5933_STATUS_DATA_VALID) == 0)
	{
		status = AD5933_GetRegisterValue(AD5933_STATUS_REG,1);
	}
}

/******************************************************************************
* @brief Calculate gain factor
*
* @param calibrationImpedance - Known value of connected impedance for calibration.
*
* @param freqFunction - Select Repeat Frequency Sweep.
*
* @return gainFactor.
******************************************************************************/
double AD5933_CalculateGainFactor(unsigned long calibrationImpedance,
							   char freqFunction)
{
	double       gainFactor = 0;
	double       magnitude  = 0;
	int          status     = 0;
	signed short realData   = 0;
	signed short imgData    = 0;

	// Repeat frequency sweep with last set parameters
	AD5933_SetRegisterValue(AD5933_CONTROL_REG_HB,
							AD5933_CONTROL_FUNCTION(freqFunction),
							1);

	// Wait for data received to be valid
	while((status & AD5933_STATUS_DATA_VALID) == 0)
	{
		status = AD5933_GetRegisterValue(AD5933_STATUS_REG, 1);
	}

	// Read real and imaginary data
	realData = AD5933_GetRegisterValue(AD5933_REAL_REG_HB,2);
	imgData  = AD5933_GetRegisterValue(AD5933_IMG_REG_HB,2);

	// Calculate magnitude
	magnitude = sqrtf((realData * realData) + (imgData * imgData));

	// Calculate gain factor
	gainFactor = 1000000000 / (magnitude * calibrationImpedance);
	//xil_printf("\n\r <GainFactor : %0.f	, %d	, %d> \n\r",gainFactor, magnitude, calibrationImpedance);

	return(gainFactor);
}

/******************************************************************************
* @brief Calculate impedance.
*
* @param gainFactor - Gain factor calculated using a known impedance.
*
* @param freqFunction - Select Repeat Frequency Sweep.
*
* @return impedance.
******************************************************************************/
double AD5933_CalculateImpedance(double gainFactor,
								 char freqFunction)
{
	signed short realData   = 0;
	signed short imgData    = 0;
	double       magnitude  = 0;
	double       impedance  = 0;
	int          status, frequen, stap     = 0;

	/*
	//Default settings for resistors (1MHZ Freq.)
	startFreq = 0x0F5C28;
	incSteps = 0x01F4;
	incFreq =  0x000D16;
		AD5933_ConfigSweep(0x0F5C28,
					   0x01F4,
					   0x000D16);
	AD5933_ConfigSweep(startFrequentie,
					   aantalSteps,
					   frequentiePerStep);
*/
	// Start the sweep
	//AD5933_StartSweep();
/*
	while (1)
	{
		AD5933_StartSweep();
		// Repeat frequency sweep with last set parameters
		if(stap == 0)
		{
			AD5933_SetRegisterValue(AD5933_CONTROL_REG_HB,
									AD5933_CONTROL_FUNCTION(AD5933_REPEAT_FREQ),
									1);
		}
		if(stap < recordFreqSteps)
		{
		AD5933_SetRegisterValue(AD5933_CONTROL_REG_HB,
								AD5933_CONTROL_FUNCTION(AD5933_INCR_FREQ),
								1);
		}
		if(stap > recordFreqSteps)
		{
			break;
		}
		stap ++;
		// Wait for data received to be valid
		while((status & AD5933_STATUS_DATA_VALID) == 0)
		{
			status = AD5933_GetRegisterValue(AD5933_STATUS_REG,1);
		}
		// Read real and imaginary data
		realData = AD5933_GetRegisterValue(AD5933_REAL_REG_HB,2);
		imgData  = AD5933_GetRegisterValue(AD5933_IMG_REG_HB,2);
		// Calculate magnitude
		magnitude = sqrtf((realData * realData) + (imgData * imgData));
		// Calculate impedance
		impedance = 1 / (magnitude * gainFactor / 1000000000);
		printf("\n\r <%d,%d,%0.2f,%0.2f>  \n\r",realData, imgData, magnitude,impedance);
		//usleep_A9(sleepTime);
	}
	*/

	//AD5933_StartSweep();


	// Start frequency sweep
	AD5933_SetRegisterValue(AD5933_CONTROL_REG_HB,
							0x21,  //AD5933_CONTROL_FUNCTION(AD5933_START_FREQ_SWEEP)
							1);


	// Wait for data to be valid
	while((status & AD5933_STATUS_DATA_VALID) == 0)
	{
		if ((AD5933_GetRegisterValue(AD5933_STATUS_REG, 1) & 0x02))
		{
			realData = AD5933_GetRegisterValue(AD5933_REAL_REG_HB,2);
			imgData  = AD5933_GetRegisterValue(AD5933_IMG_REG_HB,2);
		}
		status = AD5933_GetRegisterValue(AD5933_STATUS_REG,1);
	}

	// Read real and imaginary data
		realData = AD5933_GetRegisterValue(AD5933_REAL_REG_HB,2);
		imgData  = AD5933_GetRegisterValue(AD5933_IMG_REG_HB,2);

		// Calculate magnitude
		magnitude = sqrtf((realData * realData) + (imgData * imgData));

		// Calculate impedance
		impedance = 1 / (magnitude * gainFactor / 1000000000);
		printf("\n\r <%d,%d,%0.2f,%0.2f>  \n\r",realData, imgData, magnitude,impedance);
	return(impedance);

}

/******************************************************************************
* @calibration program for AD5933
*
* @param none.
*
* @return none.
******************************************************************************/
void calibration(int rcalval)
{

	// Read temperature from device
	temperature = AD5933_GetTemperature();
	xil_printf("\n\rCalibrating with a temperature of %d C�.\n\r",temperature);

	// Set sweep parameters
	//startFrequentie 		0x0F5C28
	//#define aantalSteps 			0x01F4
	//#define frequentiePerStep 	0x000D16
	AD5933_ConfigSweep(startFrequentie,
						stepCount,
						frequentiePerStep);

	// Start the sweep
	AD5933_StartSweep();

	// Calculate gain factor for an impedance of 47kohms, change value here for chosen resistor
	gainFactor = AD5933_CalculateGainFactor(rcalval,
											AD5933_REPEAT_FREQ);

	// Change the resistor used for calibration with the one you wish to measure
	xil_printf("Calibration complete.\n\r");
	printf("\n\r GainFactor : %0.2f	,  \n\r",gainFactor);

}

/******************************************************************************
* @brief Demo program for AD5933
*
* @param none.
*
* @return none.
******************************************************************************/
void measureImpedance(void)
{
	// Read temperature from device
	temperature = AD5933_GetTemperature();
	xil_printf("\n\rTemperature on board: %d C\n\r",temperature);

	AD5933_ConfigSweepCycle();

	int steps = 0;
	while(steps < 150)
	{
	// Calculate impedance between Vout and Vin
	impedance = AD5933_CalculateImpedance(gainFactor,
										  AD5933_REPEAT_FREQ);
	impedanceOhms = (unsigned long)impedance;

	// Print impedance
	xil_printf("Impedance read: %d ohms\n\r", impedanceOhms);

	// Calculate impedance in kohm
	impedanceKohms = impedanceOhms / 1000;
	impedanceOhms %= 1000;

	// Print impedance in kohm
	if (impedanceOhms < 10)
	{
		xil_printf("Impedance read: %d.00%d Kohms\n\r", impedanceKohms, impedanceOhms);
	}
	else if (impedanceOhms < 100)
	{
		xil_printf("Impedance read: %d.0%d Kohms\n\r", impedanceKohms, impedanceOhms);
	}
	else
	{
		xil_printf("Impedance read: %d.%d Kohms\n\r", impedanceKohms, impedanceOhms);
	}

	xil_printf("Measurement complete.\n\r\n\r");
	AD5933_SetRegisterValue(AD5933_CONTROL_REG_HB,
							0x31,
							1 );
	steps++;
	}
}
