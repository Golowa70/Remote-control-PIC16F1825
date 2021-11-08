
_ManReceiveStart:

;Manch_16f1825_rx.c,82 :: 		void ManReceiveStart (void)
;Manch_16f1825_rx.c,85 :: 		INTCON.GIE=0;                                       //запретить все прерывания
	BCF        INTCON+0, 7
;Manch_16f1825_rx.c,87 :: 		OPTION_REG=0b10000101;                             //предделитель на 64 (частота счета 16000000/4 / 32 = 125000 Hz = 8 uS)  //64?????????
	MOVLW      133
	MOVWF      OPTION_REG+0
;Manch_16f1825_rx.c,88 :: 		INTCON.TMR0IE=1;                                   //прерывание при переполнении
	BSF        INTCON+0, 5
;Manch_16f1825_rx.c,91 :: 		INTCON.IOCIE=1;                                   //разрешение прерываний по изменению уровня
	BSF        INTCON+0, 3
;Manch_16f1825_rx.c,92 :: 		IOCAP.IOCAP4=1;                                   //включить прерывание  по фронту от RA5
	BSF        IOCAP+0, 4
;Manch_16f1825_rx.c,93 :: 		IOCAN.IOCAN4=1;                                   //включить прерывание  по спаду от RA5
	BSF        IOCAN+0, 4
;Manch_16f1825_rx.c,98 :: 		ManFlags &= ~(bTIM0_OVF| bDATA_ENBL);             //очистить флаг наличия данных и флаг переполнения
	MOVLW      252
	ANDWF      _ManFlags+0, 1
;Manch_16f1825_rx.c,99 :: 		ManFlags |= bHEADER_RCV;                          //включить режим приема заголовка
	BSF        _ManFlags+0, 3
;Manch_16f1825_rx.c,100 :: 		ByteCounter = 0;                                  //начать прием с начала
	CLRF       _ByteCounter+0
;Manch_16f1825_rx.c,101 :: 		ByteIn = 0x00;                                    //очистить байт приемник
	CLRF       _ByteIn+0
;Manch_16f1825_rx.c,103 :: 		INTCON.GIE=1;                                     //разрешить все прерывания
	BSF        INTCON+0, 7
;Manch_16f1825_rx.c,104 :: 		}
L_end_ManReceiveStart:
	RETURN
; end of _ManReceiveStart

_ManReceiveStop:

;Manch_16f1825_rx.c,109 :: 		void ManReceiveStop (void)
;Manch_16f1825_rx.c,111 :: 		INTCON.GIE=0;
	BCF        INTCON+0, 7
;Manch_16f1825_rx.c,112 :: 		INTCON.TMR0IE=0;                                  //выключить "прерывание при переполнении Т0"
	BCF        INTCON+0, 5
;Manch_16f1825_rx.c,113 :: 		INTCON.IOCIE=0;                                   //выключить "внешнее прерывание от IOC"
	BCF        INTCON+0, 3
;Manch_16f1825_rx.c,114 :: 		INTCON.GIE=1;
	BSF        INTCON+0, 7
;Manch_16f1825_rx.c,115 :: 		}
L_end_ManReceiveStop:
	RETURN
; end of _ManReceiveStop

_ManRcvDataCheck:

;Manch_16f1825_rx.c,121 :: 		unsigned char* ManRcvDataCheck (void)
;Manch_16f1825_rx.c,123 :: 		if (ManFlags & bDATA_ENBL)                             //проверка наличия принятых данных
	BTFSS      _ManFlags+0, 0
	GOTO       L_ManRcvDataCheck0
;Manch_16f1825_rx.c,125 :: 		ManFlags &= ~bDATA_ENBL;                       //очистить флаг наличия данных
	BCF        _ManFlags+0, 0
;Manch_16f1825_rx.c,126 :: 		return ManBuffer;                              //при наличии данных - возвращаем указатель на буффер
	MOVLW      _ManBuffer+0
	MOVWF      R0
	MOVLW      hi_addr(_ManBuffer+0)
	MOVWF      R1
	GOTO       L_end_ManRcvDataCheck
;Manch_16f1825_rx.c,127 :: 		}
L_ManRcvDataCheck0:
;Manch_16f1825_rx.c,128 :: 		return 0;                                              //при отсутствии данных - возвращаем 0
	CLRF       R0
	CLRF       R1
;Manch_16f1825_rx.c,129 :: 		}
L_end_ManRcvDataCheck:
	RETURN
; end of _ManRcvDataCheck

_interrupt:

;Manch_16f1825_rx.c,135 :: 		void interrupt (void)  {
;Manch_16f1825_rx.c,136 :: 		if( PIE3.TMR6IE && PIR3.TMR6IF ) {
	BTFSS      PIE3+0, 3
	GOTO       L_interrupt3
	BTFSS      PIR3+0, 3
	GOTO       L_interrupt3
L__interrupt64:
;Manch_16f1825_rx.c,137 :: 		PIR3.TMR6IF=0;
	BCF        PIR3+0, 3
;Manch_16f1825_rx.c,138 :: 		if( timeOffOut_counter <0)
	MOVLW      128
	XORWF      _timeOffOut_counter+1, 0
	MOVWF      R0
	MOVLW      128
	SUBWF      R0, 0
	BTFSS      STATUS+0, 2
	GOTO       L__interrupt70
	MOVLW      0
	SUBWF      _timeOffOut_counter+0, 0
L__interrupt70:
	BTFSC      STATUS+0, 0
	GOTO       L_interrupt4
;Manch_16f1825_rx.c,139 :: 		timeOffOut_counter++;
	INCF       _timeOffOut_counter+0, 1
	BTFSC      STATUS+0, 2
	INCF       _timeOffOut_counter+1, 1
L_interrupt4:
;Manch_16f1825_rx.c,140 :: 		if(timeOffDevice_counter <0)
	MOVLW      128
	XORWF      _timeOffDevice_counter+1, 0
	MOVWF      R0
	MOVLW      128
	SUBWF      R0, 0
	BTFSS      STATUS+0, 2
	GOTO       L__interrupt71
	MOVLW      0
	SUBWF      _timeOffDevice_counter+0, 0
L__interrupt71:
	BTFSC      STATUS+0, 0
	GOTO       L_interrupt5
;Manch_16f1825_rx.c,141 :: 		timeOffDevice_counter++;
	INCF       _timeOffDevice_counter+0, 1
	BTFSC      STATUS+0, 2
	INCF       _timeOffDevice_counter+1, 1
L_interrupt5:
;Manch_16f1825_rx.c,143 :: 		asm {clrwdt};                                    //сброс собаки
	CLRWDT
;Manch_16f1825_rx.c,144 :: 		}
L_interrupt3:
;Manch_16f1825_rx.c,149 :: 		if (INTCON.T0IF && INTCON.T0IE)
	BTFSS      INTCON+0, 2
	GOTO       L_interrupt8
	BTFSS      INTCON+0, 5
	GOTO       L_interrupt8
L__interrupt63:
;Manch_16f1825_rx.c,152 :: 		INTCON.T0IF=0;                                              //сбросить флаг переполнения таймера
	BCF        INTCON+0, 2
;Manch_16f1825_rx.c,153 :: 		if (INTCON.IOCIE==1)                                        //если ожидали внеш прерывания от IOC -
	BTFSS      INTCON+0, 3
	GOTO       L_interrupt9
;Manch_16f1825_rx.c,154 :: 		ManFlags |= bTIM0_OVF;                              // - отметить переполнение
	BSF        _ManFlags+0, 1
	GOTO       L_interrupt10
L_interrupt9:
;Manch_16f1825_rx.c,158 :: 		if (MAN_IN_PIN & MAN_IN_LINE)    {
	BTFSS      PORTA+0, 4
	GOTO       L_interrupt11
;Manch_16f1825_rx.c,159 :: 		ManFlags |= bLINE_VAL;
	BSF        _ManFlags+0, 2
;Manch_16f1825_rx.c,160 :: 		}
	GOTO       L_interrupt12
L_interrupt11:
;Manch_16f1825_rx.c,163 :: 		ManFlags &= ~bLINE_VAL;
	BCF        _ManFlags+0, 2
;Manch_16f1825_rx.c,164 :: 		}
L_interrupt12:
;Manch_16f1825_rx.c,166 :: 		INTCON.IOCIE=1;                                        //включить внешние прерывания от IOC
	BSF        INTCON+0, 3
;Manch_16f1825_rx.c,167 :: 		INTCON.IOCIF=0;                                        //сбросить возможно проскочившее прерывание
	BCF        INTCON+0, 0
;Manch_16f1825_rx.c,168 :: 		IOCAF.IOCAF4=0;                                        //--''--
	BCF        IOCAF+0, 4
;Manch_16f1825_rx.c,170 :: 		asm {clrwdt};                                    //сброс собаки
	CLRWDT
;Manch_16f1825_rx.c,171 :: 		}
L_interrupt10:
;Manch_16f1825_rx.c,173 :: 		}
L_interrupt8:
;Manch_16f1825_rx.c,177 :: 		if (INTCON.IOCIE && INTCON.IOCIF && IOCAF.IOCAF4)
	BTFSS      INTCON+0, 3
	GOTO       L_interrupt15
	BTFSS      INTCON+0, 0
	GOTO       L_interrupt15
	BTFSS      IOCAF+0, 4
	GOTO       L_interrupt15
L__interrupt62:
;Manch_16f1825_rx.c,181 :: 		TimerVal = TMR0;
	MOVF       TMR0+0, 0
	MOVWF      _TimerVal+0
;Manch_16f1825_rx.c,182 :: 		TMR0 = 255 - ((MAN_PERIOD_LEN )* 3 / 4);               //счетчик таймера настроить на 3/4 длины периода MANCHESTER бита данных
	MOVLW      209
	MOVWF      TMR0+0
;Manch_16f1825_rx.c,183 :: 		INTCON.IOCIE=0;                                        //выключить внешнее прерывание
	BCF        INTCON+0, 3
;Manch_16f1825_rx.c,184 :: 		INTCON.IOCIF=0;                                        //на случай ВЧ сигнала сбросить возможно проскочившее повторное прерывание
	BCF        INTCON+0, 0
;Manch_16f1825_rx.c,185 :: 		IOCAF.IOCAF4=0;
	BCF        IOCAF+0, 4
;Manch_16f1825_rx.c,188 :: 		if ( (TimerVal > (MAN_PERIOD_LEN/2)) || (ManFlags & bTIM0_OVF))
	MOVF       _TimerVal+0, 0
	SUBLW      31
	BTFSS      STATUS+0, 0
	GOTO       L__interrupt61
	BTFSC      _ManFlags+0, 1
	GOTO       L__interrupt61
	GOTO       L_interrupt18
L__interrupt61:
;Manch_16f1825_rx.c,191 :: 		Ini :
___interrupt_Ini:
;Manch_16f1825_rx.c,192 :: 		asm {clrwdt};                                    //сброс собаки
	CLRWDT
;Manch_16f1825_rx.c,194 :: 		ManFlags &= ~(bTIM0_OVF);                       //сбросить флаг переполнения
	BCF        _ManFlags+0, 1
;Manch_16f1825_rx.c,195 :: 		ManFlags |= bHEADER_RCV;                        //ожидать прием заголовка
	BSF        _ManFlags+0, 3
;Manch_16f1825_rx.c,196 :: 		ByteCounter = 0;                                //начать прием с начала
	CLRF       _ByteCounter+0
;Manch_16f1825_rx.c,197 :: 		ByteIn = 0x00;                                  //очистить байт приемник
	CLRF       _ByteIn+0
;Manch_16f1825_rx.c,199 :: 		}
L_interrupt18:
;Manch_16f1825_rx.c,202 :: 		ByteIn <<= 1;                                           //сдвигаем байт перед записью бита
	LSLF       _ByteIn+0, 1
;Manch_16f1825_rx.c,204 :: 		if (! (ManFlags & bLINE_VAL))   {
	BTFSC      _ManFlags+0, 2
	GOTO       L_interrupt19
;Manch_16f1825_rx.c,205 :: 		ByteIn |= 1;
	BSF        _ByteIn+0, 0
;Manch_16f1825_rx.c,206 :: 		}
L_interrupt19:
;Manch_16f1825_rx.c,210 :: 		if (ManFlags & bHEADER_RCV)
	BTFSS      _ManFlags+0, 3
	GOTO       L_interrupt20
;Manch_16f1825_rx.c,213 :: 		if (ByteCounter == 0)
	MOVF       _ByteCounter+0, 0
	XORLW      0
	BTFSS      STATUS+0, 2
	GOTO       L_interrupt21
;Manch_16f1825_rx.c,215 :: 		Invert = ~ManIdentifier [0];
	COMF       _ManIdentifier+0, 0
	MOVWF      _Invert+0
;Manch_16f1825_rx.c,217 :: 		if (ByteIn != ManIdentifier [0]) {                            //?????? я добавил скобки
	MOVF       _ByteIn+0, 0
	XORWF      _ManIdentifier+0, 0
	BTFSC      STATUS+0, 2
	GOTO       L_interrupt22
;Manch_16f1825_rx.c,219 :: 		if (ByteIn != Invert)
	MOVF       _ByteIn+0, 0
	XORWF      _Invert+0, 0
	BTFSC      STATUS+0, 2
	GOTO       L_interrupt23
;Manch_16f1825_rx.c,221 :: 		return;                                        //пока нет совпадения - выход
	GOTO       L__interrupt69
L_interrupt23:
;Manch_16f1825_rx.c,222 :: 		}
L_interrupt22:
;Manch_16f1825_rx.c,223 :: 		if (ByteIn == ManIdentifier [0]) {
	MOVF       _ByteIn+0, 0
	XORWF      _ManIdentifier+0, 0
	BTFSS      STATUS+0, 2
	GOTO       L_interrupt24
;Manch_16f1825_rx.c,224 :: 		ManFlags &= ~bLINE_INV;                //прямое совпадение
	BCF        _ManFlags+0, 4
;Manch_16f1825_rx.c,226 :: 		}
	GOTO       L_interrupt25
L_interrupt24:
;Manch_16f1825_rx.c,229 :: 		ManFlags |= bLINE_INV;                //инверсное совпадение
	BSF        _ManFlags+0, 4
;Manch_16f1825_rx.c,230 :: 		}
L_interrupt25:
;Manch_16f1825_rx.c,231 :: 		BitCounter = 0;                                        //готовимся к приему следующих байтов хедера
	CLRF       _BitCounter+0
;Manch_16f1825_rx.c,232 :: 		ByteCounter++;
	INCF       _ByteCounter+0, 1
;Manch_16f1825_rx.c,233 :: 		return;
	GOTO       L__interrupt69
;Manch_16f1825_rx.c,234 :: 		}
L_interrupt21:
;Manch_16f1825_rx.c,236 :: 		asm {clrwdt};                                    //сброс собаки
	CLRWDT
;Manch_16f1825_rx.c,238 :: 		if (++BitCounter < 8)                                //ждем заполнения байта
	INCF       _BitCounter+0, 1
	MOVLW      8
	SUBWF      _BitCounter+0, 0
	BTFSC      STATUS+0, 0
	GOTO       L_interrupt26
;Manch_16f1825_rx.c,239 :: 		return;
	GOTO       L__interrupt69
L_interrupt26:
;Manch_16f1825_rx.c,241 :: 		if (ManFlags & bLINE_INV)                        //если сигнал инверсный
	BTFSS      _ManFlags+0, 4
	GOTO       L_interrupt27
;Manch_16f1825_rx.c,242 :: 		ByteIn = ~ByteIn;
	COMF       _ByteIn+0, 1
L_interrupt27:
;Manch_16f1825_rx.c,244 :: 		if (ManIdentifier [ByteCounter])        //если хедер еще не закончен
	MOVLW      _ManIdentifier+0
	MOVWF      FSR0L
	MOVLW      hi_addr(_ManIdentifier+0)
	MOVWF      FSR0H
	MOVF       _ByteCounter+0, 0
	ADDWF      FSR0L, 1
	BTFSC      STATUS+0, 0
	INCF       FSR0H, 1
	MOVF       INDF0+0, 0
	BTFSC      STATUS+0, 2
	GOTO       L_interrupt28
;Manch_16f1825_rx.c,246 :: 		if (ByteIn != ManIdentifier [ByteCounter]){             //проверяем идентичность хедера
	MOVLW      _ManIdentifier+0
	MOVWF      FSR0L
	MOVLW      hi_addr(_ManIdentifier+0)
	MOVWF      FSR0H
	MOVF       _ByteCounter+0, 0
	ADDWF      FSR0L, 1
	BTFSC      STATUS+0, 0
	INCF       FSR0H, 1
	MOVF       _ByteIn+0, 0
	XORWF      INDF0+0, 0
	BTFSC      STATUS+0, 2
	GOTO       L_interrupt29
;Manch_16f1825_rx.c,247 :: 		goto Ini;                                        //байт не соответствует хедеру - рестарт
	GOTO       ___interrupt_Ini
;Manch_16f1825_rx.c,248 :: 		}
L_interrupt29:
;Manch_16f1825_rx.c,249 :: 		BitCounter = 0;
	CLRF       _BitCounter+0
;Manch_16f1825_rx.c,250 :: 		ByteCounter++;                                         //ожидаем следующий байт хедера
	INCF       _ByteCounter+0, 1
;Manch_16f1825_rx.c,251 :: 		return;
	GOTO       L__interrupt69
;Manch_16f1825_rx.c,252 :: 		}
L_interrupt28:
;Manch_16f1825_rx.c,255 :: 		if (ByteIn > MAN_BUF_LENGTH)  {
	MOVF       _ByteIn+0, 0
	SUBLW      16
	BTFSC      STATUS+0, 0
	GOTO       L_interrupt30
;Manch_16f1825_rx.c,257 :: 		goto Ini;                                             //размер блока данных превышает допустимый - рестарт
	GOTO       ___interrupt_Ini
;Manch_16f1825_rx.c,259 :: 		}
L_interrupt30:
;Manch_16f1825_rx.c,261 :: 		DataLength = ByteIn;                                        //запомним длину пакета
	MOVF       _ByteIn+0, 0
	MOVWF      _DataLength+0
;Manch_16f1825_rx.c,263 :: 		CheckSummByte = 0;                                         //очистить байт контрольной суммы
	CLRF       _CheckSummByte+0
;Manch_16f1825_rx.c,264 :: 		CheckSumm (ByteIn);                                        //подсчет контрольки, начиная с байта длины пакета
	MOVF       _ByteIn+0, 0
	MOVWF      FARG_CheckSumm_dataa+0
	CALL       _CheckSumm+0
;Manch_16f1825_rx.c,266 :: 		ManFlags &= ~bHEADER_RCV;                                 //переходим к приему основного файла
	BCF        _ManFlags+0, 3
;Manch_16f1825_rx.c,267 :: 		BitCounter = 0;
	CLRF       _BitCounter+0
;Manch_16f1825_rx.c,268 :: 		ByteCounter = 0;
	CLRF       _ByteCounter+0
;Manch_16f1825_rx.c,269 :: 		return;
	GOTO       L__interrupt69
;Manch_16f1825_rx.c,270 :: 		}
L_interrupt20:
;Manch_16f1825_rx.c,271 :: 		asm {clrwdt};                                    //сброс собаки
	CLRWDT
;Manch_16f1825_rx.c,273 :: 		if (++BitCounter < 8)                                        //ждем накопления байта
	INCF       _BitCounter+0, 1
	MOVLW      8
	SUBWF      _BitCounter+0, 0
	BTFSC      STATUS+0, 0
	GOTO       L_interrupt31
;Manch_16f1825_rx.c,274 :: 		return;
	GOTO       L__interrupt69
L_interrupt31:
;Manch_16f1825_rx.c,275 :: 		BitCounter = 0;
	CLRF       _BitCounter+0
;Manch_16f1825_rx.c,277 :: 		if (ManFlags & bLINE_INV)                                   //необходима ли инверсия
	BTFSS      _ManFlags+0, 4
	GOTO       L_interrupt32
;Manch_16f1825_rx.c,278 :: 		ByteIn = ~ByteIn;
	COMF       _ByteIn+0, 1
L_interrupt32:
;Manch_16f1825_rx.c,280 :: 		CheckSumm (ByteIn);                                         //подсчет контрольки
	MOVF       _ByteIn+0, 0
	MOVWF      FARG_CheckSumm_dataa+0
	CALL       _CheckSumm+0
;Manch_16f1825_rx.c,282 :: 		if (DataLength--) {                                         //если это еще байты пакета -
	MOVF       _DataLength+0, 0
	MOVWF      R0
	DECF       _DataLength+0, 1
	MOVF       R0, 0
	BTFSC      STATUS+0, 2
	GOTO       L_interrupt33
;Manch_16f1825_rx.c,283 :: 		ManBuffer [ByteCounter++] = ByteIn;                   // - сохраняем принятый байт
	MOVLW      _ManBuffer+0
	MOVWF      FSR1L
	MOVLW      hi_addr(_ManBuffer+0)
	MOVWF      FSR1H
	MOVF       _ByteCounter+0, 0
	ADDWF      FSR1L, 1
	BTFSC      STATUS+0, 0
	INCF       FSR1H, 1
	MOVF       _ByteIn+0, 0
	MOVWF      INDF1+0
	INCF       _ByteCounter+0, 1
;Manch_16f1825_rx.c,284 :: 		}
	GOTO       L_interrupt34
L_interrupt33:
;Manch_16f1825_rx.c,288 :: 		if (CheckSummByte) {                              //если контролька не верна (не 0) -
	MOVF       _CheckSummByte+0, 0
	BTFSC      STATUS+0, 2
	GOTO       L_interrupt35
;Manch_16f1825_rx.c,289 :: 		goto Ini;                                   // - рестарт
	GOTO       ___interrupt_Ini
;Manch_16f1825_rx.c,290 :: 		}
L_interrupt35:
;Manch_16f1825_rx.c,293 :: 		ManFlags |= bDATA_ENBL;                          //установить флаг наличия данных
	BSF        _ManFlags+0, 0
;Manch_16f1825_rx.c,294 :: 		ManReceiveStop ();                               //тормозим дальнейший прием
	CALL       _ManReceiveStop+0
;Manch_16f1825_rx.c,295 :: 		}
L_interrupt34:
;Manch_16f1825_rx.c,298 :: 		asm {clrwdt};                                    //сброс собаки
	CLRWDT
;Manch_16f1825_rx.c,299 :: 		}
L_interrupt15:
;Manch_16f1825_rx.c,300 :: 		}
L_end_interrupt:
L__interrupt69:
	RETFIE     %s
; end of _interrupt

_CheckSumm:

;Manch_16f1825_rx.c,308 :: 		void CheckSumm(unsigned char dataa)
;Manch_16f1825_rx.c,309 :: 		{              unsigned char i=0;
	CLRF       CheckSumm_i_L0+0
;Manch_16f1825_rx.c,310 :: 		for ( i=0; i<8; i++)
	CLRF       CheckSumm_i_L0+0
L_CheckSumm36:
	MOVLW      8
	SUBWF      CheckSumm_i_L0+0, 0
	BTFSC      STATUS+0, 0
	GOTO       L_CheckSumm37
;Manch_16f1825_rx.c,312 :: 		unsigned char temp = dataa;
	MOVF       FARG_CheckSumm_dataa+0, 0
	MOVWF      R2+0
;Manch_16f1825_rx.c,313 :: 		temp ^= CheckSummByte;
	MOVF       _CheckSummByte+0, 0
	XORWF      FARG_CheckSumm_dataa+0, 0
	MOVWF      R1
	MOVF       R1, 0
	MOVWF      R2+0
;Manch_16f1825_rx.c,315 :: 		if (temp & 0x01)        {CheckSummByte ^= 0x18; temp = 0x80;}
	BTFSS      R1, 0
	GOTO       L_CheckSumm39
	MOVLW      24
	XORWF      _CheckSummByte+0, 1
	MOVLW      128
	MOVWF      R2+0
	GOTO       L_CheckSumm40
L_CheckSumm39:
;Manch_16f1825_rx.c,316 :: 		else                                temp = 0;
	CLRF       R2+0
L_CheckSumm40:
;Manch_16f1825_rx.c,318 :: 		CheckSummByte >>= 1;
	LSRF       _CheckSummByte+0, 1
;Manch_16f1825_rx.c,319 :: 		CheckSummByte |= temp;
	MOVF       R2+0, 0
	IORWF       _CheckSummByte+0, 1
;Manch_16f1825_rx.c,320 :: 		dataa >>= 1;
	LSRF       FARG_CheckSumm_dataa+0, 1
;Manch_16f1825_rx.c,310 :: 		for ( i=0; i<8; i++)
	INCF       CheckSumm_i_L0+0, 1
;Manch_16f1825_rx.c,321 :: 		}
	GOTO       L_CheckSumm36
L_CheckSumm37:
;Manch_16f1825_rx.c,322 :: 		}
L_end_CheckSumm:
	RETURN
; end of _CheckSumm

_main:

;Manch_16f1825_rx.c,325 :: 		void main (void){
;Manch_16f1825_rx.c,327 :: 		OSCCON=0b11111111;                                  //  16 MHz HF
	MOVLW      255
	MOVWF      OSCCON+0
;Manch_16f1825_rx.c,328 :: 		TRISA=0b00010000;
	MOVLW      16
	MOVWF      TRISA+0
;Manch_16f1825_rx.c,329 :: 		PORTA=0b00000000;
	CLRF       PORTA+0
;Manch_16f1825_rx.c,330 :: 		TRISC=0b00000000;
	CLRF       TRISC+0
;Manch_16f1825_rx.c,331 :: 		PORTC=0b00000000;
	CLRF       PORTC+0
;Manch_16f1825_rx.c,332 :: 		ANSELA  = 0;                                       //выключение АЦП
	CLRF       ANSELA+0
;Manch_16f1825_rx.c,333 :: 		ANSELC  = 0;
	CLRF       ANSELC+0
;Manch_16f1825_rx.c,334 :: 		CM1CON0=0;                                         //выключение компаратора
	CLRF       CM1CON0+0
;Manch_16f1825_rx.c,335 :: 		CM2CON0=0;
	CLRF       CM2CON0+0
;Manch_16f1825_rx.c,337 :: 		T6CON=0b11111111;                              //настройка TMR6.Прескалер 64,постскалер 16.
	MOVLW      255
	MOVWF      T6CON+0
;Manch_16f1825_rx.c,338 :: 		PIE3.TMR6IE=1;                                 //разрешение прерываний от TMR6
	BSF        PIE3+0, 3
;Manch_16f1825_rx.c,340 :: 		asm {clrwdt};                                    //сброс собаки
	CLRWDT
;Manch_16f1825_rx.c,342 :: 		PWM1_Init(1500);                                 //частота ШИМ
	BSF        T2CON+0, 0
	BSF        T2CON+0, 1
	MOVLW      166
	MOVWF      PR2+0
	CALL       _PWM1_Init+0
;Manch_16f1825_rx.c,343 :: 		PWM1_Start();
	CALL       _PWM1_Start+0
;Manch_16f1825_rx.c,344 :: 		PWM1_Set_Duty(0);
	CLRF       FARG_PWM1_Set_Duty_new_duty+0
	CALL       _PWM1_Set_Duty+0
;Manch_16f1825_rx.c,345 :: 		PWM2_Init(1500);
	BSF        T2CON+0, 0
	BSF        T2CON+0, 1
	MOVLW      166
	MOVWF      PR2+0
	CALL       _PWM2_Init+0
;Manch_16f1825_rx.c,346 :: 		PWM2_Start();
	CALL       _PWM2_Start+0
;Manch_16f1825_rx.c,347 :: 		PWM2_Set_Duty(0);
	CLRF       FARG_PWM2_Set_Duty_new_duty+0
	CALL       _PWM2_Set_Duty+0
;Manch_16f1825_rx.c,349 :: 		PWR_RECEIVER =1;                                // включить питание приемника
	BSF        LATC+0, 4
;Manch_16f1825_rx.c,350 :: 		ManReceiveStart ();                             // старт приема данных
	CALL       _ManReceiveStart+0
;Manch_16f1825_rx.c,351 :: 		timeOffDevice_counter = - 9000;                //таймер полного выключения 9000-10 мин
	MOVLW      216
	MOVWF      _timeOffDevice_counter+0
	MOVLW      220
	MOVWF      _timeOffDevice_counter+1
;Manch_16f1825_rx.c,353 :: 		while (1)
L_main41:
;Manch_16f1825_rx.c,357 :: 		unsigned char *pBuf = ManRcvDataCheck();                    //проверка наличия данных
	CALL       _ManRcvDataCheck+0
	MOVF       R0, 0
	MOVWF      main_pBuf_L1+0
	MOVF       R1, 0
	MOVWF      main_pBuf_L1+1
;Manch_16f1825_rx.c,358 :: 		asm {clrwdt};                                    //сброс собаки
	CLRWDT
;Manch_16f1825_rx.c,359 :: 		if (pBuf)                                                   //если указатель не нулевой, значит данные поступили
	MOVF       main_pBuf_L1+0, 0
	IORWF       main_pBuf_L1+1, 0
	BTFSC      STATUS+0, 2
	GOTO       L_main43
;Manch_16f1825_rx.c,361 :: 		timeOffDevice_counter = -9000;                        //обновляем таймер выключения
	MOVLW      216
	MOVWF      _timeOffDevice_counter+0
	MOVLW      220
	MOVWF      _timeOffDevice_counter+1
;Manch_16f1825_rx.c,362 :: 		codeButtons = *pBuf;                                //копируем первый байт буфера
	MOVF       main_pBuf_L1+0, 0
	MOVWF      FSR0L
	MOVF       main_pBuf_L1+1, 0
	MOVWF      FSR0H
	MOVF       INDF0+0, 0
	MOVWF      R1
	MOVF       R1, 0
	MOVWF      _codeButtons+0
;Manch_16f1825_rx.c,363 :: 		*pBuf++;
	INCF       main_pBuf_L1+0, 1
	BTFSC      STATUS+0, 2
	INCF       main_pBuf_L1+1, 1
;Manch_16f1825_rx.c,364 :: 		speedLevel = *pBuf;
	MOVF       main_pBuf_L1+0, 0
	MOVWF      FSR0L
	MOVF       main_pBuf_L1+1, 0
	MOVWF      FSR0H
	MOVF       INDF0+0, 0
	MOVWF      _speedLevel+0
;Manch_16f1825_rx.c,366 :: 		if(codeButtons & (1<<0))                          //проверяем какая кнопка нажата
	BTFSS      R1, 0
	GOTO       L_main44
;Manch_16f1825_rx.c,368 :: 		if( ++timerStartForward_counter <5)  FORWARD(255); //если вперед,при старте ШИМ на 100
	INCF       _timerStartForward_counter+0, 1
	MOVLW      5
	SUBWF      _timerStartForward_counter+0, 0
	BTFSC      STATUS+0, 0
	GOTO       L_main45
	MOVLW      255
	MOVWF      FARG_PWM1_Set_Duty_new_duty+0
	CALL       _PWM1_Set_Duty+0
	GOTO       L_main46
L_main45:
;Manch_16f1825_rx.c,370 :: 		FORWARD(speedLevel);                          //передаем в ШИМ значение скорости
	MOVF       _speedLevel+0, 0
	MOVWF      FARG_PWM1_Set_Duty_new_duty+0
	CALL       _PWM1_Set_Duty+0
L_main46:
;Manch_16f1825_rx.c,371 :: 		}
	GOTO       L_main47
L_main44:
;Manch_16f1825_rx.c,374 :: 		timerStartForward_counter=0;
	CLRF       _timerStartForward_counter+0
;Manch_16f1825_rx.c,375 :: 		FORWARD(0);
	CLRF       FARG_PWM1_Set_Duty_new_duty+0
	CALL       _PWM1_Set_Duty+0
;Manch_16f1825_rx.c,376 :: 		}
L_main47:
;Manch_16f1825_rx.c,378 :: 		if(codeButtons & (1<<1))                              //если назад,передаем в ШИМ значение скорости
	BTFSS      _codeButtons+0, 1
	GOTO       L_main48
;Manch_16f1825_rx.c,380 :: 		if(++ timerStartReverse_counter<5) REVERSE(255);
	INCF       _timerStartReverse_counter+0, 1
	MOVLW      5
	SUBWF      _timerStartReverse_counter+0, 0
	BTFSC      STATUS+0, 0
	GOTO       L_main49
	MOVLW      255
	MOVWF      FARG_PWM2_Set_Duty_new_duty+0
	CALL       _PWM2_Set_Duty+0
	GOTO       L_main50
L_main49:
;Manch_16f1825_rx.c,382 :: 		REVERSE (speedlevel);
	MOVF       _speedLevel+0, 0
	MOVWF      FARG_PWM2_Set_Duty_new_duty+0
	CALL       _PWM2_Set_Duty+0
L_main50:
;Manch_16f1825_rx.c,383 :: 		}
	GOTO       L_main51
L_main48:
;Manch_16f1825_rx.c,386 :: 		timerStartReverse_counter=0;
	CLRF       _timerStartReverse_counter+0
;Manch_16f1825_rx.c,387 :: 		REVERSE(0);
	CLRF       FARG_PWM2_Set_Duty_new_duty+0
	CALL       _PWM2_Set_Duty+0
;Manch_16f1825_rx.c,388 :: 		}
L_main51:
;Manch_16f1825_rx.c,391 :: 		if(codeButtons & (1<<2)) LEFT =1;                    //в лево
	BTFSS      _codeButtons+0, 2
	GOTO       L_main52
	BSF        LATC+0, 2
	GOTO       L_main53
L_main52:
;Manch_16f1825_rx.c,392 :: 		else  LEFT=0;
	BCF        LATC+0, 2
L_main53:
;Manch_16f1825_rx.c,394 :: 		if(codeButtons & (1<<3)) RIGHT =1;                   //в право
	BTFSS      _codeButtons+0, 3
	GOTO       L_main54
	BSF        LATC+0, 1
	GOTO       L_main55
L_main54:
;Manch_16f1825_rx.c,395 :: 		else  RIGHT=0;
	BCF        LATC+0, 1
L_main55:
;Manch_16f1825_rx.c,397 :: 		if(codeButtons & (1<<4)) TRIGGER =1;                 //включаем свет
	BTFSS      _codeButtons+0, 4
	GOTO       L_main56
	BSF        LATC+0, 0
	GOTO       L_main57
L_main56:
;Manch_16f1825_rx.c,399 :: 		if(codeButtons & (1<<5))
	BTFSS      _codeButtons+0, 5
	GOTO       L_main58
;Manch_16f1825_rx.c,400 :: 		TRIGGER=0;                                      //выключаем свет
	BCF        LATC+0, 0
L_main58:
;Manch_16f1825_rx.c,401 :: 		}
L_main57:
;Manch_16f1825_rx.c,404 :: 		ManBuffer[1]=0;                                     // обнуляем первый байт буфера данных
	CLRF       _ManBuffer+1
;Manch_16f1825_rx.c,405 :: 		ManBuffer[2]=0;                                     // обнуляем второй байт буфера данных
	CLRF       _ManBuffer+2
;Manch_16f1825_rx.c,406 :: 		codeButtons=0;                                      // обнуляем переменную кнопок
	CLRF       _codeButtons+0
;Manch_16f1825_rx.c,407 :: 		speedLevel=0;                                      // обнуляем переменную скорости
	CLRF       _speedLevel+0
;Manch_16f1825_rx.c,409 :: 		timeOffOut_counter = -3;                          //запуск таймера выключения выходов 2-160мс,3-240мсек.
	MOVLW      253
	MOVWF      _timeOffOut_counter+0
	MOVLW      255
	MOVWF      _timeOffOut_counter+1
;Manch_16f1825_rx.c,411 :: 		ManReceiveStart ();                                //перезапуск процесса чтения MANCHESTER данных
	CALL       _ManReceiveStart+0
;Manch_16f1825_rx.c,414 :: 		}
L_main43:
;Manch_16f1825_rx.c,417 :: 		asm {clrwdt};                                    //сброс собаки
	CLRWDT
;Manch_16f1825_rx.c,419 :: 		if(timeOffOut_counter>=0)                       //таймер отсчитал
	MOVLW      128
	XORWF      _timeOffOut_counter+1, 0
	MOVWF      R0
	MOVLW      128
	SUBWF      R0, 0
	BTFSS      STATUS+0, 2
	GOTO       L__main74
	MOVLW      0
	SUBWF      _timeOffOut_counter+0, 0
L__main74:
	BTFSS      STATUS+0, 0
	GOTO       L_main59
;Manch_16f1825_rx.c,421 :: 		FORWARD(0);                              //выключаем выходы
	CLRF       FARG_PWM1_Set_Duty_new_duty+0
	CALL       _PWM1_Set_Duty+0
;Manch_16f1825_rx.c,422 :: 		REVERSE(0);
	CLRF       FARG_PWM2_Set_Duty_new_duty+0
	CALL       _PWM2_Set_Duty+0
;Manch_16f1825_rx.c,423 :: 		LEFT=0;
	BCF        LATC+0, 2
;Manch_16f1825_rx.c,424 :: 		RIGHT=0;
	BCF        LATC+0, 1
;Manch_16f1825_rx.c,426 :: 		}
L_main59:
;Manch_16f1825_rx.c,427 :: 		if(timeOffDevice_counter >=0 )
	MOVLW      128
	XORWF      _timeOffDevice_counter+1, 0
	MOVWF      R0
	MOVLW      128
	SUBWF      R0, 0
	BTFSS      STATUS+0, 2
	GOTO       L__main75
	MOVLW      0
	SUBWF      _timeOffDevice_counter+0, 0
L__main75:
	BTFSS      STATUS+0, 0
	GOTO       L_main60
;Manch_16f1825_rx.c,429 :: 		TRIGGER=0;                                //выключаем свет
	BCF        LATC+0, 0
;Manch_16f1825_rx.c,430 :: 		FORWARD(0);                              //выключаем выходы
	CLRF       FARG_PWM1_Set_Duty_new_duty+0
	CALL       _PWM1_Set_Duty+0
;Manch_16f1825_rx.c,431 :: 		REVERSE(0);
	CLRF       FARG_PWM2_Set_Duty_new_duty+0
	CALL       _PWM2_Set_Duty+0
;Manch_16f1825_rx.c,432 :: 		LEFT=0;
	BCF        LATC+0, 2
;Manch_16f1825_rx.c,433 :: 		RIGHT=0;
	BCF        LATC+0, 1
;Manch_16f1825_rx.c,434 :: 		PWR_RECEIVER =0;                         //выключить питание приемника
	BCF        LATC+0, 4
;Manch_16f1825_rx.c,435 :: 		asm {sleep};                            //всем спать ))
	SLEEP
;Manch_16f1825_rx.c,436 :: 		}
L_main60:
;Manch_16f1825_rx.c,438 :: 		}
	GOTO       L_main41
;Manch_16f1825_rx.c,443 :: 		}
L_end_main:
	GOTO       $+0
; end of _main
