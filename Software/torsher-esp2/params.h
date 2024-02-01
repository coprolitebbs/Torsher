#include <ESP8266WiFi.h>

#define GL_VERSION 2       // код версии прошивки

const char *ssid = "Torsher";
const char *password = "123456789";
const char NTPserver[] = /*"pool.ntp.org"*/"192.168.0.37";



#define CFG_SIZE 12
struct Config {
  byte workFrom = 0;      // часы работы (0,1.. 23)
  byte workTo = 0;        // часы работы (0,1.. 23)
  byte doplight_from = 22;
  byte mainlight_m_from = 6;
  byte mainlight_m_to = 7;
  byte mainlight_e_from = 19;
  byte mainlight_e_to = 22;
  
  //byte matrix = 1;        // тип матрицы 1.. 8

  //int16_t length = 16;    // длина ленты
  //int16_t width = 16;     // ширина матрицы

  byte GMT = 4;          // часовой пояс +4
  uint32_t cityID = 1;    // city ID

  byte state = 1;         // состояние 0 выкл, 1 вкл
  byte group = 1;         // группа девайса (1-10)
  byte role = 0;          // 0 slave, 1 master
  byte WiFimode = 0;      // 0 AP, 1 local
  //byte presetAmount = 3;  // количество режимов
  //byte manualOff = 0;     // выключали вручную?
  //int8_t curPreset = 0;   // текущий режим
  //int16_t minLight = 0;   // мин освещённость
  //int16_t maxLight = 1023;// макс освещённость
  char ssid[32];          // логин wifi
  char pass[32];          // пароль wifi
  IPAddress localIp;

  byte version = GL_VERSION;
  byte update = 0;
};

String toString(const IPAddress& address) {
  return String() + address[0] + "." + address[1] + "." + address[2] + "." + address[3];
}
