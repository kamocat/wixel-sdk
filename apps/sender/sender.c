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

#define RADIO_COUNT 2
#define MAX_TIMEOUT 50
#define MAX_PACKET_LEN 5

int32 CODE param_radio_channel = 128; // 0 - 255. Seperate by 2 channels to avoid crosstalk

uint8 XDATA rx_packet[MAX_PACKET_LEN + 4]; // room to store CRC
uint8 XDATA tx_packet[MAX_PACKET_LEN];
volatile uint8 PDATA radio_tx_available = 0;
volatile uint8 DATA respond_state = 0;
#define MESSAGE_RCV 1
#define BAD_CRC 2
#define RADIO_ID_ERROR 3
#define RADIO_TIMEOUT 4



/** Functions *****************************************************************/

void radioInitAddendum() {
	CHANNR = param_radio_channel;
	PKTLEN = MAX_PACKET_LEN;
}

void radioMacEventHandler( uint8 event) {
	if(radio_tx_available){
		radioMacTx( tx_packet );
		LED_RED(1);
		radio_tx_available = 0;
	} else {
		LED_RED(0);
	}
}

void main( void ) {
	uint8 PDATA i = 1; // counter
	char DATA letter = 'z';
	
	systemInit();
	usbInit();
	radioMacInit();
	radioInitAddendum();

	while(1){
		boardService();
		usbComService();
		
		
		if( --i == 0 ) {
			i = 100;
			if( !radio_tx_available ){
				if( usbComTxAvailable() > 1){
					usbComTxSendByte( letter );
				}
				tx_packet[1] = letter;
				tx_packet[0] = 1; // lenght of packet, excluding this byte
				radio_tx_available = 1;
				radioMacStrobe();
			}
		}
		
		if( usbComRxAvailable() ) {
			letter = usbComRxReceiveByte();
			i = 1; //send the new character right away
		} else {
			delayMs(5);
		}

	}
}