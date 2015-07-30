/** radio8 app:
This app allows realtime status updates between up to 8 devices.


== Description ==

Currently we are testing sequencing, to ensure all devices get time to speak.

*/

/** Dependencies **************************************************************/

#include <radio_mac.h>
#include <board.h> // for LEDs
#include <cc2511_map.h>
#include <dma.h>
#include <radio_registers.h>

#include <usb.h>
#include <usb_com.h>

#include <stdio.h>
#include <string.h>


/** Global Variables *********************************/

int32 CODE param_loop_delay = 10;

#define RADIO_COUNT 9
#define MAX_TIMEOUT 50
#define MAX_PACKET_LEN 30

#define TX_ARRAY_SIZE 4
#define TX_ARRAY_MAX (TX_ARRAY_SIZE - 1)

int32 CODE param_radio_channel = 128; // 0 - 255. Seperate by 2 channels to avoid crosstalk

uint8 XDATA * tx_array[TX_ARRAY_SIZE];
uint8 DATA tx_array_front = 0;
uint8 DATA tx_array_len = 0;
uint8 XDATA * tx_packet;

uint8 XDATA nums[10][MAX_PACKET_LEN];

/** Functions *****************************************************************/

#define ENABLE_RADIO_INT() IEN2 |= 0x01
#define DISABLE_RADIO_INT() IEN2 &= 0xFE

/* This function takes the packet as an argument, because we already have the data allocated.
If it supplied data, we would in fact need two functions: One to retrieve the space so we can
put data into it, and another to submit it to the buffer. Perhaps we would call these "allocate"
and "submit" */
void enqueue_tx_packet(uint8 XDATA * new_packet ) {
	DISABLE_RADIO_INT();
	if( tx_array_len < TX_ARRAY_MAX ) {
		++tx_array_len;
		if(tx_array_front)
			--tx_array_front;
		else
			tx_array_front = TX_ARRAY_MAX;
	}
	tx_array[tx_array_front] = new_packet; //overwrite the latest data if necessary
	
	ENABLE_RADIO_INT();
}

uint8 XDATA * dequeue_tx_packet( ){
	uint8 tmp = tx_array_front + tx_array_len;
	if( tmp > TX_ARRAY_MAX ) {
		tmp -= TX_ARRAY_SIZE;
	}
	if( tx_array_len > 0 ) {
		--tx_array_len;
	}
	return tx_array[tmp]; //Always return something, because null pointers are bad
}

void setup_numerals( void ) {
	uint8 i = 0;
	for(; i < 10; ++i ) {
		nums[i][1] = i;
	}
	strncpy( &nums[0][2], " Zero", 5 );
	strncpy( &nums[1][2], "  One", 5 );
	strncpy( &nums[2][2], " Two ", 5 );
	strncpy( &nums[3][2], "Three", 5 );
	strncpy( &nums[4][2], " Four", 5 );
	strncpy( &nums[5][2], " Five", 5 );
	strncpy( &nums[6][2], " Six ", 5 );
	strncpy( &nums[7][2], "Seven", 5 );
	strncpy( &nums[8][2], "Eight", 5 );
	strncpy( &nums[9][2], " Nine", 5 );
}

void radioMacEventHandler( uint8 event) {
	if(tx_array_len){
		LED_RED(1);
		radioMacTx( dequeue_tx_packet() );
	} else {
		LED_RED(0);
	}
}

void main( void ) {
	uint8 PDATA i = 1; // counter
	uint8 PDATA n = 0;	//radio #
	char DATA letter = '\x0041';
	uint8 XDATA * new_packet;
	
	
	systemInit();
	usbInit();
	radioMacInit();
	CHANNR = param_radio_channel;
	PKTLEN = MAX_PACKET_LEN;
	setup_numerals();

	while(1){
		boardService();
		usbComService();
		


		i = 2;
		n = n < RADIO_COUNT ? (n+1) : 1;
		new_packet = nums[n];
		
		new_packet[0] = 6; // lenght of packet
		enqueue_tx_packet( new_packet );
		
		if( !LED_RED_STATE ) { //if it's not currently transmitting, get it started
			radioMacStrobe();
		}
		delayMs(param_loop_delay);

	}
}