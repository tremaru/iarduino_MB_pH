#include "iarduino_MB_pH.h"			//	╔═══════════════════════╗
									//	║ РЕГИСТР ДИАГНОСТИКИ   ║
									//	╠═══════════╤═══════════╣
									//	║ ERR_Power │ 8-------- ║ Флаг ожидания   стабилизации после подачи питания.
									//	║ ERR_Data  │ -7------- ║ Флаг ошибки при чтении данных из АЦП.
									//	║ ERR_pH    │ --6------ ║ Флаг указывает  на то, что показания pH "скачут".
									//	║ ERR_Vout  │ ---5----- ║ Флаг первышения напряжения на выходе ОУ (pH < 0 ).
									//	║ ERR_Vout  │ ----4---- ║ Флаг занижения  напряжения на выходе ОУ (pH > 14).
									//	║ ERR_Vo    │ -----3--- ║ Флаг первышения напряжения подаваемого на датчик (Vo > 0.5Vcc + 0.1).
									//	║ ERR_Vo    │ ------2-- ║ Флаг занижения  напряжения подаваемого на датчик (Vo < 0.5Vcc - 0.1).
									//	║ ERR_5V    │ -------1- ║ Флаг занижения  напряжения питания 5В после DC-DC преобразователя < 4В. Стабильная работа не гарантируется.
									//	║ ERR_Init  │ --------0 ║ Флаг провала    инициализации АЦП.
									//	╚═══════════╧═══════════╝
									//	╔═══════════════╦═══════╗
									//	║ РЕГИСТРЫ «DO» ║ Биты: ║
									//	╠═══════════════╬═══════╣
#define DO_HL_DATA		0			//	║ DO_HL_DATA    ║   0   ║ Включение светодиода обнаружения устройства.
#define DO_CALC_START	1			//	║ DO_CALC_START ║   0   ║ Запустить калибровку.
#define DO_CALC_STOP	2			//	║ DO_CALC_STOP  ║   0   ║ Отменить калибровку.
									//	╟───────────────╫───────╢
//						0x0100		//	║ DO_CHANGE_ID  ║   0   ║ Флаг устанавливается самостоятельно после подтверждения смены адреса на шине.
//						0x0101		//	║ DO_BAN_ID 1   ║   0   ║ Бит  запрещает изменять или устанавливать адрес 1.      Только для режима смены адреса на случайное симло.
//						...			//	║ DO_BAN_ID ... ║   0   ║ Биты запрещают изменять или устанавливать адреса 2-246. Только для режима смены адреса на случайное симло.
//						0x01F7		//	║ DO_BAN_ID 247 ║   0   ║ Бит  запрещает изменять или устанавливать адрес 147.    Только для режима смены адреса на случайное симло.
									//	╚═══════════════╩═══════╝
									//	╔═══════════════╦═══════╗
									//	║ РЕГИСТРЫ «DI» ║ Биты: ║
									//	╠═══════════════╬═══════╣
#define DI_CALC_ERR		0			//	║ DI_CALC_ERR   ║   0   ║ Флаг ошибки калибровки (0-нет, 1-ошибка). Регистр сбрасывается при нажатии на кнопку калибровки или установке DO_CALC_START.
#define DI_STATUS_1		1			//	║ DI_STATUS_1   ║   0   ║ Флаг выполнения 1 стадии калибровки (0-нет, 1-выполняется).
#define DI_STAY			2			//	║ DI_STAY       ║   0   ║ Флаг ожидания смены калибровочной жидкости (0-нет, 1-ожидается).
#define DI_STATUS_2		3			//	║ DI_STATUS_2   ║   0   ║ Флаг выполнения 2 стадии калибровки (0-нет, 1-выполняется).
#define DI_STABLE_PH	4			//	║ DI_STABLE_PH  ║   0   ║ Флаг нормализации показаний кислотности.
									//	╚═══════════════╩═══════╝
									//	╔═══════════════╦═══════╗
									//	║ РЕГИСТРЫ «AO» ║Данные:║
									//	╠═══════════════╬═══════╣
#define AO_KNOWN_PH_1	0			//	║ AO_KNOWN_PH_1 ║ 04.000║ Известное pH, для 1 стадии калибровки.
#define AO_KNOWN_PH_2	1			//	║ AO_KNOWN_PH_2 ║ 09.180║ Известное pH, для 2 стадии калибровки.
#define AO_VSTP			2			//	║ AO_VSTP       ║ 059.16║ Шаг смещения U датчика на 1pH  в сотых долях мВ. Смещение напряжения датчиком на 1 Vstp соответствует изменению кислотности на 1 pH.
#define AO_PHN			3			//	║ AO_PHN        ║ 07.000║ Нейтральная кислотность датчика в тысячных долях. Кислотность жидкости при которой отсутствует смещение напряжения датчиком.
#define AO_STABLE_PH	4			//	║ AO_STABLE_PH  ║ 00.002║ Допустимая флуктуация кислотности в тысячных долях pH. Если реальная флуктуация ниже, значит показания    стабильны (DI_STABLE_PH=1).
#define AO_UNSTABLE_PH	5			//	║ AO_UNSTABLE_PH║ 00.003║ Высокая    флуктуация кислотности в тысячных долях pH. Если реальная флуктуация выше, значит показания не стабильны (DI_STABLE_PH=0).
									//	╟───────────────╫───────╢
#define AO_CALC_SAVE	6			//	║ AO_CALC_SAVE  ║   0   ║ Код разрешения записи калибровочных значений в Flash память. Сбрасывается автоматически. Должен быть установлен в значение (0x2709) при запуске калибровки и до записи калибровочных значений напрямую через регистры (без калибровки).
									//	╟───────────────╫───────╢
//						0x0100		//	║ AO_ACK_ID     ║   0   ║ Запись значения 0xF0A5 подтверждает смену адреса на шине. При чтении всегда возвращает 0.
#define AO_ACK_SPEED	0x0101		//	║ AO_ACK_SPEED  ║   0   ║ Запись значения 0xF0A5 подтверждает смену скорости  шины. При чтении всегда возвращает 0.
#define AO_ACK_TYPE		0x0102		//	║ AO_ACK_TYPE   ║   0   ║ Запись значения 0xF0A5 подтверждает смену типа протокола. При чтении всегда возвращает 0.
//						0x0110		//	║ AO_SET_ID     ║  xxID ║ Запись приводит к смене адреса на шине, xx=00 до отключения питания, xx=FF с сохранеием в ПЗУ. Требуется подтверждение. При чтении xx=случайное число.
#define AO_SET_SPEED	0x0111		//	║ AO_SET_SPEED  ║  xxSP ║ Запись приводит к смене скорости  шины, xx=00 до отключения питания, xx=FF с сохранеием в ПЗУ. Требуется подтверждение. При чтении xx=случайное число.
#define AO_SET_TYPE		0x0112		//	║ AO_SET_TYPE   ║  xxTP ║ Запись приводит к смене типа протокола, xx=00 до отключения питания, xx=FF с сохранеием в ПЗУ. Требуется подтверждение. При чтении xx=случайное число.
#define AO_USER_DATA	0x0120		//	║ AO_USER_DATA  ║   0   ║ Пользовательское число хранится в Flash памяти модуля.
									//	╚═══════════════╩═══════╝
									//	╔═══════════════╦═══════╗
									//	║ РЕГИСТРЫ «AI» ║Данные:║
									//	╠═══════════════╬═══════╣
#define AI_5V			0			//	║ AI_5V         ║   0   ║ Напряжение питания 5В после DC-DC преобразователя в тысячных долях вольт ( 0...65,535В ).
									//	╟───────────────╫───────╢
#define AI_V0			1			//	║ AI_V0         ║   0   ║ Напряжение на входе  датчика в десятитысячных долях В. ( 0...6,5535В ).
#define AI_VOUT			2			//	║ AI_VOUT       ║   0   ║ Напряжение на выходе датчика в десятитысячных долях В. ( 0...6,5535В ).
									//	╟───────────────╫───────╢
#define AI_PH			3			//	║ AI_PH         ║   0   ║ Кислотность жидкости в тысячных долях pH ( 0...65,535 pH ). pH = pHn - ΔVout/Vstp, где ΔVout = Vin-Vout.
									//	╚═══════════════╩═══════╝
																															//
//		КОНСТРУКТОР КЛАССА:																									//
		iarduino_MB_pH::iarduino_MB_pH(ModbusClient &obj){objModbus = &obj;}												//	&obj - ссылка на объект для работы по протоколу Modbus.
																															//
//		ФУНКЦИЯ ИНИЦИАЛИЗАЦИИ pH-МЕТРА:																						//	Возвращает результат инициализации: true-успех / false-провал.
bool	iarduino_MB_pH::begin(uint8_t id){																					//	id - адрес модуля на шине.
			objModbus->codeError=ERROR_GATEWAY_NO_DEVICE;																	//	Ошибкой выполнения данной функции может быть только отсутствие устройства.
		//	Самостоятельный поиск id устройства:																			//
			if( id==0 )						{ if( objModbus->findID(DEF_MODEL_pH       ) ){id=objModbus->read();}}			//	Если адрес не указан, ищем адрес первого устройства с совпавшим идентификатором.
			if( id==0 )						{ if( objModbus->findID(DEF_MODEL_pH_zummer) ){id=objModbus->read();}}			//	Если адрес не указан, ищем адрес первого устройства с совпавшим идентификатором.
		//	Проверяем устройство:																							//
			if( id==0 )						{ return false; }																//	Адрес устройства не указан и не найден.
			if( MB_getInfo(id)<15 )			{ return false; }																//	Устройство не найдено, или информация об устройстве неполная.
			if( objModbus->read()!=0x77 )	{ return false; }																//	Идентификатор линейки устройств не соответствует устройствам iArduino.
				objModbus->read();																							//	Индикатор пуска не проверяем (00=OFF, FF=ON).
			if( objModbus->read()!=id )		{ return false; }																//	Адрес полученный из информации об устройстве не совпадает с фактическим адресом устройства.
			uint8_t model = objModbus->read();																				//	Получаем идентификатор модели устройства.
			if( model!=DEF_MODEL_pH && model!=DEF_MODEL_pH_zummer ){ return false; }										//	Идентификатор устройства не совпадает с DEF_MODEL_pH и DEF_MODEL_pH_zummer.
		//	Устройство прошло проверку:																						//
			valID=id;																										//	Сохраняем адрес устройства.
			valVers=objModbus->read();																						//	Сохраняем версию прошивки устройства.
			return true;																									//
}																															//
																															//
//		ФУНКЦИЯ ПЕРЕЗАГРУЗКИ МОДУЛЯ:																						//	Возвращает результат перезагрузки: true-успех / false-провал.
bool	iarduino_MB_pH::reset(void){																						//	
			objModbus->codeError=ERROR_SYNTAX; if( !valID ){ return false; }												//	Ошибка синтаксиса, модуль не инициализирован.
			return ( MB_diagnostic(1,0xFF00)<0? false:true );																//	Выполняем команду диагностики, функция 0x0001 с очисткой журнала событий 0xFF00.
}																															//
																															//
//		ФУНКЦИЯ СМЕНЫ ID-АДРЕСА НА ШИНЕ:																					//	Возвращает результат смены ID: true-успех / false-провал.
bool	iarduino_MB_pH::changeID(uint8_t id){																				//	id - новый адрес модуля на шине (1-247).
			objModbus->codeError=ERROR_SYNTAX; if( !valID ){ return false; }												//	Ошибка синтаксиса, модуль не инициализирован.
			return MB_changeID(id);																							//	Меняем адрес устройства с valID на id, после чего valID=id. Функция возвращает 1 при успехе, 0 при неудаче.
}																															//
																															//
//		ФУНКЦИЯ СМЕНЫ СКОРОСТИ ПЕРЕДАЧИ ДАННЫХ:																				//	Возвращает результат смены скорости: true-успех / false-провал. Скорость необходимо подтвердить в течении 2 секунд.
bool	iarduino_MB_pH::setSpeedMB(uint32_t s){																				//	s - скорость передачи данных (2400/4800/9600/19200/38400/57600/115200)
			objModbus->codeError=ERROR_SYNTAX; if( !valID ){ return false; }												//	Ошибка синтаксиса, модуль не инициализирован.
			if( s!=2400 && s!=4800 && s!=9600 && s!=19200 && s!=38400 && s!=57600 && s!=115200 ){ return false; }			//	Проверяем значение скорости.
			return MB_writeAO(AO_SET_SPEED, 0xFF00|(s/2400));																//	Записываем значение 0xFFXX в регистр "Holding Register" AO[AO_SET_SPEED]. Функция возвращает 1 при успехе, 0 при неудаче.
}																															//
																															//
//		ФУНКЦИЯ ПОДТВЕРЖДЕНИЯ СКОРОСТИ ПЕРЕДАЧИ ДАННЫХ:																		//	Возвращает результат подтверждения скорости: true-успех / false-провал. 
bool	iarduino_MB_pH::ackSpeedMB(void){																					//
			objModbus->codeError=ERROR_SYNTAX; if( !valID ){ return false; }												//	Ошибка синтаксиса, модуль не инициализирован.
			if( !MB_writeAO(AO_ACK_SPEED, 0xF0A5) ){ return false; }														//	Записываем значение 0xF0A5 в регистр "Holding Register" AO[AO_ACK_SPEED]. Функция возвращает 1 при успехе, 0 при неудаче.
			delay(50);																										//	Ждём 50 мс, пока модуль не сохранит новую скорость в flash память.
			return true;																									//	Возвращаем флаг успеха.
}																															//
																															//
//		ФУНКЦИЯ СМЕНЫ ТИПА ПРОТОКОЛА MODBUS:																				//	Возвращает результат смены типа протокола Modbus: true-успех / false-провал. Тип необходимо подтвердить в течении 2 секунд.
bool	iarduino_MB_pH::setTypeMB(uint8_t type){																			//	type - тип протокола Modbus (MODBUS_RTU/MODBUS_ASCII).
			objModbus->codeError=ERROR_SYNTAX; if( !valID ){ return false; }												//	Ошибка синтаксиса, модуль не инициализирован.
			if( type!=MODBUS_RTU && type!=MODBUS_ASCII ){ return false; }													//	Проверяем значение типа протокола Modbus.
			return MB_writeAO(AO_SET_TYPE, (type==MODBUS_ASCII?0xFFFF:0xFF00));												//	Записываем значение 0xFFXX в регистр "Holding Register" AO[AO_SET_TYPE]. Функция возвращает 1 при успехе, 0 при неудаче.
}																															//
																															//
//		ФУНКЦИЯ ПОДТВЕРЖДЕНИЯ ТИПА ПРОТОКОЛА MODBUS:																		//	Возвращает результат подтверждения типа протокола Modbus: true-успех / false-провал. 
bool	iarduino_MB_pH::ackTypeMB(void){																					//
			objModbus->codeError=ERROR_SYNTAX; if( !valID ){ return false; }												//	Ошибка синтаксиса, модуль не инициализирован.
			if( !MB_writeAO(AO_ACK_TYPE, 0xF0A5) ){ return false; }															//	Записываем значение 0xF0A5 в регистр "Holding Register" AO[AO_ACK_TYPE]. Функция возвращает 1 при успехе, 0 при неудаче.
			delay(50);																										//	Ждём 50 мс, пока модуль не сохранит новый тип протокола Modbus в flash память.
			return true;																									//	Возвращаем флаг успеха.
}																															//
																															//
//		ФУНКЦИЯ ЗАПИСИ ПОЛЬЗОВАТЕЛЬСКОГО ЗНАЧЕНИЯ В FLASH ПАМЯТЬ МОДУЛЯ:													//	Возвращает результат записи: true-успех / false-провал.
bool	iarduino_MB_pH::writeFlash(uint16_t data){																			//	data - целое беззнаковое число от 0 до 65535.
			objModbus->codeError=ERROR_SYNTAX; if( !valID ){ return false; }												//	Ошибка синтаксиса, модуль не инициализирован.
			if( !MB_writeAO(AO_USER_DATA, data) ){ return false; }															//	Записываем значение data в регистр "Holding Register" AO[AO_USER_DATA]. Функция возвращает 1 при успехе, 0 при неудаче.
			delay(50);																										//	Ждём 50 мс, пока модуль не сохранит записанное значение в flash память.
			return true;																									//	Возвращаем флаг успеха.
}																															//
																															//
//		ФУНКЦИЯ ЧТЕНИЯ ПОЛЬЗОВАТЕЛЬСКОГО ЗНАЧЕНИЯ ИЗ FLASH ПАМЯТИ:															//	Возвращает прочитанное значение, или -1 при провале чтения.
int32_t	iarduino_MB_pH::readFlash(void){																					//
			objModbus->codeError=ERROR_SYNTAX; if( !valID ){ return -1; }													//	Ошибка синтаксиса, модуль не инициализирован.
			return MB_readAO(AO_USER_DATA);																					//	Читаем значение из регистра "Holding Register" AO[AO_USER_DATA]. Функция возвращает значение, или -1 при неудаче.
}																															//
																															//
//		ФУНКЦИЯ ПОЛУЧЕНИЯ НАПРЯЖЕНИЯ ПИТАНИЯ 5В ПОСЛЕ DC-DC ПРЕОБРАЗОВАТЕЛЯ:												//	Возвращает напряжение питания в Вольтах, или -1 при провале чтения.
float	iarduino_MB_pH::getPWR(void){																						//
			objModbus->codeError=ERROR_SYNTAX; if( !valID ){ return -1.0f; }												//	Ошибка синтаксиса, модуль не инициализирован.
			int32_t Vcc = MB_readAI(AI_5V);																					//	Читаем напряжение из регистра "Input Register" AI[AI_5V]. Функция возвращает значение, или -1 при неудаче.
			if( Vcc<0 ){ return -1.0f; }else{ return ((float)Vcc)/1000.0f; }												//	Возвращаем -1 (ошибка), или напряжение в Вольтах.
}																															//
																															//
//		ФУНКЦИЯ ИЗМЕНЕНИЯ СОСТОЯНИЯ СВЕТОДИОДА ОБНАРУЖЕНИЯ УСТРОЙСТВА:														//	Возвращает результат изменения состояния светодиода: true-успех / false-провал.
bool	iarduino_MB_pH::setIDLED(bool f){																					//	f - состояние светодиода (true/false)
			objModbus->codeError=ERROR_SYNTAX; if( !valID ){ return false; }												//	Ошибка синтаксиса, модуль не инициализирован.
			return MB_writeDO(DO_HL_DATA, f);																				//	Записываем f в регистр "Coil" DO[DO_HL_DATA].
}																															//
																															//
//		ФУНКЦИЯ ПОЛУЧЕНИЯ ФЛАГОВ ОШИБОК ИЗ РЕГИСТРА ДИАГНОСТИКИ:															//	Возвращает значение регистра диагностики, каждый из 16 бит которого является флагом ошибки, или -1 при провале чтения.
int32_t	iarduino_MB_pH::getErr(void){																						//
			objModbus->codeError=ERROR_SYNTAX; if( !valID ){ return -1; }													//	Ошибка синтаксиса, модуль не инициализирован.
			return MB_diagnostic(2);																						//	Выполняем команду диагностики, номер функции диагностики 0x0002 = получить регистр диагностики. Функция возвращает данные результата выполнения функции диагностики, или -1 при неудаче.
}																															//
																															//
//		ФУНКЦИЯ ЗАПУСКА КАЛИБРОВКИ:																							//	Возвращает результат запуска калибровки: true-успех / false-провал.
bool	iarduino_MB_pH::setCalibration(void){																				//
			objModbus->codeError=ERROR_SYNTAX; if( !valID ){ return false; }												//	Ошибка синтаксиса, модуль не инициализирован.
			if( !MB_writeAO(AO_CALC_SAVE, 0x2709) ){ return false; }														//	Записываем значение 0x2709 в регистр "Holding Register" AO[AO_CALC_SAVE]. Функция возвращает 1 при успехе, 0 при неудаче.
			if( !MB_writeDO(DO_CALC_START, true ) ){ return false; }														//	Записываем единицу в регистр "Coil" DO[DO_CALC_START].
			return true;																									//
}																															//
																															//
//		ФУНКЦИЯ ПОЛУЧЕНИЯ ТЕКУЩЕЙ СТАДИИ КАЛИБРОВКИ:																		//	Возвращает стадию калибровки: 1 или 2, 0-не выполняется, 3-ожидание смены жидкости, или -1 при провале чтения.
int8_t	iarduino_MB_pH::getCalibration(void){																				//
			objModbus->codeError=ERROR_SYNTAX; if( !valID ){ return -1; }													//	Ошибка синтаксиса, модуль не инициализирован.
			if( MB_requestFrom(DISCRETE_INPUTS, DI_STATUS_1, 3) != 3 ){ return -1; }										//	Читаем регистры "Discrete Inputs" начиная с регистра DI[DI_STATUS_1], всего 3 регистра. Функция возвращает количество прочитанных значений, или 0 при неудаче.
			if( objModbus->read() ){ return 1; }																			//	Если установлен регистр "Discrete Inputs" DI[DI_STATUS_1], значит выполняется 1 стадия калибровки.
			if( objModbus->read() ){ return 3; }																			//	Если установлен регистр "Discrete Inputs" DI[DI_STAY    ], значит ожидания смена калибровочной жидкости.
			if( objModbus->read() ){ return 2; }																			//	Если установлен регистр "Discrete Inputs" DI[DI_STATUS_2], значит выполняется 2 стадия калибровки.
			return 0;																										//	Если ни один из перечисленных регистров не установлен    , значит калибровка не выполняется.
}																															//
																															//
//		ФУНКЦИЯ ОТМЕНЫ КАЛИБРОВКИ:																							//	Возвращает результат отмены калибровки: true-успех / false-провал.
bool	iarduino_MB_pH::setStopCalib(void){																					//
			objModbus->codeError=ERROR_SYNTAX; if( !valID ){ return false; }												//	Ошибка синтаксиса, модуль не инициализирован.
			if( !MB_writeAO(AO_CALC_SAVE, 0x2709) ){ return false; }														//	Записываем значение 0x2709 в регистр "Holding Register" AO[AO_CALC_SAVE]. Функция возвращает 1 при успехе, 0 при неудаче.
			if( !MB_writeDO(DO_CALC_STOP, true  ) ){ return false; }														//	Записываем единицу в регистр "Coil" DO[DO_CALC_STOP].
			return true;																									//
}																															//
																															//
//		ФУНКЦИЯ ПОЛУЧЕНИЯ РЕЗУЛЬТАТА ПОСЛЕДНЕЙ КАЛИБРОВКИ:																	//	Возвращает флаг результат калибровки: true-успех / false-провал, или -1 при провале чтения.
int8_t	iarduino_MB_pH::getResultCalib(void){																				//
			objModbus->codeError=ERROR_SYNTAX; if( !valID ){ return -1; }													//	Ошибка синтаксиса, модуль не инициализирован.
			int8_t i = MB_readDI(DI_CALC_ERR);																				//	Читаем регистр ошибки калибровки "Discrete Inputs" DI[DI_CALC_ERR]. Функция возвращает прочитанное значение (0/1), или -1 при неудаче.
			if( i==0 ){ i=1; }else if( i==1 ){ i=0; } return i;																//	Меняем флаг ошибки на флаг результата (0=1, 1=0) и возвращаем его.
}																															//
																															//
//		ФУНКЦИЯ ПОЛУЧЕНИЯ ФЛАГА НОРМАЛИЗАЦИИ ПОКАЗАНИЙ:																		//	Возвращает флаг стабильности показаний pH: true-стабильны / false-меняются, или -1 при провале чтения.
int8_t	iarduino_MB_pH::getStability(void){																					//
			objModbus->codeError=ERROR_SYNTAX; if( !valID ){ return -1; }													//	Ошибка синтаксиса, модуль не инициализирован.
			return MB_readDI(DI_STABLE_PH);																					//	Возвращаем значение регистра "Discrete Inputs" DI[DI_STABLE_PH]. Функция возвращает прочитанное значение (0/1), или -1 при неудаче.
}																															//
																															//
//		ФУНКЦИЯ ПОЛУЧЕНИЯ ГРАНИЦЫ ГИСТЕРЕЗИСА ФЛУКТУАЦИЙ:																	//	Возвращает границу флуктуаций pH по которой определяется флаг стабильности показаний pH, или -1 при провале чтения.
float	iarduino_MB_pH::getFluctuation(uint8_t type){																		//	type - тип границы STABLE_PH/UNSTABLE_PH.
			objModbus->codeError=ERROR_SYNTAX; if( !valID ){ return -1.0f; }												//	Ошибка синтаксиса, модуль не инициализирован.
			float i= -1.0f;																									//
			if( type==STABLE_PH   ){ i=(float)MB_readAO(AO_STABLE_PH  ); }													//	Читаем регистр "Holding Register" AO[AO_STABLE_PH  ]. Функция возвращает прочитанное значение (0...65535), или -1 при неудаче.
			if( type==UNSTABLE_PH ){ i=(float)MB_readAO(AO_UNSTABLE_PH); }													//	Читаем регистр "Holding Register" AO[AO_UNSTABLE_PH]. Функция возвращает прочитанное значение (0...65535), или -1 при неудаче.
			if( i>=0 ){ i/=1000.0f; } return i;																				//	Возврашаем границу флуктуаций кислотности полученную в тысячных долях pH.
}																															//
																															//
//		ФУНКЦИЯ УСТАНОВКИ ГРАНИЦЫ ГИСТЕРЕЗИСА ФЛУКТУАЦИЙ:																	//	Возвращает результат записи границы флуктуаций: true-успех / false-провал.
bool	iarduino_MB_pH::setFluctuation(uint8_t type, float pH){																//	type - тип границы STABLE_PH/UNSTABLE_PH, pH - значение флуктуации pH за пределами которого будет меняться флаг стабильности показаний pH.
			bool i;																											//
			objModbus->codeError=ERROR_SYNTAX; if( !valID ){ return false; }												//	Ошибка синтаксиса, модуль не инициализирован.
			if( type!=STABLE_PH && type!=UNSTABLE_PH ){ return false; }														//	Возвращаем флаг ошибки синтаксиса.
			if( pH<0.001f || pH>14.0f ){ return false; }																	//	Возвращаем флаг ошибки синтаксиса.
			if( !MB_writeAO(AO_CALC_SAVE, 0x2709) ){ return false; }														//	Записываем значение 0x2709 в регистр "Holding Register" AO[AO_CALC_SAVE]. Функция возвращает 1 при успехе, 0 при неудаче.
			if( type==STABLE_PH   ){ i = MB_writeAO(AO_STABLE_PH  , (uint16_t)(pH*1000.0f) ); }								//	Записываем в регистр "Holding Register" AO[AO_STABLE_PH  ] кислотность в тысячных долях pH. Функция возвращает 1 при успехе, 0 при неудаче.
			if( type==UNSTABLE_PH ){ i = MB_writeAO(AO_UNSTABLE_PH, (uint16_t)(pH*1000.0f) ); }								//	Записываем в регистр "Holding Register" AO[AO_UNSTABLE_PH] кислотность в тысячных долях pH. Функция возвращает 1 при успехе, 0 при неудаче.
			delay(50);																										//	Ждём 50 мс, пока модуль не сохранит новые данные в flash память.
			return i;																										//
}																															//
																															//
//		ФУНКЦИЯ ПОЛУЧЕНИЯ PH КАЛИБРОВОЧНЫХ ЖИДКОСТЕЙ:																		//	Возвращает pH калибровочной жидкости, или -1 при провале чтения.
float	iarduino_MB_pH::getKnownPH(uint8_t stage){																			//	stage - стадия 1 или 2.
			objModbus->codeError=ERROR_SYNTAX; if( !valID ){ return -1.0f; }												//	Ошибка синтаксиса, модуль не инициализирован.
			float i= -1.0f;																									//
			if( stage==1 ){ i=(float)MB_readAO(AO_KNOWN_PH_1); }															//	Читаем регистр "Holding Register" AO[AO_KNOWN_PH_1]. Функция возвращает прочитанное значение (0...65535), или -1 при неудаче.
			if( stage==2 ){ i=(float)MB_readAO(AO_KNOWN_PH_2); }															//	Читаем регистр "Holding Register" AO[AO_KNOWN_PH_2]. Функция возвращает прочитанное значение (0...65535), или -1 при неудаче.
			if( i>=0 ){ i/=1000.0f; } return i;																				//	Возврашаем кислотность полученную в тысячных долях pH.
}																															//
																															//
//		ФУНКЦИЯ УСТАНОВКИ PH КАЛИБРОВОЧНЫХ ЖИДКОСТЕЙ:																		//	Возвращает результат записи калибровочной жидкости: true-успех / false-провал.
bool	iarduino_MB_pH::setKnownPH(uint8_t stage, float pH){																//	stage - стадия 1 или 2, pH - кислотность от 0.000 до 14.000.
			bool i;																											//
			objModbus->codeError=ERROR_SYNTAX; if( !valID ){ return false; }												//	Ошибка синтаксиса, модуль не инициализирован.
			if( stage!=1 && stage!=2 ){ return false; }																		//	Возвращаем флаг ошибки синтаксиса.
			if( pH<0.0f  || pH>14.0f ){ return false; }																		//	Возвращаем флаг ошибки синтаксиса.
			if( !MB_writeAO(AO_CALC_SAVE, 0x2709) ){ return false; }														//	Записываем значение 0x2709 в регистр "Holding Register" AO[AO_CALC_SAVE]. Функция возвращает 1 при успехе, 0 при неудаче.
			if( stage==1 ){ i = MB_writeAO(AO_KNOWN_PH_1, (uint16_t)(pH*1000.0f) ); }										//	Записываем в регистр "Holding Register" AO[AO_KNOWN_PH_1] кислотность в тысячных долях pH. Функция возвращает 1 при успехе, 0 при неудаче.
			if( stage==2 ){ i = MB_writeAO(AO_KNOWN_PH_2, (uint16_t)(pH*1000.0f) ); }										//	Записываем в регистр "Holding Register" AO[AO_KNOWN_PH_2] кислотность в тысячных долях pH. Функция возвращает 1 при успехе, 0 при неудаче.
			delay(50);																										//	Ждём 50 мс, пока модуль не сохранит новые данные в flash память.
			return i;																										//
}																															//
																															//
//		ФУНКЦИЯ ПОЛУЧЕНИЯ ШАГА СМЕЩЕНИЯ НАПРЯЖЕНИЯ ДАТЧИКА НА 1PH:															//	Возвращает шаг смещения напряжения датчика в мВ/pH, или -1 при провале чтения.
float	iarduino_MB_pH::getVstp(void){																						//
			objModbus->codeError=ERROR_SYNTAX; if( !valID ){ return -1.0f; }												//	Ошибка синтаксиса, модуль не инициализирован.
			float i=(float)MB_readAO(AO_VSTP);																				//	Читаем напряжение из регистра "Holding Register" AO[AO_VSTP]. Функция возвращает прочитанное значение (0...65535), или -1 при неудаче.
			if( i>=0 ){ i/=100.0f; } return i;																				//	Возврашаем напряжение полученное в сотых долях мВ/pH.
}																															//
																															//
//		ФУНКЦИЯ УСТАНОВКИ ШАГА СМЕЩЕНИЯ НАПРЯЖЕНИЯ ДАТЧИКА НА 1PH:															//	Возвращает результат записи шага смещения: true-успех / false-провал.
bool	iarduino_MB_pH::setVstp(float v){																					//	v - напряжение от 0.01 мВ до 655.35 мВ.
			objModbus->codeError=ERROR_SYNTAX; if( !valID ){ return false; }												//	Ошибка синтаксиса, модуль не инициализирован.
			if( v<0.01f || v>655.35f ){ return false; }																		//	Возвращаем флаг ошибки синтаксиса.
			if( !MB_writeAO(AO_CALC_SAVE, 0x2709) ){ return false; }														//	Записываем значение 0x2709 в регистр "Holding Register" AO[AO_CALC_SAVE]. Функция возвращает 1 при успехе, 0 при неудаче.
			bool i = MB_writeAO(AO_VSTP, (uint16_t)(v*100.0f) );															//	Записываем в регистр "Holding Register" AO[AO_VSTP] напряжение в сотых долях В. Функция возвращает 1 при успехе, 0 при неудаче.
			delay(50); 																										//	Ждём 50 мс, пока модуль не сохранит новые данные в flash память.
			return i;																										//
}																															//
																															//
//		ФУНКЦИЯ ПОЛУЧЕНИЯ НАПРЯЖЕНИЯ НА ВХОДЕ ДАТЧИКА:																		//	Возвращает напряжение поданное на вход датчика, или -1 при провале чтения.
float	iarduino_MB_pH::getVin(void){																						//
			objModbus->codeError=ERROR_SYNTAX; if( !valID ){ return -1.0f; }												//	Ошибка синтаксиса, модуль не инициализирован.
			float i=(float)MB_readAI(AI_V0);																				//	Читаем напряжение из регистра "Input Register" AI[AI_V0]. Функция возвращает прочитанное значение (0...65535), или -1 при неудаче.
			if( i>=0 ){ i/=10000.0f; } return i;																			//	Возврашаем напряжение полученное в десятитысячных долях В.
}																															//
																															//
//		ФУНКЦИЯ ПОЛУЧЕНИЯ НАПРЯЖЕНИЯ НА ВЫХОДЕ ДАТЧИКА:																		//	Возвращает напряжение на выходе датчика, или -1 при провале чтения.
float	iarduino_MB_pH::getVout(void){																						//
			objModbus->codeError=ERROR_SYNTAX; if( !valID ){ return -1.0f; }												//	Ошибка синтаксиса, модуль не инициализирован.
			float i=(float)MB_readAI(AI_VOUT);																				//	Читаем напряжение из регистра "Input Register" AI[AI_VOUT]. Функция возвращает прочитанное значение (0...65535), или -1 при неудаче.
			if( i>=0 ){ i/=10000.0f; } return i;																			//	Возврашаем напряжение полученное в десятитысячных долях В.
}																															//
																															//
//		ФУНКЦИЯ ПОЛУЧЕНИЯ НЕЙТРАЛЬНОГО PH ДЛЯ ДАТЧИКА:																		//	Возвращает  нейтральный pH щупа, или -1 при провале чтения.
float	iarduino_MB_pH::getPHn(void){																						//
			objModbus->codeError=ERROR_SYNTAX; if( !valID ){ return -1.0f; }												//	Ошибка синтаксиса, модуль не инициализирован.
			float i=(float)MB_readAO(AO_PHN);																				//	Читаем кислотность из регистра "Holding Register" AO[AO_PHN]. Функция возвращает прочитанное значение (0...65535), или -1 при неудаче.
			if( i>=0 ){ i/=1000.0f; } return i;																				//	Возврашаем кислотность полученную в тысячных долях pH.
}																															//
																															//
//		ФУНКЦИЯ УСТАНОВКИ НЕЙТРАЛЬНОГО PH ДЛЯ ДАТЧИКА:																		//	Возвращает результат записи нейтрального pH щупа: true-успех / false-провал.
bool	iarduino_MB_pH::setPHn(float pH){																					//	pH - кислотность от 0.000 до 14.000.
			objModbus->codeError=ERROR_SYNTAX; if( !valID ){ return false; }												//	Ошибка синтаксиса, модуль не инициализирован.
			if( pH<0.0f || pH>14.0f ){ return false; }																		//	Возвращаем флаг ошибки синтаксиса.
			if( !MB_writeAO(AO_CALC_SAVE, 0x2709) ){ return false; }														//	Записываем значение 0x2709 в регистр "Holding Register" AO[AO_CALC_SAVE]. Функция возвращает 1 при успехе, 0 при неудаче.
			bool i = MB_writeAO(AO_PHN, (uint16_t)(pH*1000.0f) );															//	Записываем в регистр "Holding Register" AO[AO_PHN] кислотность в тысячных долях pH. Функция возвращает 1 при успехе, 0 при неудаче.
			delay(50); 																										//	Ждём 50 мс, пока модуль не сохранит новые данные в flash память.
			return i;																										//
}																															//
																															//
//		ФУНКЦИЯ ПОЛУЧЕНИЯ ВОДОРОДНОГО ПОКАЗАТЕЛЯ ЖИДКОСТИ:																	//	Возвращает измеренную кислотность жидкости, или -1 при провале чтения.
float	iarduino_MB_pH::getPH(void){																						//
			objModbus->codeError=ERROR_SYNTAX; if( !valID ){ return -1.0f; }												//	Ошибка синтаксиса, модуль не инициализирован.
			float i=(float)MB_readAI(AI_PH);																				//	Читаем кислотность из регистра "Input Register" AI[AI_PH]. Функция возвращает прочитанное значение (0...65535), или -1 при неудаче.
			if( i>=0 ){ i/=1000.0f; } return i;																				//	Возврашаем кислотность полученную в тысячных долях pH.
}																															//
																															//
/**		ФУНКЦИИ ДУБЛЁРЫ ОБЪЕКТА objModbus ДЛЯ ЧТЕНИЯ/ЗАПИСИ С НЕСКОЛЬКИХ ПОПЫТОК	**/										//
																															//
int8_t		iarduino_MB_pH::MB_readDO		(uint16_t reg							){ uint8_t cntRW=maxRW; while(cntRW){ int8_t   i=objModbus->coilRead            (valID, reg);        if( i>-1 ){ return i;    }else{ cntRW--; } } return -1;    }
int8_t		iarduino_MB_pH::MB_readDI		(uint16_t reg							){ uint8_t cntRW=maxRW; while(cntRW){ int8_t   i=objModbus->discreteInputRead   (valID, reg);        if( i>-1 ){ return i;    }else{ cntRW--; } } return -1;    }
int32_t		iarduino_MB_pH::MB_readAO		(uint16_t reg							){ uint8_t cntRW=maxRW; while(cntRW){ int32_t  i=objModbus->holdingRegisterRead (valID, reg);        if( i>-1 ){ return i;    }else{ cntRW--; } } return -1;    }
int32_t		iarduino_MB_pH::MB_readAI		(uint16_t reg							){ uint8_t cntRW=maxRW; while(cntRW){ int32_t  i=objModbus->inputRegisterRead   (valID, reg);        if( i>-1 ){ return i;    }else{ cntRW--; } } return -1;    }
uint16_t	iarduino_MB_pH::MB_requestFrom	(uint8_t  type, uint16_t reg, uint16_t n){ uint8_t cntRW=maxRW; while(cntRW){ uint16_t i=objModbus->requestFrom         (valID, type,reg,n); if( i> 0 ){ return i;    }else{ cntRW--; } } return  0;    }
uint8_t		iarduino_MB_pH::MB_getInfo		(uint8_t  id							){ uint8_t cntRW=maxRW; while(cntRW){ uint8_t  i=objModbus->getInfo             (id);                if( i> 0 ){ return i;    }else{ cntRW--; } } return  0;    }
int32_t		iarduino_MB_pH::MB_diagnostic	(uint16_t func, uint16_t data			){ uint8_t cntRW=maxRW; while(cntRW){ int32_t  i=objModbus->diagnostic          (valID, func, data); if( i>-1 ){ return i;    }else{ cntRW--; } } return -1;    }
bool		iarduino_MB_pH::MB_changeID		(uint8_t  newID							){ uint8_t cntRW=maxRW; while(cntRW){        if( objModbus->changeID            (valID, newID)  ){ valID=newID;  return true; }else{ cntRW--; } } return false; }
bool		iarduino_MB_pH::MB_writeDO		(uint16_t reg, bool val					){ uint8_t cntRW=maxRW; while(cntRW){        if( objModbus->coilWrite           (valID, reg, val)             ){ return true; }else{ cntRW--; } } return false; }
bool		iarduino_MB_pH::MB_writeAO		(uint16_t reg, uint16_t val				){ uint8_t cntRW=maxRW; while(cntRW){        if( objModbus->holdingRegisterWrite(valID, reg, val)             ){ return true; }else{ cntRW--; } } return false; }
