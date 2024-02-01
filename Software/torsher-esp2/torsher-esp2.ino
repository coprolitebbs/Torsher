// Часть взята с GyverLamp: https://github.com/AlexGyver/GyverLamp2/tree/main

//Parameters:
//Board: Lolin Wemos D1 mini (clone)
//Flash size: 4Mb (FS 1MB OTA)

//or
//Board: LOLIN(Wemos) D1 Mini lite
// Flash: 1Mb (FS noneOTA:502Kb)

#include <ESP8266WiFi.h>
//#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
//#include <OneWire.h>
#include "RTClib.h"
#include <time.h>
#include "NTPClient.h"
#include "WiFiUdp.h"
#include "params.h"
#include <EEPROM.h>
#include <DNSServer.h>
//#include <Esp.h>



#define RELAYPIN D1

#define EE_TOUT 30000       // таймаут сохранения епром после изменения, мс
#define DEBUG_SERIAL        // закомментируй чтобы выключить отладку (скорость 115200)
#define EE_KEY 56           // ключ сброса eeprom
#define NTP_UPD_PRD 180     // период обновления времени с NTP сервера, минут
#define GL_KEY "GL"         // ключ сети



#ifdef DEBUG_SERIAL
#define DEBUGLN(x) Serial.println(x)
#define DEBUG(x) Serial.print(x)
#else
#define DEBUGLN(x)
#define DEBUG(x)
#endif

//OneWire oneWire(D5);
//DeviceAddress devaddr;

IPAddress myIP;
ESP8266WebServer server(80);
MDNSResponder mdns;
//WiFiServer server(80);
//float temperature = 0;

//RTC_DS1307 RTC;
RTC_DS3231 RTC;

#define GPIO_ADDR     0x27
#define BUTTON_PIN A0
#define ANTIBEAT 100

#define MODE_ALLOFF 0
#define MODE_AUTOMATE 1
#define MODE_FORCE 2

#define AMODE_ALLOFF 0
#define AMODE_DOPLIGHT 1
#define AMODE_MAINLIGHT 2


#define REL1 D3
#define REL2 D4

#define CONNECT_STEPS 160

byte mode = MODE_ALLOFF;
byte oldmode = 255;
byte amode = AMODE_ALLOFF;
byte old_amode = 255;
int aa = 0;
byte oldhour = 0;

bool hour_display = false;

int update_counter = 0;
DateTime now;

unsigned long previousMillis = 0;
unsigned long previousMillis2 = 0;
const long interval = 10000;
const long interval2 = 3000;

bool force_remote = false;
bool mainflashed = false;

bool wifi_ap = false;

byte nmode = AMODE_ALLOFF;

const long utcOffsetInSeconds = 36000;

String reslt = "";

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTPserver, utcOffsetInSeconds);

// DNS server
const byte DNS_PORT = 53;
DNSServer dnsServer;

///adds
uint16_t portNum = 0;
Config cfg;
bool EEcfgFlag = false;
String html_header = "<!DOCTYPE html><html><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\"><head><title>Торшер</title><style>body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }</style></head>";

int counter = 0;
uint32_t timer = 0;
unsigned long currentMillis = 0;

void myDelay(uint32_t how) {
  timer = millis();
  counter = 0;
  while (millis() - timer < how) {
    //timer = millis();
    counter++;
    //ESP.wdtFeed();
    //wdt_reset();
    yield();
  }
}

void make_header() {
  now = RTC.now();
  reslt = "";
  reslt += html_header;
  reslt += "<body><form method=\"POST\" action=\"ok\"><input name=\"ssid\" value=\"" + String(cfg.ssid) + "\">&nbsp;WIFI</br><input name=\"pswd\" type=\"password\" value=\"" + String(cfg.pass) + "\">&nbsp;Пароль&nbsp;сети</br><input name=\"lip\" placeholder=\"192.168.1.2\" value=\"" + toString(cfg.localIp) + "\">&nbsp;Локальный&nbsp;IP</br>";
  reslt += "</br>От&nbsp;<input name=\"dpp\" size=\"3\" placeholder=\"" + String(cfg.doplight_from, DEC) + "\" value=\"" + String(cfg.doplight_from, DEC) + "\">&nbsp;до24&nbsp;Доп.&nbsp;подсветка";
  reslt += "</br>От&nbsp;<input name=\"mp1\" size=\"3\" placeholder=\"" + String(cfg.mainlight_m_from, DEC) + "\" value=\"" + String(cfg.mainlight_m_from, DEC) + "\" type=\"number\">&nbsp;до&nbsp;<input name=\"mp2\" size=\"3\" placeholder=\"" + String(cfg.mainlight_m_to, DEC) + "\" value=\"" + String(cfg.mainlight_m_to, DEC) + "\" type=\"number\">&nbsp;Утр.&nbsp;работа";
  reslt += "</br>От&nbsp;<input name=\"mp3\" size=\"3\" placeholder=\"" + String(cfg.mainlight_e_from, DEC) + "\" value=\"" + String(cfg.mainlight_e_from, DEC) + "\" type=\"number\">&nbsp;до&nbsp;<input name=\"mp4\" size=\"3\" placeholder=\"" + String(cfg.mainlight_e_to, DEC) + "\" value=\"" + String(cfg.mainlight_e_to, DEC) + "\" type=\"number\">&nbsp;Веч.&nbsp;работа";
  reslt += "</br></br>Установленное время:</br><input name=\"hh1\" size=\"3\" placeholder=\"" + String(now.hour(), DEC) + "\" value=\"" + String(now.hour(), DEC) + "\" type=\"number\">&nbsp;:&nbsp;<input name=\"mm1\" size=\"3\" placeholder=\"" + String(now.minute(), DEC) + "\" value=\"" + String(now.minute(), DEC) + "\" type=\"number\">&nbsp;:" + String(now.second(), DEC) + "&nbsp;";

  reslt += "</br></br><input type=SUBMIT value=\"Сохранить\"></form></body></html>";
  //return reslt;
}



int getYear() {
  time_t rawtime = timeClient.getEpochTime();
  struct tm * ti;
  ti = localtime (&rawtime);
  int year = ti->tm_year + 1900;

  return year;
}


int getMonth() {
  time_t rawtime = timeClient.getEpochTime();
  struct tm * ti;
  ti = localtime (&rawtime);
  int month = (ti->tm_mon + 1) < 10 ? 0 + (ti->tm_mon + 1) : (ti->tm_mon + 1);

  return month;
}

int getDays() {
  time_t rawtime = timeClient.getEpochTime();
  struct tm * ti;
  ti = localtime (&rawtime);
  int month = (ti->tm_mday) < 10 ? 0 + (ti->tm_mday) : (ti->tm_mday);

  return month;
}


void failBlink() {
  byte mig = 0;
  for (mig = 0; mig < 3; mig++) {
    digitalWrite(REL1, HIGH);
    myDelay(100);
    digitalWrite(REL1, LOW);
    myDelay(100);
  }
  myDelay(200);
}


void goodBlink() {
  byte mig = 0;
  for (mig = 0; mig < 2; mig++) {
    digitalWrite(REL1, HIGH);
    myDelay(200);
    digitalWrite(REL1, LOW);
    myDelay(200);
  }
  myDelay(200);
}



void checkUpdate() {
  if (cfg.update) {   // было ОТА обновление
    if (cfg.version != GL_VERSION) {
      cfg.version = GL_VERSION;
      DEBUG("Update to");
      DEBUGLN(GL_VERSION);
    } else {
      DEBUGLN("Update to current");
    }
    cfg.update = 0;
    EE_updCfg();
  } else {
    if (cfg.version != GL_VERSION) {
      cfg.version = GL_VERSION;
      DEBUG("Update to");
      DEBUGLN(GL_VERSION);
    }
  }
}



void EE_startup() {
  // старт епром
  EEPROM.begin(1000);   // старт епром
  myDelay(500);
  if (EEPROM.read(0) != EE_KEY) {
    EEPROM.write(0, EE_KEY);
    EEPROM.put(1, cfg);
    EEPROM.commit();
    DEBUGLN("First start");
  }
  EEPROM.get(1, cfg);
  myDelay(500);


  DEBUG("EEPR size: ");
  DEBUGLN(sizeof(cfg) + 1);
}

void EE_updateCfg() {
  EEcfgFlag = true;
}

void checkEEupdate() {
  if (EEcfgFlag) {
    EEcfgFlag = false;
    EEPROM.put(1, cfg);
    DEBUGLN("save cfg");
  }
  EEPROM.commit();

}

void EE_updCfgRst() {
  EE_updCfg();
  myDelay(500);
  ESP.restart();
}
void EE_updCfg() {
  EEPROM.put(1, cfg);
  EEPROM.commit();
  myDelay(500);
}


void handleRoot() {
  make_header();

  server.send( 200, "text/html", /*str*/ reslt );
  DEBUGLN("Send root html");
}

void handleOk() {
  bool updtd = false;
  String ssid_ap = "";
  String pass_ap = "";
  String lip_ap = "";
  String dhr_ap = "";
  byte nh = 0, nm = 0;
  unsigned char* buf = new unsigned char[64];

  String str = "";
  str += html_header;
  str += "<body>";

  ssid_ap = server.arg(0);
  pass_ap = server.arg(1);
  lip_ap = server.arg(2);
  dhr_ap = server.arg(3);

  if (ssid_ap != "") {

    ssid_ap.toCharArray(cfg.ssid, ssid_ap.length() + 1);
    pass_ap.toCharArray(cfg.pass, pass_ap.length() + 1);
    cfg.doplight_from = dhr_ap.toInt();
    if (cfg.doplight_from > 24) {
      cfg.doplight_from = 22;
    }
    if (cfg.doplight_from < 19) {
      cfg.doplight_from = 19;
    }

    dhr_ap = server.arg(4);
    cfg.mainlight_m_from = dhr_ap.toInt();
    if (cfg.mainlight_m_from > cfg.mainlight_m_to) {
      cfg.mainlight_m_from = cfg.mainlight_m_to;
    }
    if (cfg.mainlight_m_from < 0) {
      cfg.mainlight_m_from = 0;
    }

    dhr_ap = server.arg(5);
    cfg.mainlight_m_to = dhr_ap.toInt();
    if (cfg.mainlight_m_to > 24) {
      cfg.mainlight_m_to = 24;
    }
    if (cfg.mainlight_m_to < cfg.mainlight_m_from) {
      cfg.mainlight_m_to = cfg.mainlight_m_from;
    }

    dhr_ap = server.arg(6);
    cfg.mainlight_e_from = dhr_ap.toInt();
    if (cfg.mainlight_e_from > cfg.mainlight_e_to) {
      cfg.mainlight_e_from = cfg.mainlight_e_to;
    }
    if (cfg.mainlight_e_from < 0) {
      cfg.mainlight_e_from = 0;
    }

    dhr_ap = server.arg(7);
    cfg.mainlight_e_to = dhr_ap.toInt();
    if (cfg.mainlight_e_to > 24) {
      cfg.mainlight_e_to = 24;
    }
    if (cfg.mainlight_e_to < cfg.mainlight_e_from) {
      cfg.mainlight_e_to = cfg.mainlight_e_from;
    }

    now = RTC.now();

    dhr_ap = server.arg(8);
    nh = dhr_ap.toInt();

    dhr_ap = server.arg(9);
    nm = dhr_ap.toInt();

    if (nh > 23) {
      nh = 23;
    }
    if (nm > 59) {
      nm = 59;
    }

    RTC.adjust(DateTime(now.year(), now.month(), now.day(), nh, nm, 0));

    if ((cfg.localIp.fromString(lip_ap)) && (ssid_ap.length() > 0)) { // try to parse into the IPAddress
      updtd = true;
      cfg.WiFimode = 1;
      str += "Configuration saved in FLASH</br>Changes applied after reboot</p></br></br>";
    } else {
      str += "UnParsable IP</br>";
    }
  }
  else {
    str += "No WIFI Net</br>";
  }
  str += "</body></html>";
  server.send ( 200, "text/html", str );
  if (updtd) {
    EE_updCfgRst();
  }
}

void handleSentVar() {
  if (server.hasArg("flash")) { // this is the variable sent from the client
    int readingInt = server.arg("flash").toInt();
    char readingToPrint[5];
    itoa(readingInt, readingToPrint, 10); //integer to string conversion for OLED library
    String ss = "";
    if (readingInt == 0) {
      //server.send(200, "text/html", "0\r\n");
      ss = "0";
      if (mode != MODE_FORCE) {
        force_remote = true;
        mainflashed = false;
        digitalWrite(REL1, LOW);
        digitalWrite(REL2, LOW);
      }
    }
    if (readingInt == 1) {
      //server.send(200, "text/html", "1\r\n");
      ss = "1";
      if (mode != MODE_FORCE) {
        force_remote = true;
        mainflashed = true;
        digitalWrite(REL1, LOW);
        digitalWrite(REL2, HIGH);
      }

    }

    String s = ss + "\r\n";
    server.send(200, "text/html", s);    //Reply to the client

    if (readingInt == 1) {
      //sendi2c(1);
      //opened = true;
    } else {
      //sendi2c(2);
      //opened = false;
    }

    //server.send(200, "text/html", "Boil start");
  }
  if (server.hasArg("mode")) {
    String mm = "0";
    if (mode == MODE_FORCE) {
      mm = "1";
    }
    if (mainflashed) mm = "2";
    if (mode == MODE_AUTOMATE) {
      mm = "3";
      if (amode == AMODE_MAINLIGHT) {
        mm = "4";
      }
      if (amode == AMODE_DOPLIGHT) {
        mm = "5";
      }
      if (mainflashed) mm = "6";
    }
    String s = mm + "\r\n";
    server.send(200, "text/html", s);    //Reply to the client
  }

  if (server.hasArg("vl")) {
    String s = String(analogRead(BUTTON_PIN)) + "\r\n";
    server.send(200, "text/html", s);    //Reply to the client
  }

}


boolean isIp(String str) {
  for (size_t i = 0; i < str.length(); i++) {
    int c = str.charAt(i);
    if (c != '.' && (c < '0' || c > '9')) {
      return false;
    }
  }
  return true;
}

String toStringIp(IPAddress ip) {
  String res = "";
  for (int i = 0; i < 3; i++) {
    res += String((ip >> (8 * i)) & 0xFF) + ".";
  }
  res += String(((ip >> 8 * 3)) & 0xFF);
  return res;
}


boolean captivePortal() {
  if (!isIp(server.hostHeader())) {
    Serial.println("Request redirected to captive portal");
    server.sendHeader("Location", String("http://") + toStringIp(server.client().localIP()), true);
    server.send(302, "text/plain", "");
    server.client().stop();
    return true;
  }
  return false;
}




void setupAP() {
  //WiFi.softAPdisconnect();
  WiFi.disconnect();
  //WiFi.forceSleepWake();
  WiFi.mode(WIFI_AP);
  myDelay(100);

  IPAddress apIP(192, 168, 1, 2);
  IPAddress apGW(192, 168, 1, 1);
  WiFi.softAPConfig(apIP, apGW, IPAddress(255, 255, 255, 0));
  WiFi.softAP(ssid, password);

  /* Setup the DNS server redirecting all the domains to the apIP */
  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(DNS_PORT, "*", apIP);

  server.on("/", handleRoot);
  server.on("/ok", handleOk);
  server.on("/generate_204", handleRoot);

  server.onNotFound([]() {
    make_header();
    server.send(200, "text/html", reslt );
  });
  server.begin();

  //MDNS.addService("http", "tcp", 80);

  myIP = WiFi.softAPIP();

  DEBUGLN("Setting AP Mode");
  DEBUG("AP IP: ");
  DEBUGLN(myIP);

  wifi_ap = true;
}

void setupLocal() {
  if (cfg.ssid[0] == NULL && cfg.pass[0] == NULL) {
    DEBUGLN("WiFi not configured");
    setupAP();
    failBlink();
    return;
  } else {
    DEBUGLN("Connecting to AP...");

    IPAddress gateway(cfg.localIp[0], cfg.localIp[1], cfg.localIp[2], 1);
    IPAddress subnet(255, 255, 255, 0);
    bool connect = false;

    byte ppr = 0;

    while (ppr < 1) {
      if (!connect) {
        WiFi.disconnect();
        myDelay(10);
        //delay(10);


        WiFi.setPhyMode(WIFI_PHY_MODE_11G);
        WiFi.setOutputPower(8);
        //WiFi.setOutputPower(19.25);

        WiFi.begin(cfg.ssid, cfg.pass);

        if (WiFi.config(cfg.localIp, gateway, subnet) == false) {
          DEBUGLN("Configuration failed.");
          failBlink();
          setupAP();
          return;
        } else {
          DEBUGLN("");
          DEBUGLN("Configuration ok.");
          ppr = 1;
        }

        byte ppp = 0;
        while (ppp < CONNECT_STEPS) {
          //while (WiFi.status() != WL_CONNECTED) {
          if (WiFi.status() != WL_CONNECTED) {
            myDelay(500);
            //          delay(100);
            DEBUG(".");
            yield();
            ppp++;
          } else {
            ppp = CONNECT_STEPS;
            ppr = 1;
            connect = true;
          }
        }

      } else {
        ppr = 1;
      }
    }

    if (!connect) {
      DEBUGLN("");
      DEBUGLN("Not connected, start AP");
      failBlink();
      //WiFi.disconnect();
      setupAP();
      return;
    } else {

      myIP = WiFi.localIP();

      WiFi.setAutoReconnect(true);
      WiFi.persistent(true);

      yield();
      server.on("/data/", HTTP_GET, handleSentVar);
      server.begin();
      yield();

      MDNS.addService("http", "tcp", 80);

      wifi_ap = false;
      yield();
      //goodBlink();
      //yield();

      DEBUGLN("");
      DEBUGLN("Connected");

      DEBUG("Local IP: ");
      DEBUGLN(WiFi.localIP());
      DEBUG("Subnet Mask: ");
      DEBUGLN(WiFi.subnetMask());
      DEBUG("Gateway IP: ");
      DEBUGLN(WiFi.gatewayIP());

    }
  }
}

void startWiFi() {
  if (!cfg.WiFimode) setupAP();   // режим точки доступа
  else setupLocal();              // подключаемся к точке

  //restartUDP();
}




//Основные настройки
void setup() {
  // put your setup code here, to run once:
  //ESP.wdtEnable();
  //wdt_disable();
#ifdef DEBUG_SERIAL
  Serial.begin(115200);

  myDelay(200);
  yield();

  Serial.println("Init...");
#endif

  char GLkey[] = GL_KEY;
  portNum = 17;
  for (byte i = 0; i < strlen(GLkey); i++) portNum *= GLkey[i];
  portNum %= 15000;
  portNum += 50000;

  EE_startup();         // читаем епром

  checkUpdate();        // проверка было ли обновление конфига
  //checkGroup();

  //Устанавливаем пины реле
  pinMode(REL1, OUTPUT);
  pinMode(REL2, OUTPUT);
  digitalWrite(1, LOW);

  RTC.begin();

  if (RTC.lostPower()) {

    DEBUGLN("RTC lost power, let's set the time!");

    // When time needs to be set on a new device, or after a power loss, the
    // following line sets the RTC to the date & time this sketch was compiled
    RTC.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }

  myDelay(100);
  now = RTC.now();

  startWiFi();

  myDelay(200);
  yield();

  if (!wifi_ap) {
    timeClient.begin(123);
    if (timeClient.update()) {

      RTC.adjust(DateTime(getYear(), getMonth(), getDays(), timeClient.getHours(), timeClient.getMinutes(), timeClient.getSeconds()));

    }
    myDelay(100);
  }

  now = RTC.now();

#ifdef DEBUG_SERIAL
  Serial.print("Time: ");
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.print(now.second(), DEC);
  Serial.println();
#endif



}




void loop() {
  // put your main code here, to run repeatedly:
  currentMillis = millis();

  yield();

  server.handleClient();

  if (!wifi_ap) {

    if (currentMillis - previousMillis2 >= interval2) {
      previousMillis2 = currentMillis;
      //if (WiFi.status() != WL_CONNECTED) wifi_connect();
      now = RTC.now();

    }

    aa = analogRead(BUTTON_PIN);

    if (aa <= 250) {
      mode = MODE_ALLOFF;
      //force_remote = false;
    }
    if ((aa >= 251) && (aa < 999)) {
      mode = MODE_AUTOMATE;
    }
    if (aa > 1000) {
      mode = MODE_FORCE;
    }


    byte hr = now.hour();
    if (hr > 24) {
      hr -= 24;
    }

    if (mode == MODE_AUTOMATE) {

      if (force_remote == false) {
        nmode = AMODE_ALLOFF;
        if ((hr > cfg.mainlight_e_from) && (hr < cfg.mainlight_e_to)) {
          nmode = AMODE_MAINLIGHT;
        }
        if ((hr >= cfg.mainlight_m_from) && (hr < cfg.mainlight_m_to)) {
          nmode = AMODE_MAINLIGHT;
        }
        //if ((hr >= 23) || (hr < 1)) {
        if  (hr >= cfg.doplight_from) {
          nmode = AMODE_DOPLIGHT;
        }
        if (nmode != amode) amode = nmode;
      }

      if (amode != old_amode) {
        switch (amode) {
          case AMODE_ALLOFF:
            digitalWrite(REL1, LOW);
            digitalWrite(REL2, LOW);
            break;
          case AMODE_DOPLIGHT:
            digitalWrite(REL1, HIGH);
            digitalWrite(REL2, LOW);
            digitalWrite(REL1, HIGH);
            digitalWrite(REL2, LOW);
            break;
          case AMODE_MAINLIGHT:
            digitalWrite(REL1, LOW);
            digitalWrite(REL2, HIGH);
            break;
        }
        old_amode = amode;
        oldmode = amode;
      }

    } else {
      if (mode != oldmode) {
        switch (mode) {
          case MODE_ALLOFF:
            digitalWrite(REL1, LOW);
            digitalWrite(REL2, LOW);
            force_remote = false;
            mainflashed = false;
            break;
          /*      case MODE_AUTOMATE:
                  digitalWrite(REL1, HIGH);
                  digitalWrite(REL2, LOW);
                  break;*/
          case MODE_FORCE:
            digitalWrite(REL1, LOW);
            digitalWrite(REL2, HIGH);
            force_remote = false;
            mainflashed = false;
            break;
        }
        oldmode = mode;
        old_amode = mode;
      }
    }


    if (!hour_display) {
      oldhour = hr;
      byte mig = 0;
      for (int i = 0; i < 1; i++) {
        for (mig = 0; mig < hr; mig++) {
          digitalWrite(REL1, HIGH);
          myDelay(200);
          digitalWrite(REL1, LOW);
          myDelay(200);
        }
        myDelay(2000);
      }
      hour_display = true;
    }



    //MDNS.update();

    /*    if (currentMillis - previousMillis >= interval) {
          previousMillis = currentMillis;
          if (WiFi.status() != WL_CONNECTED) startWiFi(); //wifi_connect();
        }
    */

    myDelay(10);


  } else {
    // In AP mode
    dnsServer.processNextRequest();

  }


}
