// ПРИМЕР ПОЛУЧЕНИЯ И ВЫЧИСЛЕНИЯ ДАННЫХ:                                           //
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
}                                                                                  //   Если адрес не указан sensor.begin(), то он будет найден, но это займёт некоторое время.
                                                                                   //
void loop(){                                                                       //
     Serial.println( F("Кислотность = pHn + (Vn-Vout)/(Vstep*Ky)") );              //   Формула по которой модуль вычисляет кислотность жидкости.
     Serial.print  ( F("            = "                          ) );              //
     Serial.print  ( sensor.getPHn()         , 1                   );              //   pHn  - Нейтральная кислотность щупа - кислотность при которой на выводах щупа нет разности потенциалов.
     Serial.print  ( "+("                                          );              //
     Serial.print  ( sensor.getVn()          , 2                   );              //   Vn   - Напряжение на выходе усилителя при нейтральной кислотности жидкости.
     Serial.print  ( "-"                                           );              //
     Serial.print  ( sensor.getVout()        , 2                   );              //   Vout - Напряжение на выходе усилителя.
     Serial.print  ( ")/("                                         );              //
     Serial.print  ( sensor.getVstp()/1000.0 , 5                   );              //   Vstp - Отношение напряжения щупа к кислотности жидкости мВ/pH.
     Serial.print  ( "*"                                           );              //
     Serial.print  ( sensor.getKy()          , 0                   );              //   Ky   - Коэффициент усиления усилителя к которому подключён щуп.
     Serial.print  ( ") = "                                        );              //
     Serial.print  ( sensor.getPH()          , 1                   );              //   pH   - Рассчитанная кислотность жидкости в которую опущен щуп.
     Serial.println( " pH.\r\n"                                    );              //
     delay(1000);                                                                  //
}                                                                                  //
