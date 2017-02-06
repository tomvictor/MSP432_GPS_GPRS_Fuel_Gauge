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
void gprsInit(void);
void gsmInit(void)  ;


char sleep[] = { "at+qsclk=1\n\r" },
        sleep_Check[]= { "at+qsclk?\n\r" },
        at[] = { "at\n\r"},ATV1[] = { "ATV1\n\r"},
        ATE1[] = { "ATE1\n\r"},
        cmee[] = { "AT+CMEE=2\n\r"},
        ipr[] = { "AT+IPR?\n\r"},
        ipr1[] = { "AT+IPR=115200\n\r"},
        CPIN[] = {"AT+CPIN?\n\r"},
        CSQ[]   = {"AT+CSQ\n\r"} ,
        CREG[]  = {"AT+CREG?\n\r"},
        CGREG[] = {"AT+CGREG?\n\r"},
        COPS[]  = {"AT+COPS?\n\r"},
        w[] = { "AT&W\n\r"},
        QIFGCNT[] = {"AT+QIFGCNT=0\n\r"},
        QICSGP[] = {"AT+QICSGP=1\n\r"} ,
        CMNET[] = {"CMNET\n\r"},
        QIREGAPP[] = {"AT+QIREGAPP\n\r"},
        QIACT[]    = {"AT+QIACT\n\r"},
        QIDEACT[] = {"AT+QIDEACT\n\r"} ;





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
    UART_init();

    __delay_cycles(1000000);





    if (!BQ27441_initConfig())
    {
        //UART_transmitString("Error initializing BQ27441 Config\r\n");
        //UART_transmitString("Make sure BOOSTXL-BATPAKMKII is connected and switch is flipped to \"CONNECTED\"\r\n");
        GPIO_setOutputHighOnPin(GPIO_PORT_P1,GPIO_PIN0) ;
    }

    while (!BQ27441_initOpConfig())
    {
        __delay_cycles(1000000);
        //UART_transmitString("Clearing BIE in Operation Configuration\r\n");
    }

    BQ27441_control(BAT_INSERT, 1000);
    __delay_cycles(1000000);

    /* Display Battery information */
    while(1)
    {

        //
        //		short result16 = 0;
        //		char str[64];
        //		UART_transmitString("\r\n");
        //		UART_transmitString("\r\n");
        //		UART_transmitString("*************************************\r\n");
        //		UART_transmitString("Battery Information\r\n");
        //		UART_transmitString("*************************************\r\n");
        //
        //		/* Read Design Capacity */
        //		if(!BQ27441_read16(DESIGN_CAPACITY, &result16, 1000))
        //			UART_transmitString("Error Reading Design Capacity \r\n");
        //		else
        //		{
        //			sprintf(str, "Design Capacity: %dmAh\r\n", result16);
        //			UART_transmitString(str);
        //		}
        //
        //		/* Read Remaining Capacity */
        //		if(!BQ27441_read16(REMAINING_CAPACITY, &result16, 1000))
        //			UART_transmitString("Error Reading Remaining Capacity \r\n");
        //		else
        //		{
        //			sprintf(str, "REMAINING_CAPACITY : %dmAh \r\n", result16);
        //			UART_transmitString(str);
        //		}
        //
        //		/* Read State Of Charge */
        //		if(!BQ27441_read16(STATE_OF_CHARGE, &result16, 1000))
        //			UART_transmitString("Error Reading State Of Charge \r\n");
        //		else
        //		{
        //			sprintf(str, "State of Charge: %d%%\r\n", (unsigned short)result16);
        //		    //sprintf(str, "(%d%%)\r\n", (unsigned short)result16);
        //			UART_transmitString(str);
        //		}
        //
        //		/* Read Temperature */
        //		if(!BQ27441_read16(TEMPERATURE, &result16, 1000))
        //			UART_transmitString("Error Reading Temperature \r\n");
        //		else
        //		{
        //			sprintf(str, "Temperature: %dC\r\n", result16/10 - 273);
        //			UART_transmitString(str);
        //		}
        //
        //		/* Read Voltage */
        //		if(!BQ27441_read16(VOLTAGE, &result16, 1000))
        //			UART_transmitString("Error Reading Voltage \r\n");
        //		else
        //		{
        //			sprintf(str, "Voltage: %dmV\r\n", result16);
        //			UART_transmitString(str);
        //		}
        //
        //		/* Read Average Current */
        //		if(!BQ27441_read16(AVERAGE_CURRENT, &result16, 1000))
        //			UART_transmitString("Error Reading Average Current \r\n");
        //		else
        //		{
        //			sprintf(str, "Average Current: %dmA\r\n", result16);
        //			UART_transmitString(str);
        //			if (result16 > 0) {
        //				UART_transmitString("Status : charging\r\n");
        //			} else {
        //				UART_transmitString("Status : discharging\r\n");
        //			}
        //		}


        gsmInit();
        gprsInit();
        serialTx1("tom\n\r")    ;
        __delay_cycles(20000000);
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





void gprsInit(){

    char getUrl[] = {"/log/?lat=value1&lng=99&id=99&bat=99"} ;


    serialTx0(QIFGCNT)    ;
    serialTx0(QICSGP) ;
    serialTx0(CMNET)  ;
    serialTx0(QIREGAPP)   ;
    serialTx0(QIACT);

    serialTx0("AT+QHTTPURL=51,30")    ;

    //getUrl = "/log/?lat=value1&lng=99&id=99&bat=99";

    serialTx0(getUrl) ;

    serialTx0("AT+QHTTPGET=10")   ;
    serialTx0(QIDEACT)    ;
}

void gsmInit(){
    serialTx0(at) ;
    serialTx0(ATV1)   ;
    serialTx0(ATE1)   ;
    serialTx0(CPIN)   ;
    serialTx0(CSQ)    ;
    serialTx0(CREG)   ;
    serialTx0(CGREG)  ;
    serialTx0(COPS)   ;
}
