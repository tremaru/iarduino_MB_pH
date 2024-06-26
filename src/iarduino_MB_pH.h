//	Библиотека для работы с pH-метром iarduino (датчиком кислотности жидкости) по шине Modbus:
//	Датчик кислотности жидкости (pH-метр) с дисплеем, RS485/Modbus: https://iarduino.ru/shop/Sensory-Datchiki/ph-Modbus.html
//	Датчик кислотности жидкости (pH-метр) без дисплея, RS485/Modbus: https://iarduino.ru/shop/Sensory-Datchiki/ph-Modbus-zumer.html
//  Версия: 1.2.5
//  Последнюю версию библиотеки Вы можете скачать по ссылке: https://iarduino.ru/file/599.html
//  Подробное описание функции бибилиотеки доступно по ссылке: https://wiki.iarduino.ru/page/ph-sensor-modbus/
//  Библиотека является собственностью интернет магазина iarduino.ru и может свободно использоваться и распространяться!
//  При публикации устройств или скетчей с использованием данной библиотеки, как целиком, так и её частей,
//  в том числе и в некоммерческих целях, просим Вас опубликовать ссылку: http://iarduino.ru
//  Автор библиотеки: Панькин Павел
//  Если у Вас возникли технические вопросы, напишите нам: shop@iarduino.ru
//	Дополнительно требуется установить библиотеку реализации протокола Modbus RTU/ASCII на шине UART->RS485: https://iarduino.ru/file/591.html


#ifndef iarduino_MB_pH_h
#define iarduino_MB_pH_h

#if defined(ARDUINO) && (ARDUINO >= 100)
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

#include <iarduino_Modbus.h>														//	Подключаем файл iarduino_Modbus.h - для работы по протоколу Modbus.
																					//
#define DEF_MODEL_pH 7																//	Идентификатор модели - константа (блок pH-метра с дисплеем).
#define DEF_MODEL_pH_zummer 9														//	Идентификатор модели - константа (блок pH-метра с зуммером).
																					//
#define ERR_MB_pH_Power	0b0000000100000000											//	Флаг регистра диагностики указывающий на ожидание стабилизации после подачи питания.
#define ERR_MB_pH_Data	0b0000000010000000											//	Флаг регистра диагностики указывающий на некорректное чтение данных с АЦП.
#define ERR_MB_pH_Jumps	0b0000000001000000											//	Флаг регистра диагностики указывающий на указывает на то, что показания pH "скачут".
#define ERR_MB_pH_Vout	0b0000000000110000											//	Флаг регистра диагностики указывающий на некорректное напряжения на выходе датчика (0>pH>14).
#define ERR_MB_pH_Vin	0b0000000000001100											//	Флаг регистра диагностики указывающий на некорректное напряжение подаваемое на датчик (Vo != 0.5Vcc ± 0.1).
#define ERR_MB_pH_5V	0b0000000000000010											//	Флаг регистра диагностики указывающий на низкое напряжение питания 5В после DC-DC преобразователя < 4В. Стабильная работа не гарантируется.
#define ERR_MB_pH_Init	0b0000000000000001											//	Флаг регистра диагностики указывающий на провал инициализации АЦП.
																					//
#define STABLE_PH		4															//	Первый параметр функций getFluctuation() и setFluctuation().
#define UNSTABLE_PH		5															//	Первый параметр функций getFluctuation() и setFluctuation().
																					//
class iarduino_MB_pH{																//
	public:																			//
	/**	Конструктор класса **/														//
		iarduino_MB_pH					(ModbusClient&	);							//	Объявляем конструктор класса.											Параметры: ссылка на объект работы по протоколу Modbus.
	/**	Пользовательские функции **/												//
		bool			begin			(uint8_t id=0	);							//	Объявляем  функцию инициализации модуля.								Параметры функции: ID-адрес на шине. Функция возвращает результат инициализации: true-успех / false-провал.
		bool			reset			(void			);							//	Объявляем  функцию перезагрузки модуля.									Параметры функции: нет. Функция возвращает результат перезагрузки: true-успех / false-провал.
		bool			changeID		(uint8_t new_id	);							//	Объявляем  функцию смены ID-адреса на шине (выполняется >120мс).		Параметры функции: новый ID. Функция возвращает результат смены ID: true-успех / false-провал.
		bool			setSpeedMB		(uint32_t speed	);							//	Объявляем  функцию смены скорости передачи данных.						Параметры функции: новая скорость. Функция возвращает результат смены скорости: true-успех / false-провал. Скорость необходимо подтвердить в течении 2 секунд.
		bool			ackSpeedMB		(void			);							//	Объявляем  функцию подтверждения скорости (выполняется >50мс).			Параметры функции: нет. Функция возвращает результат подтверждения скорости: true-успех / false-провал. 
		bool			setTypeMB		(uint8_t type	);							//	Объявляем  функцию смены типа протокола Modbus.							Параметры функции: MODBUS_RTU/MODBUS_ASCII. Функция возвращает результат смены типа протокола Modbus: true-успех / false-провал. Тип необходимо подтвердить в течении 2 секунд.
		bool			ackTypeMB		(void			);							//	Объявляем  функцию подтверждения типа протокола (выполняется >50мс).	Параметры функции: нет. Функция возвращает результат подтверждения типа протокола Modbus: true-успех / false-провал. 
		bool			writeFlash		(uint16_t data	);							//	Объявляем  функцию записи пользовательского значения в Flash память.	Параметры функции: целое беззнаковое число от 0 до 65535. Функция возвращает результат записи: true-успех / false-провал. 
		int32_t			readFlash		(void			);							//	Объявляем  функцию чтения пользовательского значения из Flash памяти.	Параметры функции: нет. Функция возвращает прочитанное значение, или -1 при провале чтения.
		uint8_t			getID			(void			){ return valID;	}		//	Определяем функцию получения текущего ID (адрес модуля на шине).		Параметры функции: нет. Функция возвращает текущий ID-адрес, или 0 если модуль не определён.
		uint8_t			getVersion		(void			){ return valVers;	}		//	Определяем функцию получения текущей версии прошивки модуля.			Параметры функции: нет. Функция возвращает версию прошивки, или 0 если модуль не определён.
		float			getPWR			(void			);							//	Объявляем  функцию получения напряжения питания +5V.					Параметры функции: нет. Функция возвращает напряжение питания в Вольтах, или -1 при провале чтения.
		bool			setIDLED		(bool on 		);							//	Объявляем  функцию смены состояния светодиода обнаружения устройства.	Параметры функции: состояние светодиода true/false. Функция возвращает результат изменения состояния: true-успех / false-провал.
		int32_t			getErr			(void			);							//	Объявляем  функцию получения флагов ошибок из регистра диагностики.		Параметры функции: нет. Функция возвращает значение регистра диагностики, каждый из 16 бит которого является флагом ошибки, или -1 при провале чтения.
		bool			setCalibration	(void					);					//	Объявляем  функцию запуска калибровки.									Параметры функции: нет. Функция возвращает результат запуска калибровки: true-успех / false-провал.
		int8_t			getCalibration	(void					);					//	Объявляем  функцию получения текущей стадии калибровки.					Параметры функции: нет. Функция возвращает стадию калибровки: 1 или 2, 0-не выполняется, 3-ожидание смены жидкости, или -1 при провале чтения.
		bool			setStopCalib	(void					);					//	Объявляем  функцию отмены калибровки.									Параметры функции: нет. Функция возвращает результат отмены калибровки: true-успех / false-провал.
		int8_t			getResultCalib	(void					);					//	Объявляем  функцию получения результата последней калибровки.			Параметры функции: нет. Функция возвращает флаг результат калибровки: true-успех / false-провал, или -1 при провале чтения.
		int8_t			getStability	(void					);					//	Объявляем  функцию получения флага нормализации показаний.				Параметры функции: нет. Функция возвращает флаг стабильности показаний pH: true-стабильны / false-меняются, или -1 при провале чтения.
		float			getFluctuation	(uint8_t type			);					//	Объявляем  функцию получения границы гистерезиса флуктуаций.			Параметры функции: тип границы STABLE_PH/UNSTABLE_PH. По этим границам определяется флаг стабильности показаний pH.
		bool			setFluctuation	(uint8_t type,  float pH);					//	Объявляем  функцию установки границы гистерезиса флуктуаций.			Параметры функции: тип границы STABLE_PH/UNSTABLE_PH, значение границы в pH. По этим границам определяется флаг стабильности показаний pH.
		float			getKnownPH		(uint8_t stage			);					//	Объявляем  функцию получения pH калибровочных жидкостей.				Параметры функции: стадия 1 или 2. Функция возвращает pH калибровочной жидкости, или -1 при провале чтения.
		bool			setKnownPH		(uint8_t stage, float pH);					//	Объявляем  функцию установки pH калибровочных жидкостей.				Параметры функции: стадия 1 или 2, pH от 0 до 14,000. Функция возвращает результат записи калибровочной жидкости: true-успех / false-провал.
		float			getVstp			(void					);					//	Объявляем  функцию получения шага смещения напряжения датчика на 1pH.	Параметры функции: нет. Функция возвращает шаг смещения напряжения датчика мВ/pH, или -1 при провале чтения.
		bool			setVstp			(float mV				);					//	Объявляем  функцию установки шага смещения напряжения датчика на 1pH.	Параметры функции: напряжение от 0,01 мВ до 655,35 мВ). Функция возвращает результат записи шага смещения: true-успех / false-провал.
		float			getVin			(void					);					//	Объявляем  функцию получения напряжения на входе датчика.				Параметры функции: нет. Функция возвращает напряжение поданное на вход датчика, или -1 при провале чтения.
		float			getVout			(void					);					//	Объявляем  функцию получения напряжения на выходе датчика.				Параметры функции: нет. Функция возвращает напряжение на выходе ОУ, или -1 при провале чтения.
		float			getPHn			(void					);					//	Объявляем  функцию получения нейтрального pH для датчика.				Параметры функции: нет. Функция возвращает нейтральный pH щупа, или -1 при провале чтения.
		bool			setPHn			(float pH				);					//	Объявляем  функцию установки нейтрального pH для датчика.				Параметры функции: pH от 0 до 14,000. Функция возвращает результат записи нейтрального pH щупа: true-успех / false-провал.
		float			getPH			(void					);					//	Объявляем  функцию получения водородного показателя жидкости.			Параметры функции: нет. Функция возвращает измеренную кислотность жидкости, или -1 при провале чтения.
	private:																		//	
	/**	Внутренние переменные **/													//
		ModbusClient*	objModbus;													//	Объявляем  указатель на объект работы по протоколу Modbus.				Указатель получит адрес объекта в конструкторе класса.
		uint8_t			valID			= 0;										//	Определяем переменную для хранения ID-адреса который был проверен.
		uint8_t			valVers			= 0;										//	Определяем переменную для хранения версии прошивки.
		uint8_t			maxRW			= 5;										//	Определяем максимальное количество попыток чтения/записи.
	/**	Внутренние функции **/														//
		int8_t			MB_readDO		(uint16_t reg							);	//	Дублер функции objModbus.coilRead();             Чтение одного регистра DO, из  модуля valID, maxRW попыток. Параметры функции: адрес регистра. Функция возвращает прочитанное значение (0/1), или -1 при неудаче.
		int8_t			MB_readDI		(uint16_t reg							);	//	Дублер функции objModbus.discreteInputRead();    Чтение одного регистра DI, из  модуля valID, maxRW попыток. Параметры функции: адрес регистра. Функция возвращает прочитанное значение (0/1), или -1 при неудаче.
		int32_t			MB_readAO		(uint16_t reg							);	//	Дублер функции objModbus.holdingRegisterRead();  Чтение одного регистра AO, из  модуля valID, maxRW попыток. Параметры функции: адрес регистра. Функция возвращает прочитанное значение (0...65535), или -1 при неудаче.
		int32_t			MB_readAI		(uint16_t reg							);	//	Дублер функции objModbus.inputRegisterRead();    Чтение одного регистра AI, из  модуля valID, maxRW попыток. Параметры функции: адрес регистра. Функция возвращает прочитанное значение (0...65535), или -1 при неудаче.
		uint16_t		MB_requestFrom	(uint8_t  type, uint16_t reg, uint16_t n);	//	Дублер функции objModbus.requestFrom();          Чтение нескольк регистров, из  модуля valID, maxRW попыток. Параметры функции: тип регистра (COILS/DISCRETE_INPUTS/HOLDING_REGISTERS/INPUT_REGISTERS), адрес первого регистра, количество регистров. Функция возвращает количество прочитанных значений, или 0 при неудаче.
		uint8_t			MB_getInfo		(uint8_t  id							);	//	Дублер функции objModbus.getInfo();              Чтение информации о устройстве модуля id   , maxRW попыток. Параметры функции: id модуля. Функция возвращает количество байт данных об устройстве, доступных для чтения функцией read() и available().
		int32_t			MB_diagnostic	(uint16_t func, uint16_t data=0			);	//	Дублер функции objModbus.diagnostic();           Выполнение команды диагностики модуля valID, maxRW попыток. Параметры функции: номер функции диагностики, данные. Функция возвращает данные результата выполнения функции диагностики, или -1 при неудаче.
		bool			MB_changeID		(uint8_t  newID							);	//	Дублер функции objModbus.changeID();             Сохранение нового ID     , для модуля valID, maxRW попыток. Параметры функции: новый id модуля. Функция возвращает результат сохранения адреса (0/1).
		bool			MB_writeDO		(uint16_t reg, bool val					);	//	Дублер функции objModbus.coilWrite();            Запись в один регистр  DO, для модуля valID, maxRW попыток. Параметры функции: адрес регистра, значение (0/1). Функция возвращает 1 при успехе, 0 при неудаче.
		bool			MB_writeAO		(uint16_t reg, uint16_t val				);	//	Дублер функции objModbus.holdingRegisterWrite(); Запись в один регистр  AO, для модуля valID, maxRW попыток. Параметры функции: адрес регистра, значение (0...65535). Функция возвращает 1 при успехе, 0 при неудаче.
};

#endif
