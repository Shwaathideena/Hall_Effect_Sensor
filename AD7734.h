/*
 * AD7734.h
 *
 *  Created on: Feb 16, 2022
 *      Author: shwaa
 */

#ifndef AD7734_H_
#define AD7734_H_

/* **** Includes **** */
#include <stdint.h>

//*****************************************************************************
//
// Length of bytes to transmit and receive from AD7734 Chip.
//
//*****************************************************************************
#define NUM_SSI_TX_DATA             1
#define NUM_SSI_RX_DATA             2

extern uint8_t dataTx[NUM_SSI_TX_DATA];
extern uint32_t dataRx[NUM_SSI_RX_DATA];


//READ & WRITE
#define CONFIG_READ             1 << 6
#define CONFIG_WRITE            0 << 6


//ADDR REGISTERS Table 11 of AD7734 Datasheet
#define COMM_R_ADDR             0x00
#define IO_PORT_R_ADDR          0x01
#define REV_R_ADDR              0x02
//#define TEST_R_ADDR               0x03                //Do not change R/W to this register
#define ADC_STATUS_R_ADDR       0x04
#define CHECKSUM_R_ADDR         0x05
#define ADC_0_CAL_ADDR          0x06
#define ADC_FULL_CAL_ADDR       0x07


//ADDR REGISTERS MACROS Table 11 of AD7734 Datasheet
#define CHANNEL_DATA_ADDR(adc_channel_number)                   (0x08 + adc_channel_number)
#define CHANNEL_ZERO_SCALE_CAL_ADDR(adc_channel_number)         (0x10 + adc_channel_number)
#define CHANNEL_FULL_SCALE_CAL_ADDR(adc_channel_number)         (0x18 + adc_channel_number)
#define CHANNEL_STATUS_ADDR(adc_channel_number)                 (0x20 + adc_channel_number)
#define CHANNEL_SETUP_ADDR(adc_channel_number)                  (0x28 + adc_channel_number)
#define CHANNEL_CONV_TIME_ADDR(adc_channel_number)              (0x30 + adc_channel_number)
#define MODE_ADDR(adc_channel_number)                           (0x38 + adc_channel_number)


//OPERATION MODE Table 12 of AD7734 Datasheet
#define IDLE                    0 << 5          //IDLE
#define CONTINOUS_CONV          1 << 5          //CONTINOUS CONVERSION
#define SINGLE_CONV             2 << 5          //SINGLE CONVERSION
#define POWER_DOWN              3 << 5          //POWER DOWN

//Chop Enable
#define CHOP_ENABLE             1
#define CHOP_DISABLE            0

//INPUT RANGE
#define N10V_to_P10V            0               // -10V to +10V
#define V_to_P10V               1               // 0V to 10V
#define N5V_to_P5V              2               // -5V to +5V
#define V_to_P5V                3               // 0V to 5V


//ADC Channels Table 14 of AD7734 Datasheet
#define ADC_CHANNEL0            0               //AIN0
#define ADC_CHANNEL1            1               //AIN1
#define ADC_CHANNEL2            2               //AIN2
#define ADC_CHANNEL3            3               //AIN3

//Default Register Data Values
#define ADC_STATUS_R_DEFAULT        0x00


void waitMillisecond(uint32_t ms);

void SSI_MasterWriteCommReg(uint8_t data);

void SSI_MasterReadCommReg(uint8_t data);

void SSI_MasterReadData(uint8_t length);

void setup_AD7734_voltage_range(void);

bool read_adc_status_register(void);

void check_adc_status_register(void);

void set_channel_conversion_time_register(uint8_t Value_FW);

void single_mode_conversion(void);

void single_conv_mode_processing(uint8_t adc_channel_number);

void continuous_mode_conversion(void);

void continuous_conv_mode_processing(void);

#endif /* AD7734_H_ */
