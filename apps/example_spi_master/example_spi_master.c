/** example_spi_master app
This app demonstrates the SPI communication on the wixel
*/

#include <wixel.h>
#include <usb.h>
#include <usb_com.h>
#include <stdio.h>
#include <gpio.h>

#define SPI0_CLEAR_SS() setDigitalOutput(01, 1) //Not super sure about which pins are used, so I'll just pick a different one here
#define SPI0_SET_SS() setDigitalOutput(01, 0)
#include <spi0_master.h>

int32 CODE param_blink_period_ms = 10;

int32 CODE param_SPI_clock_hz = 10000; // 1khz

uint32 lastToggle = 0;

#define MAX_XFER_SIZE 10

uint8 XDATA tx[MAX_XFER_SIZE];
uint8 XDATA rx[MAX_XFER_SIZE];

#define DISPLAY_TEST 2

#define SPIN_LEN 4
uint8 XDATA spin[SPIN_LEN] = {0x10, 0x01, 0x04, 0x08};
uint8 spin_index = 0;

void updateLeds()
{
	usbShowStatusWithGreenLed();

	LED_YELLOW(0);

	if (getMs() - lastToggle >= param_blink_period_ms/2)
	{
		lastToggle = getMs();
		LED_RED(!LED_RED_STATE);
		
		#if DISPLAY_TEST == 2
		tx[0] = 0x0F;
		tx[1] ^= 0xFF;
		spi0MasterTransfer( tx, rx, 2 );
		#else
		spin_index = (spin_index + 1) % SPIN_LEN;
		tx[6] = 1;	// digit 1
		tx[7] = spin[spin_index];
		spi0MasterTransfer( tx, rx, 2 );
		#endif
		
	}
}

void init_SPI() 
{
	setDigitalOutput( (1,1), 1);
	spi0MasterInit();
	spi0MasterSetBitOrder(SPI_BIT_ORDER_MSB_FIRST);
	spi0MasterSetClockPhase(SPI_PHASE_EDGE_LEADING);
	spi0MasterSetClockPolarity(SPI_POLARITY_IDLE_LOW);
	spi0MasterSetFrequency(param_SPI_clock_hz);
	
	/*
	//Now we set up the seven-segment display
	//First, set brightness to max.
	tx[0] = 0x0A; tx[1] = 15;
	
	//Now tell it there are 8 digits
	tx[2] = 0x0B; tx[3] = 7;
	
	//Now turn on the display
	tx[4] = 0x0C; tx[5] = 0x01;
	
	setDigitalOutput( (1,1), 0);
	spi0MasterTransfer( tx, rx, 6 );
	*/
}
	

void main()
{
	systemInit();
	usbInit();
	init_SPI(); 

	while(1)
	{
		boardService();
		updateLeds();
		usbComService();
	}
}
