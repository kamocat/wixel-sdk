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
    LED_RED_TOGGLE();
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
	
	#if 1
	buffer += 3; // now we write the 16 bit values
	write_int16( my.torque, buffer );
	write_int16( my.speed, buffer );
	write_int16( my.damper, buffer );
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
	uint8 XDATA i = 0; // just for doling out processor time
	
	systemInit();
	usbInit();
	radioMacInit();
	radioInitAddendum();
	
	my_ID = param_radio_ID;
	my.torque = ('T' << 8) | 'q';
	my.speed = ('S'<<8) | 'p';
	my.damper = ('D' << 8) | 'm';
	
	
	while(1){
		++i;
		
		boardService();
		updateLeds();
		usbComService();
		
		build_packet();
		
	}
}