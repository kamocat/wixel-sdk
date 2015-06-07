#include <radio_mac.h>
#include <board.h> // for LEDs
#include <cc2511_map.h>
#include <dma.h>
#include <radio_registers.h>

/**
==Notes==
This uses the yellow LED to show packet transmission

We need to take control over the calibration, because I want to make
use of the rx_timout for packet scheduling.

*/

#define RADIO_COUNT 8
#define MAX_TIMEOUT 9 // We want to send at least every 10ms
#define MAX_PACKET_LEN 20
#define RX_BUFFER_COUNT 16
#define TX_BUFFER_COUNT 3


/** Global Variables */
int32 CODE param_radio_channel = 128; // 0 - 255. Seperate by 2 to avoid crosstalk

uint8 XDATA rx_buffer[RX_BUFFER_COUNT][MAX_PACKET_LEN];
int8 XDATA rx_buffer_index;
uint8 XDATA tx_buffer[TX_BUFFER_COUNT][MAX_PACKET_LEN];
int8 XDATA tx_buffer_index;

uint8 XDATA my_ID;
uint8 XDATA * rx_packet;
uint8 XDATA * tx_packet;
uint8 XDATA * store[RADIO_COUNT];

#if 1
#define rx_next_buffer() rx_buffer[rx_buffer_index = (rx_buffer_index ? rx_buffer_index : RX_BUFFER_COUNT) - 1 ]
#define tx_next_buffer() tx_buffer[tx_buffer_index = (tx_buffer_index ? tx_buffer_index : TX_BUFFER_COUNT) - 1 ]
#else // the function-based equivelent, for debugging
uint8 XDATA * rx_next_buffer() {
	rx_buffer_index = (rx_buffer_index ? rx_buffer_index : RX_BUFFER_COUNT) - 1;
	return rx_buffer[rx_buffer_index];
}
uint8 XDATA * tx_next_buffer() {
	tx_buffer_index = (tx_buffer_index ? tx_buffer_index : TX_BUFFER_COUNT) - 1;
	return tx_buffer[tx_buffer_index];
}
#endif

void radioInitAddendum() {
	//MCSM0 = 0x04;	// FS_AUTOCAL = 0, so we need to calibrate the oscillator manually
	rx_packet = rx_next_buffer();
	tx_packet = tx_next_buffer();	// this is full of junk data, but we can't allow a seg fault
	CHANNR = param_radio_channel;
	PKTLEN = MAX_PACKET_LEN;
}


/* This function is declared in radio_mac.h, but needs to be defined here */
void radioMacEventHandler( uint8 event) {
	uint8 rx_ID;
	uint8 rx_timeout;
	
	switch( event ) {
		case RADIO_MAC_EVENT_RX: //Packet recieved
			if( !radioCrcPassed() ){
				//calibrate radio, then call radioMacRx()
				LED_RED(1);
				//RFST=SCAL; //This may take more work than I want
				radioMacRx( rx_packet, MAX_TIMEOUT );
			
			}else if( rx_packet[1] >= RADIO_COUNT ) {
				// Something is wrong. Force your way in anyway.
				radioMacTx( tx_packet );
				//LED_YELLOW(1);
			}else{ // YAY! The CRC matched. Store the data.
				LED_RED(0);
				rx_ID = rx_packet[1];
				store[rx_ID] = rx_packet; // WARNING: could result in two IDs pointing to the same data
				rx_packet = rx_next_buffer();
				rx_timeout = (my_ID > rx_ID) ? (my_ID - rx_ID - 1) : (my_ID + RADIO_COUNT - rx_ID);
				
				if( rx_timeout == 0 ){ // We are next! Send away.
					radioMacTx( tx_packet );
					//LED_YELLOW(1);
				}else{ // Wait for more devices. It's not our turn yet
					radioMacRx( rx_packet, rx_timeout );
				}
			}
			break;
			
		case RADIO_MAC_EVENT_RX_TIMEOUT: // We have waited for others to send, and now it is our turn!
			radioMacTx( tx_packet );
			//LED_YELLOW(1);
			break;
		
		default:
		//case RADIO_MAC_EVENT_STROBE:
		case RADIO_MAC_EVENT_TX: //Packet successfully sent
			radioMacRx( rx_packet, MAX_TIMEOUT );
			LED_YELLOW_TOGGLE();
			break;
	}
}