#line 1 "C:/Users/Public/Documents/Mikroelektronika/mikroC PRO for PIC/Projects/Manch_16f1825_rx/Manch_16f1825_rx.c"
#line 30 "C:/Users/Public/Documents/Mikroelektronika/mikroC PRO for PIC/Projects/Manch_16f1825_rx/Manch_16f1825_rx.c"
void ManReceiveStart (void);
void ManReceiveStop (void);
unsigned char* ManRcvDataCheck (void);
void CheckSumm (unsigned char dataa);





unsigned char ManBuffer [ 16 ];
unsigned char ManIdentifier [] = { "sh" };
unsigned char CheckSummByte;
unsigned char BitCounter;
unsigned char ByteCounter;
unsigned char ByteIn;
unsigned char DataLength;
unsigned char TimerVal;
unsigned char Invert;


unsigned char codeButtons;
unsigned char speedLevel;
signed int timeOffOut_counter;
signed int timeOffDevice_counter;
unsigned char timerStartForward_counter;
unsigned char timerStartReverse_counter;

volatile char ManFlags;
#line 82 "C:/Users/Public/Documents/Mikroelektronika/mikroC PRO for PIC/Projects/Manch_16f1825_rx/Manch_16f1825_rx.c"
void ManReceiveStart (void)
{

 INTCON.GIE=0;

 OPTION_REG=0b10000101;
 INTCON.TMR0IE=1;


 INTCON.IOCIE=1;
 IOCAP.IOCAP4=1;
 IOCAN.IOCAN4=1;




 ManFlags &= ~( (1<<1) |  (1<<0) );
 ManFlags |=  (1<<3) ;
 ByteCounter = 0;
 ByteIn = 0x00;

 INTCON.GIE=1;
}




void ManReceiveStop (void)
{
 INTCON.GIE=0;
 INTCON.TMR0IE=0;
 INTCON.IOCIE=0;
 INTCON.GIE=1;
}





unsigned char* ManRcvDataCheck (void)
{
 if (ManFlags &  (1<<0) )
 {
 ManFlags &= ~ (1<<0) ;
 return ManBuffer;
 }
 return 0;
}





void interrupt (void) {
 if( PIE3.TMR6IE && PIR3.TMR6IF ) {
 PIR3.TMR6IF=0;
 if( timeOffOut_counter <0)
 timeOffOut_counter++;
 if(timeOffDevice_counter <0)
 timeOffDevice_counter++;

 asm {clrwdt};
 }




if (INTCON.T0IF && INTCON.T0IE)
 {

 INTCON.T0IF=0;
 if (INTCON.IOCIE==1)
 ManFlags |=  (1<<1) ;

 else
 {
 if ( PORTA  &  (1<<4) ) {
 ManFlags |=  (1<<2) ;
 }

 else {
 ManFlags &= ~ (1<<2) ;
 }

 INTCON.IOCIE=1;
 INTCON.IOCIF=0;
 IOCAF.IOCAF4=0;

 asm {clrwdt};
 }

 }



if (INTCON.IOCIE && INTCON.IOCIF && IOCAF.IOCAF4)
 {


 TimerVal = TMR0;
 TMR0 = 255 - (( 8000000  /4 / 32 / 1000  )* 3 / 4);
 INTCON.IOCIE=0;
 INTCON.IOCIF=0;
 IOCAF.IOCAF4=0;


 if ( (TimerVal > ( 8000000  /4 / 32 / 1000 /2)) || (ManFlags &  (1<<1) ))
 {

 Ini :
 asm {clrwdt};

 ManFlags &= ~( (1<<1) );
 ManFlags |=  (1<<3) ;
 ByteCounter = 0;
 ByteIn = 0x00;

 }


 ByteIn <<= 1;

 if (! (ManFlags &  (1<<2) )) {
 ByteIn |= 1;
 }



 if (ManFlags &  (1<<3) )
 {

 if (ByteCounter == 0)
 {
 Invert = ~ManIdentifier [0];

 if (ByteIn != ManIdentifier [0]) {

 if (ByteIn != Invert)

 return;
 }
 if (ByteIn == ManIdentifier [0]) {
 ManFlags &= ~ (1<<4) ;

 }

 else {
 ManFlags |=  (1<<4) ;
 }
 BitCounter = 0;
 ByteCounter++;
 return;
 }

 asm {clrwdt};

 if (++BitCounter < 8)
 return;

 if (ManFlags &  (1<<4) )
 ByteIn = ~ByteIn;

 if (ManIdentifier [ByteCounter])
 {
 if (ByteIn != ManIdentifier [ByteCounter]){
 goto Ini;
 }
 BitCounter = 0;
 ByteCounter++;
 return;
 }


 if (ByteIn >  16 ) {

 goto Ini;

 }

 DataLength = ByteIn;

 CheckSummByte = 0;
 CheckSumm (ByteIn);

 ManFlags &= ~ (1<<3) ;
 BitCounter = 0;
 ByteCounter = 0;
 return;
 }
 asm {clrwdt};

 if (++BitCounter < 8)
 return;
 BitCounter = 0;

 if (ManFlags &  (1<<4) )
 ByteIn = ~ByteIn;

 CheckSumm (ByteIn);

 if (DataLength--) {
 ManBuffer [ByteCounter++] = ByteIn;
 }

 else
 {
 if (CheckSummByte) {
 goto Ini;
 }


 ManFlags |=  (1<<0) ;
 ManReceiveStop ();
 }


 asm {clrwdt};
 }
 }







void CheckSumm(unsigned char dataa)
{ unsigned char i=0;
 for ( i=0; i<8; i++)
 {
 unsigned char temp = dataa;
 temp ^= CheckSummByte;

 if (temp & 0x01) {CheckSummByte ^= 0x18; temp = 0x80;}
 else temp = 0;

 CheckSummByte >>= 1;
 CheckSummByte |= temp;
 dataa >>= 1;
 }
}


void main (void){

OSCCON=0b11111111;
TRISA=0b00010000;
PORTA=0b00000000;
TRISC=0b00000000;
PORTC=0b00000000;
ANSELA = 0;
ANSELC = 0;
CM1CON0=0;
CM2CON0=0;

T6CON=0b11111111;
PIE3.TMR6IE=1;

asm {clrwdt};

 PWM1_Init(1500);
 PWM1_Start();
 PWM1_Set_Duty(0);
 PWM2_Init(1500);
 PWM2_Start();
 PWM2_Set_Duty(0);

  LATC.B4  =1;
 ManReceiveStart ();
 timeOffDevice_counter = - 9000;

 while (1)
 {


 unsigned char *pBuf = ManRcvDataCheck();
 asm {clrwdt};
 if (pBuf)
 {
 timeOffDevice_counter = -9000;
 codeButtons = *pBuf;
 *pBuf++;
 speedLevel = *pBuf;

 if(codeButtons & (1<<0))
 {
 if( ++timerStartForward_counter <5)  PWM1_Set_Duty (255);
 else
  PWM1_Set_Duty (speedLevel);
 }
 else
 {
 timerStartForward_counter=0;
  PWM1_Set_Duty (0);
 }

 if(codeButtons & (1<<1))
 {
 if(++ timerStartReverse_counter<5)  PWM2_Set_Duty (255);
 else
  PWM2_Set_Duty  (speedlevel);
 }

 else {
 timerStartReverse_counter=0;
  PWM2_Set_Duty (0);
 }


 if(codeButtons & (1<<2))  LATC.B2  =1;
 else  LATC.B2 =0;

 if(codeButtons & (1<<3))  LATC.B1  =1;
 else  LATC.B1 =0;

 if(codeButtons & (1<<4))  LATC.B0  =1;
 else {
 if(codeButtons & (1<<5))
  LATC.B0 =0;
 }


 ManBuffer[1]=0;
 ManBuffer[2]=0;
 codeButtons=0;
 speedLevel=0;

 timeOffOut_counter = -3;

 ManReceiveStart ();


 }


 asm {clrwdt};

 if(timeOffOut_counter>=0)
 {
  PWM1_Set_Duty (0);
  PWM2_Set_Duty (0);
  LATC.B2 =0;
  LATC.B1 =0;

 }
 if(timeOffDevice_counter >=0 )
 {
  LATC.B0 =0;
  PWM1_Set_Duty (0);
  PWM2_Set_Duty (0);
  LATC.B2 =0;
  LATC.B1 =0;
  LATC.B4  =0;
 asm {sleep};
 }

 }




}
