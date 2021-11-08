
_ManInit:

;Manch_16f1825_tx.c,57 :: 		void ManInit (void)
;Manch_16f1825_tx.c,59 :: 		TRANSMIT_TRIS &= ~TRANSMIT_LINE;                        //лини€ на вывод
	BCF        TRISC+0, 0
;Manch_16f1825_tx.c,60 :: 		}
L_end_ManInit:
	RETURN
; end of _ManInit

_ManBufAddByte:

;Manch_16f1825_tx.c,65 :: 		void ManBufAddByte (unsigned char place, unsigned char byte)
;Manch_16f1825_tx.c,67 :: 		if (place >= MAN_BUF_LENGTH)        return;
	MOVLW      16
	SUBWF      FARG_ManBufAddByte_place+0, 0
	BTFSS      STATUS+0, 0
	GOTO       L_ManBufAddByte0
	GOTO       L_end_ManBufAddByte
L_ManBufAddByte0:
;Manch_16f1825_tx.c,68 :: 		ManTransmitDataBuf [place] = byte;
	MOVLW      _ManTransmitDataBuf+0
	MOVWF      FSR1L
	MOVLW      hi_addr(_ManTransmitDataBuf+0)
	MOVWF      FSR1H
	MOVF       FARG_ManBufAddByte_place+0, 0
	ADDWF      FSR1L, 1
	BTFSC      STATUS+0, 0
	INCF       FSR1H, 1
	MOVF       FARG_ManBufAddByte_byte+0, 0
	MOVWF      INDF1+0
;Manch_16f1825_tx.c,69 :: 		}
L_end_ManBufAddByte:
	RETURN
; end of _ManBufAddByte

_ManTransmitData:

;Manch_16f1825_tx.c,74 :: 		void ManTransmitData (unsigned char BufLen){
;Manch_16f1825_tx.c,75 :: 		unsigned char  i=0;
	CLRF       ManTransmitData_i_L0+0
	CLRF       ManTransmitData_u_L0+0
	CLRF       ManTransmitData_a_L0+0
	CLRF       ManTransmitData_byte_L0+0
;Manch_16f1825_tx.c,81 :: 		INTCON.GIE =0;                                         //запрет всех прерываний
	BCF        INTCON+0, 7
;Manch_16f1825_tx.c,83 :: 		for ( i=0; i< MAN_PILOT_LEN; i++) {                     //передача пилотного сигнала
	CLRF       ManTransmitData_i_L0+0
L_ManTransmitData1:
	MOVLW      8
	SUBWF      ManTransmitData_i_L0+0, 0
	BTFSC      STATUS+0, 0
	GOTO       L_ManTransmitData2
;Manch_16f1825_tx.c,84 :: 		ManTransmitBit (1);
	MOVLW      1
	MOVWF      FARG_ManTransmitBit_bit_t+0
	CALL       _ManTransmitBit+0
;Manch_16f1825_tx.c,83 :: 		for ( i=0; i< MAN_PILOT_LEN; i++) {                     //передача пилотного сигнала
	INCF       ManTransmitData_i_L0+0, 1
;Manch_16f1825_tx.c,85 :: 		}
	GOTO       L_ManTransmitData1
L_ManTransmitData2:
;Manch_16f1825_tx.c,90 :: 		while (1)   {
L_ManTransmitData4:
;Manch_16f1825_tx.c,92 :: 		byte = ManIdentifier [a];
	MOVLW      _ManIdentifier+0
	MOVWF      FSR0L
	MOVLW      hi_addr(_ManIdentifier+0)
	MOVWF      FSR0H
	MOVF       ManTransmitData_a_L0+0, 0
	ADDWF      FSR0L, 1
	BTFSC      STATUS+0, 0
	INCF       FSR0H, 1
	MOVF       INDF0+0, 0
	MOVWF      R0
	MOVF       R0, 0
	MOVWF      ManTransmitData_byte_L0+0
;Manch_16f1825_tx.c,93 :: 		a++;
	INCF       ManTransmitData_a_L0+0, 1
;Manch_16f1825_tx.c,94 :: 		if (byte)ManTransmitByte (byte);
	MOVF       R0, 0
	BTFSC      STATUS+0, 2
	GOTO       L_ManTransmitData6
	MOVF       ManTransmitData_byte_L0+0, 0
	MOVWF      FARG_ManTransmitByte_byte+0
	CALL       _ManTransmitByte+0
	GOTO       L_ManTransmitData7
L_ManTransmitData6:
;Manch_16f1825_tx.c,96 :: 		break;
	GOTO       L_ManTransmitData5
L_ManTransmitData7:
;Manch_16f1825_tx.c,97 :: 		}
	GOTO       L_ManTransmitData4
L_ManTransmitData5:
;Manch_16f1825_tx.c,101 :: 		CheckSummByte = 0;                                   //обнулить контрольку
	CLRF       _CheckSummByte+0
;Manch_16f1825_tx.c,102 :: 		ManTransmitByte (BufLen);
	MOVF       FARG_ManTransmitData_BufLen+0, 0
	MOVWF      FARG_ManTransmitByte_byte+0
	CALL       _ManTransmitByte+0
;Manch_16f1825_tx.c,105 :: 		for (  u=0; u<(BufLen); u++) {
	CLRF       ManTransmitData_u_L0+0
L_ManTransmitData8:
	MOVF       FARG_ManTransmitData_BufLen+0, 0
	SUBWF      ManTransmitData_u_L0+0, 0
	BTFSC      STATUS+0, 0
	GOTO       L_ManTransmitData9
;Manch_16f1825_tx.c,107 :: 		ManTransmitByte (ManTransmitDataBuf [u]);
	MOVLW      _ManTransmitDataBuf+0
	MOVWF      FSR0L
	MOVLW      hi_addr(_ManTransmitDataBuf+0)
	MOVWF      FSR0H
	MOVF       ManTransmitData_u_L0+0, 0
	ADDWF      FSR0L, 1
	BTFSC      STATUS+0, 0
	INCF       FSR0H, 1
	MOVF       INDF0+0, 0
	MOVWF      FARG_ManTransmitByte_byte+0
	CALL       _ManTransmitByte+0
;Manch_16f1825_tx.c,105 :: 		for (  u=0; u<(BufLen); u++) {
	INCF       ManTransmitData_u_L0+0, 1
;Manch_16f1825_tx.c,108 :: 		}
	GOTO       L_ManTransmitData8
L_ManTransmitData9:
;Manch_16f1825_tx.c,112 :: 		ManTransmitByte (CheckSummByte);
	MOVF       _CheckSummByte+0, 0
	MOVWF      FARG_ManTransmitByte_byte+0
	CALL       _ManTransmitByte+0
;Manch_16f1825_tx.c,114 :: 		ManTransmitBit (0);
	CLRF       FARG_ManTransmitBit_bit_t+0
	CALL       _ManTransmitBit+0
;Manch_16f1825_tx.c,117 :: 		INTCON.GIE =1;                                       //разрешение прерываний
	BSF        INTCON+0, 7
;Manch_16f1825_tx.c,119 :: 		}
L_end_ManTransmitData:
	RETURN
; end of _ManTransmitData

_ManTransmitByte:

;Manch_16f1825_tx.c,124 :: 		void ManTransmitByte (unsigned char byte)
;Manch_16f1825_tx.c,125 :: 		{       unsigned char i=0;
	CLRF       ManTransmitByte_i_L0+0
;Manch_16f1825_tx.c,126 :: 		ManCheckSumm (byte);
	MOVF       FARG_ManTransmitByte_byte+0, 0
	MOVWF      FARG_ManCheckSumm_data_t+0
	CALL       _ManCheckSumm+0
;Manch_16f1825_tx.c,128 :: 		for ( i=0; i<8; i++) {
	CLRF       ManTransmitByte_i_L0+0
L_ManTransmitByte11:
	MOVLW      8
	SUBWF      ManTransmitByte_i_L0+0, 0
	BTFSC      STATUS+0, 0
	GOTO       L_ManTransmitByte12
;Manch_16f1825_tx.c,130 :: 		if (byte & 0x80)        ManTransmitBit (1);
	BTFSS      FARG_ManTransmitByte_byte+0, 7
	GOTO       L_ManTransmitByte14
	MOVLW      1
	MOVWF      FARG_ManTransmitBit_bit_t+0
	CALL       _ManTransmitBit+0
	GOTO       L_ManTransmitByte15
L_ManTransmitByte14:
;Manch_16f1825_tx.c,131 :: 		else                        ManTransmitBit (0);
	CLRF       FARG_ManTransmitBit_bit_t+0
	CALL       _ManTransmitBit+0
L_ManTransmitByte15:
;Manch_16f1825_tx.c,132 :: 		byte <<= 1;
	LSLF       FARG_ManTransmitByte_byte+0, 1
;Manch_16f1825_tx.c,128 :: 		for ( i=0; i<8; i++) {
	INCF       ManTransmitByte_i_L0+0, 1
;Manch_16f1825_tx.c,133 :: 		}
	GOTO       L_ManTransmitByte11
L_ManTransmitByte12:
;Manch_16f1825_tx.c,134 :: 		}
L_end_ManTransmitByte:
	RETURN
; end of _ManTransmitByte

_ManTransmitBit:

;Manch_16f1825_tx.c,138 :: 		void ManTransmitBit (unsigned char bit_t)
;Manch_16f1825_tx.c,140 :: 		if (bit_t) {
	MOVF       FARG_ManTransmitBit_bit_t+0, 0
	BTFSC      STATUS+0, 2
	GOTO       L_ManTransmitBit16
;Manch_16f1825_tx.c,142 :: 		TRANSMIT_PORT &= ~(TRANSMIT_LINE);        ManPause ();
	BCF        PORTC+0, 0
	CALL       _ManPause+0
;Manch_16f1825_tx.c,143 :: 		TRANSMIT_PORT |= TRANSMIT_LINE;                ManPause ();                                                                                   ///
	BSF        PORTC+0, 0
	CALL       _ManPause+0
;Manch_16f1825_tx.c,144 :: 		}
	GOTO       L_ManTransmitBit17
L_ManTransmitBit16:
;Manch_16f1825_tx.c,147 :: 		TRANSMIT_PORT |= TRANSMIT_LINE;                ManPause ();
	BSF        PORTC+0, 0
	CALL       _ManPause+0
;Manch_16f1825_tx.c,148 :: 		TRANSMIT_PORT &= ~TRANSMIT_LINE;        ManPause ();
	BCF        PORTC+0, 0
	CALL       _ManPause+0
;Manch_16f1825_tx.c,149 :: 		}
L_ManTransmitBit17:
;Manch_16f1825_tx.c,150 :: 		}
L_end_ManTransmitBit:
	RETURN
; end of _ManTransmitBit

_ManPause:

;Manch_16f1825_tx.c,153 :: 		void ManPause (void)
;Manch_16f1825_tx.c,155 :: 		delay_us (500000 / MAN_SPEED);
	MOVLW      3
	MOVWF      R12
	MOVLW      151
	MOVWF      R13
L_ManPause18:
	DECFSZ     R13, 1
	GOTO       L_ManPause18
	DECFSZ     R12, 1
	GOTO       L_ManPause18
	NOP
	NOP
;Manch_16f1825_tx.c,156 :: 		}
L_end_ManPause:
	RETURN
; end of _ManPause

_ManCheckSumm:

;Manch_16f1825_tx.c,165 :: 		void ManCheckSumm(unsigned char data_t)
;Manch_16f1825_tx.c,166 :: 		{           unsigned char i=0;
	CLRF       ManCheckSumm_i_L0+0
;Manch_16f1825_tx.c,168 :: 		for ( i=0; i<8; i++) {
	CLRF       ManCheckSumm_i_L0+0
L_ManCheckSumm19:
	MOVLW      8
	SUBWF      ManCheckSumm_i_L0+0, 0
	BTFSC      STATUS+0, 0
	GOTO       L_ManCheckSumm20
;Manch_16f1825_tx.c,170 :: 		unsigned char temp = data_t;
	MOVF       FARG_ManCheckSumm_data_t+0, 0
	MOVWF      R2+0
;Manch_16f1825_tx.c,171 :: 		temp ^= CheckSummByte;
	MOVF       _CheckSummByte+0, 0
	XORWF      FARG_ManCheckSumm_data_t+0, 0
	MOVWF      R1
	MOVF       R1, 0
	MOVWF      R2+0
;Manch_16f1825_tx.c,173 :: 		if (temp & 0x01) {
	BTFSS      R1, 0
	GOTO       L_ManCheckSumm22
;Manch_16f1825_tx.c,175 :: 		CheckSummByte ^= 0x18;
	MOVLW      24
	XORWF      _CheckSummByte+0, 1
;Manch_16f1825_tx.c,176 :: 		temp = 0x80;
	MOVLW      128
	MOVWF      R2+0
;Manch_16f1825_tx.c,177 :: 		}
	GOTO       L_ManCheckSumm23
L_ManCheckSumm22:
;Manch_16f1825_tx.c,179 :: 		else        temp = 0;
	CLRF       R2+0
L_ManCheckSumm23:
;Manch_16f1825_tx.c,181 :: 		CheckSummByte >>= 1;
	LSRF       _CheckSummByte+0, 1
;Manch_16f1825_tx.c,182 :: 		CheckSummByte |= temp;
	MOVF       R2+0, 0
	IORWF       _CheckSummByte+0, 1
;Manch_16f1825_tx.c,183 :: 		data_t >>= 1;
	LSRF       FARG_ManCheckSumm_data_t+0, 1
;Manch_16f1825_tx.c,168 :: 		for ( i=0; i<8; i++) {
	INCF       ManCheckSumm_i_L0+0, 1
;Manch_16f1825_tx.c,184 :: 		}
	GOTO       L_ManCheckSumm19
L_ManCheckSumm20:
;Manch_16f1825_tx.c,185 :: 		}
L_end_ManCheckSumm:
	RETURN
; end of _ManCheckSumm

_interrupt:

;Manch_16f1825_tx.c,188 :: 		void interrupt (void){
;Manch_16f1825_tx.c,189 :: 		if(INTCON.IOCIE & INTCON.IOCIF){                            //если прерывание по изменению...
	BTFSS      INTCON+0, 3
	GOTO       L__interrupt49
	BTFSS      INTCON+0, 0
	GOTO       L__interrupt49
	BSF        3, 0
	GOTO       L__interrupt50
L__interrupt49:
	BCF        3, 0
L__interrupt50:
	BTFSS      3, 0
	GOTO       L_interrupt24
;Manch_16f1825_tx.c,190 :: 		INTCON.IOCIF=0;                                          //сбросим флаги
	BCF        INTCON+0, 0
;Manch_16f1825_tx.c,191 :: 		IOCAF.IOCAF0=0;
	BCF        IOCAF+0, 0
;Manch_16f1825_tx.c,192 :: 		IOCAF.IOCAF1=0;
	BCF        IOCAF+0, 1
;Manch_16f1825_tx.c,193 :: 		IOCAF.IOCAF2=0;
	BCF        IOCAF+0, 2
;Manch_16f1825_tx.c,194 :: 		IOCAF.IOCAF4=0;
	BCF        IOCAF+0, 4
;Manch_16f1825_tx.c,195 :: 		IOCAF.IOCAF5=0;
	BCF        IOCAF+0, 5
;Manch_16f1825_tx.c,198 :: 		}
L_interrupt24:
;Manch_16f1825_tx.c,201 :: 		}
L_end_interrupt:
L__interrupt48:
	RETFIE     %s
; end of _interrupt

_main:

;Manch_16f1825_tx.c,205 :: 		void main (void) {
;Manch_16f1825_tx.c,206 :: 		OSCCON=0b11111111;                                       //тактова€ частота
	MOVLW      255
	MOVWF      OSCCON+0
;Manch_16f1825_tx.c,207 :: 		TRISA=0b111111;
	MOVLW      63
	MOVWF      TRISA+0
;Manch_16f1825_tx.c,208 :: 		ANSELA=0;                                                //отключение ј÷ѕ
	CLRF       ANSELA+0
;Manch_16f1825_tx.c,209 :: 		PORTA=0b000000;
	CLRF       PORTA+0
;Manch_16f1825_tx.c,210 :: 		WPUA=0b111111;                                           //подт€гивающие резист.
	MOVLW      63
	MOVWF      WPUA+0
;Manch_16f1825_tx.c,211 :: 		TRISC=0b000000;
	CLRF       TRISC+0
;Manch_16f1825_tx.c,212 :: 		PORTC=0b000000;
	CLRF       PORTC+0
;Manch_16f1825_tx.c,213 :: 		OPTION_REG=0b00000000;
	CLRF       OPTION_REG+0
;Manch_16f1825_tx.c,214 :: 		INTCON=0b00001000;                                       //настройки прерываний
	MOVLW      8
	MOVWF      INTCON+0
;Manch_16f1825_tx.c,215 :: 		WDTCON=0b00010000;                                       //делитель собаки ???
	MOVLW      16
	MOVWF      WDTCON+0
;Manch_16f1825_tx.c,216 :: 		IOCAN=0b00110111;                                        //прерывание по изменению порта
	MOVLW      55
	MOVWF      IOCAN+0
;Manch_16f1825_tx.c,218 :: 		ManInit ();                                              //инициализаци€  передачи манчестер сигнала
	CALL       _ManInit+0
;Manch_16f1825_tx.c,220 :: 		while(1){
L_main25:
;Manch_16f1825_tx.c,222 :: 		asm{clrwdt};                                       //сброс собаки
	CLRWDT
;Manch_16f1825_tx.c,223 :: 		PWR_TRANSEIVER=1;                                  // включаем питание передатчика
	BSF        LATC+0, 1
;Manch_16f1825_tx.c,224 :: 		if( Button(FORWARD))  dataButtons |=(1<<0);        //опрос кнопок
	MOVLW      PORTA+0
	MOVWF      FARG_Button_port+0
	MOVLW      hi_addr(PORTA+0)
	MOVWF      FARG_Button_port+1
	CLRF       FARG_Button_pin+0
	MOVLW      20
	MOVWF      FARG_Button_time_ms+0
	CLRF       FARG_Button_active_state+0
	CALL       _Button+0
	MOVF       R0, 0
	BTFSC      STATUS+0, 2
	GOTO       L_main27
	BSF        _dataButtons+0, 0
L_main27:
;Manch_16f1825_tx.c,225 :: 		if( Button(REVERSE))  dataButtons |=(1<<1);
	MOVLW      PORTA+0
	MOVWF      FARG_Button_port+0
	MOVLW      hi_addr(PORTA+0)
	MOVWF      FARG_Button_port+1
	MOVLW      1
	MOVWF      FARG_Button_pin+0
	MOVLW      20
	MOVWF      FARG_Button_time_ms+0
	CLRF       FARG_Button_active_state+0
	CALL       _Button+0
	MOVF       R0, 0
	BTFSC      STATUS+0, 2
	GOTO       L_main28
	BSF        _dataButtons+0, 1
L_main28:
;Manch_16f1825_tx.c,226 :: 		if( Button(LEFT ))    dataButtons |=(1<<2);
	MOVLW      PORTA+0
	MOVWF      FARG_Button_port+0
	MOVLW      hi_addr(PORTA+0)
	MOVWF      FARG_Button_port+1
	MOVLW      2
	MOVWF      FARG_Button_pin+0
	MOVLW      20
	MOVWF      FARG_Button_time_ms+0
	CLRF       FARG_Button_active_state+0
	CALL       _Button+0
	MOVF       R0, 0
	BTFSC      STATUS+0, 2
	GOTO       L_main29
	BSF        _dataButtons+0, 2
L_main29:
;Manch_16f1825_tx.c,227 :: 		if( Button(RIGHT))    dataButtons |=(1<<3);
	MOVLW      PORTA+0
	MOVWF      FARG_Button_port+0
	MOVLW      hi_addr(PORTA+0)
	MOVWF      FARG_Button_port+1
	MOVLW      4
	MOVWF      FARG_Button_pin+0
	MOVLW      20
	MOVWF      FARG_Button_time_ms+0
	CLRF       FARG_Button_active_state+0
	CALL       _Button+0
	MOVF       R0, 0
	BTFSC      STATUS+0, 2
	GOTO       L_main30
	BSF        _dataButtons+0, 3
L_main30:
;Manch_16f1825_tx.c,229 :: 		if( Button(TRIGGER_PASS))  {
	MOVLW      PORTA+0
	MOVWF      FARG_Button_port+0
	MOVLW      hi_addr(PORTA+0)
	MOVWF      FARG_Button_port+1
	MOVLW      5
	MOVWF      FARG_Button_pin+0
	MOVLW      20
	MOVWF      FARG_Button_time_ms+0
	MOVLW      1
	MOVWF      FARG_Button_active_state+0
	CALL       _Button+0
	MOVF       R0, 0
	BTFSC      STATUS+0, 2
	GOTO       L_main31
;Manch_16f1825_tx.c,230 :: 		flagOldstate=1;
	MOVLW      1
	MOVWF      _flagOldstate+0
;Manch_16f1825_tx.c,231 :: 		}
L_main31:
;Manch_16f1825_tx.c,233 :: 		if( Button (TRIGGER_ACT) && flagOldstate ) {        //управление светом фар
	MOVLW      PORTA+0
	MOVWF      FARG_Button_port+0
	MOVLW      hi_addr(PORTA+0)
	MOVWF      FARG_Button_port+1
	MOVLW      5
	MOVWF      FARG_Button_pin+0
	MOVLW      20
	MOVWF      FARG_Button_time_ms+0
	CLRF       FARG_Button_active_state+0
	CALL       _Button+0
	MOVF       R0, 0
	BTFSC      STATUS+0, 2
	GOTO       L_main34
	MOVF       _flagOldstate+0, 0
	BTFSC      STATUS+0, 2
	GOTO       L_main34
L__main39:
;Manch_16f1825_tx.c,234 :: 		flagOldstate=0;
	CLRF       _flagOldstate+0
;Manch_16f1825_tx.c,235 :: 		flagTrigger = ~flagTrigger;
	COMF       _flagTrigger+0, 0
	MOVWF      R0
	MOVF       R0, 0
	MOVWF      _flagTrigger+0
;Manch_16f1825_tx.c,236 :: 		if(flagTrigger){
	MOVF       R0, 0
	BTFSC      STATUS+0, 2
	GOTO       L_main35
;Manch_16f1825_tx.c,237 :: 		dataButtons |=(1<<4);                      //дл€ триггера(свет фар)используем два бита,один(4)вкл.
	BSF        _dataButtons+0, 4
;Manch_16f1825_tx.c,238 :: 		dataButtons &=~(1<<5);                     //второй(5) выкл.
	BCF        _dataButtons+0, 5
;Manch_16f1825_tx.c,239 :: 		}
	GOTO       L_main36
L_main35:
;Manch_16f1825_tx.c,241 :: 		dataButtons &=~(1<<4);                   //очистить бит включени€ света
	BCF        _dataButtons+0, 4
;Manch_16f1825_tx.c,242 :: 		dataButtons |=(1<<5);                    //команда выключить свет
	BSF        _dataButtons+0, 5
;Manch_16f1825_tx.c,243 :: 		}
L_main36:
;Manch_16f1825_tx.c,244 :: 		}
L_main34:
;Manch_16f1825_tx.c,247 :: 		if(dataButtons){                                    // если кнопка нажата...
	MOVF       _dataButtons+0, 0
	BTFSC      STATUS+0, 2
	GOTO       L_main37
;Manch_16f1825_tx.c,249 :: 		ManBufAddByte(0,dataButtons );                //поместить в 0 €чейку буфера байт данных кнопок
	CLRF       FARG_ManBufAddByte_place+0
	MOVF       _dataButtons+0, 0
	MOVWF      FARG_ManBufAddByte_byte+0
	CALL       _ManBufAddByte+0
;Manch_16f1825_tx.c,250 :: 		ManBufAddByte(1,speedLevel);                  //поместить в 1 €чейку буфера байт данных скорости
	MOVLW      1
	MOVWF      FARG_ManBufAddByte_place+0
	MOVF       _speedLevel+0, 0
	MOVWF      FARG_ManBufAddByte_byte+0
	CALL       _ManBufAddByte+0
;Manch_16f1825_tx.c,252 :: 		ManTransmitData (2);                         //передать два байта данных из буфера
	MOVLW      2
	MOVWF      FARG_ManTransmitData_BufLen+0
	CALL       _ManTransmitData+0
;Manch_16f1825_tx.c,253 :: 		dataButtons=0;                               //обнулить переменную опроса кнопок
	CLRF       _dataButtons+0
;Manch_16f1825_tx.c,255 :: 		}
	GOTO       L_main38
L_main37:
;Manch_16f1825_tx.c,257 :: 		flagOldstate=1;                           //выставим флаг триггера
	MOVLW      1
	MOVWF      _flagOldstate+0
;Manch_16f1825_tx.c,258 :: 		PWR_TRANSEIVER=0;                         //выключаем питание передатчика
	BCF        LATC+0, 1
;Manch_16f1825_tx.c,259 :: 		asm{sleep};                               //идем спать
	SLEEP
;Manch_16f1825_tx.c,261 :: 		}
L_main38:
;Manch_16f1825_tx.c,262 :: 		}
	GOTO       L_main25
;Manch_16f1825_tx.c,265 :: 		}
L_end_main:
	GOTO       $+0
; end of _main
