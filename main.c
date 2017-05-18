#include "mcc_generated_files/mcc.h"
#include "ff.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

FATFS FatFs;	/* FatFs work area needed for each volume */
FIL Fil;		/* File object needed for each open file */

void main(void) {

	SYSTEM_Initialize();

    T0CON  =    0B10000100;
    ADCON0 =    0b00000111;
    ADCON1 =    0b00000000;
    ADCON2 =    0b10000000;
    
    TRISCbits.TRISC6 = 0;  // tx1 output
    PIE3bits.RC2IE = 0;     // disable Rx  interupt
    PIE3bits.TX2IE = 0;
    PIE1bits.RC1IE = 0;
    PIE1bits.TX1IE = 0;

    TXSTA1   = 0b10100000;  //set alle benodige waarden     hex:0xA0
    RCSTA1   = 0b10000000;  //                                  0x00
    BAUDCON1 = 0b11000000;  //                                  0xb0
    SPBRG1   = 51;  //                                  0x33  decimaal 51
    
    INTCONbits.TMR0IE = 1;
    INTCONbits.TMR0IF = 0;
    
    ei();

	while (1);
}

void interrupt myIsr(void) {
    
    void blinkLight(void);
    void write(float temp);
    int Analog_Digital_Convertor(void);
    void newValue(float newValue);   
    
    if(INTCONbits.TMR0IE && INTCONbits.TMR0IF) {
        INTCONbits.TMR0IF = 0;
        TMR0H = 11;
        TMR0L = 220;
        
        float temp = Analog_Digital_Convertor() * 0.122;
        
        printf("%.2f\n", temp);
        
        blinkLight();
        write(temp);
        
        newValue(Analog_Digital_Convertor());
    }
}

int Analog_Digital_Convertor(void) {
    ADCON0bits.nDONE = 1;
    return ADRESH<<8|ADRESL;
}

int secondsPassed, minutesPassed;

float currentValue;
float secondValues[60];
float minuteValues[60];
float hourValues[24];

void newValue(float newValue) {   

    void shiftArray(float array[], int arrayLength);
    float avg(float array[], int arrayLength);
    
    currentValue = newValue;
    
    secondsPassed++;
    shiftArray(secondValues, 60);
    secondValues[0] = newValue;
    if(secondsPassed > 60) {
        secondsPassed = 0;
        minutesPassed++;
    } else {
        return;
    }
    
    shiftArray(minuteValues, 60);
    minuteValues[0] = avg(secondValues, 60);
    if(minutesPassed > 60) {
        minutesPassed = 0;
    } else {
        return;
    }
    
    shiftArray(hourValues, 24);
    hourValues[0] = avg(minuteValues, 60);
}

void shiftArray(float array[], int arrayLength) {
    for (int k = arrayLength; k > 0; k--) 
        array[k]=array[k-1];
}

float avg(float array[], int arrayLength) {
    float total;
    for(float f = 0; f < arrayLength; f++)
        total += f;
    return total / arrayLength;
}

void blinkLight(void) {
    LATCbits.LATC2 = !LATCbits.LATC2;
}

void write(float value) {

    UINT bw;
    if (f_mount(&FatFs, "", 1) == FR_OK) {	/* Mount SD */
		if (f_open(&Fil, "test.txt", FA_OPEN_ALWAYS | FA_READ | FA_WRITE) == FR_OK) {	/* Open or create a file */
			if ((Fil.fsize != 0) && (f_lseek(&Fil, Fil.fsize) != FR_OK)) goto endSD;	/* Jump to the end of the file */
			f_write(&Fil, "TEST\r\n", 54, &bw);	/* Write data to the file */
			endSD: f_close(&Fil);								/* Close the file */
		}
	}
}

void putch (char c){
    while(TXSTA1bits.TRMT== 0);
    TXREG1 = c;
}