/** radio8 app:
This app allows realtime status updates between up to 8 devices.


== Description ==

Currently we are testing sequencing, to ensure all devices get time to speak.

*/

/** Dependencies **************************************************************/

#include "radio8.h"
#include <usb.h>
#include <usb_com.h>

#include <stdio.h>
#include <string.h>

/** Functions *****************************************************************/

void main( void ) {	
	uint8 XDATA * buffer;
	uint8 DATA tmp;
	
	
	systemInit();
	usbInit();
	radioMacInit();
	radioInitAddendum();

	while(1){
		boardService();
		//usbShowStatusWithGreenLed();
		usbComService();
		
		#if 1
		if(usbComTxAvailable() > 50){
			/* Events can get lost here, if they occur while this is running, or occur twice
			before this can handle them. However, this is a realtime system, so we should
			just drop the extra events and handle what we can. */
			switch( respond_state ) {
				case RADIO_ID_ERROR:
				case BAD_CRC:
				case MESSAGE_RCV:
					usbComTxSendByte(rx_processing[1]);
					break;
				
				default:
					break;
			}
			respond_state = 0;
		}
		#endif
		
		if( usbComRxAvailable() ) {
			buffer = tx_next_buffer();
			tmp = usbComRxReceiveByte();
			usbComTxSendByte( tmp );
			buffer[1] = tmp;
			buffer[0] = 2;
			tx_packet = buffer;
			radioMacStrobe();
		}
		
	}
}