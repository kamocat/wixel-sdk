/** radio8 app:
This app allows realtime status updates between up to 8 devices.


== Description ==

Currently we are testing sequencing, to ensure all devices get time to speak.

*/

/** Dependencies **************************************************************/

#include <radio_mac.h>
#include <board.h>
#include <cc2511_map.h>
#include <dma.h>
#include <radio_registers.h>
#include <usb.h>
#include <usb_com.h>

/** Global Variables *********************************/

#define MAX_TIMEOUT 50
#define MAX_PACKET_LEN 5

int32 CODE param_radio_channel = 128; // 0 - 255. Seperate by 2 channels to avoid crosstalk

uint8 XDATA rx_packet[MAX_PACKET_LEN + 4]; // room to store CRC
volatile uint8 DATA message_available = 0;



/** Functions *****************************************************************/


/* This function is declared in radio_mac.h, but needs to be defined here */
void radioMacEventHandler( uint8 event) {
		
	if( event == RADIO_MAC_EVENT_RX){ //Packet recieved
		message_available = 1;
		LED_YELLOW_TOGGLE();
	}
	radioMacRx( rx_packet, MAX_TIMEOUT );
}

void main( void ) {	
	
	CHANNR = param_radio_channel;
	PKTLEN = MAX_PACKET_LEN;
	systemInit();
	usbInit();
	radioMacInit();
	radioMacStrobe();

	while(1){
		usbComService();
		
		if( message_available && usbComTxAvailable() ){
			usbComTxSendByte(rx_packet[1]);
			message_available = 0;
		}
	}
}