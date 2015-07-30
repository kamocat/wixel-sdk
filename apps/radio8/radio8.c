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
#define RADIO_COUNT 10

int32 CODE param_radio_channel = 128; // 0 - 255. Seperate by 2 channels to avoid crosstalk

uint8 XDATA * rx_packet;
uint8 XDATA rx_array[RX_ARRAY_SIZE][MAX_PACKET_LEN + 4];// room to store CRC
uint8 DATA rx_array_front = 0;
uint8 DATA rx_array_len = 0;

uint8 XDATA packet_storage[RADIO_COUNT][MAX_PACKET_LEN];

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
	uint8 tmp;
	/* We have to leave one space so that the main() loop can process data without fear of it
	being overwritten. If that's all that's left, then we overwrite the most recent data, because
	returning a null pointer risks a seg fault. */
	if( rx_array_len < RX_ARRAY_MAX ) {
		++rx_array_len;
	}
	tmp = rx_array_len + rx_array_front;
	if( tmp > RX_ARRAY_MAX ) {
		tmp -= RX_ARRAY_SIZE;
	}
	return rx_array[tmp];
}
		

void copy_to_storage( uint8 * XDATA packet ){
	uint8 DATA i = 0;
	if( packet == 0 ){ // invalid pointer
		return;
	}
	if( packet[0] > MAX_PACKET_LEN ){
		packet[0] = MAX_PACKET_LEN; // Don't discard the data, but we can't hold it all.
	}
	if( packet[1] >= RADIO_COUNT ){
		return;	// Invalid radio ID
	}
	/* At this point, the packet may have corrupted data,
	   but at least it won't break our storage. */
	for( ; i <= packet[0]; ++i ){
		packet_storage[packet[1]][i] = packet[i];
	}
	return;
}

void print_packet( uint8 * XDATA packet ){
	uint8 PDATA avail = usbComTxAvailable();
	uint8 PDATA len;
	if( packet == 0){
		return;
	}
	if( packet[0] == 0 ){ // If we haven't written data there yet
		return;
	}
	len = packet[0] - 1;
	if( len > avail ){
		len = avail;
	}
	usbComTxSend(&packet[2], len); //Start after the radio ID
	
	return;
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
	char DATA command;
	uint8 DATA j;
	CHANNR = param_radio_channel;
	PKTLEN = MAX_PACKET_LEN;
	systemInit();
	usbInit();
	radioMacInit();
	radioMacStrobe();
	
	/* Clear the storage array */
	for( j = 0; j < RADIO_COUNT; ++j ){
		packet_storage[j][0] = 0;
	}

	while(1){
		usbComService();
		
		if( RX_AVAILABLE() ){
			rx_processing = dequeue_rx_packet();
			
			copy_to_storage( rx_processing );
		}
		if( usbComRxAvailable() ){
			command = usbComRxReceiveByte();
			if( (command <= '9') && (command >= '0') ){
				command -= '0';
				if( command < RADIO_COUNT ){
					print_packet(packet_storage[command]);
				}
			} else if( command == 'a' ) {
				for( j = 0; j < RADIO_COUNT; ++j ){
					print_packet(packet_storage[j]);
				}
				command = '\r';
			} if( command == '\r' ){
				if( usbComTxAvailable() >= 2 ){
					usbComTxSend( (uint8 XDATA *)"\r\n", 2 );
				}
			}
		}
	}
}