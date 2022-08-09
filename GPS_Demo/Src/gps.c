/*
 * gps.c
 *
 *  Created on: Mar 15, 2019
 *      Author: Asus
 */

#include "string.h"
#include "stdlib.h"
#include "stdint.h"
#include "stdio.h"
#include "stm32l0xx_hal.h"

#include "gps.h"

static void gps_parse_nmea(GPS_t *gpsHandle, uint8_t *data, uint16_t size);

GPS_t *gps_open(GPS_data_type_e dataType)
{
	GPS_t *gpsHandle = calloc(1, sizeof(GPS_t));
	if (gpsHandle != NULL)
	{
		gpsHandle->dataType = GPS_DATA_TYPE_NMEA;
	}

	return gpsHandle;
}

void gps_set_input_buffer(GPS_t *gpsHandle, uint8_t *data, uint16_t size)
{
	switch (gpsHandle->dataType)
	{
	case GPS_DATA_TYPE_NMEA:
		gps_parse_nmea(gpsHandle, data, size);
		break;
	case GPS_DATA_TYPE_UBX:
		break;
	}
}

extern UART_HandleTypeDef huart1;
static void gps_parse_nmea(GPS_t *gpsHandle, uint8_t *data, uint16_t size)
{

	char *dollar_pointer = strstr((char *)data, "$");
	if (dollar_pointer == NULL)
	{
		return;
	}
	char *star_pointer = strstr((char *)data, "*");
	if (star_pointer == NULL)
	{
		return;
	}

	// Parse
	char *GPGGA = strstr((char *)data, "GPGGA");
	if (GPGGA != NULL)
	{
		   const char s[2] = ",";
		   char *token;

		   GPS_GPGGA gpgga;
		   uint8_t degreeLon;
		   uint8_t degreeLat;
		   float minute;
		   token = strtok(GPGGA, s);
		   uint8_t tokenIndex = 0;
		   while( token != NULL )
		   {
			   switch(tokenIndex)
			   {
			   case 0:
				   //"GPGGA"
				   break;
			   case 1:
				   //"fixTakenAt"
				   token[6] = 0;
				   gpgga.fixTakenAt.Second = strtol(token+4, (char **)NULL, 10);
				   token[4] = 0;
				   gpgga.fixTakenAt.Minute = strtol(token+2, (char **)NULL, 10);
				   token[2] = 0;
				   gpgga.fixTakenAt.Hour = strtol(token, (char **)NULL, 10);
				   break;
			   case 2:
				   //Latitude 4807.038
				   degreeLat = strtol(token, (char **)NULL, 10)/100;
				   minute = strtof (token, NULL)-(degreeLat*100);
				   gpgga.latitude = minute/60 + degreeLat;
				   break;
			   case 3:
				   //Latitude NS
				   if (token[0] == 'S')
				   {
					   gpgga.latitude *= -1;
				   }
				   break;
			   case 4:
				   //Longitude 4807.038
				   degreeLon = strtol(token, (char **)NULL, 10)/100;
				   minute = strtof (token, NULL)-(degreeLon*100);
				   gpgga.longitude = minute/60 + degreeLon;
				   break;
			   case 5:
				   //Longitude EW
				   if (token[0] == 'W')
				   {
					   gpgga.longitude *= -1;
				   }
				   break;
			   case 6:
				   //FixQuality
				   gpgga.fixQuality = strtol(token, (char **)NULL, 10);
				   break;
			   case 7:
				   //Number of satellites
				   gpgga.numSatellites = strtol(token, (char **)NULL, 10);
				   break;
			   case 8:
				   //horizontalDilution
				   gpgga.horizontalDilution = strtof (token, NULL);
				   break;
			   case 9:
				   //Altitude
				   gpgga.altitude = strtof (token, NULL);
				   break;
			   case 10:
				   //Altitude quantity
				   break;
			   case 11:
				   // Height of geoid (mean sea level) above WGS84 ellipsoid
				   gpgga.heightOfGeoid = strtof (token, NULL);
				   break;
			   case 12:
				   // Height of geoid quantity
				   break;
			   }

			   tokenIndex++;
			   HAL_UART_Transmit(&huart1, (uint8_t *)token, strlen(token), 100);
			   token = strtok(NULL, s);
		   }
	}
}















