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
	uint8 DATA copy;
	buffer[0] = 9;		//Size of packet
	buffer[1] = my_ID;
	//buffer[2] = devices_seen(); // update this to determine devices seen
	
	#if 0
	buffer += 3; // now we write the 16 bit values
	write_int16( my.torque, buffer );
	write_int16( my.speed, buffer );
	write_int16( my.damper, buffer );
	#elif 1
	strncpy( buffer + 3, "sent XXX packets\r\n",  18);
	copy = rx_count;
	u8toc( buffer, copy, 8);
	buffer[0] = 21;
	buffer[2] = '	';
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
	uint8 XDATA crc_msg[50];
	uint8 XDATA id_msg[50];
	uint8 XDATA timeout_msg[50];
	uint8 XDATA oops[20];
	
	systemInit();
	usbInit();
	radioMacInit();
	radioInitAddendum();
	
	strncpy( crc_msg, "	XXX messages had bad CRC.\r\n", 28);
	strncpy( id_msg, "	XXX was the radio ID recieved.\r\n", 33);
	strncpy( timeout_msg, "	Radio has timed out.\r\n", 23);
	strncpy( oops, "	Oops.\r\n", 8 );
	
	
	
	build_packet();
	radioMacStrobe();

	while(1){
		boardService();
		updateLeds();
		usbComService();
		build_packet();
		
		
		if(usbComTxAvailable() > 50){
			/* Events can get lost here, if they occur while this is running, or occur twice
			before this can handle them. However, this is a realtime system, so we should
			just drop the extra events and handle what we can. */
			switch( respond_state ) {
				case MESSAGE_RCV:
					rx_processing[0] = '^';
					rx_processing[1] = '_';
					rx_processing[2] = '^';
					usbComTxSend(rx_processing+1, rx_processing[0] - 1);
					break;
				
				case BAD_CRC:
					 // Print the number of failed CRCs
					u8toc( crc_msg, bad_crc, 1);
					usbComTxSend( crc_msg, 28);
				
				
				case RADIO_ID_ERROR:
					// Print the radio ID we recieved
					u8toc( id_msg, rx_packet[1], 1);
					usbComTxSend( id_msg, 33);
					break;
				
				case RADIO_TIMEOUT:
					usbComTxSend( timeout_msg, 23);
				
				default:
					//usbComTxSend( oops, 7);
					break;
			}
			respond_state = 0;
		}
		
		if( usbComRxAvailable() ) {
			usbComRxReceiveByte();
			radioMacStrobe();
		}
		
	}
}

#endif