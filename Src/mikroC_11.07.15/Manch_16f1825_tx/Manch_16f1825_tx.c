/* ***Программа передатчика системы радиоуправления с использованием кода манчестера***
  Если нажата кнопка(кнопки),непрерывно передаются пакеты с кодом нажатой кнопки(кнопок)
  ,кроме кнопки триггера,где передается только один пакет.Если кнопка не нажата,
  контроллер переходит в режим SLEEP.
*/

// ***Кодер Манчестер-кода***
//  в начале передается несколько импульсов (пилотный сигнал) чтобы разбудить приемник ,
//  потом заголовок - идентификатор, например 's'  (один байт),
//  потом байт длины пакета, потом передача байта данных и байт CRC 
// (т.е. минимальный пакет с 1 байтом данных, займет 4 байта)
//  байт0 = 's' - идентификатор (может быть от 1 до 16 латинских символьных байта)
//  байт1 = 0x01 - байт длины блока данных (может быть от 1 байта до максимального значения размера буфера)
//  байт2 = 0xXX - байт(ы) блока данных (количество байт данных, равно предыдущему значению)
//  байт3 = 0xXX - байт CRC

//**********************************************************************************************************

#define F_CPU 16000000                                             //частота процессора
#define MAN_SPEED                    1000                          /*1000-4000 бит\сек - частота MANCHESTER сигнала*/
#define MAN_IDENTIFIER                "sh"                         /*1-16 лат. симв. строковой - ИДЕНТИФИКАТОР ПАКЕТА.*/
#define MAN_PILOT_LEN                  8                           /*1-16 бит. Длина пилотного сигнала*/
#define MAN_BUF_LENGTH                16                           /*1-255 байт. Размер буфера данных*/

#define TRANSMIT_TRIS                 TRISC                       //порт вывода сигнала MANCHESTER
#define TRANSMIT_PORT                 PORTC
#define TRANSMIT_LINE                 (1<<0)                      //линия вывода сигнала MANCHESTER
//------------------------------------------------------------------------------
#define   FORWARD                     &PORTA,0,20,0               //кнопки
#define   REVERSE                     &PORTA,1,20,0
#define   LEFT                        &PORTA,2,20,0
#define   RIGHT                       &PORTA,4,20,0
#define   TRIGGER_ACT                 &PORTA,5,20,0
#define   TRIGGER_PASS                &PORTA,5,20,1

#define   PWR_TRANSEIVER              LATC.B1                    //выход питания передатчика
//------------------------------------------------------------------------------
//прототипы функций
void ManInit (void);                                             //инициализация библиотеки
void ManBufAddByte (unsigned char place, unsigned char byte);    //функция помещения байта данных в буфер передачи
void ManTransmitData (unsigned char BufLen);                     //функция передачи пакета в кодировке manchester
void ManTransmitByte (unsigned char byte);                       //функция передачи байта
void ManTransmitBit (unsigned char bit_t);                       //функция передачи бита
void ManPause (void);                                            //функция задержки для отработки посылок manchester
void ManCheckSumm(unsigned char data_t);                         //функция подсчета контрольной суммы

//глобальные переменные --------------------------------------------------------
unsigned char ManTransmitDataBuf [MAN_BUF_LENGTH];            //буфер передаваемых данных
unsigned char ManIdentifier [] = {MAN_IDENTIFIER};            //строковой - ИДЕНТИФИКАТОР ПАКЕТА.
unsigned char CheckSummByte;                                  //байт подсчета CRC
unsigned char dataButtons=0;                                  //переменная опроса кнопок
unsigned char speedLevel= 128;                                 //уровень скорости
unsigned char flagOldstate=0;                                 //флаг состояния кнопки триггера
unsigned char flagTrigger=0;                                  //флаг триггера

//настройка библиотеки ---------------------------------------------------------
void ManInit (void)
{
        TRANSMIT_TRIS &= ~TRANSMIT_LINE;                        //линия на вывод
}
//------------------------------------------------------------------------------
//ФУНКЦИЯ ПОМЕЩЕНИЯ БАЙТА В БУФЕР ДАННЫХ
//АРГУМЕНТ1 - номер ячейки буфера, куда запишется байт данных
//АРГУМЕНТ2 - байт данных помещаемый в буфер
void ManBufAddByte (unsigned char place, unsigned char byte)
{
        if (place >= MAN_BUF_LENGTH)        return;
        ManTransmitDataBuf [place] = byte;
}
//------------------------------------------------------------------------------
//ФУНКЦИЯ ПЕРЕДАЧИ ПАКЕТА В КОДИРОВКЕ MANCHESTER
//АРГУМЕНТ - количество байт буфера ManTransmitDataBuf[], которые необходимо передать
//на время вызова функции, прерывания запрещаются
void ManTransmitData (unsigned char BufLen){
         unsigned char  i=0;
         unsigned char  u=0;
         unsigned char  a=0;
         unsigned char  byte =0 ;

       // unsigned char srbuf = STATUS;                        //сохраняем состояние прерываний ?
        INTCON.GIE =0;                                         //запрет всех прерываний

       for ( i=0; i< MAN_PILOT_LEN; i++) {                     //передача пилотного сигнала
                ManTransmitBit (1);
              }


        //передаем идентификатор
        
           while (1)   {
        
                    byte = ManIdentifier [a];
                       a++;
                         if (byte)ManTransmitByte (byte);
                    else
                    break;
               }


        //передаем размер блока данных
        CheckSummByte = 0;                                   //обнулить контрольку
        ManTransmitByte (BufLen);

        //передаем данные буфера
        for (  u=0; u<(BufLen); u++) {
        
                ManTransmitByte (ManTransmitDataBuf [u]);
              }


        //передача контрольной суммы
          ManTransmitByte (CheckSummByte);

        ManTransmitBit (0);

       // STATUS = srbuf;                                    //восстанавливаем состояние прерываний ?
        INTCON.GIE =1;                                       //разрешение прерываний

}

//------------------------------------------------------------------------------
//передача байта
//АРГУМЕНТ - байт передаваемых данных
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
//передача бита
//АРГУМЕНТ - байт со значением 0 или 1 (значение передаваемого бита)
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
//Функция паузы
 void ManPause (void)
{
        delay_us (500000 / MAN_SPEED);
}
//-----------------------------------------------------------------------------
//ФУНКЦИЯ ПОДСЧЕТА КОНТРОЛЬНОЙ СУММЫ
//АРГУМЕНТ - байт участвующий в формировании контрольной суммы
//CheckSummByte - глобальная переменная контрольной суммы
//в начале обмена необходимо обнулить CheckSummByte
//по завершении обмена (пропустить через ф-цию все байты пакета 
// и байт контрольной суммы) в CheckSummByte должно быть 0

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
  if(INTCON.IOCIE & INTCON.IOCIF){                            //если прерывание по изменению...
     INTCON.IOCIF=0;                                          //сбросим флаги
     IOCAF.IOCAF0=0;
     IOCAF.IOCAF1=0;
     IOCAF.IOCAF2=0;
     IOCAF.IOCAF4=0;
     IOCAF.IOCAF5=0;
     PORTA;                                                   //прочитаем порт ?
 
    }


}

//-----------------------------------------------------------------------------

void main (void) {
     OSCCON=0b11111111;                                       //тактовая частота
     TRISA=0b111111;
     ANSELA=0;                                                //отключение АЦП
     PORTA=0b000000;
     WPUA=0b111111;                                           //подтягивающие резист.
     TRISC=0b000000;
     PORTC=0b000000;
     OPTION_REG=0b00000000;
     INTCON=0b00001000;                                       //настройки прерываний
     WDTCON=0b00010000;                                       //делитель собаки ???
     IOCAN=0b00110111;                                        //прерывание по изменению порта
      
     ManInit ();                                              //инициализация  передачи манчестер сигнала

     while(1){

           asm{clrwdt};                                       //сброс собаки
           PWR_TRANSEIVER=1;                                  // включаем питание передатчика
           if( Button(FORWARD))  dataButtons |=(1<<0);        //опрос кнопок
           if( Button(REVERSE))  dataButtons |=(1<<1);
           if( Button(LEFT ))    dataButtons |=(1<<2);
           if( Button(RIGHT))    dataButtons |=(1<<3);
           
           if( Button(TRIGGER_PASS))  {
               flagOldstate=1;
              }
              
           if( Button (TRIGGER_ACT) && flagOldstate ) {        //управление светом фар
                 flagOldstate=0;
                 flagTrigger = ~flagTrigger;
                 if(flagTrigger){
                    dataButtons |=(1<<4);                      //для триггера(свет фар)используем два бита,один(4)вкл.
                    dataButtons &=~(1<<5);                     //второй(5) выкл.
                  }
                 else {
                      dataButtons &=~(1<<4);                   //очистить бит включения света
                      dataButtons |=(1<<5);                    //команда выключить свет
                      }
              }
                 
                 
         if(dataButtons){                                    // если кнопка нажата...

               ManBufAddByte(0,dataButtons );                //поместить в 0 ячейку буфера байт данных кнопок
               ManBufAddByte(1,speedLevel);                  //поместить в 1 ячейку буфера байт данных скорости
               
               ManTransmitData (2);                         //передать два байта данных из буфера
               dataButtons=0;                               //обнулить переменную опроса кнопок

            }
          else  {                                           // если кнопка не нажата
                  flagOldstate=1;                           //выставим флаг триггера
                  PWR_TRANSEIVER=0;                         //выключаем питание передатчика
                  asm{sleep};                               //идем спать

                }
     }
     

}

//------------------------------------------------------------------------------