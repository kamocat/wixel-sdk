/** radio8 app:
This app allows realtime status updates between up to 8 devices.


== Description ==

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
#define MAX_PACKET_LEN 30
#define RX_ARRAY_SIZE 4
#define RX_ARRAY_MAX (RX_ARRAY_SIZE - 1)

int32 CODE param_radio_channel = 128; // 0 - 255. Seperate by 2 channels to avoid crosstalk

uint8 XDATA * rx_packet;
uint8 XDATA rx_array[RX_ARRAY_SIZE][MAX_PACKET_LEN + 4];// room to store CRC
uint8 DATA rx_array_front = 0;
uint8 DATA rx_array_len = 0;

/** Functions *****************************************************************/

#define RX_AVAILABLE() (rx_array_len > 0) // We need to reserve one space for the active recieving

#define ENABLE_RADIO_INT() IEN2 |= 0x01
#define DISABLE_RADIO_INT() IEN2 &= 0xFE

uint8 XDATA * dequeue_rx_packet(){
	uint8 tmp = 0;
	if( RX_AVAILABLE() ) { 
		tmp = rx_array_front;
		
		DISABLE_RADIO_INT();
		
		--rx_array_len;
		if( rx_array_front == RX_ARRAY_MAX )
			rx_array_front = 0;
		else
			++rx_array_front;
		
		ENABLE_RADIO_INT();
		
	}
	return rx_array[tmp];
}



uint8 XDATA * enqueue_rx_packet() {
	uint8 tmp = rx_array_front;
	if( rx_array_len < RX_ARRAY_SIZE ) { // If there's no room, overwrite the most recent
		++rx_array_len;
	}
	tmp += rx_array_len;
	if( tmp > RX_ARRAY_MAX ) {
		tmp -= RX_ARRAY_SIZE;
	}
	return rx_array[tmp];
}
		



/* This function is declared in radio_mac.h, but needs to be defined here */
void radioMacEventHandler( uint8 event) {
		
	if( event == RADIO_MAC_EVENT_RX){ //Packet recieved
		LED_YELLOW_TOGGLE();
		rx_packet = enqueue_rx_packet();
	}
	radioMacRx( rx_packet, MAX_TIMEOUT );
}

void main( void ) {	
	uint8 XDATA * rx_processing;
	CHANNR = param_radio_channel;
	PKTLEN = MAX_PACKET_LEN;
	systemInit();
	usbInit();
	radioMacInit();
	radioMacStrobe();

	while(1){
		usbComService();
		
		if( RX_AVAILABLE() && usbComTxAvailable() ){
			rx_processing = dequeue_rx_packet();
			usbComTxSendByte(rx_processing[1]);
		}
	}
}