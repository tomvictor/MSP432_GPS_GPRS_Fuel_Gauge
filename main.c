//****************************************************************************
//
// main.c - MSP-EXP432P401R + Battery Boosterpack MkII - Fuel Guage demo
//
//          Initializes BQ27441 Fuel Gauge configurations suitable for the
//          included battery and periodically prints information to serial
//          backchannel uart.
//
//****************************************************************************

#include <stdlib.h>
#include <stdio.h>
#include <driverlib.h>
#include "HAL_I2C.h"
#include "HAL_UART.h"
#include "HAL_BQ27441.h"

void CS_init(void);
void GPIO_init(void);
char *itoa(int, char*, int);
void readBattery(void);
void initBattGauge(void)    ;


short result16 = 0;
char str[64] ;
unsigned int batt_percent,batt_remining, batt_voltage   ;

void main(void)
{
    /* Halting WDT and disabling master interrupts */
    MAP_WDT_A_holdTimer();
    MAP_Interrupt_disableMaster();

    GPIO_init();
    CS_init();

    __delay_cycles(1000000);

    /* Initialize I2C */
    I2C_initGPIO();
    I2C_init();

    /* Initialize UART */
    UART_initGPIO();
    /* Initialize UART for gps */
    GPSUART_initGPIO();
    /* Initialize UART for GPRS */
    GPSUART_initGPIO();

    UART_init();
    GPSUART_init();
    GPRSUART_init();

    __delay_cycles(1000000);


    UART_transmitStringGPS("This is the GPS Serial port test string\r\n");
    UART_transmitStringGPRS("This is the GPRS Serial port test string\r\n");


    initBattGauge()    ;

    /* Display Battery information */
    while(1)
    {
        readBattery();


    }
}


/* Initializes Clock System */
void CS_init()
{
    /* Set the core voltage level to VCORE1 */
    MAP_PCM_setCoreVoltageLevel(PCM_VCORE1);

    /* Set 2 flash wait states for Flash bank 0 and 1*/
    MAP_FlashCtl_setWaitState(FLASH_BANK0, 2);
    MAP_FlashCtl_setWaitState(FLASH_BANK1, 2);

    /* Initializes Clock System */
    MAP_CS_setDCOCenteredFrequency(CS_DCO_FREQUENCY_48);
    MAP_CS_initClockSignal(CS_MCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1 );
    MAP_CS_initClockSignal(CS_HSMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1 );
    MAP_CS_initClockSignal(CS_SMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1 );
    MAP_CS_initClockSignal(CS_ACLK, CS_REFOCLK_SELECT, CS_CLOCK_DIVIDER_1);
}


/* Initializes GPIO */
void GPIO_init()
{
    /* Terminate all GPIO pins to Output LOW to minimize power consumption */
    MAP_GPIO_setAsOutputPin(GPIO_PORT_PA, PIN_ALL16);
    MAP_GPIO_setAsOutputPin(GPIO_PORT_PB, PIN_ALL16);
    MAP_GPIO_setAsOutputPin(GPIO_PORT_PC, PIN_ALL16);
    MAP_GPIO_setAsOutputPin(GPIO_PORT_PD, PIN_ALL16);
    MAP_GPIO_setAsOutputPin(GPIO_PORT_PE, PIN_ALL16);
    MAP_GPIO_setOutputLowOnPin(GPIO_PORT_PA, PIN_ALL16);
    MAP_GPIO_setOutputLowOnPin(GPIO_PORT_PB, PIN_ALL16);
    MAP_GPIO_setOutputLowOnPin(GPIO_PORT_PC, PIN_ALL16);
    MAP_GPIO_setOutputLowOnPin(GPIO_PORT_PD, PIN_ALL16);
    MAP_GPIO_setOutputLowOnPin(GPIO_PORT_PE, PIN_ALL16);
}



void readBattery(void){
    /* Read Remaining Capacity */
          if(!BQ27441_read16(REMAINING_CAPACITY, &result16, 1000))
              UART_transmitString("Error Reading Remaining Capacity \r\n");
          else
          {
              sprintf(str, "REMAINING_CAPACITY : %dmAh \r\n", result16);
              batt_remining = result16 ;
              UART_transmitString(str);
          }

          /* Read State Of Charge */
          if(!BQ27441_read16(STATE_OF_CHARGE, &result16, 1000))
              UART_transmitString("Error Reading State Of Charge \r\n");
          else
          {
              sprintf(str, "State of Charge: %d%%\r\n", (unsigned short)result16);
              batt_percent = (unsigned short)result16 ;
              UART_transmitString(str);
          }

          /* Read Voltage */
          if(!BQ27441_read16(VOLTAGE, &result16, 1000))
              UART_transmitString("Error Reading Voltage \r\n");
          else
          {
              sprintf(str, "Voltage: %dmV\r\n", result16);
              batt_voltage = result16  ;
              UART_transmitString(str);
          }
          __delay_cycles(20000000);
}


void initBattGauge(void){
    if (!BQ27441_initConfig())
    {
        UART_transmitString("Error initializing BQ27441 Config\r\n");
        UART_transmitString("Make sure BOOSTXL-BATPAKMKII is connected and switch is flipped to \"CONNECTED\"\r\n");
    }

    while (!BQ27441_initOpConfig())
    {
        __delay_cycles(1000000);
        UART_transmitString("Clearing BIE in Operation Configuration\r\n");
    }

    BQ27441_control(BAT_INSERT, 1000);
    __delay_cycles(1000000);
}
