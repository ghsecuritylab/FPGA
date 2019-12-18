#include <stdio.h>
#include <math.h>
#include "AD5933.h"
#include "xiic_l.h"
#include "MCP23S17.h"

//0x005CFF

volatile int   rxData;
int            temperature     = 0;
unsigned long  impedanceKohms  = 0;
unsigned long  impedanceOhms   = 0;
float          impedance       = 0.0f;
float          gainFactor[stepCount];

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
void AD5933_ConfigFixedSweep(int startFreq,
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

	// Start frequency sweep
	AD5933_SetRegisterValue(AD5933_CONTROL_REG_HB,
							0x21,  //AD5933_CONTROL_FUNCTION(AD5933_START_FREQ_SWEEP)
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
	int          status, frequen = 0;

	// Start frequency sweep
	AD5933_SetRegisterValue(AD5933_CONTROL_REG_HB,
							0x21,  //AD5933_CONTROL_FUNCTION(AD5933_START_FREQ_SWEEP)
							1);


	// Wait for data to be valid
	while((status & AD5933_STATUS_DATA_VALID) == 0)
	{

		/*
		if ((AD5933_GetRegisterValue(AD5933_STATUS_REG, 1) & 0x02))
		{
			realData = AD5933_GetRegisterValue(AD5933_REAL_REG_HB,2);
			imgData  = AD5933_GetRegisterValue(AD5933_IMG_REG_HB,2);
		}
		*/
		status = AD5933_GetRegisterValue(AD5933_STATUS_REG,1);
	}

	// Read real and imaginary data
		realData = AD5933_GetRegisterValue(AD5933_REAL_REG_HB,2);
		imgData  = AD5933_GetRegisterValue(AD5933_IMG_REG_HB,2);

		// Calculate magnitude
		magnitude = sqrtf((realData * realData) + (imgData * imgData));

		// Calculate impedance
		impedance = 1 / (magnitude * gainFactor / 1000000000);
		impedanceOhms = (unsigned long)impedance;

		if (mode == 2)
		{
		/* THIS WILL BE FOR THE END SWEEP VISUALIZATION
		// Keep record Freq and Impedance
		measuredData[probeCurrentCycle-1][probeVoltCycle-1][steps].frequency = currentFreq;
		measuredData[probeCurrentCycle-1][probeVoltCycle-1][steps].impedance = impedanceOhms;

		//Keep track of Magnitude, Real, Imaginary data
		measuredData[probeCurrentCycle-1][probeVoltCycle-1][steps].magnitude = magnitude;
		measuredData[probeCurrentCycle-1][probeVoltCycle-1][steps].imaginary = imgData;
		measuredData[probeCurrentCycle-1][probeVoltCycle-1][steps].real = realData;
		*/
		RcalMeasuredData[steps].frequency = currentFreq;
		RcalMeasuredData[steps].impedance = impedanceOhms;

			//Keep track of Magnitude, Real, Imaginary data
		RcalMeasuredData[steps].magnitude = magnitude;
		RcalMeasuredData[steps].imaginary = imgData;
		RcalMeasuredData[steps].real = realData;
		}

		else if (mode == 1)
		{
			measuredData[probeCurrentCycle-1][probeVoltCycle-1].frequency = currentFreq;
			measuredData[probeCurrentCycle-1][probeVoltCycle-1].impedance = impedanceOhms;

			//Keep track of Magnitude, Real, Imaginary data
			measuredData[probeCurrentCycle-1][probeVoltCycle-1].magnitude = magnitude;
			measuredData[probeCurrentCycle-1][probeVoltCycle-1].imaginary = imgData;
			measuredData[probeCurrentCycle-1][probeVoltCycle-1].real = realData;

		}
		//printf("\n\r <%d,%d,%0.2f,%0.2f>  \n\r",realData, imgData, magnitude,impedance);
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

	static int stap = 0;
	static int wholeFactor1, wholeFactor2 = 0;
	// Read temperature from device
	temperature = AD5933_GetTemperature();
	printf("Temperature: %d Celcius\n",temperature);

	// Set sweep parameters
	//startFrequentie 		0x0F5C28
	//#define aantalSteps 			0x01F4
	//#define frequentiePerStep 	0x000D16
	/*AD5933_ConfigSweep(startFrequentie,
						stepCount,
						frequentiePerStep);
						*/

	// Start the sweep
	//AD5933_StartSweep();

	AD5933_ConfigSweepCycle();
	stap = 0;
	wholeFactor1 = 0;
	wholeFactor2 = 0;
	while (stap <= stepCount)
	{
		// Calculate gain factor for an chosen RCAL on all frequencies of sweep.
		gainFactor[stap] = AD5933_CalculateGainFactor(rcalval,
												AD5933_REPEAT_FREQ);
		stap++;
		//increase freq
		AD5933_SetRegisterValue(AD5933_CONTROL_REG_HB,
								0x31,
								1 );
	}

	wholeFactor1 = gainFactor[0];
    wholeFactor2 = gainFactor[stap];
	// Change the resistor used for calibration with the one you wish to measure
	print("Calibration complete.\n");//First GainFactor: %0.2f AND Last GainFactor: %0.2f\n\r",gainFactor[0], gainFactor[stepCount-1]);
	//xil_printf("First GainFactor: %d AND Last GainFactor: %d\r",wholeFactor1, wholeFactor2);

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
	printf("Measuring..Temperature chip: %d Celcius.\n",temperature);

	if (mode == 2)
	{
		AD5933_ConfigSweepCycle();

		steps = 0;
		currentFreq = 10000;
		//Start at 10K Frequency and increase as much steps we defined
		while(steps < stepCount)
		{

		//Gives correcte values with the first gainfcator of 10KHz
		// Calculate impedance between Vout and Vin
		impedance = AD5933_CalculateImpedance(gainFactor[0], AD5933_REPEAT_FREQ);

		//increase freq for next sweep.
		AD5933_SetRegisterValue(AD5933_CONTROL_REG_HB,
								0x31,
								1 );
		steps++;
		currentFreq +=500; //Add formula here for calculating freq per step.
		}
	}

	else if (mode == 1)
	{
		currentFreq = 10000;
		AD5933_ConfigFixedSweep(startFrequentie, 0x00, 0x00);
		// Calculate impedance on fixed frequency for prototype
		impedance = AD5933_CalculateImpedance(gainFactor[0], AD5933_REPEAT_FREQ);

		/* Print impedance in kohm
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
		 */
	}
	print("Measure Impedance Completed.\n");
}


void writeSerialImpedanceArray()
{
	static int i, wholeMagn, thousandthsMagn = 0;
	if (mode == 1)
	{
		for(int j=0;j<=8;j++)
		{
			for(int y=0;y<=5;y++)
			{
				xil_printf("Cycle:%d, Probe:%d, Freq:%d, Impedance:%d, Magnitude:%f, Imaginary:%hu Real:%hu;\r",j, y, measuredData[j][y].frequency, measuredData[j][y].impedance, measuredData[j][y].magnitude, measuredData[j][y].real, measuredData[j][y].imaginary);
				/* for 3D end array
				for(int i=0;i<=stepCount;i++)
				{
					printf("Cycle:%d, Probe:%d, Freq:%d, Impedance:%d, Magnitude:%f, Imaginary:%hu Real:%hu ;\r",j, y, measuredData[j][y][i].frequency, measuredData[j][y][i].impedance, measuredData[j][y][i].magnitude, measuredData[j][y][i].real, measuredData[j][y][i].imaginary);
				}
				*/
			}
		}
		print("\n");
	}

	else if (mode == 2)
	{
		for(i=0;i<stepCount;i++)
		{
			//Need to find new print variable with float. %".
		    wholeMagn = RcalMeasuredData[i].magnitude;
		    thousandthsMagn = (RcalMeasuredData[i].magnitude - wholeMagn) * 1000;
			xil_printf("Freq:%d, Impedance:%d, Magnitude:%d.%3d, Imaginary:%hu Real:%hu;\r",RcalMeasuredData[i].frequency, RcalMeasuredData[i].impedance, wholeMagn, thousandthsMagn, RcalMeasuredData[i].real, RcalMeasuredData[i].imaginary);
		}
		print("\n");
		//usleep(900);
	}
}
