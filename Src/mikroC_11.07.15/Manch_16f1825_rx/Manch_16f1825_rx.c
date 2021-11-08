/*  ***Программа приемника системы радиоуправления с применением кода Манчестера.
      Принимаем код кнопки(первый байт данных) и значение скорости(второй байт
      данных).
*/
// *** Декодер Манчестер кода***
//1. ждем приема  идентификатора (в нашем случае 2 байта "sh")
//2. принимаем байт размера блока данных.
//3. принимаем сам блок данных
//4. прием байта контрольной суммы

//Переменную\счетчик контрольной суммы обнуляем перед п2
//Потом считаем контрольную сумму во время приема п2, п3 и п4
//При правильном приеме, в счетчике контрольной суммы в конце должно получится 0...
//Если байт контрольной суммы не ноль, пакет считается неверным...
//------------------------------------------------------------------------------

// таймер T0 (для подсчета длительности импульсов MANCHESTER сигнала)
// линия внешнего прерывания IOC (вход декодирования сигнала MANCHESTER)

#define F_CPU 8000000
#define MAN_SPEED                             1000                        /*800-4000 бит\сек - частота MANCHESTER сигнала*/
#define MAN_BUF_LENGTH                        16                                /*1-255 байта. размер буфера данных*/
#define MAN_IDENTIFIER                        "sh"                        /*1-16 латинских символов - строковой ИДЕНТИФИКАТОР ПАКЕТА.*/

#define MAN_IN_PIN                          PORTA                        /*порт приема (ДОЛЖЕН БЫТЬ ПОРТ INT0)*/
#define MAN_IN_LINE                         (1<<4)                        /*линия приема (ДОЛЖЕН БЫТЬ ВЫВОД INT0)*/


//объявления функций
void ManReceiveStart (void);                                //функция запуска процесса ожидания входящего сообщения
void ManReceiveStop (void);                                 //функция остановки процесса ожидания входящего сообщения
unsigned char* ManRcvDataCheck (void);                      //функция проверки наличия входящего сообщения
void CheckSumm (unsigned char  dataa);                      //функция подсчета контрольной суммы


//------------------------------------------------------------------------

 //глобальные переменные
unsigned char ManBuffer [MAN_BUF_LENGTH];                       //буфер для накопления принимаемых MANCHESTER данных
unsigned char ManIdentifier [] = {MAN_IDENTIFIER};              //идентификационный заголовок сообщения
unsigned char CheckSummByte;                                    //байт подсчета CRC
unsigned char BitCounter;                                       //счетчик количества принятых бит
unsigned char ByteCounter;                                      //счетчик принимаемых байтов
unsigned char ByteIn;                                           //накопитель принимаемого байта
unsigned char DataLength;                                       //длина блока данных в принимаемой\передаваемой серии (может быть <= MAN_BUF_LENGTH)
unsigned char TimerVal;
unsigned char Invert;
//------------------------------------------------------------------------------------------
//unsigned char *pBuf;
unsigned char codeButtons;
unsigned char speedLevel;
signed int timeOffOut_counter;                                            //
signed int timeOffDevice_counter;                                        //
unsigned char timerStartForward_counter;                                   //таймер старта ШИМ вперед
unsigned char timerStartReverse_counter;                                   //

volatile char ManFlags;                                                //байт флагов
        #define  bDATA_ENBL              (1<<0)                        /*флаг наличия в буфере принятых данных*/
        #define bTIM0_OVF                (1<<1)                        /*флаг наличия переполнения Т0*/
        #define bLINE_VAL                (1<<2)                        /*уровень в линии на ровном участке сигнала (1\4-я слота сигнала)*/
        #define bHEADER_RCV               (1<<3)                        /*флаг приема заголовка*/
        #define bLINE_INV                (1<<4)                        /*флаг необходимости инверсии сигнала в линии*/

        #define MAN_PERIOD_LEN            F_CPU /4 / 32 / MAN_SPEED       /*длительность периода MANCHESTER сигнала (в тиках таймера)    ?????????????
                                                                        (частота контроллерара/4/ предделитель/ частота MANCHESTER сигнала)*/

//------------------------------------------------------------------------------------------------------------------------
 #define   FORWARD                     PWM1_Set_Duty                    //выходы
#define    REVERSE                     PWM2_Set_Duty
#define    LEFT                        LATC.B2
#define    RIGHT                       LATC.B1
#define    TRIGGER                     LATC.B0
#define    PWR_RECEIVER                LATC.B4
//---------------------------------------------------------------------------------------------------------------------





//---------------------------------------------------------------------------------------------------------------------
//ИНИЦИАЛИЗАЦИЯ И СТАРТ ПРИЕМА ДАННЫХ
void ManReceiveStart (void)
{
        //НАСТРОЙКА ТАЙМЕРА СЧЕТЧИКА T0 (для подсчета длительности импульсов MANCHESTER)
        INTCON.GIE=0;                                       //запретить все прерывания

        OPTION_REG=0b10000101;                             //предделитель на 64 (частота счета 16000000/4 / 32 = 125000 Hz = 8 uS)  //64?????????
        INTCON.TMR0IE=1;                                   //прерывание при переполнении

        //НАСТРОЙКА ВНЕШНЕГО ПРЕРЫВАНИЯ
        INTCON.IOCIE=1;                                   //разрешение прерываний по изменению уровня
        IOCAP.IOCAP4=1;                                   //включить прерывание  по фронту от RA5
        IOCAN.IOCAN4=1;                                   //включить прерывание  по спаду от RA5
        
        

        //НАСТРОЙКА ПЕРЕМЕННЫХ
        ManFlags &= ~(bTIM0_OVF| bDATA_ENBL);             //очистить флаг наличия данных и флаг переполнения
        ManFlags |= bHEADER_RCV;                          //включить режим приема заголовка
        ByteCounter = 0;                                  //начать прием с начала
        ByteIn = 0x00;                                    //очистить байт приемник
        
        INTCON.GIE=1;                                     //разрешить все прерывания
}


//--------------------------------------------------------------------------------------------------------------------------
//ОСТАНОВ ПРИЕМА ДАННЫХ
void ManReceiveStop (void)
{
        INTCON.GIE=0;
        INTCON.TMR0IE=0;                                  //выключить "прерывание при переполнении Т0"
        INTCON.IOCIE=0;                                   //выключить "внешнее прерывание от IOC"
        INTCON.GIE=1;
}


//--------------------------------------------------------------------------------------------------------------------------
//ПРОВЕРКА НАЛИЧИЯ MANCHESTER ДАННЫХ В БУФЕРЕ
//ЗНАЧЕНИЕ - если данных нет - возвращаем 0, иначе - возвращает указатель на буфер данных
unsigned char* ManRcvDataCheck (void)
{
        if (ManFlags & bDATA_ENBL)                             //проверка наличия принятых данных
        {
                ManFlags &= ~bDATA_ENBL;                       //очистить флаг наличия данных
                return ManBuffer;                              //при наличии данных - возвращаем указатель на буффер
        }
        return 0;                                              //при отсутствии данных - возвращаем 0
}


//--------------------------------------------------------------------------------------------------------------------------


void interrupt (void)  {
    if( PIE3.TMR6IE && PIR3.TMR6IF ) {
        PIR3.TMR6IF=0;
       if( timeOffOut_counter <0)
           timeOffOut_counter++;
       if(timeOffDevice_counter <0)
           timeOffDevice_counter++;

       asm {clrwdt};                                    //сброс собаки
    }



//Обработка прерывания при переполнении TIMER/COUNTER 0
if (INTCON.T0IF && INTCON.T0IE)
   {
        //PIE3.TMR6IE=0 ;                                              //запретить
        INTCON.T0IF=0;                                              //сбросить флаг переполнения таймера
        if (INTCON.IOCIE==1)                                        //если ожидали внеш прерывания от IOC -
                ManFlags |= bTIM0_OVF;                              // - отметить переполнение

        else                                                       //если внеш прерывания от IOC были выключены -
        {                                                          // - запомнить уровень линии
                if (MAN_IN_PIN & MAN_IN_LINE)    {
                       ManFlags |= bLINE_VAL;
                   }
                   
                else  {
                      ManFlags &= ~bLINE_VAL;
                   }
                   
                INTCON.IOCIE=1;                                        //включить внешние прерывания от IOC
                INTCON.IOCIF=0;                                        //сбросить возможно проскочившее прерывание
                IOCAF.IOCAF4=0;                                        //--''--
                //PIE3.TMR6IE=1;                                         //разрешить
                asm {clrwdt};                                    //сброс собаки
           }
          
   }
   
  //-----------------------------------------------------------------------------------------------------
      //Обработка внешнего прерывания INT0
if (INTCON.IOCIE && INTCON.IOCIF && IOCAF.IOCAF4)
   {    
        // PIE3.TMR6IE=0;                                          //запретить
        //настроим таймер на паузу в 3/4 периода (после такой паузы - попадем в 1/4 слота следующего бита)
        TimerVal = TMR0;
        TMR0 = 255 - ((MAN_PERIOD_LEN )* 3 / 4);               //счетчик таймера настроить на 3/4 длины периода MANCHESTER бита данных
        INTCON.IOCIE=0;                                        //выключить внешнее прерывание
        INTCON.IOCIF=0;                                        //на случай ВЧ сигнала сбросить возможно проскочившее повторное прерывание
        IOCAF.IOCAF4=0;
        
        //проверка на корректность длительности замера (измеряли начиная с 1/4 бита до середина бита (момент перепада))
          if ( (TimerVal > (MAN_PERIOD_LEN/2)) || (ManFlags & bTIM0_OVF))
             {

  Ini :
               asm {clrwdt};                                    //сброс собаки
  
                ManFlags &= ~(bTIM0_OVF);                       //сбросить флаг переполнения
                ManFlags |= bHEADER_RCV;                        //ожидать прием заголовка
                ByteCounter = 0;                                //начать прием с начала
                ByteIn = 0x00;                                  //очистить байт приемник

              }

        //задвигаем принятый бит
        ByteIn <<= 1;                                           //сдвигаем байт перед записью бита
        
        if (! (ManFlags & bLINE_VAL))   {
                ByteIn |= 1;
              }
              
              
        //крутимся тут до окончания приема хедера
        if (ManFlags & bHEADER_RCV)
        {
                //крутимся тут пока не совпадет первый байт хедера
                if (ByteCounter == 0)
                {
                         Invert = ~ManIdentifier [0];

                        if (ByteIn != ManIdentifier [0]) {                            //?????? я добавил скобки

                                 if (ByteIn != Invert)

                                        return;                                        //пока нет совпадения - выход
                                 }
                        if (ByteIn == ManIdentifier [0]) {
                                ManFlags &= ~bLINE_INV;                //прямое совпадение

                            }
                                
                        else    {
                                 ManFlags |= bLINE_INV;                //инверсное совпадение
                                }
                        BitCounter = 0;                                        //готовимся к приему следующих байтов хедера
                        ByteCounter++;
                        return;
                }

              asm {clrwdt};                                    //сброс собаки
                //принимаем остальные байты хедера и байт длины пакета
                if (++BitCounter < 8)                                //ждем заполнения байта
                        return;

                if (ManFlags & bLINE_INV)                        //если сигнал инверсный
                        ByteIn = ~ByteIn;

                if (ManIdentifier [ByteCounter])        //если хедер еще не закончен
                {
                        if (ByteIn != ManIdentifier [ByteCounter]){             //проверяем идентичность хедера
                                goto Ini;                                        //байт не соответствует хедеру - рестарт
                            }
                        BitCounter = 0;
                        ByteCounter++;                                         //ожидаем следующий байт хедера
                        return;
                }

                //хедер закончился, значит принят байт длины блока данных
                if (ByteIn > MAN_BUF_LENGTH)  {

                        goto Ini;                                             //размер блока данных превышает допустимый - рестарт

                       }

                DataLength = ByteIn;                                        //запомним длину пакета

                CheckSummByte = 0;                                         //очистить байт контрольной суммы
                CheckSumm (ByteIn);                                        //подсчет контрольки, начиная с байта длины пакета

                ManFlags &= ~bHEADER_RCV;                                 //переходим к приему основного файла
                BitCounter = 0;
                ByteCounter = 0;
                return;
        }
           asm {clrwdt};                                    //сброс собаки
        //принимаем основной блок данных
        if (++BitCounter < 8)                                        //ждем накопления байта
                return;
        BitCounter = 0;

        if (ManFlags & bLINE_INV)                                   //необходима ли инверсия
                ByteIn = ~ByteIn;

        CheckSumm (ByteIn);                                         //подсчет контрольки

        if (DataLength--) {                                         //если это еще байты пакета -
              ManBuffer [ByteCounter++] = ByteIn;                   // - сохраняем принятый байт
            }
                
        else                                                       //если пакет закончен -
        {
                if (CheckSummByte) {                              //если контролька не верна (не 0) -
                      goto Ini;                                   // - рестарт
                    }

                //контролька правильная
                ManFlags |= bDATA_ENBL;                          //установить флаг наличия данных
                ManReceiveStop ();                               //тормозим дальнейший прием
        }
        
      // PIE3.TMR6IE=1;
      asm {clrwdt};                                    //сброс собаки
   }
 }

//-----------------------------------------------------------------------------------------------------------------------
//ФУНКЦИЯ ПОДСЧЕТА КОНТРОЛЬНОЙ СУММЫ
//АРГУМЕНТ - байт участвующий в формировании контрольной суммы
//CheckSummByte - глобальная переменная контрольной суммы
//в начале обмена необходимо обнулить CheckSummByte
//по завершении обмена (пропустить через ф-цию все байты пакета и байт контрольной суммы) в CheckSummByte должно быть 0
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
//WDTCON=0b00001000;                              //делитель собаки  ???
OSCCON=0b11111111;                                  //  16 MHz HF
TRISA=0b00010000;
PORTA=0b00000000;
TRISC=0b00000000;
PORTC=0b00000000;
ANSELA  = 0;                                       //выключение АЦП
ANSELC  = 0;
CM1CON0=0;                                         //выключение компаратора
CM2CON0=0;

T6CON=0b11111111;                              //настройка TMR6.Прескалер 64,постскалер 16.
PIE3.TMR6IE=1;                                 //разрешение прерываний от TMR6

asm {clrwdt};                                    //сброс собаки

 PWM1_Init(1500);                                 //частота ШИМ
 PWM1_Start();
 PWM1_Set_Duty(0);
 PWM2_Init(1500);
 PWM2_Start();
 PWM2_Set_Duty(0);
 
 PWR_RECEIVER =1;                                // включить питание приемника
 ManReceiveStart ();                             // старт приема данных
 timeOffDevice_counter = - 9000;                //таймер полного выключения 9000-10 мин

   while (1)
        {


                unsigned char *pBuf = ManRcvDataCheck();                    //проверка наличия данных
                    asm {clrwdt};                                    //сброс собаки
                if (pBuf)                                                   //если указатель не нулевой, значит данные поступили
                {
                        timeOffDevice_counter = -9000;                        //обновляем таймер выключения
                        codeButtons = *pBuf;                                //копируем первый байт буфера
                        *pBuf++;
                        speedLevel = *pBuf;
                        
                        if(codeButtons & (1<<0))                          //проверяем какая кнопка нажата
                           {
                              if( ++timerStartForward_counter <5)  FORWARD(255); //если вперед,при старте ШИМ на 100
                                  else
                                  FORWARD(speedLevel);                          //передаем в ШИМ значение скорости
                             }
                           else 
                              {
                                timerStartForward_counter=0;
                                FORWARD(0);
                               }

                       if(codeButtons & (1<<1))                              //если назад,передаем в ШИМ значение скорости
                          {
                            if(++ timerStartReverse_counter<5) REVERSE(255);
                              else
                              REVERSE (speedlevel);
                           }
                          
                       else  {
                         timerStartReverse_counter=0;
                         REVERSE(0);
                        }
                          
                        
                        if(codeButtons & (1<<2)) LEFT =1;                    //в лево
                          else  LEFT=0;
                        
                        if(codeButtons & (1<<3)) RIGHT =1;                   //в право
                          else  RIGHT=0;
                          
                       if(codeButtons & (1<<4)) TRIGGER =1;                 //включаем свет
                           else {
                             if(codeButtons & (1<<5))
                               TRIGGER=0;                                      //выключаем свет
                            }


                         ManBuffer[1]=0;                                     // обнуляем первый байт буфера данных
                         ManBuffer[2]=0;                                     // обнуляем второй байт буфера данных
                         codeButtons=0;                                      // обнуляем переменную кнопок
                         speedLevel=0;                                      // обнуляем переменную скорости

                          timeOffOut_counter = -3;                          //запуск таймера выключения выходов 2-160мс,3-240мсек.
                         
                         ManReceiveStart ();                                //перезапуск процесса чтения MANCHESTER данных
                         
                         
                }
                

                      asm {clrwdt};                                    //сброс собаки
                           
                      if(timeOffOut_counter>=0)                       //таймер отсчитал
                           {
                             FORWARD(0);                              //выключаем выходы
                             REVERSE(0);
                             LEFT=0;
                             RIGHT=0;

                            }
                      if(timeOffDevice_counter >=0 )
                          {
                             TRIGGER=0;                                //выключаем свет
                             FORWARD(0);                              //выключаем выходы
                             REVERSE(0);
                             LEFT=0;
                             RIGHT=0;
                             PWR_RECEIVER =0;                         //выключить питание приемника
                             asm {sleep};                            //всем спать ))
                          }
                            
        }




}

//------------------------------------------------------------------------------