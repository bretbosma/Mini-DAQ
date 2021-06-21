#ifndef CUSTOM_PDO_NAME_H
#define CUSTOM_PDO_NAME_H

//-------------------------------------------------------------------//
//                                                                   //
//     This file has been created by the Easy Configurator tool      //
//                                                                   //
//     Easy Configurator project MiniDAQ_FOSWEC_V3.prj
//                                                                   //
//-------------------------------------------------------------------//


#define CUST_BYTE_NUM_OUT	0
#define CUST_BYTE_NUM_IN	38
#define TOT_BYTE_NUM_ROUND_OUT	0
#define TOT_BYTE_NUM_ROUND_IN	40


typedef union												//---- output buffer ----
{
	uint8_t  Byte [TOT_BYTE_NUM_ROUND_OUT];
	struct
	{
	}Cust;
} PROCBUFFER_OUT;


typedef union												//---- input buffer ----
{
	uint8_t  Byte [TOT_BYTE_NUM_ROUND_IN];
	struct
	{
		float       roll;
		float       pitch;
		float       yaw;
		float       acc_x;
		float       acc_y;
		float       acc_z;
		float       temperature;
		uint16_t    ADC3;
		uint16_t    ADC0;
		uint16_t    ADC1;
		uint16_t    ADC2;
		int16_t     current0;
	}Cust;
} PROCBUFFER_IN;

#endif