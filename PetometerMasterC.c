#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <avr/portpins.h>
#include <avr/pgmspace.h>
#include "lcd.h"
#include "usart_ATmega1284.h"
#include "bit.h"
#include "adxl335.h"

// FreeRTOS include files
// #include "FreeRTOS.h"
// #include "task.h"
// #include "croutine.h"
// #include "queue.h"

unsigned char receivedData;

enum Master_Tick {Init,State1, State2} Master_State;
enum Temp_Tick{Temp_init} Temp_State;
enum Accel_Tick{Accel_init} Accel_State;
enum Queue_Tick{QueueSend_init} Queue_State;

float xavg, yavg, zavg;
int state;			//0 = accel, 1 = temp
Queue char_to_send;

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
void print_each_digit(int x)
{
	unsigned char my_char3;
	if(x >= 10)
	print_each_digit(x / 10);

	int digit = x % 10;
	my_char3 = digit + '0';
	SPI_MasterTransmit(my_char3);
	//QueueEnqueue(char_to_send,my_char3);
	if(state == 1){
		_delay_ms(500);
	}
	else if(state == 0){
		_delay_ms(250);
	}
}
int accelvec = 0;
int steps = 0;
void Accel_Init(){
	Accel_State = Accel_init;
}
void Accel_Tick(){
	int x_axis, y_axis, z_axis;
	switch(Accel_State){
		case Accel_init:
		x_axis = read_adxl335_x_value(0);
		y_axis=read_adxl335_y_value(1);
		z_axis=read_adxl335_z_value(2);
		//accelvec = sqrt(x_axis^2 + y_axis^2 + z_axis^2);
		accelvec = sqrt(((x_axis-xavg)*(x_axis-xavg)) + ((y_axis-yavg)*(y_axis-yavg)) + ((z_axis-zavg)*(z_axis-zavg)));
		break;
		default:{
			break;
		}
	}
	switch(Accel_State){
		case Accel_init:
		Accel_State = Accel_init;
		break;
		default:{
			Accel_State = Accel_init;
			break;
		}
	}
}

void AccelTask(){
	Accel_Init();
	for(;;){
		Accel_Tick();
		vTaskDelay(0);
	}
}

void StartSecPulse1(unsigned portBASE_TYPE Priority)
{
	xTaskCreate(AccelTask, (signed portCHAR *)"Accel Task", configMINIMAL_STACK_SIZE, NULL, Priority, NULL );
}

void Master_Init(){
	Master_State = Init;
}
void Master_Tick(){
	int threshold = 30;
		int flag = 0;
		state = 0;
	//char buffer[21];
	//unsigned char my_char;
	switch(Master_State){
		case Init:
		//print_each_digit(accelvec);
		print_each_digit(steps);
		SPI_MasterTransmit('$');
		//QueueEnqueue(char_to_send, '$');
		_delay_ms(250);
		LCD_Number_Write(accelvec, 10);
		break;
		case State1:
		steps++;
		//print_each_digit(accelvec);
// 		print_each_digit(steps);
// 		SPI_MasterTransmit('$');
// 		_delay_ms(250);
		LCD_Number_Write(accelvec, 10);
		break;
		case State2:
// 		print_each_digit(steps);
// 		SPI_MasterTransmit('$');
// 		_delay_ms(250);
		//print_each_digit(accelvec);
		LCD_Number_Write(accelvec, 10);
		//_delay_ms(500);
		break;
		default:{
			break;
		}
	}
	switch(Master_State){
		case Init:
		if(accelvec >= threshold && flag == 0){
			flag == 1;
			Master_State = State1;
		}
		else if(accelvec < threshold && flag == 1){
			flag == 0;
			Master_State = Init;
		}
		else{
			Master_State = Init;
		}
		break;
		case State1:
		Master_State = State2;
		break;
		case State2:
		Master_State = Init;
		break;
		default:{
			Master_State = Init;
			break;
		}
	}
}

void MasterTask(){
	Master_Init();
	for(;;){
		Master_Tick();
		vTaskDelay(0);
	}
}

void StartSecPulse2(unsigned portBASE_TYPE Priority)
{
	xTaskCreate(MasterTask, (signed portCHAR *)"Master Task", configMINIMAL_STACK_SIZE, NULL, Priority, NULL );
}

void Temp_Init(){
	Temp_State = Temp_init;
}
float prevtemp = 0;
int tempflag = 0;
void Temp_Tick(){
	float n = read_adc_channel(3);
	char my_char2;
	switch(Temp_State){
		case Init:
		state = 1;
		n = read_adc_channel(3);
		n = n / 8.6;
		n = n * 1.8;
		n = n + 32;

		// 			n = n * 5;
		// 			n = n/1024;
		// 			n = (n-.5) * 100;
		// 			n = ((n * 9.0) / 5.0) + 32.0;

		// 			n = n * (5000/1024);
		// 			n = (n - 500)/10;
		// 			n = n * 1.8;
		// 			n = n + 32;
// 		if(n == prevtemp){
// 			tempflag = 1;
// 		}
// 		else{
// 			tempflag == 0;
// 		}
// 		prevtemp = n;
		
		LCD_Number_Write(n, 10);
/*		if(tempflag == 0){*/
		print_each_digit(n);
		SPI_MasterTransmit('*');
		//QueueEnqueue(char_to_send, '*');
		_delay_ms(500);
/*		}*/
		break;
		default:{
			break;
		}
	}
	switch(Temp_State){
		case Init:
		Temp_State = Temp_init;
		break;
		default:{
			Temp_State = Temp_init;
			break;
		}
	}
}

void TempTask(){
	Temp_Init();
	for(;;){
		Temp_Tick();
		vTaskDelay(5500);
	}
}

void StartSecPulse3(unsigned portBASE_TYPE Priority)
{
	xTaskCreate(TempTask, (signed portCHAR *)"Temp Task", configMINIMAL_STACK_SIZE, NULL, Priority, NULL );
}

void QueueSend_Init(){
	Queue_State = QueueSend_init;
}
void Queue_Tick(){
	switch(Queue_State){
		case QueueSend_init:
			SPI_MasterTransmit(char_to_send->front);
			_delay_ms(500);
		break;
		default:{
			break;
		}
	}
	switch(Queue_State){
		case QueueSend_init:
			Queue_State = QueueSend_init;
			break;
		default:{
			Queue_State = QueueSend_init;
			break;
		}
	}
}

void QueueTask(){
	QueueSend_Init();
	for(;;){
		Queue_Tick();
		vTaskDelay(0);
	}
}

void StartSecPulse4(unsigned portBASE_TYPE Priority)
{
	xTaskCreate(QueueTask, (signed portCHAR *)"Queue Task", configMINIMAL_STACK_SIZE, NULL, Priority, NULL );
}

void calibrate(){
	float xval[100] = {0};
	float yval[100] = {0};
	float zval[100] = {0};
	float sumx = 0;
	float sumy = 0;
	float sumz = 0;
	for(int i=0; i<100; i++)
	{
		xval[i] = read_adxl335_x_value(0);
		sumx = sumx + xval[i];
	}
	_delay_ms(100);
	for(int j=0; j<100; j++)
	{
		yval[j] = read_adxl335_y_value(1);
		sumy = sumy + yval[j];
	}
	_delay_ms(100);
	for(int k=0; k<100; k++)
	{
		zval[k] = read_adxl335_z_value(2);
		sumz = sumz + zval[k];
	}
	_delay_ms(100);
	xavg = sumx/100;
	yavg = sumy/100;
	zavg = sumz/100;
}

int main(void)
{
	DDRA = 0x00; PORTA = 0xFF;
	DDRB = 0xFF; PORTB = 0x00;
	DDRC = 0xFF; PORTC = 0x00;
	DDRD = 0xFF; PORTD = 0x00;
	
	adc_init();
	calibrate();			//calibrates the accelerometer
	QueueInit(255);
	
	
	
	// 	int x_axis, y_axis, z_axis, accelvec;
	// 	char accelv[16];
	/*	adc_init();*/
	LCD_init();
	SPI_MasterInit();
	//	Accel_Init();
	
	// 	Master_Init();
	// 	Temp_Init();
	StartSecPulse1(1);
	StartSecPulse2(1);
	StartSecPulse3(2);
	//StartSecPulse4(3);
	vTaskStartScheduler();
	while(1)
	{
	}
}
