#line 1 "C:/Users/Public/Documents/Mikroelektronika/mikroC PRO for PIC/Projects/Manch_16f1825_tx/Manch_16f1825_tx.c"
#line 39 "C:/Users/Public/Documents/Mikroelektronika/mikroC PRO for PIC/Projects/Manch_16f1825_tx/Manch_16f1825_tx.c"
void ManInit (void);
void ManBufAddByte (unsigned char place, unsigned char byte);
void ManTransmitData (unsigned char BufLen);
void ManTransmitByte (unsigned char byte);
void ManTransmitBit (unsigned char bit_t);
void ManPause (void);
void ManCheckSumm(unsigned char data_t);


unsigned char ManTransmitDataBuf [ 16 ];
unsigned char ManIdentifier [] = { "sh" };
unsigned char CheckSummByte;
unsigned char dataButtons=0;
unsigned char speedLevel= 128;
unsigned char flagOldstate=0;
unsigned char flagTrigger=0;


void ManInit (void)
{
  TRISC  &= ~ (1<<0) ;
}




void ManBufAddByte (unsigned char place, unsigned char byte)
{
 if (place >=  16 ) return;
 ManTransmitDataBuf [place] = byte;
}




void ManTransmitData (unsigned char BufLen){
 unsigned char i=0;
 unsigned char u=0;
 unsigned char a=0;
 unsigned char byte =0 ;


 INTCON.GIE =0;

 for ( i=0; i<  8 ; i++) {
 ManTransmitBit (1);
 }




 while (1) {

 byte = ManIdentifier [a];
 a++;
 if (byte)ManTransmitByte (byte);
 else
 break;
 }



 CheckSummByte = 0;
 ManTransmitByte (BufLen);


 for ( u=0; u<(BufLen); u++) {

 ManTransmitByte (ManTransmitDataBuf [u]);
 }



 ManTransmitByte (CheckSummByte);

 ManTransmitBit (0);


 INTCON.GIE =1;

}




void ManTransmitByte (unsigned char byte)
{ unsigned char i=0;
 ManCheckSumm (byte);

 for ( i=0; i<8; i++) {

 if (byte & 0x80) ManTransmitBit (1);
 else ManTransmitBit (0);
 byte <<= 1;
 }
}



void ManTransmitBit (unsigned char bit_t)
{
 if (bit_t) {

  PORTC  &= ~( (1<<0) ); ManPause ();
  PORTC  |=  (1<<0) ; ManPause ();
 }

 else {
  PORTC  |=  (1<<0) ; ManPause ();
  PORTC  &= ~ (1<<0) ; ManPause ();
 }
}


 void ManPause (void)
{
 delay_us (500000 /  1000 );
}








void ManCheckSumm(unsigned char data_t)
{ unsigned char i=0;

 for ( i=0; i<8; i++) {

 unsigned char temp = data_t;
 temp ^= CheckSummByte;

 if (temp & 0x01) {

 CheckSummByte ^= 0x18;
 temp = 0x80;
 }

 else temp = 0;

 CheckSummByte >>= 1;
 CheckSummByte |= temp;
 data_t >>= 1;
 }
}


void interrupt (void){
 if(INTCON.IOCIE & INTCON.IOCIF){
 INTCON.IOCIF=0;
 IOCAF.IOCAF0=0;
 IOCAF.IOCAF1=0;
 IOCAF.IOCAF2=0;
 IOCAF.IOCAF4=0;
 IOCAF.IOCAF5=0;
 PORTA;

 }


}



void main (void) {
 OSCCON=0b11111111;
 TRISA=0b111111;
 ANSELA=0;
 PORTA=0b000000;
 WPUA=0b111111;
 TRISC=0b000000;
 PORTC=0b000000;
 OPTION_REG=0b00000000;
 INTCON=0b00001000;
 WDTCON=0b00010000;
 IOCAN=0b00110111;

 ManInit ();

 while(1){

 asm{clrwdt};
  LATC.B1 =1;
 if( Button( &PORTA,0,20,0 )) dataButtons |=(1<<0);
 if( Button( &PORTA,1,20,0 )) dataButtons |=(1<<1);
 if( Button( &PORTA,2,20,0  )) dataButtons |=(1<<2);
 if( Button( &PORTA,4,20,0 )) dataButtons |=(1<<3);

 if( Button( &PORTA,5,20,1 )) {
 flagOldstate=1;
 }

 if( Button ( &PORTA,5,20,0 ) && flagOldstate ) {
 flagOldstate=0;
 flagTrigger = ~flagTrigger;
 if(flagTrigger){
 dataButtons |=(1<<4);
 dataButtons &=~(1<<5);
 }
 else {
 dataButtons &=~(1<<4);
 dataButtons |=(1<<5);
 }
 }


 if(dataButtons){

 ManBufAddByte(0,dataButtons );
 ManBufAddByte(1,speedLevel);

 ManTransmitData (2);
 dataButtons=0;

 }
 else {
 flagOldstate=1;
  LATC.B1 =0;
 asm{sleep};

 }
 }


}
