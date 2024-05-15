// ПРИМЕР УПРАВЛЕНИЯ ВСЕМИ pH-МЕТРАМИ НА ШИНЕ:                                     //
// без указания их адресов в скетче.                                               //
                                                                                   //
#include <SoftwareSerial.h>                                                        //   Подключаем библиотеку для работы с программной шиной UART.
#include <iarduino_Modbus.h>                                                       //   Подключаем библиотеку для работы по протоколу Modbus.
#include <iarduino_MB_pH.h>                                                        //   Подключаем библиотеку для работы с pH-метрами.
                                                                                   //
SoftwareSerial  rs485(8,9);                                                        //   Создаём объект для работы с программной шиной UART-RS485 указывая выводы RX, TX.
ModbusClient    modbus(rs485, 2);                                                  //   Создаём объект для работы по протоколу Modbus указывая объект программной шины UART-RS485 и вывод DE конвертера UART-RS485.
iarduino_MB_pH* sensor;                                                            //   Создаём указатель который будет переопределён в массив объектов работы с pH-метрами.
                                                                                   //
uint8_t sum=0;                                                                     //   Определяем переменную для хранения количества найденных pH-метров.
                                                                                   //
void setup(){                                                                      //
     Serial.begin(9600);   while(!Serial);                                         //   Инициируем передачу данных в монитор последовательного порта, указав его скорость.
     rs485.begin(9600); // while(!rs485 );                                         //   Инициируем работу с программной шиной UART-RS485 указав её скорость.
     modbus.begin();                                                               //   Инициируем работу по протоколу Modbus.
//   modbus.setTimeout(10);                                                        //   Указываем максимальное время ожидания ответа по протоколу Modbus.
//   modbus.setDelay(4);                                                           //   Указываем минимальный интервал между отправляемыми сообщениями по протоколу Modbus.
//   modbus.setTypeMB( MODBUS_RTU );                                               //   Указываем тип протокола Modbus: MODBUS_RTU (по умолчанию), или MODBUS_ASCII.
//   Выполняем поиск всех pH-метров на шине (занимает несколько секунд):           //
     Serial.println("Поиск pH-метров ..." );                                       //
     sum = modbus.findID( DEF_MODEL_pH );                                          //   Ищем адреса всех устройств с идентификатором pH-метра DEF_MODEL_pH        (для блоков с дисплеем).
//   sum = modbus.findID( DEF_MODEL_pH_zummer );                                   //   Ищем адреса всех устройств с идентификатором pH-метра DEF_MODEL_pH_zummer (для блоков с зуммером).
     uint8_t arrID[sum], num=0;                                                    //   Объявляем массив arrID для хранения найденных адресов.
     while( modbus.available() ){ arrID[num++]=modbus.read(); }                    //   Заполняем массив arrID найденными адресами.
     if( sum ){                                                                    //
     //  Переопределяем указатель sensor в массив объектов:                        //
         sensor = (iarduino_MB_pH*) malloc( sizeof(iarduino_MB_pH)*sum );          //   Выделяем под массив sensor требуемый объем памяти.
     //  Инициируем работу с pH-метрами:                                           //
         for(uint8_t i=0; i<sum; i++){                                             //
             sensor[i]=modbus;                                                     //   Указываем очередному объекту работы с pH-метром, объект протокола Modbus.
             sensor[i].begin( arrID[i] );                                          //   Инициируем работу с очередным pH-метром, указав очередной адрес из ранее найденных.
         }                                                                         //
     }                                                                             //
//   Выводим сообщение:                                                            //
     Serial.print((String) "Найдено "+sum+" pH-метров, с адресами: " );            //
     for(uint8_t i=0; i<sum; i++){ Serial.print(arrID[i]); Serial.print(", "); }   //
     Serial.println();                                                             //
}                                                                                  //
                                                                                   //
void loop(){                                                                       //
     for(uint8_t i=0; i<sum; i++){                                                 //   Проходим по всем pH-метрам.
         Serial.print( "Модуль с адресом "        );                               //
         Serial.print( sensor[i].getID()          );                               //
         Serial.print( " показывает кислотность " );                               //
         Serial.print( sensor[i].getPH()          );                               //
         Serial.print( " pH. \r\n"                );                               //
         delay(1000);                                                              //
     }                                                                             //
}                                                                                  //
