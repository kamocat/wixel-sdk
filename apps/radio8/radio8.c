#ifndef RADIO8_H
#define RADIO8_H

/** radio8 app:
This app allows realtime status updates between up to 8 devices.


== Description ==

Each packet should include:
* Packet length (to allow packet sniffer to work)
* Device number (0-7)
* Mask of which devices it has valid data for
* Status update
* Checksum

*/

/** Dependencies **************************************************************/
#include <cc2511_map.h>
#include <board.h>
#include <time.h>

#include <usb.h>
#include <usb_com.h>
#include "radio8.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h>

/** Types **********************************************************************/
struct rower {
	int16 timestamp;
	int16 torque;
	int16 speed;
	int16 damper;
};

/** Parameters ****************************************************************/
int32 CODE param_radio_ID = 1;




/** Global Variables **********************************************************/

struct rower XDATA my;

/** Functions *****************************************************************/

#define read_int16(array) array[0]<<8 | array[1]; array += 2
#define write_int16(num,array)  array[0] = (num) >> 8; array[1] = (num) & 0xFF; array +=2


void updateLeds()
{
    usbShowStatusWithGreenLed();
}

uint8 devices_seen() {
	uint8 seen = 0;
	uint8 id;
	uint8 i = 0;
	for(; i < RADIO_COUNT; ++i) {
		id = store[i][1];
		if( i == id )
			seen |= 1<<id;
	}
	return seen;
}

void build_packet( void ) {
	uint8 XDATA * buffer = tx_next_buffer();
	buffer[0] = 9;		//Size of packet
	buffer[1] = my_ID;
	buffer[2] = devices_seen(); // update this to determine devices seen
	
	#if 0
	buffer += 3; // now we write the 16 bit values
	write_int16( my.torque, buffer );
	write_int16( my.speed, buffer );
	write_int16( my.damper, buffer );
	#elif 1
	strncpy( buffer + 3, "Test packet", 11 );
	buffer[0] = 13;
	#else
	buffer[3] = send.torque >> 8;
	buffer[4] = send.torque & 0xFF;
	buffer[5] = send.speed >> 8;
	buffer[6] = send.speed & 0xFF;
	buffer[7] = send.damper >> 8;
	buffer[8] = send.damper & 0xFF;
	#endif
	
	tx_packet = buffer;
	
	return;
}

void main( void ) {	
	uint32 XDATA i = 0; // just for doling out processor time
	uint8 XDATA buffer[50];
	uint8 len;
	uint8 rx_prev_count = 0;
	
	systemInit();
	usbInit();
	radioMacInit();
	radioInitAddendum();
	
	/*
	my_ID = param_radio_ID;
	my.torque = ('T' << 8) | 'q';
	my.speed = ('S'<<8) | 'p';
	my.damper = ('D' << 8) | 'm';

	buffer = tx_next_buffer();
	strncpy( buffer, " Jesus", MAX_PACKET_LEN );
	buffer[0] = 5;
	buffer = tx_next_buffer();
	strncpy( buffer, " loves", MAX_PACKET_LEN );
	buffer[0] = 5;
	buffer = tx_next_buffer();
	strncpy( buffer, " you!", MAX_PACKET_LEN );
	buffer[0] = 4;
	*/
	
	radioMacStrobe();
	
	build_packet();
	
	buffer[0] = '\r';
	buffer[1] = '\n';
	buffer[5] = '\t';
	
	while(1){
		++i;
		
		boardService();
		updateLeds();
		usbComService();
		
		
		if((rx_count != rx_prev_count) && (usbComTxAvailable() > 26) ) {
			len = u8toc( buffer, rx_count, 2 );
			strncpy( (buffer + len) , rx_processing, rx_processing[0] );
			usbComTxSend( buffer, (len + rx_processing[0]) );
		}
		
		//build_packet();
		
	}
}

#endif