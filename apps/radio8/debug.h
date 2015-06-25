#ifndef WIXEL_DEBUG_H
#define WIXEL_DEBUG_H

#include <stdio.h>
#include <string.h>

#ifdef SDCC
#include <wixel.h>
#include <usb.h>
#include <usb_com.h>
#else
/** An unsigned 8-bit integer.  The range of this data type is 0 to 255. **/
typedef unsigned char  uint8;

/** A signed 8-bit integer.  The range of this data type is -128 to 127. **/
typedef signed   char  int8;

/** An unsigned 16-bit integer.  The range of this data type is 0 to 65,535. **/
typedef unsigned short uint16;

/** A signed 16-bit integer.  The range of this data type is -32,768 to 32,767. **/
typedef signed   short int16;

/** An unsigned 32-bit integer.  The range of this data type is 0 to 4,294,967,295. **/
typedef unsigned long  uint32;

/** A signed 32-bit integer.  The range of this data type is -2,147,483,648 to 2,147,483,647. **/
typedef signed   long  int32;

#define CODE
#define DATA
#define PDATA
#define XDATA
#define usbComTxAvailable() 128
void usbComTxSendByte(uint8 byte){
	printf("%c", byte);
}
void usbComTxSend(const uint8 XDATA * buffer, uint8 size){
	char XDATA string[256]; // large enough to fit any size
	strncpy( string, buffer, size );
	string[size] = 0;//null character
	printf("%s", string);
}
#endif


/* This function converts a number into a char array. */
uint8 u8toc( char* dest, uint8 source, uint8 offset ) {
	uint8 PDATA hundreds, tens, ones;
	hundreds = source / 100;
	tens = source / 10;
	ones = source - (10 * tens);
	tens -= 10 * hundreds;
	
	dest[offset++] = hundreds + '0';
	dest[offset++] = tens + '0';
	dest[offset++] = ones + '0';
	
	return offset;
}


#endif