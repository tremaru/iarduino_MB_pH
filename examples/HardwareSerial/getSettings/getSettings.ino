// ПРИМЕР ПОЛУЧЕНИЯ НАСТРОЕК МОДУЛЯ:                                               //
                                                                                   //
#include <iarduino_Modbus.h>                                                       //   Подключаем библиотеку для работы по протоколу Modbus.
#include <iarduino_MB_pH.h>                                                        //   Подключаем библиотеку для работы с pH-метрами.
                                                                                   //
ModbusClient   modbus(Serial1, 2);                                                 //   Создаём объект для работы по протоколу Modbus указывая объект шины UART-RS485 и вывод DE конвертера UART-RS485.
iarduino_MB_pH sensor(modbus);                                                     //   Создаём объект для работы с pH-метром указывая объект протокола Modbus.
                                                                                   //
void setup(){                                                                      //
     Serial.begin(9600); while(!Serial);                                           //   Инициируем передачу данных в монитор последовательного порта, указав его скорость.
     Serial1.begin(9600); while(!Serial1);                                         //   Инициируем работу с шиной UART-RS485 указав её скорость.
     modbus.begin();                                                               //   Инициируем работу по протоколу Modbus.
//   modbus.setTimeout(10);                                                        //   Указываем максимальное время ожидания ответа по протоколу Modbus.
//   modbus.setDelay(4);                                                           //   Указываем минимальный интервал между отправляемыми сообщениями по протоколу Modbus.
//   modbus.setTypeMB( MODBUS_RTU );                                               //   Указываем тип протокола Modbus: MODBUS_RTU (по умолчанию), или MODBUS_ASCII.
     sensor.begin(4);                                                              //   Инициируем работу с pH-метром, указав его адрес.
                                                                                   //   Если адрес не указан sensor.begin(), то он будет найден, но это займёт некоторое время.
//   Выводим параметры хранящиеся в энергонезависимой памяти модуля:               //
     Serial.print(           "pH1  = " ); Serial.print( sensor.getKnownPH(1) ,3 ); //   Выводим кислотность жидкости (0...14,000 pH) требуемую для 1 стадии калибровки модуля.
     Serial.print( "pH.\r\n" "pH2  = " ); Serial.print( sensor.getKnownPH(2) ,3 ); //   Выводим кислотность жидкости (0...14,000 pH) требуемую для 2 стадии калибровки модуля.
     Serial.print( "pH.\r\n" "Ky   = " ); Serial.print( sensor.getKy()       ,3 ); //   Выводим коэффициент усиления (1...65,535) операционного усилителя к которому подключён щуп.
     Serial.print( "\r\n"    "Vstp = " ); Serial.print( sensor.getVstp()     ,2 ); //   Выводим шаг смещения напряжения датчика (0,01...655,35 мВ) при смещении кислотности на 1 pH.
     Serial.print( "мВ.\r\n" "pHn  = " ); Serial.print( sensor.getPHn()      ,3 ); //   Выводим нейтральную кислотность для датчика (0...14,000 pH) при которой на выводах щупа нет разности потенциалов.
     Serial.print( "pH.\r\n"           );                                          //
}                                                                                  //
                                                                                   //
void loop(){                                                                       //
}                                                                                  //