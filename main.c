#include "mcc_generated_files/mcc.h"
#include "ff.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

FATFS FatFs;	/* FatFs work area needed for each volume */
FIL Fil;		/* File object needed for each open file */

/*
 * Initialiseert alle registers op de microcontroller
 */
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

int timePassed;

float secondValues[60];
float minuteValues[60];
float hourValues[24];

void interrupt myIsr(void) {

    if(INTCONbits.TMR0IE && INTCONbits.TMR0IF) { //elke seconde
        INTCONbits.TMR0IF = 0;
        TMR0H = 11;
        TMR0L = 220;
        
        int Analog_Digital_Convertor(void);
        float temp = Analog_Digital_Convertor() * 0.122;

        void blinkLight(void);
        blinkLight();
        
        void newValue(float newValue);
        newValue(temp);

        void writeCSV(float temp);
        float avg(float array[], int arraySize);      
        writeCSV(avg(secondValues, 60));
        
        void printSerial(float value);
        printSerial(temp);
        
        timePassed++;
    }  
    void minutePassed(void);
    
    if(timePassed % 60 == 0)
        minutePassed();
}

/*
 * Mapt een spanning van 0-5v op AN1 om naar een waarde van 0-4096 (12-bit ADC)
 */
int Analog_Digital_Convertor(void) {
    ADCON0bits.nDONE = 1;
    return ADRESH<<8|ADRESL;
}

int secondCount, minuteCount;

float currentValue;

/*
 * Wordt elke second gecallt met als parameter de waarde van de ADC.
 * Zorgt er voor dat de waarden van de afgelopen seconde, minuut, uur en dag
 * in een array wordt opgeslagen.
 */
void newValue(float newValue) {   

    void shiftArray(float array[], int arraySize);
    float avg(float array[], int arraySize);

    currentValue = newValue;
    
    secondCount++;
    shiftArray(secondValues, 60);
    secondValues[0] = newValue;
    if(secondCount > 60) {
        secondCount = 0;
        minuteCount++;
    } else {
        return;
    }
    
    shiftArray(minuteValues, 60);
    minuteValues[0] = avg(secondValues, 60);
    if(minuteCount > 60) {
        minuteCount = 0;
    } else {
        return;
    }
    
    shiftArray(hourValues, 24);
    hourValues[0] = avg(minuteValues, 60);
}

/*
 * Schuift alle waarden in een array één op naar "rechts"
 */
void shiftArray(float array[], int arraySize) {
    for (int k = arraySize; k > 0; k--) 
        array[k]=array[k-1];
}

/*
 * Returnt de gemiddelde waarde van een array
 * Als een element van een array 0 is, wordt
 * deze overgeslagen en niet meegenomen in het gemiddelde
 */
float avg(float array[], int arraySize) {
    float total = 0;
    int skip;
    for(int i = 0; i < arraySize; i++) {
        total += array[i];
        if(array[i] < 0.005 && array[i] > -0.005)
            skip++;
    }
    return total / (arraySize - skip++);
}

/*
 * Fuctie wordt gecallt als er een minuut voorbij is gegaan
 */
void minutePassed(void) {
    printf("MINUTE PASSED\r\n");
}

/*
 * Verandert de status van de LED die op C2 is aangesloten
 */
void blinkLight(void) {
    LATCbits.LATC2 = !LATCbits.LATC2;
}

/*
 * Opent een bestand genaamd "test.csv" op de sd kaart en voegt een regel
 * er aan toe met daarin @value
 */
void writeCSV(float value) {

    char str[10];

    sprintf(str, "%2.2f C,\r\n", value);  

    UINT bw;
    if (f_mount(&FatFs, "", 1) == FR_OK) {	/* Mount SD */
		if (f_open(&Fil, "test.csv", FA_OPEN_ALWAYS | FA_READ | FA_WRITE) == FR_OK) {	/* Open or create a file */
			if ((Fil.fsize != 0) && (f_lseek(&Fil, Fil.fsize) != FR_OK)) goto endSD;	/* Jump to the end of the file */
			f_write(&Fil, str, sizeof(str), &bw);	/* Write data to the file */
			endSD: f_close(&Fil);								/* Close the file */
		}
	}
}

/*
 * Verstuurt een float waarde over USB als een string
 */
void printSerial(float value) {
    printf("%.2f\r\n", value);
}

/*
 * Functie is nodig om seriele waarden te versturen over de seriele lijn
 */
void putch (char c){
    while(TXSTA1bits.TRMT== 0);
    TXREG1 = c;
}