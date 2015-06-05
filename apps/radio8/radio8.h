#include <radio_mac.h>

#defiine RADIO_COUNT 8
#define MAX_TIMEOUT 9

#define next_index() 

/* This function is declared in radio_mac.h, but needs to be defined here */
radioMacEventHandler( uint8 event) {
switch( event ) {
	default:
	case RADIO_MAC_EVENT_STROBE: //this function was manually entered
	case RADIO_MAC_EVENT_TX: //Packet successfully sent
		radioMacRx( rx_packet, MAX_TIMEOUT ); // we want to send every 10ms
		break;

	case RADIO_MAC_EVENT_RX: //Packet recieved
		If( bad CRC){
			calibrate radio, then call radioMacRx()
			
		}else{
			rx_ID = packet[1];
			store[ID] = rx_packet; // Faster than copying, but could result in two IDs pointing to the same data
			rx_packet = next_index();
			rx_timeout = (my_ID - rx_ID - 1) % RADIO_COUNT;
			
			if( rx_timeout == 0 ){
				radioMacTx( tx_packet );
			}else{
				radioMacRx( rx_packet, rx_timeout );
			}
		}
		break;
		
	case RADIO_MAC_EVENT_RX_TIMEOUT:
		radioMacTx( tx_packet );
		break;
}