// ПРИМЕР СМЕНЫ АДРЕСА:                                                            //
                                                                                   //
uint8_t nowAddress = 4;                                                            //   Текущий адрес ( 1 - 247 ).
uint8_t newAddress = 10;                                                           //   Новый адрес ( 1 - 247 ).
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
     int f;                                                                        //
     Serial.begin(9600);   while(!Serial);                                         //   Инициируем передачу данных в монитор последовательного порта, указав его скорость.
     rs485.begin(9600); // while(!rs485 );                                         //   Инициируем работу с программной шиной UART-RS485 указав её скорость.
     modbus.begin();                                                               //   Инициируем работу по протоколу Modbus.
//   modbus.setTimeout(10);                                                        //   Указываем максимальное время ожидания ответа по протоколу Modbus.
//   modbus.setDelay(4);                                                           //   Указываем минимальный интервал между отправляемыми сообщениями по протоколу Modbus.
//   modbus.setTypeMB( MODBUS_RTU );                                               //   Указываем тип протокола Modbus: MODBUS_RTU (по умолчанию), или MODBUS_ASCII.
//   Инициируем работу с pH-метром:                                                //
     f =    sensor.begin(nowAddress);                                              //   Инициируем работу с pH-метром, указав текущий адрес модуля nowAddress.
     if(f){ Serial.println("pH-метр найден");            }                         //   Если адрес не указан f=sensor.begin(), то он будет найден, но это займёт некоторое время.
     else { Serial.println("pH-метр не найден"); return; }                         //
//   Меняем адрес pH-метра:                                                        //
     f =    sensor.changeID(newAddress);                                           //   Меняем адрес pH-метра на новый newAddress.
     if(f){ Serial.println("Адрес изменён");            }                          //
     else { Serial.println("Адрес не изменён"); return; }                          //
//   Выводим сообшение о pH-метре:                                                 //
     Serial.println((String) "Текущий адрес      = "+sensor.getID()      );        //   Выводим текущий адрес pH-метра.
     Serial.println((String) "Версия прошивки    = "+sensor.getVersion() );        //   Выводим версию прошивки pH-метра.
}                                                                                  //
                                                                                   //
void loop(){                                                                       //
//   Мигаем светодиодом обнаружения устройства (на разъёме):                       //
     sensor.setIDLED(false); delay(1000);                                          //
     sensor.setIDLED(true ); delay(1000);                                          //
}                                                                                  //