// **************************************************************/
// System Clock                             :8MHz
// Software                                    :Atmel Studio 6
// USART Baud Rate                     :9600
// USART Data Bits                       :8
// USART Stop Bits                       :1
// USART Mode                            :Asynchronous Mode
// USART Parity                            :No Parity
// **************************************************************//
//
#include<avr/io.h>
/*Includes io.h header file where all the Input/Output Registers and its Bits are defined for all AVR microcontrollers*/

#define            F_CPU           8000000
/*Defines a macro for the delay.h header file. F_CPU is the microcontroller frequency value for the delay.h header file. Default value of F_CPU in delay.h header file is 1000000(1MHz)*/

#include<util/delay.h>
/*Includes delay.h header file which defines two functions, _delay_ms (millisecond delay) and _delay_us (microsecond delay)*/

#include "hc05.h"
/*Includes hc05.h header file which defines different functions for HC-05 Bluetooth Module. HC-05 header file version is 1.1*/
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <avr/portpins.h>
#include <avr/pgmspace.h>
#include "bit.h"
#include "scheduler.h"
#include "keypad.h"
#include "lcd.h"
#include "usart_ATmega1284.h"
#include "adxl335.h"
unsigned char receivedData;

enum Servant_Tick {Init} Servant_State;
enum Temp_Tick {Temp_Init, Temp_State1} Temp_State;

//Master code
void SPI_MasterInit(void) {
	// Set DDRB to have MOSI, SCK, and SS as output and MISO as input
	//DDR_SPI = (1<<DD_MOSI)|(1<<DD_SCK)|(1<<DD_SS);
	DDRB = 0xB0;
	// Set SPCR register to enable SPI, enable master, and use SCK frequency
	//   of fosc/16  (pg. 168)
	SPCR |= 0x51;
	//SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR0);
	// Make sure global interrupts are enabled on SREG register (pg. 9)
	SREG = SREG|0x80;
	//SREG = 0x80;
}

void SPI_MasterTransmit(unsigned char cData) {
	// data in SPDR will be transmitted, e.g. SPDR = cData;
	SPDR = cData;
	// set SS low
	while(!(SPSR & (1<<SPIF))) { // wait for transmission to complete
		;
	}
	// set SS high
}

//Servant code
void SPI_ServantInit(void) {
	// set DDRB to have MISO line as output and MOSI, SCK, and SS as input
	DDRB = 0x40;
	//DDR_SPI = (1<<DD_MISO);
	// set SPCR register to enable SPI and enable SPI interrupt (pg. 168)
	SPCR = SPCR|0xC0;
	//SPCR = (1<<SPE);
	// make sure global interrupts are enabled on SREG register (pg. 9)
	SREG = SREG|0x80;
}

ISR(SPI_STC_vect) {
	// this is enabled in with the SPCR register’s “SPI
	// Interrupt Enable”
	// SPDR contains the received data, e.g. unsigned char receivedData =
	// SPDR;
	//return SPDR;
	receivedData = SPDR;
}

void Servant_Init(){
	Servant_State = Init;
}

void Servant_Tick(){
	unsigned char x[1];
	switch(Servant_State){
		case Init:
			x[0] = receivedData;
			hc_05_bluetooth_transmit_string(x);
		break;
		default:{
			break;
		}
	}
	switch(Servant_State){
		case Init:
			Servant_State = Init;
		break;
		default:{
			Servant_State = Init;
			break;
		}	
	}
}

// void Temp_Init(){
// 	Temp_State = Temp_Init;
// }
// void Temp_Tick(){
// 	switch(Temp_State){
// 		case Temp_Init:
// 			hc_05_bluetooth_transmit_string()
// 		break;
// 		case Temp_State1:
// 		break;
// 		default:{
// 			break;
// 		}
// 	}
// 	switch(Temp_State){
// 		case Temp_Init:
// 		break;
// 		case Temp_State1:
// 		break;
// 		default:{
// 			Temp_State = Temp_Init;
// 			break;
// 		}
// 	}
// }


int main(void)
{
	DDRB = 0xFF; PORTB = 0x00;
	DDRD = 0xFF; PORTD = 0x00;

	
	tasksNum = 1;
	task tsks[1];
	tasks = tsks;
	unsigned char i = 0;
	tasks[i].state = -1;
	tasks[i].period = 500;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFct = &Servant_Tick;
	
	usart_init();
	SPI_ServantInit();
	Servant_Init();
	
	TimerSet(500);
	TimerOn();
	while(1){
	}
}


