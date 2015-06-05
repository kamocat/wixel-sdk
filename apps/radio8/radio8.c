/** radio8 app:
This app allows realtime status updates between up to 8 devices.


== Description ==

It keeps an array of the data received, and resends its data if it
another device didn't recieve it correctly.

Initially, wait to see if other devices are about to send.
Wait long enough that all the devices have sent packets. Pick
the next available number. The numbers trjansmit in sequence; each
has their own timeslot.

Each packet should include:
Device number
Mask of which devices it has valid data for
Status update
Checksum

If we include the packet length, then this allows the packet sniffer to work.
I think we can use the radio_queue library for this.

*/

/** Dependencies **************************************************************/
#include <cc2511_map.h>
#include <board.h>
#include <random.h>
#include <time.h>

#include <usb.h>
#include <usb_com.h>
#include <radio_queue.h>

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
#define MAX_ROWER_COUNT 8
int32 CODE param_radio_ID = 1;
uint8 my_id = param_radio_ID;

int32 CODE param_tx_period = 500;

// radio_channel: See description in radio_link.h.




/** Global Variables **********************************************************/

uint8 devices_seen = 0;	// this is a bitmask of each of the radio IDs that are seen
struct rower XDATA rowers[MAX_ROWER_COUNT];
uint8 blink = 0;

/** Functions *****************************************************************/

#define read_int16(array) array[0]<<8 | array[1]; array += 2
#define write_int16(num,array)  array[0] = (num) >> 8; array[1] = (num) & 0xFF; array +=2


void updateLeds()
{
    usbShowStatusWithGreenLed();

    LED_YELLOW(radioQueueRxCurrentPacket());
	
    LED_RED(blink);
}


void build_packet( void ) {
	uint8 XDATA * buffer = radioQueueTxCurrentPacket();
	buffer[0] = 9;		//Size of packet
	buffer[1] = my_id;
	buffer[2] = devices_seen;
	
	#if 1
	buffer += 3; // now we write the 16 bit values
	write_int16( rowers[my_id].torque, buffer );
	write_int16( rowers[my_id].speed, buffer );
	write_int16( rowers[my_id].damper, buffer );
	#else
	buffer[3] = send.torque >> 8;
	buffer[4] = send.torque & 0xFF;
	buffer[5] = send.speed >> 8;
	buffer[6] = send.speed & 0xFF;
	buffer[7] = send.damper >> 8;
	buffer[8] = send.damper & 0xFF;
	#endif
	
	radioQueueTxSendPacket();
	
	return;
}

void store_packet( void ) {
	if( radioQueueTxQueued() ){
		uint8 XDATA * buffer = radioQueueRxCurrentPacket();
		uint8 id = buffer[1] & 0x07;
		devices_seen |= 1<<id;
		rowers[id].timestamp = getMs();
		rowers[id].torque = read_int16(buffer);
		rowers[id].speed = read_int16(buffer);
		rowers[id].damper = read_int16(buffer);
		
		radioQueueRxDoneWithPacket();
	}
	
	return;
}


void main( void ) {
	
	uint8 i = 0;	// this is just for doling out processor time
	char * sample = "TqSpDm";
	int32 oldtime;
	
	systemInit();
	usbInit();
	radioQueueInit();
	
	
	rowers[my_id].torque = read_int16(sample);
	rowers[my_id].speed = read_int16(sample);
	rowers[my_id].damper = read_int16(sample);
	
	
	while(1){
		++i;
		
        boardService();
        updateLeds();
        usbComService();
		
		
		if( ( getMs() - oldtime) > param_tx_period ){ // once per period
			oldtime = getMs();
			blink = !blink;
			build_packet();
		}
		store_packet();
	}
	
	return;
}