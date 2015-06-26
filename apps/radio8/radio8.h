#include <radio_mac.h>
#include <board.h> // for LEDs
#include <cc2511_map.h>
#include <dma.h>
#include <radio_registers.h>
#include "debug.h"

/**
==Notes==
This uses the red LED to show packet transmission

We need to take control over the calibration, because I want to make
use of the rx_timout for packet scheduling.

*/

#define RADIO_COUNT 2
#define MAX_TIMEOUT 250
#define MAX_PACKET_LEN 25
#define RX_BUFFER_COUNT 16
#define TX_BUFFER_COUNT 3


/** Global Variables */
int32 CODE param_radio_channel = 128; // 0 - 255. Seperate by 2 channels to avoid crosstalk

uint8 XDATA rx_buffer[RX_BUFFER_COUNT][MAX_PACKET_LEN];
int8 PDATA rx_buffer_index;
uint8 XDATA tx_buffer[TX_BUFFER_COUNT][MAX_PACKET_LEN];
int8 PDATA tx_buffer_index;

uint8 XDATA my_ID;
volatile uint8 XDATA * rx_packet;
volatile uint8 XDATA * tx_packet;
volatile uint8 XDATA * rx_processing;
volatile uint8 PDATA rx_ID = 0;
volatile uint8 PDATA rx_timeout = MAX_TIMEOUT;
volatile uint8 PDATA rx_count = 0;
volatile uint8 PDATA bad_crc = 0;
volatile uint8 XDATA * store[RADIO_COUNT+1];

volatile uint8 DATA respond_state = 0;
#define MESSAGE_RCV 1
#define BAD_CRC 2
#define RADIO_ID_ERROR 3
#define RADIO_TIMEOUT 4

#define rx_next_buffer() rx_buffer[rx_buffer_index = (rx_buffer_index ? rx_buffer_index : RX_BUFFER_COUNT) - 1 ]
#define tx_next_buffer() tx_buffer[tx_buffer_index = (tx_buffer_index ? tx_buffer_index : TX_BUFFER_COUNT) - 1 ]


void radioInitAddendum() {
	//MCSM0 = 0x04;	// FS_AUTOCAL = 0, so we need to calibrate the oscillator manually
	rx_packet = rx_next_buffer();
	tx_packet = tx_next_buffer();	// this is full of junk data, but we can't allow a seg fault
	CHANNR = param_radio_channel;
	PKTLEN = MAX_PACKET_LEN;
}


/* This function is declared in radio_mac.h, but needs to be defined here */
void radioMacEventHandler( uint8 event) {
	
	switch( event ) {
		
		case RADIO_MAC_EVENT_RX: //Packet recieved
			usbComTxSendByte('R');
			if( !radioCrcPassed() ){
				//If we get enough bad CRCs, that tells us our radio needs calibration.
				++bad_crc;
				radioMacRx( rx_packet, rx_timeout );
				respond_state = BAD_CRC;
			
			}else if( rx_packet[1] >= RADIO_COUNT ) {
				// Something is wrong.
				radioMacRx( rx_packet, rx_timeout );
				respond_state = RADIO_ID_ERROR;
				
			}else{ // YAY! The CRC matched. Store the data.
				++rx_count;
				rx_ID = rx_packet[1];
				store[rx_ID] = rx_packet; // WARNING: could result in two IDs pointing to the same data
				rx_processing = rx_packet;
				rx_packet = rx_next_buffer();
				respond_state = MESSAGE_RCV;
				
				// We are next! Send away.
				radioMacTx( tx_packet );
				LED_RED(1);
			}
			break;
			
		case RADIO_MAC_EVENT_STROBE:
			radioMacTx( tx_packet );
			LED_RED(1);
			break;
		
		case RADIO_MAC_EVENT_RX_TIMEOUT:
			//respond_state = RADIO_TIMEOUT;
			// continue to default
		default:
			radioMacRx( rx_packet, rx_timeout );
			LED_RED(0);
			break;
	}
}