/*  ***��������� ��������� ������� ��������������� � ����������� ���� ����������.
      ��������� ��� ������(������ ���� ������) � �������� ��������(������ ����
      ������).
*/
// *** ������� ��������� ����***
//1. ���� ������  �������������� (� ����� ������ 2 ����� "sh")
//2. ��������� ���� ������� ����� ������.
//3. ��������� ��� ���� ������
//4. ����� ����� ����������� �����

//����������\������� ����������� ����� �������� ����� �2
//����� ������� ����������� ����� �� ����� ������ �2, �3 � �4
//��� ���������� ������, � �������� ����������� ����� � ����� ������ ��������� 0...
//���� ���� ����������� ����� �� ����, ����� ��������� ��������...
//------------------------------------------------------------------------------

// ������ T0 (��� �������� ������������ ��������� MANCHESTER �������)
// ����� �������� ���������� IOC (���� ������������� ������� MANCHESTER)

#define F_CPU 8000000
#define MAN_SPEED                             1000                        /*800-4000 ���\��� - ������� MANCHESTER �������*/
#define MAN_BUF_LENGTH                        16                                /*1-255 �����. ������ ������ ������*/
#define MAN_IDENTIFIER                        "sh"                        /*1-16 ��������� �������� - ��������� ������������� ������.*/

#define MAN_IN_PIN                          PORTA                        /*���� ������ (������ ���� ���� INT0)*/
#define MAN_IN_LINE                         (1<<4)                        /*����� ������ (������ ���� ����� INT0)*/


//���������� �������
void ManReceiveStart (void);                                //������� ������� �������� �������� ��������� ���������
void ManReceiveStop (void);                                 //������� ��������� �������� �������� ��������� ���������
unsigned char* ManRcvDataCheck (void);                      //������� �������� ������� ��������� ���������
void CheckSumm (unsigned char  dataa);                      //������� �������� ����������� �����


//------------------------------------------------------------------------

 //���������� ����������
unsigned char ManBuffer [MAN_BUF_LENGTH];                       //����� ��� ���������� ����������� MANCHESTER ������
unsigned char ManIdentifier [] = {MAN_IDENTIFIER};              //����������������� ��������� ���������
unsigned char CheckSummByte;                                    //���� �������� CRC
unsigned char BitCounter;                                       //������� ���������� �������� ���
unsigned char ByteCounter;                                      //������� ����������� ������
unsigned char ByteIn;                                           //���������� ������������ �����
unsigned char DataLength;                                       //����� ����� ������ � �����������\������������ ����� (����� ���� <= MAN_BUF_LENGTH)
unsigned char TimerVal;
unsigned char Invert;
//------------------------------------------------------------------------------------------
//unsigned char *pBuf;
unsigned char codeButtons;
unsigned char speedLevel;
signed int timeOffOut_counter;                                            //
signed int timeOffDevice_counter;                                        //
unsigned char timerStartForward_counter;                                   //������ ������ ��� ������
unsigned char timerStartReverse_counter;                                   //

volatile char ManFlags;                                                //���� ������
        #define  bDATA_ENBL              (1<<0)                        /*���� ������� � ������ �������� ������*/
        #define bTIM0_OVF                (1<<1)                        /*���� ������� ������������ �0*/
        #define bLINE_VAL                (1<<2)                        /*������� � ����� �� ������ ������� ������� (1\4-� ����� �������)*/
        #define bHEADER_RCV               (1<<3)                        /*���� ������ ���������*/
        #define bLINE_INV                (1<<4)                        /*���� ������������� �������� ������� � �����*/

        #define MAN_PERIOD_LEN            F_CPU /4 / 32 / MAN_SPEED       /*������������ ������� MANCHESTER ������� (� ����� �������)    ?????????????
                                                                        (������� �������������/4/ ������������/ ������� MANCHESTER �������)*/

//------------------------------------------------------------------------------------------------------------------------
 #define   FORWARD                     PWM1_Set_Duty                    //������
#define    REVERSE                     PWM2_Set_Duty
#define    LEFT                        LATC.B2
#define    RIGHT                       LATC.B1
#define    TRIGGER                     LATC.B0
#define    PWR_RECEIVER                LATC.B4
//---------------------------------------------------------------------------------------------------------------------





//---------------------------------------------------------------------------------------------------------------------
//������������� � ����� ������ ������
void ManReceiveStart (void)
{
        //��������� ������� �������� T0 (��� �������� ������������ ��������� MANCHESTER)
        INTCON.GIE=0;                                       //��������� ��� ����������

        OPTION_REG=0b10000101;                             //������������ �� 64 (������� ����� 16000000/4 / 32 = 125000 Hz = 8 uS)  //64?????????
        INTCON.TMR0IE=1;                                   //���������� ��� ������������

        //��������� �������� ����������
        INTCON.IOCIE=1;                                   //���������� ���������� �� ��������� ������
        IOCAP.IOCAP4=1;                                   //�������� ����������  �� ������ �� RA5
        IOCAN.IOCAN4=1;                                   //�������� ����������  �� ����� �� RA5
        
        

        //��������� ����������
        ManFlags &= ~(bTIM0_OVF| bDATA_ENBL);             //�������� ���� ������� ������ � ���� ������������
        ManFlags |= bHEADER_RCV;                          //�������� ����� ������ ���������
        ByteCounter = 0;                                  //������ ����� � ������
        ByteIn = 0x00;                                    //�������� ���� ��������
        
        INTCON.GIE=1;                                     //��������� ��� ����������
}


//--------------------------------------------------------------------------------------------------------------------------
//������� ������ ������
void ManReceiveStop (void)
{
        INTCON.GIE=0;
        INTCON.TMR0IE=0;                                  //��������� "���������� ��� ������������ �0"
        INTCON.IOCIE=0;                                   //��������� "������� ���������� �� IOC"
        INTCON.GIE=1;
}


//--------------------------------------------------------------------------------------------------------------------------
//�������� ������� MANCHESTER ������ � ������
//�������� - ���� ������ ��� - ���������� 0, ����� - ���������� ��������� �� ����� ������
unsigned char* ManRcvDataCheck (void)
{
        if (ManFlags & bDATA_ENBL)                             //�������� ������� �������� ������
        {
                ManFlags &= ~bDATA_ENBL;                       //�������� ���� ������� ������
                return ManBuffer;                              //��� ������� ������ - ���������� ��������� �� ������
        }
        return 0;                                              //��� ���������� ������ - ���������� 0
}


//--------------------------------------------------------------------------------------------------------------------------


void interrupt (void)  {
    if( PIE3.TMR6IE && PIR3.TMR6IF ) {
        PIR3.TMR6IF=0;
       if( timeOffOut_counter <0)
           timeOffOut_counter++;
       if(timeOffDevice_counter <0)
           timeOffDevice_counter++;

       asm {clrwdt};                                    //����� ������
    }



//��������� ���������� ��� ������������ TIMER/COUNTER 0
if (INTCON.T0IF && INTCON.T0IE)
   {
        //PIE3.TMR6IE=0 ;                                              //���������
        INTCON.T0IF=0;                                              //�������� ���� ������������ �������
        if (INTCON.IOCIE==1)                                        //���� ������� ���� ���������� �� IOC -
                ManFlags |= bTIM0_OVF;                              // - �������� ������������

        else                                                       //���� ���� ���������� �� IOC ���� ��������� -
        {                                                          // - ��������� ������� �����
                if (MAN_IN_PIN & MAN_IN_LINE)    {
                       ManFlags |= bLINE_VAL;
                   }
                   
                else  {
                      ManFlags &= ~bLINE_VAL;
                   }
                   
                INTCON.IOCIE=1;                                        //�������� ������� ���������� �� IOC
                INTCON.IOCIF=0;                                        //�������� �������� ������������ ����������
                IOCAF.IOCAF4=0;                                        //--''--
                //PIE3.TMR6IE=1;                                         //���������
                asm {clrwdt};                                    //����� ������
           }
          
   }
   
  //-----------------------------------------------------------------------------------------------------
      //��������� �������� ���������� INT0
if (INTCON.IOCIE && INTCON.IOCIF && IOCAF.IOCAF4)
   {    
        // PIE3.TMR6IE=0;                                          //���������
        //�������� ������ �� ����� � 3/4 ������� (����� ����� ����� - ������� � 1/4 ����� ���������� ����)
        TimerVal = TMR0;
        TMR0 = 255 - ((MAN_PERIOD_LEN )* 3 / 4);               //������� ������� ��������� �� 3/4 ����� ������� MANCHESTER ���� ������
        INTCON.IOCIE=0;                                        //��������� ������� ����������
        INTCON.IOCIF=0;                                        //�� ������ �� ������� �������� �������� ������������ ��������� ����������
        IOCAF.IOCAF4=0;
        
        //�������� �� ������������ ������������ ������ (�������� ������� � 1/4 ���� �� �������� ���� (������ ��������))
          if ( (TimerVal > (MAN_PERIOD_LEN/2)) || (ManFlags & bTIM0_OVF))
             {

  Ini :
               asm {clrwdt};                                    //����� ������
  
                ManFlags &= ~(bTIM0_OVF);                       //�������� ���� ������������
                ManFlags |= bHEADER_RCV;                        //������� ����� ���������
                ByteCounter = 0;                                //������ ����� � ������
                ByteIn = 0x00;                                  //�������� ���� ��������

              }

        //��������� �������� ���
        ByteIn <<= 1;                                           //�������� ���� ����� ������� ����
        
        if (! (ManFlags & bLINE_VAL))   {
                ByteIn |= 1;
              }
              
              
        //�������� ��� �� ��������� ������ ������
        if (ManFlags & bHEADER_RCV)
        {
                //�������� ��� ���� �� �������� ������ ���� ������
                if (ByteCounter == 0)
                {
                         Invert = ~ManIdentifier [0];

                        if (ByteIn != ManIdentifier [0]) {                            //?????? � ������� ������

                                 if (ByteIn != Invert)

                                        return;                                        //���� ��� ���������� - �����
                                 }
                        if (ByteIn == ManIdentifier [0]) {
                                ManFlags &= ~bLINE_INV;                //������ ����������

                            }
                                
                        else    {
                                 ManFlags |= bLINE_INV;                //��������� ����������
                                }
                        BitCounter = 0;                                        //��������� � ������ ��������� ������ ������
                        ByteCounter++;
                        return;
                }

              asm {clrwdt};                                    //����� ������
                //��������� ��������� ����� ������ � ���� ����� ������
                if (++BitCounter < 8)                                //���� ���������� �����
                        return;

                if (ManFlags & bLINE_INV)                        //���� ������ ���������
                        ByteIn = ~ByteIn;

                if (ManIdentifier [ByteCounter])        //���� ����� ��� �� ��������
                {
                        if (ByteIn != ManIdentifier [ByteCounter]){             //��������� ������������ ������
                                goto Ini;                                        //���� �� ������������� ������ - �������
                            }
                        BitCounter = 0;
                        ByteCounter++;                                         //������� ��������� ���� ������
                        return;
                }

                //����� ����������, ������ ������ ���� ����� ����� ������
                if (ByteIn > MAN_BUF_LENGTH)  {

                        goto Ini;                                             //������ ����� ������ ��������� ���������� - �������

                       }

                DataLength = ByteIn;                                        //�������� ����� ������

                CheckSummByte = 0;                                         //�������� ���� ����������� �����
                CheckSumm (ByteIn);                                        //������� ����������, ������� � ����� ����� ������

                ManFlags &= ~bHEADER_RCV;                                 //��������� � ������ ��������� �����
                BitCounter = 0;
                ByteCounter = 0;
                return;
        }
           asm {clrwdt};                                    //����� ������
        //��������� �������� ���� ������
        if (++BitCounter < 8)                                        //���� ���������� �����
                return;
        BitCounter = 0;

        if (ManFlags & bLINE_INV)                                   //���������� �� ��������
                ByteIn = ~ByteIn;

        CheckSumm (ByteIn);                                         //������� ����������

        if (DataLength--) {                                         //���� ��� ��� ����� ������ -
              ManBuffer [ByteCounter++] = ByteIn;                   // - ��������� �������� ����
            }
                
        else                                                       //���� ����� �������� -
        {
                if (CheckSummByte) {                              //���� ���������� �� ����� (�� 0) -
                      goto Ini;                                   // - �������
                    }

                //���������� ����������
                ManFlags |= bDATA_ENBL;                          //���������� ���� ������� ������
                ManReceiveStop ();                               //�������� ���������� �����
        }
        
      // PIE3.TMR6IE=1;
      asm {clrwdt};                                    //����� ������
   }
 }

//-----------------------------------------------------------------------------------------------------------------------
//������� �������� ����������� �����
//�������� - ���� ����������� � ������������ ����������� �����
//CheckSummByte - ���������� ���������� ����������� �����
//� ������ ������ ���������� �������� CheckSummByte
//�� ���������� ������ (���������� ����� �-��� ��� ����� ������ � ���� ����������� �����) � CheckSummByte ������ ���� 0
void CheckSumm(unsigned char dataa)
{              unsigned char i=0;
        for ( i=0; i<8; i++)
        {
                unsigned char temp = dataa;
                temp ^= CheckSummByte;

                if (temp & 0x01)        {CheckSummByte ^= 0x18; temp = 0x80;}
                else                                temp = 0;

                CheckSummByte >>= 1;
                CheckSummByte |= temp;
                dataa >>= 1;
        }
}
//----------------------------------------------------------------------------

void main (void){
//WDTCON=0b00001000;                              //�������� ������  ???
OSCCON=0b11111111;                                  //  16 MHz HF
TRISA=0b00010000;
PORTA=0b00000000;
TRISC=0b00000000;
PORTC=0b00000000;
ANSELA  = 0;                                       //���������� ���
ANSELC  = 0;
CM1CON0=0;                                         //���������� �����������
CM2CON0=0;

T6CON=0b11111111;                              //��������� TMR6.��������� 64,���������� 16.
PIE3.TMR6IE=1;                                 //���������� ���������� �� TMR6

asm {clrwdt};                                    //����� ������

 PWM1_Init(1500);                                 //������� ���
 PWM1_Start();
 PWM1_Set_Duty(0);
 PWM2_Init(1500);
 PWM2_Start();
 PWM2_Set_Duty(0);
 
 PWR_RECEIVER =1;                                // �������� ������� ���������
 ManReceiveStart ();                             // ����� ������ ������
 timeOffDevice_counter = - 9000;                //������ ������� ���������� 9000-10 ���

   while (1)
        {


                unsigned char *pBuf = ManRcvDataCheck();                    //�������� ������� ������
                    asm {clrwdt};                                    //����� ������
                if (pBuf)                                                   //���� ��������� �� �������, ������ ������ ���������
                {
                        timeOffDevice_counter = -9000;                        //��������� ������ ����������
                        codeButtons = *pBuf;                                //�������� ������ ���� ������
                        *pBuf++;
                        speedLevel = *pBuf;
                        
                        if(codeButtons & (1<<0))                          //��������� ����� ������ ������
                           {
                              if( ++timerStartForward_counter <5)  FORWARD(255); //���� ������,��� ������ ��� �� 100
                                  else
                                  FORWARD(speedLevel);                          //�������� � ��� �������� ��������
                             }
                           else 
                              {
                                timerStartForward_counter=0;
                                FORWARD(0);
                               }

                       if(codeButtons & (1<<1))                              //���� �����,�������� � ��� �������� ��������
                          {
                            if(++ timerStartReverse_counter<5) REVERSE(255);
                              else
                              REVERSE (speedlevel);
                           }
                          
                       else  {
                         timerStartReverse_counter=0;
                         REVERSE(0);
                        }
                          
                        
                        if(codeButtons & (1<<2)) LEFT =1;                    //� ����
                          else  LEFT=0;
                        
                        if(codeButtons & (1<<3)) RIGHT =1;                   //� �����
                          else  RIGHT=0;
                          
                       if(codeButtons & (1<<4)) TRIGGER =1;                 //�������� ����
                           else {
                             if(codeButtons & (1<<5))
                               TRIGGER=0;                                      //��������� ����
                            }


                         ManBuffer[1]=0;                                     // �������� ������ ���� ������ ������
                         ManBuffer[2]=0;                                     // �������� ������ ���� ������ ������
                         codeButtons=0;                                      // �������� ���������� ������
                         speedLevel=0;                                      // �������� ���������� ��������

                          timeOffOut_counter = -3;                          //������ ������� ���������� ������� 2-160��,3-240����.
                         
                         ManReceiveStart ();                                //���������� �������� ������ MANCHESTER ������
                         
                         
                }
                

                      asm {clrwdt};                                    //����� ������
                           
                      if(timeOffOut_counter>=0)                       //������ ��������
                           {
                             FORWARD(0);                              //��������� ������
                             REVERSE(0);
                             LEFT=0;
                             RIGHT=0;

                            }
                      if(timeOffDevice_counter >=0 )
                          {
                             TRIGGER=0;                                //��������� ����
                             FORWARD(0);                              //��������� ������
                             REVERSE(0);
                             LEFT=0;
                             RIGHT=0;
                             PWR_RECEIVER =0;                         //��������� ������� ���������
                             asm {sleep};                            //���� ����� ))
                          }
                            
        }




}

//------------------------------------------------------------------------------