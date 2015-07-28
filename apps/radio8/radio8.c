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
#define MAX_PACKET_LEN 5
#define RX_BUFFER_COUNT 4

int32 CODE param_radio_channel = 128; // 0 - 255. Seperate by 2 channels to avoid crosstalk

uint8 XDATA * rx_packet;
volatile uint8 DATA latest_rx_index = 0;
uint8 XDATA rx_buffer_array[RX_BUFFER_COUNT][MAX_PACKET_LEN + 4];// room to store CRC
uint8 DATA rx_buffer_index = 0;

/** Functions *****************************************************************/

#define rx_next_buffer() rx_buffer_array[rx_buffer_index = (rx_buffer_index ? rx_buffer_index : RX_BUFFER_COUNT) - 1 ]
#define tx_next_buffer() tx_buffer_array[tx_buffer_index = (tx_buffer_index ? tx_buffer_index : TX_BUFFER_COUNT) - 1 ]




/* This function is declared in radio_mac.h, but needs to be defined here */
void radioMacEventHandler( uint8 event) {
		
	if( event == RADIO_MAC_EVENT_RX){ //Packet recieved
		latest_rx_index = rx_buffer_index;
		LED_YELLOW_TOGGLE();
	}
	rx_packet = rx_next_buffer();
	radioMacRx( rx_packet, MAX_TIMEOUT );
}

void main( void ) {	
	uint8 PDATA prev_rx_index = 0;
	CHANNR = param_radio_channel;
	PKTLEN = MAX_PACKET_LEN;
	systemInit();
	usbInit();
	radioMacInit();
	radioMacStrobe();

	while(1){
		usbComService();
		
		if( (latest_rx_index != prev_rx_index) && usbComTxAvailable() ){
			prev_rx_index = latest_rx_index;
			usbComTxSendByte(rx_buffer_array[prev_rx_index][1]);
		}
	}
}