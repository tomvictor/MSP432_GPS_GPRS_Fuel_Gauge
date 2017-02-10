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
#include <string.h>
#include <driverlib.h>
#include "HAL_I2C.h"
#include "HAL_UART.h"
#include "HAL_BQ27441.h"
#include "ctype.h"

#define RedLedPin        GPIO_PIN0
#define RedLedPort       GPIO_PORT_P2

#define GreenLedPin      GPIO_PIN1
#define GreenLedPort     GPIO_PORT_P1

#define BleStatusPin     GPIO_PIN6
#define BleStatusPort    GPIO_PORT_P1

#define BleSwitchPin     GPIO_PIN0
#define BleSwitchPort    GPIO_PORT_P3

#define PowerKeyPin      GPIO_PIN6
#define PowerKeyPort     GPIO_PORT_P3



void CS_init(void);
void GPIO_init(void);
char *itoa(int, char*, int);
void gprsInit(void);
void gsmInit(void)  ;
void gpsread(void)  ;
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


char gpsValid[]={"$GPRMC,182708.000,A,1018.8587,N,07614.6607,E,5.15,92.25,280916,,,A*5A"
                 "$GPVTG,92.25,T,,M,5.15,N,9.55,K,A*09"
                 "$GPGGA,182708.000,1018.8587,N,07614.6607,E,1,4,6.38,71.2,M,-93.3,M,,*48"
                 "$GPGSA,A,3,26,08,22,03,,,,,,,,,10.77,6.38,8.68*37"
                 "$GPGSV,2,1,07,44,72,226,,22,68,211,16,03,68,271,31,27,32,121,29*73"
                 "$GPGSV,2,2,07,08,26,164,15,26,26,034,29,193,,,*42"
                 "$GPGLL,1018.8587,N,07614.6607,E,182708.000,A,A*54"
                 "$GPTXT,01,01,02,ANTSTATUS=OPEN*2B"} ;

char rootUrl[] ={"http://kranioz.com/"}, test[] = {"log/?lat=value1&lng=99&id=9&bat=89"}, getUrl[] ;

//battery variables



int idx = 0;
char c, Range = 0 ;
char gps_string[200];
char temp[100], temp2[100] ;


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
        UART_transmitString("Error initializing BQ27441 Config\r\n");
        UART_transmitString("Make sure BOOSTXL-BATPAKMKII is connected and switch is flipped to \"CONNECTED\"\r\n");
    }

    if (!BQ27441_initOpConfig())
    {
        __delay_cycles(1000000);
        UART_transmitString("Clearing BIE in Operation Configuration\r\n");
        GPIO_setOutputHighOnPin(GPIO_PORT_P1,GPIO_PIN0) ;
        __delay_cycles(3000000);
        GPIO_setOutputLowOnPin(GPIO_PORT_P1,GPIO_PIN0) ;
    }

    BQ27441_control(BAT_INSERT, 1000);
    __delay_cycles(1000000);


    gsmInit()   ;

    /* Display Battery information */
    while(1)
    {

        unsigned char t =0, lat = 1, lng = 2, bat = 3, status = 4;

        //temp2[100]={0};
        //memset

        sprintf(temp2,"%slog/?lat=%d&lng=%d&id=921&bat=%d&status=%d\n\r",rootUrl,lat,lng,bat,rand()) ;
        serialTx1(temp2)    ;

        //sprintf(temp, "AT+QHTTPURL=%d,30\r\n", sizeof(temp2));
        serialTx1(temp)    ;



        //
        //        short result16 = 0;
        //        char str[64];
        //
        //        char battState[10] = 0, battCap[10] = 0 ;
        //
        //
        //        /* Read Remaining Capacity */
        //        if(!BQ27441_read16(REMAINING_CAPACITY, &result16, 1000))
        //            UART_transmitString("Error Reading Remaining Capacity \r\n");
        //        else
        //        {
        //            sprintf(str, "REMAINING_CAPACITY : %dmAh \r\n", result16);
        //            //battCap = result16 ;
        //            sprintf(battCap, "%dmAh", result16);
        //
        //            UART_transmitString(str);
        //        }
        //
        //        /* Read State Of Charge */
        //        if(!BQ27441_read16(STATE_OF_CHARGE, &result16, 1000))
        //            UART_transmitString("Error Reading State Of Charge \r\n");
        //        else
        //        {
        //            sprintf(str, "State of Charge: %d%%\r\n", (unsigned short)result16);
        //            sprintf(battState, "%d%%",  (unsigned short)result16);
        //            UART_transmitString(str);
        //        }
        //
        //
        //        /* Read Voltage */
        //        if(!BQ27441_read16(VOLTAGE, &result16, 1000))
        //            UART_transmitString("Error Reading Voltage \r\n");
        //        else
        //        {
        //            sprintf(str, "Voltage: %dmV\r\n", result16);
        //            UART_transmitString(str);
        //        }
        //
        //        /* Read Average Current */
        //        if(!BQ27441_read16(AVERAGE_CURRENT, &result16, 1000))
        //            UART_transmitString("Error Reading Average Current \r\n");
        //        else
        //        {
        //            sprintf(str, "Average Current: %dmA\r\n", result16);
        //            UART_transmitString(str);
        //            if (result16 > 0) {
        //                UART_transmitString("Status : charging\r\n");
        //            } else {
        //                UART_transmitString("Status : discharging\r\n");
        //            }
        //        }
        //



        Range = GPIO_getInputPinValue(BleStatusPort,BleStatusPin) ;
        if (Range == 1){
            //device is in range, so turn on green led and turn off red led
            GPIO_setOutputHighOnPin(GreenLedPort,GreenLedPin) ; //turn on green led(p2.1)
            GPIO_setOutputLowOnPin(RedLedPort,RedLedPin) ; //turn off red led(p1.0)
        }
        else if(Range == 0){
            //device is out of range, so turn off green led and turn on red led
            GPIO_setOutputLowOnPin(GreenLedPort,GreenLedPin) ; //turn off green led(p2.1)
            GPIO_setOutputHighOnPin(RedLedPort,RedLedPin) ; //turn on red led(p1.0)
        }




        //serialTx0("tom\n\r")    ;
        //serialTx1("tom2\n\r")   ;

        //print echo on  ports
        //UART_transmitData(EUSCI_A0_BASE,UART_receiveData(EUSCI_A0_BASE));
        //UART_transmitData(EUSCI_A2_BASE,UART_receiveData(EUSCI_A2_BASE));


        //gprsInit();
        //serialTx0(gpsValid)    ;
        //serialTx0("\n\r")   ;
        //serialTx0(gps_string);

        //char rx ;

        //UART_transmitData(EUSCI_A0_BASE,UART_receiveData(EUSCI_A0_BASE));
        //__delay_cycles(20000000);
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

    GPIO_setAsOutputPin(BleSwitchPort, BleSwitchPin); //ble switch as output
    GPIO_setAsInputPin(BleStatusPort, BleStatusPin) ; //Ble status pin as input

    GPIO_setAsOutputPin(RedLedPort, RedLedPin); //red led pin as output
    GPIO_setAsOutputPin(GreenLedPort, RedLedPin);   //green led pin as output

    GPIO_setAsOutputPin(PowerKeyPort,PowerKeyPin)   ; //power key pin as output


}





void gprsInit(){

    //char temp[81]   ;

    serialTx1(QIFGCNT)    ;
    serialTx1(QICSGP) ;
    serialTx1(CMNET)  ;
    serialTx1(QIREGAPP)   ;
    serialTx1(QIACT);

    //sprintf(temp, "%d%%",  (unsigned short)result16);

    serialTx1("AT+QHTTPURL=51,30")    ;



    serialTx1(getUrl) ;

    serialTx1("AT+QHTTPGET=10")   ;
    serialTx1(QIDEACT)    ;
}

void gsmInit(){
    serialTx1(at) ;
    serialTx1(ATV1)   ;
    serialTx1(ATE1)   ;
    serialTx1(CPIN)   ;
    serialTx1(CSQ)    ;
    serialTx1(CREG)   ;
    serialTx1(CGREG)  ;
    serialTx1(COPS)   ;
}

