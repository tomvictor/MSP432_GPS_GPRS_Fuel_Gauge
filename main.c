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


#define RedLedPin        GPIO_PIN0
#define RedLedPort       GPIO_PORT_P1

#define GreenLedPin      GPIO_PIN1
#define GreenLedPort     GPIO_PORT_P2

#define BlueLedPin      GPIO_PIN2
#define BlueLedPort     GPIO_PORT_P2

#define BleStatusPin     GPIO_PIN6
#define BleStatusPort    GPIO_PORT_P1

#define BleSwitchPin     GPIO_PIN0
#define BleSwitchPort    GPIO_PORT_P3

#define PowerKeyPin      GPIO_PIN6
#define PowerKeyPort     GPIO_PORT_P3

#define GpsStatusPin    GPIO_PIN5
#define GpsStatusPort   GPIO_PORT_P1

#define GpsLatPin       GPIO_PIN1
#define GpsLatPort     GPIO_PORT_P4

#define GpsLngPin   GPIO_PIN3
#define GpsLngPort   GPIO_PORT_P4



void CS_init(void);
void GPIO_init(void);
char *itoa(int, char*, int);
void gprsInit(void);
void gsmInit(void)  ;
void gpsread(void)  ;
void LogToServer(void)  ;

void ProcessResponse(void);
void GetLocation(void)  ;
void PowerCheck(void);
void PowerOnGprs(void)  ;
void BattInitFn(void)   ;
void BleReq(void)   ;

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
        QICSGP[] = {"AT+QICSGP=1,\"bsnlnet\"\n\r"} ,
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

char rootUrl[] ={"http://kranioz.com/"}, getUrl[500] ;

//battery variables

unsigned char tempAt[10];
int aCount =0;

int idx = 0;
char c, Range = 0 ;
char gps_string[200];
char temp[100] ;


unsigned char t = 0, lat = 1, lng = 2, bat = 3, status = 1;
int j=0, q=0, i=0;

char latc[20],lngc[20]    ;

char battState[10], battCap[10];


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


    BatInitFn();
    PowerOnGprs(); //power on the Gprs module by giving high to low on power key
    gsmInit();  //Sending gprs init functions after the initialization


    /* Display Battery information */
    while(1)
    {

        strcpy(getUrl,"0") ; //now getUrl will filled with none
        strcpy(latc,"0") ; //now getUrl will filled with none
        strcpy(lngc,"0") ; //now getUrl will filled with none
        strcpy(temp,"0") ; //now getUrl will filled with none
        strcpy(battState,"0") ; //now getUrl will filled with none

        short result16 = 0;
        char str[64];



        __delay_cycles(1000000);
        /* Read Remaining Capacity */
        if(!BQ27441_read16(REMAINING_CAPACITY, &result16, 1000))
            UART_transmitString("Error Reading Remaining Capacity \r\n");
        else
        {
            sprintf(battCap, "%dmAh", result16);
            __delay_cycles(1000000);

        }

        __delay_cycles(1000000);
        /* Read State Of Charge */
        if(!BQ27441_read16(STATE_OF_CHARGE, &result16, 1000))
            UART_transmitString("Error Reading State Of Charge \r\n");
        else
        {
            sprintf(battState, "%d%%",  (unsigned short)result16);
            __delay_cycles(1000000);
        }

        //        serialTx0(battState);


        BleReq();


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



        //gprs code starts here

        //temp2[100]={0};
        //memset


        __delay_cycles(4000000);

        sprintf(getUrl,"%slog/?lat=%s&lng=%s&id=921&bat=%s&status=%d",rootUrl,latc,lngc,battState,Range) ;
        //serialTx0(getUrl) ;
        //    serialTx1(temp2)    ;

        //    strcat(dest, src); appending


        //for counting the length of the string
        for(j=0;getUrl[j] > 0; j++){
            __delay_cycles(100);
        }


        //gprsinit function

        serialTx1(QIFGCNT)    ;
        __delay_cycles(10000000); //nearly 3 seconds
        serialTx1(QICSGP) ;
        __delay_cycles(10000000); //nearly 3 seconds
        serialTx1(QIREGAPP)   ;
        __delay_cycles(10000000); //nearly 3 seconds
        serialTx1(QIACT);
        __delay_cycles(20000000); //nearly 3 seconds


        sprintf(temp, "AT+QHTTPURL=%d,30\r\n", j);

        serialTx1(temp)    ; //printing above

        __delay_cycles(30000000); //nearly 3 seconds

        serialTx1(getUrl)    ;   //printing the get data

        __delay_cycles(20000000); //nearly 3 seconds

        serialTx1("AT+QHTTPGET=60\r\n")   ;
        __delay_cycles(10000000); //nearly 3 seconds
        serialTx1("AT+QHTTPREAD=30\r\n")   ;
        __delay_cycles(30000000); //nearly 3 seconds


        __delay_cycles(150000000); //nearly 15 seconds

        serialTx1(QIDEACT)    ;
              __delay_cycles(30000000); //nearly 3 seconds
              //gprs code ends here


        //serialTx0("tom\n\r")    ;
        //serialTx1("tom2\n\r")   ;

        //print echo on  ports
        //UART_transmitData(EUSCI_A0_BASE,UART_receiveData(EUSCI_A0_BASE));
        //UART_transmitData(EUSCI_A2_BASE,UART_receiveData(EUSCI_A2_BASE));


              __delay_cycles(150000000); //nearly 15 seconds
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
    MAP_CS_initClockSignal(CS_ACLK, CS_REFOCLK_SELECT, CS_CLOCK_DIVIDER_64);

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

    MAP_GPIO_setOutputHighOnPin(PowerKeyPort, PIN_ALL8);
    //    MAP_GPIO_setOutputLowOnPin(GPIO_PORT_PB, PIN_ALL16);
    //    MAP_GPIO_setOutputLowOnPin(GPIO_PORT_PC, PIN_ALL16);
    MAP_GPIO_setOutputLowOnPin(GPIO_PORT_PD, PIN_ALL16);
    MAP_GPIO_setOutputLowOnPin(GPIO_PORT_PE, PIN_ALL16);


    GPIO_setAsOutputPin(BleSwitchPort, BleSwitchPin); //ble switch as output
    GPIO_setAsInputPin(BleStatusPort, BleStatusPin) ; //Ble status pin as input

    GPIO_setAsOutputPin(RedLedPort, RedLedPin); //red led pin as output
    GPIO_setAsOutputPin(GreenLedPort, GreenLedPin);   //green led pin as output
    GPIO_setAsOutputPin(BlueLedPort, BlueLedPin);   //Blue led pin as output

    GPIO_setAsOutputPin(PowerKeyPort,PowerKeyPin)   ; //power key pin as output

    GPIO_setAsOutputPin(GpsLatPort,GpsLatPin); //latitude pin as output
    GPIO_setAsOutputPin(GpsLngPort,GpsLngPin); //longitude pin as output
    GPIO_setAsInputPin(GpsStatusPort,GpsStatusPin)  ; // gps status pin as input


}


void BatInitFn(void){

    if (!BQ27441_initConfig())
    {
        UART_transmitString("Error initializing BQ27441 Config\r\n");
        UART_transmitString("Make sure BOOSTXL-BATPAKMKII is connected and switch is flipped to \"CONNECTED\"\r\n");
    }

    if (!BQ27441_initOpConfig())
    {
        __delay_cycles(1000000);
        UART_transmitString("Clearing BIE in Operation Configuration\r\n");
        // GPIO_setOutputHighOnPin(GPIO_PORT_P1,GPIO_PIN0) ;
        __delay_cycles(3000000);
        // GPIO_setOutputLowOnPin(GPIO_PORT_P1,GPIO_PIN0) ;
    }

    BQ27441_control(BAT_INSERT, 1000);
    __delay_cycles(1000000);




}

void BleReq(void){
    GPIO_setOutputLowOnPin(BleSwitchPort,BleSwitchPin) ;
    __delay_cycles(10000000);
    GPIO_setOutputHighOnPin(BleSwitchPort,BleSwitchPin) ;

    __delay_cycles(10000000); //wait for 1 Second
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


void PowerOnGprs(void){
    //power key
    // UART_transmitData(EUSCI_A2_BASE,UART_receiveData(EUSCI_A2_BASE));

    GPIO_setOutputLowOnPin(PowerKeyPort,PowerKeyPin) ;
    __delay_cycles(50000000); //nearly 4 seconds
    GPIO_setOutputHighOnPin(PowerKeyPort,PowerKeyPin) ;

    __delay_cycles(180000000); //wait for initialisation
}

void PowerCheck(void){
    serialTx1(at);

    __delay_cycles(50000000); //4 sec delet
}


void LogToServer(void){



    //temp2[100]={0};
    //memset

    //    strcpy(temp2,"0") ; //now temp2 will filled with none
    //    for(j=0;temp2[j] > 0; j++){
    //       }

    //serialTx1(temp2)    ; //testing

    sprintf(getUrl,"%slog/?lat=%s&lng=%d&id=921&bat=%d&status=%d",rootUrl,lat,lng,bat,status) ;
    serialTx0(getUrl) ;
    //    serialTx1(temp2)    ;

    //    strcat(dest, src); appending


    //for counting the length of the string
    for(j=0;getUrl[j] > 0; j++){
        __delay_cycles(100);
    }


    //gprsinit function

    serialTx1(QIFGCNT)    ;
    __delay_cycles(1000000);
    serialTx1(QICSGP) ;
    __delay_cycles(1000000);
    serialTx1(CMNET)  ;
    __delay_cycles(1000000);
    serialTx1(QIREGAPP)   ;
    __delay_cycles(1000000);
    serialTx1(QIACT);
    __delay_cycles(2000000);


    sprintf(temp, "AT+QHTTPURL=%d,30\r\n", j);

    serialTx1(temp)    ; //printing above

    __delay_cycles(1000000);
    serialTx1(getUrl)    ;   //printing the get data
    __delay_cycles(1000000);
    serialTx1("AT+QHTTPGET=60\r\n")   ;
    __delay_cycles(1000000);
    serialTx1("AT+QHTTPREAD=30\r\n")   ;
    __delay_cycles(1000000);

    serialTx1(QIDEACT)    ;
    __delay_cycles(1000000);
}




