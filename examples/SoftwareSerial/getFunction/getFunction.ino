// ПРИМЕР ПОЛУЧЕНИЯ И ВЫЧИСЛЕНИЯ ДАННЫХ:                                           //
                                                                                   //
#include <SoftwareSerial.h>                                                        //   Подключаем библиотеку для работы с программной шиной UART.
#include <iarduino_Modbus.h>                                                       //   Подключаем библиотеку для работы по протоколу Modbus.
#include <iarduino_MB_pH.h>                                                        //   Подключаем библиотеку для работы с pH-метрами.
                                                                                   //
SoftwareSerial rs485(8,9);                                                         //   Создаём объект для работы с программной шиной UART-RS485 указывая выводы RX, TX.
ModbusClient   modbus(rs485, 2);                                                   //   Создаём объект для работы по протоколу Modbus указывая объект программной шины UART-RS485 и вывод DE конвертера UART-RS485.
iarduino_MB_pH sensor(modbus);                                                     //   Создаём объект для работы с pH-метром указывая объект протокола Modbus.
                                                                                   //
void setup(){                                                                      //
     Serial.begin(9600);   while(!Serial);                                         //   Инициируем передачу данных в монитор последовательного порта, указав его скорость.
     rs485.begin(9600); // while(!rs485 );                                         //   Инициируем работу с программной шиной UART-RS485 указав её скорость.
     modbus.begin();                                                               //   Инициируем работу по протоколу Modbus.
//   modbus.setTimeout(10);                                                        //   Указываем максимальное время ожидания ответа по протоколу Modbus.
//   modbus.setDelay(4);                                                           //   Указываем минимальный интервал между отправляемыми сообщениями по протоколу Modbus.
//   modbus.setTypeMB( MODBUS_RTU );                                               //   Указываем тип протокола Modbus: MODBUS_RTU (по умолчанию), или MODBUS_ASCII.
     sensor.begin(4);                                                              //   Инициируем работу с pH-метром, указав его адрес.
}                                                                                  //   Если адрес не указан sensor.begin(), то он будет найден, но это займёт некоторое время.
                                                                                   //
void loop(){                                                                       //
     Serial.println( F("Кислотность =  pHn  - ( Vout - Vin  ) / Vstep") );         //   Формула по которой модуль вычисляет кислотность жидкости.
     Serial.print  ( F("            = "                               ) );         //
     Serial.print  ( sensor.getPHn()         , 3                        );         //   pHn  - Нейтральная кислотность щупа - кислотность при которой на выводах щупа нет разности потенциалов.
     Serial.print  ( " - ("                                             );         //
     Serial.print  ( sensor.getVout()        , 4                        );         //   Vout - Напряжение на выходе датчика.
     Serial.print  ( "-"                                                );         //
     Serial.print  ( sensor.getVin()         , 4                        );         //   Vin  - Напряжение на входе датчика.
     Serial.print  ( ") / "                                             );         //
     Serial.print  ( sensor.getVstp()/1000.0 , 5                        );         //   Vstp - Чувствительность щупа (отношение напряжения щупа к кислотности жидкости) мВ/pH.
     Serial.print  ( " = "                                              );         //
     Serial.print  ( sensor.getPH()          , 1                        );         //   pH   - Рассчитанная кислотность жидкости в которую опущен щуп.
     Serial.println( " pH.\r\n"                                         );         //
     delay(1000);                                                                  //
}                                                                                  //
