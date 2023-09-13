/**
  Generated Main Source File

  Company:
    Microchip Technology Inc.

  File Name:
    main.c

  Summary:
    This is the main file generated using PIC10 / PIC12 / PIC16 / PIC18 MCUs

  Description:
    This header file provides implementations for driver APIs for all modules selected in the GUI.
    Generation Information :
        Product Revision  :  PIC10 / PIC12 / PIC16 / PIC18 MCUs - 1.81.8
        Device            :  PIC18F46K42
        Driver Version    :  2.00
*/

/*
    (c) 2018 Microchip Technology Inc. and its subsidiaries. 
    
    Subject to your compliance with these terms, you may use Microchip software and any 
    derivatives exclusively with Microchip products. It is your responsibility to comply with third party 
    license terms applicable to your use of third party software (including open source software) that 
    may accompany Microchip software.
    
    THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER 
    EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY 
    IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS 
    FOR A PARTICULAR PURPOSE.
    
    IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, 
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND 
    WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP 
    HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO 
    THE FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL 
    CLAIMS IN ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT 
    OF FEES, IF ANY, THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS 
    SOFTWARE.
*/

// Inclusión de las bibliotecas necesarias
#include <xc.h>
#include "mcc_generated_files/mcc.h"
#include <stdlib.h>


// Declaración de variables globales para almacenar los valores de las señales
int16_t signal_DC_1 = 0;
int16_t signal_DC_2 = 0;
uint16_t signal_No_DC_1 = 0;
uint16_t signal_No_DC_2 = 0;

// Variables para almacenar los resultados de las operaciones
int16_t sum;
int16_t substract;
int16_t absValue1;
int16_t absValue2;
int32_t multiply;
int16_t result;
int16_t resulOperacion;

//variables para el DAC
uint16_t DAC_SPI = 0;
uint8_t DAC_SPI_High = 0;
uint8_t DAC_SPI_Low = 0;
uint8_t selector = 0;

static uint8_t char_hexa[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};


int16_t AddSignals(void) {
    result = signal_No_DC_1 + signal_No_DC_2;
    return result >> 1;
}

int16_t SubstractSignals(void) {
    result = signal_No_DC_1 - signal_No_DC_2;
    return result >> 1;
}

int32_t MultiplySignals(void) {
    
    uint8_t isNegative = 0;
    
    // Check for negative numbers and adjust sign
    if (signal_No_DC_1 < 0) {
        isNegative ^= 1;
        signal_No_DC_1 = -signal_No_DC_1;
    }
    if (signal_No_DC_2 < 0) {
        isNegative ^= 1;
        signal_No_DC_2 = -signal_No_DC_2;
    }

    uint8_t num1Low = (uint8_t)(signal_No_DC_1 & 0xFF);
    uint8_t num1High = (uint8_t)((uint16_t)signal_No_DC_1 >> 8);
    uint8_t num2Low = (uint8_t)(signal_No_DC_2 & 0xFF);
    uint8_t num2High = (uint8_t)((uint16_t)signal_No_DC_2 >> 8);
    
    uint8_t resultParts[4];

    // Multiply low bytes
    PRODL = num1Low;
    WREG = num2Low;
    asm("MULWF _PRODL");
    resultParts[0] = PRODL;
    resultParts[1] = PRODH;

    // Multiply high bytes
    PRODL = num1High;
    WREG = num2High;
    __asm__ volatile("MULWF _PRODL");
    resultParts[2] = PRODL;
    resultParts[3] = PRODH;

    // Multiply low byte of num1 with high byte of num2
    PRODL = num1Low;
    WREG = num2High;
    asm("MULWF _PRODL");
    resultParts[1] += PRODL;
    resultParts[2] += PRODH;

    // Multiply high byte of num1 with low byte of num2
    PRODL = num1High;
    WREG = num2Low;
    asm("MULWF _PRODL");
    resultParts[1] += PRODL;
    resultParts[2] += PRODH;
    
    // Combine results into a 32-bit integer
    int32_t combinedResult = ((int32_t)resultParts[3] << 24) | ((int32_t)resultParts[2] << 16) | ((int32_t)resultParts[1] << 8) | resultParts[0];
    
    // Adjust for negative result if needed
    combinedResult = isNegative ? -combinedResult : combinedResult;

    // Escalizar a 12 bits
    return combinedResult >> 11;
    //return combinedResult + 0b0000011111111111;
}

int16_t AbsSignal1(void) {
    result = abs(signal_No_DC_1);
    return result;
}

int16_t AbsSignal2(void) { 
    result = abs(signal_No_DC_2);
    return result;
}


void UART_WriteString(const char *str) {
    while (*str) {
        UART1_Write(*str++);
    }
}

void realizarOperacionesYEnviar() {
    sum = AddSignals();
    substract = SubstractSignals();
    absValue1 = AbsSignal1();
    absValue2 = AbsSignal2();
    multiply = MultiplySignals();
    //IO_RD0_SetHigh();
    //IO_RD0_SetLow();
    //char buffer[150];
    //sprintf(buffer,"señal 1 con offset: %d :señal 2 con offset: %d\n", signal_DC_1, signal_DC_2);
    //sprintf(buffer, "signal 1: %d, signal 2: %d, Sum: %d, Subtract: %d, Abs 1: %d, Abs 2: %d, Multiply: %d\n", signal_No_DC_1, signal_No_DC_2, sum, substract, absValue1, absValue2, multiply);
    //UART_WriteString(buffer);
}


void leerSegundaSenal(){
    signal_DC_2 = ADCC_GetSingleConversion(channel_ANA1);
    signal_No_DC_2 = signal_DC_2 - 0b0000011111111111;//offset de 1v
}

void leerPrimeraSenal(){
    signal_DC_1 = ADCC_GetSingleConversion(channel_ANA0);
    signal_No_DC_1 = signal_DC_1 - 0b0000011111111111;//offset de 1v
}

void sendToUART(){
    
    uint8_t mostrarBajo = /*(uint8_t)*/(signal_No_DC_1 & 0xFF);
    uint8_t mostrarAlto = /*(uint8_t)*/(signal_No_DC_1 >> 8);
    
    UART1_Write(char_hexa[mostrarAlto >> 4]);
    UART1_Write(char_hexa[mostrarAlto & 0B1111]);
    UART1_Write(char_hexa[mostrarBajo >> 4]);
    UART1_Write(char_hexa[mostrarBajo & 0B1111]);   
    UART1_Write('\n');
}

void main(void) {
    SYSTEM_Initialize();
    UART1_Initialize();
    SPI1_Open(SPI1_DEFAULT);
    
    while(1){
        //IO_RD0_SetHigh(); // Indicar inicio de transmisión
        //IO_RD1_SetHigh();
        leerPrimeraSenal();
        //IO_RD0_SetLow();
        
        //IO_RD1_SetHigh();
        leerSegundaSenal();
        //IO_RD1_SetLow();
        realizarOperacionesYEnviar();
        
        
        if(PIR3bits.U1RXIF){
            selector = UART1_Read();//lee dato recibido
            selector -= '0';
        }
        
        //sendToUART();
        switch(selector){
            case 1:
                sendToUART();
                resulOperacion = sum;
                break;
            case 2:
                sendToUART();
                resulOperacion = substract;
                break;
            case 3:
                sendToUART();
                resulOperacion = absValue1;
                break;
            case 4:
                sendToUART();
                resulOperacion = absValue2;
                break;
            case 5:
                sendToUART();
                resulOperacion = multiply;
                break;          
        }
        
        
        //OPERACIONES PARA DAC 1
        DAC_SPI = (uint16_t)(0b0011000000000000) | ((resulOperacion + 0b0000011111111111) /*& 0xFFF*/);//0b0011 selecciona dac 1
        DAC_SPI_High = (DAC_SPI>>8);//Toma 1 byte mas significativo de los 16 bits
        DAC_SPI_Low = (DAC_SPI /*& 0xFF*/);//Toma 1 byte menos significativo de los 16 bits para enviar al dac
        LATEbits.LATE0 = 0;//LOW para iniciar la transmisión SPI.
        LATBbits.LATB4 = 0;//LOW para actualizar la salida del DAC
        SPI1_ExchangeByte(DAC_SPI_High);
        SPI1_ExchangeByte(DAC_SPI_Low);
        LATEbits.LATE0 = 1;
        LATBbits.LATB4 = 1;
        
        //IO_RD0_SetHigh();
        //OPERACIONES PARA DAC 2
        DAC_SPI = (uint16_t)(0b1011000000000000) | (signal_No_DC_1/*resulOperacion & 0xFFF*/);
        DAC_SPI_High = (DAC_SPI>>8);//Tomar 1 byte MS del resultado
        LATEbits.LATE0 = 0;
        LATBbits.LATB4 = 0;
        SPI1_ExchangeByte(DAC_SPI_High);
        SPI1_ExchangeByte(DAC_SPI_Low);
        LATEbits.LATE0 = 1;
        LATBbits.LATB4 = 1;//estas operaciones se pueden cambiar con set high y set low
        //IO_RD0_SetLow();
         
        /*uint8_t mostrarBajo = (uint8_t)(resulOperacion & 0xFF);
        uint8_t mostrarAlto = (uint8_t)(resulOperacion >> 8) ;
        UART1_Write('\n');
        UART1_Write (mostrarAlto);
        UART1_Write (mostrarBajo);*/
    }
}
/**
 End of File
*/
