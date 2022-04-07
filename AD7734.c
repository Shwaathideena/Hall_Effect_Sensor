/*
 * AD7734.c
 *
 *  Created on: Feb 17, 2022
 *      Author: shwaa
 */

/* **** Includes **** */
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include "driverlib/ssi.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "AD7734.h"
#include "driverlib/uart.h"


/* **** Global Variable Initializations **** */
uint8_t adcDataLength = 1;      // 8 bit data is located at ADC Status Register

uint8_t channelDataLength = 2;   // Voltage Data is considered at 16-bit/24-bit

uint8_t channelStatusLength = 1;    // 8 bit Channel Status Register

uint16_t datah = 0;             // hex data value with bit shift to provide 16-bit data

float voltage = 0.0;            // Voltage at the specified channel

uint32_t conv_time_single = 808;           //(16Mhz / 3) * (151.5us)

uint32_t conv_time_continuous =  2105;           //(16Mhz / 3) * (394.69us)

//*****************************************************************************
//
// This function writes to the communications Register
// that a Write Cycle is being performed next at the specified Register
//
//*****************************************************************************
void SSI_MasterWriteCommReg(uint8_t data)
{
    SSIDataPut(SSI0_BASE, data | CONFIG_WRITE);    // Writes the next w cycle to the communications register
    while(SSIBusy(SSI0_BASE)){}     // Wait until SSI0 is done transferring all the data in the transmit FIFO.

    SysCtlDelay(1000);

    return;
}

//*****************************************************************************
//
// This function writes to the communications Register
// that a Read Cycle is being performed next at the specified Register
//
//*****************************************************************************
void SSI_MasterReadCommReg(uint8_t data)
{
    while(SSIDataGetNonBlocking(SSI0_BASE, &dataRx[0])){}   // Check receive FIFO is empty

    SSIDataPut(SSI0_BASE, data | CONFIG_READ);    // Writes the next w cycle to the communications register
    while(SSIBusy(SSI0_BASE)){}     // Wait until SSI0 is done transferring all the data in the transmit FIFO.

    return;
}

//*****************************************************************************
//
// This function reads the data from the previously specified
// Read Register
//
//*****************************************************************************
void SSI_MasterReadData(uint8_t length)
{
    uint8_t index = 0;

    for(index=0; index < length; index++){

        SSIDataGet(SSI0_BASE, &dataRx[index]);      // Receives the data from ADC chip

        dataRx[index] &= 0x00FF;                    // Using 8-bit data, masking off the MSB
    }
    return;
}

//*****************************************************************************
//
// This function sets up voltage range of 0V to 5V as Analog inputs on the AD7734 chip
//
//*****************************************************************************
void setup_AD7734_voltage_range(void)
{
    SSI_MasterWriteCommReg(CHANNEL_SETUP_ADDR(ADC_CHANNEL0));   // Write to comm register that channel0 is being set up

    SSI_MasterWriteCommReg(V_to_P5V);   // Writes the Range of channel0 to 0V to 5V

    SSI_MasterWriteCommReg(CHANNEL_SETUP_ADDR(ADC_CHANNEL1));   // Write to comm register that channel1 is being set up

    SSI_MasterWriteCommReg(V_to_P5V);   // Writes the Range of channel1 to 0V to 5V

    SSI_MasterWriteCommReg(CHANNEL_SETUP_ADDR(ADC_CHANNEL2));   // Write to comm register that channel2 is being set up

    SSI_MasterWriteCommReg(V_to_P5V);   // Writes the Range of channel2 to 0V to 5V

    SSI_MasterWriteCommReg(CHANNEL_SETUP_ADDR(ADC_CHANNEL3));   // Write to comm register that channel3 is being set up

    SSI_MasterWriteCommReg(V_to_P5V);   // Writes the Range of channel3 to 0V to 5V

    return;
}

//*****************************************************************************
//
// Reads the ADC Status Register and checks if it transmits 0
// Returns true if it reads Non-Zero
// Returns false if it is Zero
//
//*****************************************************************************
bool read_adc_status_register(void)
{

    SSI_MasterReadCommReg(ADC_STATUS_R_ADDR);   // Set communication Register to Read from ADC status Register in the next cycle

    SSI_MasterReadData(adcDataLength);          // Read the 8 bit data transmitted from the ADC chip

    if (dataRx[adcDataLength - 1] != 0)         // Checks if the ADC Status register reads 0
        return true;
    else
        return false;
}

//*****************************************************************************
//
// Checks the ADC Status register and sets to default 0x00h before starting a
// conversion mode
//
//*****************************************************************************
void check_adc_status_register(void)
{
    if (read_adc_status_register()){

        SSI_MasterWriteCommReg(ADC_STATUS_R_ADDR);      // Specifies a Write Cycle is being performed at the ADC Status Register

        SSI_MasterWriteCommReg(ADC_STATUS_R_DEFAULT);   // Writes 0x00h to the ADC Status Register
    }
    return;
}

//*****************************************************************************
//
// Set up Channel Conversion time in Channel Conversion Register
// Inputs the PySerial value to set the sampling rate
//
//*****************************************************************************
void set_channel_conversion_time_register(uint8_t Value_FW)
{
    SSI_MasterWriteCommReg(CHANNEL_CONV_TIME_ADDR(ADC_CHANNEL0));   // Specifies a Write Cycle is being performed at the Channel Conv time Register of Specific Channel

    SSI_MasterWriteCommReg(CHOP_ENABLE | Value_FW);                 // Writes the data value to the Conv time Register

    SSI_MasterWriteCommReg(CHANNEL_CONV_TIME_ADDR(ADC_CHANNEL1));   // Specifies a Write Cycle is being performed at the Channel Conv time Register of Specific Channel

    SSI_MasterWriteCommReg(CHOP_ENABLE | Value_FW);                 // Writes the data value to the Conv time Register

    SSI_MasterWriteCommReg(CHANNEL_CONV_TIME_ADDR(ADC_CHANNEL2));   // Specifies a Write Cycle is being performed at the Channel Conv time Register of Specific Channel

    SSI_MasterWriteCommReg(CHOP_ENABLE | Value_FW);                 // Writes the data value to the Conv time Register

    SSI_MasterWriteCommReg(CHANNEL_CONV_TIME_ADDR(ADC_CHANNEL3));   // Specifies a Write Cycle is being performed at the Channel Conv time Register of Specific Channel

    SSI_MasterWriteCommReg(CHOP_ENABLE | Value_FW);                 // Writes the data value to the Conv time Register

    conv_time_single = round(((Value_FW * 128) + 248)/16);

    conv_time_continuous = round(((Value_FW * 128) + 249)/16);

    return;
}

//*****************************************************************************
//
// Perform Single Mode Conversion on the AD7734 chip and Reads Data at
// various analog input channels
//
//*****************************************************************************
void single_conv_mode_processing(uint8_t adc_channel_number)
{
    SSI_MasterWriteCommReg(MODE_ADDR(adc_channel_number));          // Specifies a Write Cycle is being performed at the Mode Register of Specific Channel

    SSI_MasterWriteCommReg(SINGLE_CONV);                            // Writes Mode value to the Mode Register

    SysCtlDelay(conv_time_single);                                  // Waits for the conversion time

    SSI_MasterReadCommReg(CHANNEL_DATA_ADDR(adc_channel_number));   // Specifies a Read Cycle is being performed at the Channel Data Register

    SSI_MasterReadData(channelDataLength);                          // Reads the 16- bit Data from the Channel Register

    datah = (dataRx[1] << 8) | dataRx[0];                           // Performs bit shift as the input data is store as 8-bit

    voltage = (datah * 5)/65534;                                    // Calculates the voltage at the specific channel ranging 0V as 0x0000 and 5V as 0xFFFF

    UARTprintf("\r\nVoltage at Channel %d is %4f", adc_channel_number, voltage);

    return;

}

//*****************************************************************************
//
// Perform Single Mode Operation
//
//*****************************************************************************
void single_mode_conversion(void)
{
    single_conv_mode_processing(ADC_CHANNEL0);          // Performs single mode conversion at Channel 0

    single_conv_mode_processing(ADC_CHANNEL1);          // Performs single mode conversion at Channel 1

    single_conv_mode_processing(ADC_CHANNEL2);          // Performs single mode conversion at Channel 2

    single_conv_mode_processing(ADC_CHANNEL3);          // Performs single mode conversion at Channel 3

    return;
}

//*****************************************************************************
//
// Perform Continuous Mode Conversion on the AD7734 chip and Reads Data at
// various analog input channels
//
//*****************************************************************************
void continuous_conv_mode_processing(void)
{
    SSI_MasterWriteCommReg(MODE_ADDR(ADC_CHANNEL0));                // Specifies a Write Cycle is being performed at the Mode Register of ADC Channel0

    SSI_MasterWriteCommReg(CONTINOUS_CONV);                         // Writes Mode value to the Mode Register

    SSI_MasterReadCommReg(CHANNEL_DATA_ADDR(ADC_CHANNEL0));         // Specifies a Read Cycle is being performed at the Channel Data Register

    SysCtlDelay(conv_time_continuous);                              // Waits for the conversion time

    //*****************************************************************************
    //Reads Channel Status and Data of ADC Channel 0
    //*****************************************************************************

    SSI_MasterReadData(channelStatusLength);                        // Reads the Channel Status Register

    SSI_MasterReadData(channelDataLength);                          // Reads the 16- bit Data from the Channel Register

    datah = (dataRx[1] << 8) | dataRx[0];                           // Performs bit shift as the input data is store as 8-bit

    voltage = (datah * 5)/65534;                                    // Calculates the voltage at the specific channel ranging 0V as 0x0000 and 5V as 0xFFFF

    UARTprintf("\r\nVoltage at Channel %d is %4f", ADC_CHANNEL0, voltage);

    //*****************************************************************************
    //Reads Channel Status and Data of ADC Channel 1
    //*****************************************************************************

    SysCtlDelay(conv_time_continuous);                              // Waits for the conversion time

    SSI_MasterReadData(channelStatusLength);                        // Reads the Channel Status Register

    SSI_MasterReadData(channelDataLength);                          // Reads the 16- bit Data from the Channel Register

    datah = (dataRx[1] << 8) | dataRx[0];                           // Performs bit shift as the input data is store as 8-bit

    voltage = (datah * 5)/65534;                                    // Calculates the voltage at the specific channel ranging 0V as 0x0000 and 5V as 0xFFFF

    UARTprintf("\r\nVoltage at Channel %d is %4f", ADC_CHANNEL1, voltage);

    //*****************************************************************************
    //Reads Channel Status and Data of ADC Channel 2
    //*****************************************************************************

    SysCtlDelay(conv_time_continuous);                              // Waits for the conversion time

    SSI_MasterReadData(channelStatusLength);                        // Reads the Channel Status Register

    SSI_MasterReadData(channelDataLength);                          // Reads the 16- bit Data from the Channel Register

    datah = (dataRx[1] << 8) | dataRx[0];                           // Performs bit shift as the input data is store as 8-bit

    voltage = (datah * 5)/65534;                                    // Calculates the voltage at the specific channel ranging 0V as 0x0000 and 5V as 0xFFFF

    UARTprintf("\r\nVoltage at Channel %d is %4f", ADC_CHANNEL2, voltage);

    //*****************************************************************************
    //Reads Channel Status and Data of ADC Channel 3
    //*****************************************************************************

    SysCtlDelay(conv_time_continuous);                              // Waits for the conversion time

    SSI_MasterReadData(channelStatusLength);                        // Reads the Channel Status Register

    SSI_MasterReadData(channelDataLength);                          // Reads the 16- bit Data from the Channel Register

    datah = (dataRx[1] << 8) | dataRx[0];                           // Performs bit shift as the input data is store as 8-bit

    voltage = (datah * 5)/65534;                                    // Calculates the voltage at the specific channel ranging 0V as 0x0000 and 5V as 0xFFFF

    UARTprintf("\r\nVoltage at Channel %d is %4f", ADC_CHANNEL3, voltage);

    return;

}

//*****************************************************************************
//
// Perform Continuous Mode Operation
//
//*****************************************************************************
void continuous_mode_conversion(void)
{
    continuous_conv_mode_processing();                              // Performs continuous mode conversion at all Channels

    //Idles the ADC by writing IDLE State to the MODE Register

    SSI_MasterWriteCommReg(MODE_ADDR(ADC_CHANNEL0));                // Specifies a Write Cycle is being performed at the Mode Register of ADC Channel0

    SSI_MasterWriteCommReg(IDLE);                                   // Writes Mode value to the Mode Register

    return;
}


