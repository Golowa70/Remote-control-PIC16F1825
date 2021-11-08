/* ***��������� ����������� ������� ��������������� � �������������� ���� ����������***
  ���� ������ ������(������),���������� ���������� ������ � ����� ������� ������(������)
  ,����� ������ ��������,��� ���������� ������ ���� �����.���� ������ �� ������,
  ���������� ��������� � ����� SLEEP.
*/

// ***����� ���������-����***
//  � ������ ���������� ��������� ��������� (�������� ������) ����� ��������� �������� ,
//  ����� ��������� - �������������, �������� 's'  (���� ����),
//  ����� ���� ����� ������, ����� �������� ����� ������ � ���� CRC 
// (�.�. ����������� ����� � 1 ������ ������, ������ 4 �����)
//  ����0 = 's' - ������������� (����� ���� �� 1 �� 16 ��������� ���������� �����)
//  ����1 = 0x01 - ���� ����� ����� ������ (����� ���� �� 1 ����� �� ������������� �������� ������� ������)
//  ����2 = 0xXX - ����(�) ����� ������ (���������� ���� ������, ����� ����������� ��������)
//  ����3 = 0xXX - ���� CRC

//**********************************************************************************************************

#define F_CPU 16000000                                             //������� ����������
#define MAN_SPEED                    1000                          /*1000-4000 ���\��� - ������� MANCHESTER �������*/
#define MAN_IDENTIFIER                "sh"                         /*1-16 ���. ����. ��������� - ������������� ������.*/
#define MAN_PILOT_LEN                  8                           /*1-16 ���. ����� ��������� �������*/
#define MAN_BUF_LENGTH                16                           /*1-255 ����. ������ ������ ������*/

#define TRANSMIT_TRIS                 TRISC                       //���� ������ ������� MANCHESTER
#define TRANSMIT_PORT                 PORTC
#define TRANSMIT_LINE                 (1<<0)                      //����� ������ ������� MANCHESTER
//------------------------------------------------------------------------------
#define   FORWARD                     &PORTA,0,20,0               //������
#define   REVERSE                     &PORTA,1,20,0
#define   LEFT                        &PORTA,2,20,0
#define   RIGHT                       &PORTA,4,20,0
#define   TRIGGER_ACT                 &PORTA,5,20,0
#define   TRIGGER_PASS                &PORTA,5,20,1

#define   PWR_TRANSEIVER              LATC.B1                    //����� ������� �����������
//------------------------------------------------------------------------------
//��������� �������
void ManInit (void);                                             //������������� ����������
void ManBufAddByte (unsigned char place, unsigned char byte);    //������� ��������� ����� ������ � ����� ��������
void ManTransmitData (unsigned char BufLen);                     //������� �������� ������ � ��������� manchester
void ManTransmitByte (unsigned char byte);                       //������� �������� �����
void ManTransmitBit (unsigned char bit_t);                       //������� �������� ����
void ManPause (void);                                            //������� �������� ��� ��������� ������� manchester
void ManCheckSumm(unsigned char data_t);                         //������� �������� ����������� �����

//���������� ���������� --------------------------------------------------------
unsigned char ManTransmitDataBuf [MAN_BUF_LENGTH];            //����� ������������ ������
unsigned char ManIdentifier [] = {MAN_IDENTIFIER};            //��������� - ������������� ������.
unsigned char CheckSummByte;                                  //���� �������� CRC
unsigned char dataButtons=0;                                  //���������� ������ ������
unsigned char speedLevel= 128;                                 //������� ��������
unsigned char flagOldstate=0;                                 //���� ��������� ������ ��������
unsigned char flagTrigger=0;                                  //���� ��������

//��������� ���������� ---------------------------------------------------------
void ManInit (void)
{
        TRANSMIT_TRIS &= ~TRANSMIT_LINE;                        //����� �� �����
}
//------------------------------------------------------------------------------
//������� ��������� ����� � ����� ������
//��������1 - ����� ������ ������, ���� ��������� ���� ������
//��������2 - ���� ������ ���������� � �����
void ManBufAddByte (unsigned char place, unsigned char byte)
{
        if (place >= MAN_BUF_LENGTH)        return;
        ManTransmitDataBuf [place] = byte;
}
//------------------------------------------------------------------------------
//������� �������� ������ � ��������� MANCHESTER
//�������� - ���������� ���� ������ ManTransmitDataBuf[], ������� ���������� ��������
//�� ����� ������ �������, ���������� �����������
void ManTransmitData (unsigned char BufLen){
         unsigned char  i=0;
         unsigned char  u=0;
         unsigned char  a=0;
         unsigned char  byte =0 ;

       // unsigned char srbuf = STATUS;                        //��������� ��������� ���������� ?
        INTCON.GIE =0;                                         //������ ���� ����������

       for ( i=0; i< MAN_PILOT_LEN; i++) {                     //�������� ��������� �������
                ManTransmitBit (1);
              }


        //�������� �������������
        
           while (1)   {
        
                    byte = ManIdentifier [a];
                       a++;
                         if (byte)ManTransmitByte (byte);
                    else
                    break;
               }


        //�������� ������ ����� ������
        CheckSummByte = 0;                                   //�������� ����������
        ManTransmitByte (BufLen);

        //�������� ������ ������
        for (  u=0; u<(BufLen); u++) {
        
                ManTransmitByte (ManTransmitDataBuf [u]);
              }


        //�������� ����������� �����
          ManTransmitByte (CheckSummByte);

        ManTransmitBit (0);

       // STATUS = srbuf;                                    //��������������� ��������� ���������� ?
        INTCON.GIE =1;                                       //���������� ����������

}

//------------------------------------------------------------------------------
//�������� �����
//�������� - ���� ������������ ������
void ManTransmitByte (unsigned char byte)
{       unsigned char i=0;
        ManCheckSumm (byte);

        for ( i=0; i<8; i++) {
        
                if (byte & 0x80)        ManTransmitBit (1);
                else                        ManTransmitBit (0);
                byte <<= 1;
             }
}
//------------------------------------------------------------------------------
//�������� ����
//�������� - ���� �� ��������� 0 ��� 1 (�������� ������������� ����)
void ManTransmitBit (unsigned char bit_t)
{
        if (bit_t) {
        
                TRANSMIT_PORT &= ~(TRANSMIT_LINE);        ManPause ();
                TRANSMIT_PORT |= TRANSMIT_LINE;                ManPause ();                                                                                   ///
              }

        else {
                TRANSMIT_PORT |= TRANSMIT_LINE;                ManPause ();
                TRANSMIT_PORT &= ~TRANSMIT_LINE;        ManPause ();
               }
}
//-----------------------------------------------------------------------------
//������� �����
 void ManPause (void)
{
        delay_us (500000 / MAN_SPEED);
}
//-----------------------------------------------------------------------------
//������� �������� ����������� �����
//�������� - ���� ����������� � ������������ ����������� �����
//CheckSummByte - ���������� ���������� ����������� �����
//� ������ ������ ���������� �������� CheckSummByte
//�� ���������� ������ (���������� ����� �-��� ��� ����� ������ 
// � ���� ����������� �����) � CheckSummByte ������ ���� 0

void ManCheckSumm(unsigned char data_t)
{           unsigned char i=0;

        for ( i=0; i<8; i++) {
        
                unsigned char temp = data_t;
                temp ^= CheckSummByte;

                if (temp & 0x01) {
                
                        CheckSummByte ^= 0x18;
                        temp = 0x80;
                     }

                else        temp = 0;

                CheckSummByte >>= 1;
                CheckSummByte |= temp;
                data_t >>= 1;
            }
}

//-----------------------------------------------------------------------------
void interrupt (void){
  if(INTCON.IOCIE & INTCON.IOCIF){                            //���� ���������� �� ���������...
     INTCON.IOCIF=0;                                          //������� �����
     IOCAF.IOCAF0=0;
     IOCAF.IOCAF1=0;
     IOCAF.IOCAF2=0;
     IOCAF.IOCAF4=0;
     IOCAF.IOCAF5=0;
     PORTA;                                                   //��������� ���� ?
 
    }


}

//-----------------------------------------------------------------------------

void main (void) {
     OSCCON=0b11111111;                                       //�������� �������
     TRISA=0b111111;
     ANSELA=0;                                                //���������� ���
     PORTA=0b000000;
     WPUA=0b111111;                                           //������������� ������.
     TRISC=0b000000;
     PORTC=0b000000;
     OPTION_REG=0b00000000;
     INTCON=0b00001000;                                       //��������� ����������
     WDTCON=0b00010000;                                       //�������� ������ ???
     IOCAN=0b00110111;                                        //���������� �� ��������� �����
      
     ManInit ();                                              //�������������  �������� ��������� �������

     while(1){

           asm{clrwdt};                                       //����� ������
           PWR_TRANSEIVER=1;                                  // �������� ������� �����������
           if( Button(FORWARD))  dataButtons |=(1<<0);        //����� ������
           if( Button(REVERSE))  dataButtons |=(1<<1);
           if( Button(LEFT ))    dataButtons |=(1<<2);
           if( Button(RIGHT))    dataButtons |=(1<<3);
           
           if( Button(TRIGGER_PASS))  {
               flagOldstate=1;
              }
              
           if( Button (TRIGGER_ACT) && flagOldstate ) {        //���������� ������ ���
                 flagOldstate=0;
                 flagTrigger = ~flagTrigger;
                 if(flagTrigger){
                    dataButtons |=(1<<4);                      //��� ��������(���� ���)���������� ��� ����,����(4)���.
                    dataButtons &=~(1<<5);                     //������(5) ����.
                  }
                 else {
                      dataButtons &=~(1<<4);                   //�������� ��� ��������� �����
                      dataButtons |=(1<<5);                    //������� ��������� ����
                      }
              }
                 
                 
         if(dataButtons){                                    // ���� ������ ������...

               ManBufAddByte(0,dataButtons );                //��������� � 0 ������ ������ ���� ������ ������
               ManBufAddByte(1,speedLevel);                  //��������� � 1 ������ ������ ���� ������ ��������
               
               ManTransmitData (2);                         //�������� ��� ����� ������ �� ������
               dataButtons=0;                               //�������� ���������� ������ ������

            }
          else  {                                           // ���� ������ �� ������
                  flagOldstate=1;                           //�������� ���� ��������
                  PWR_TRANSEIVER=0;                         //��������� ������� �����������
                  asm{sleep};                               //���� �����

                }
     }
     

}

//------------------------------------------------------------------------------