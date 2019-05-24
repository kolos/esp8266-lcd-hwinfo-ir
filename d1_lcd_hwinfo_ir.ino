#include <LiquidCrystal.h>
#include <ESP8266WiFi.h>
#include <ESP8266NetBIOS.h>
#include <ESPAsyncTCP.h>
#include <asyncHTTPrequest.h>
#include <WiFiUdp.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <Ticker.h>
#include <time.h>
#include <simpleDSTadjust.h>
#include <Dusk2Dawn.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include "constants.h"

const int RS = D2, EN = D3, d4 = D5, d5 = D6, d6 = D7, d7 = D8;   
LiquidCrystal lcd(RS, EN, d4, d5, d6, d7);

WiFiUDP udp;
ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;
const uint16_t kIrLed = D1;
IRsend irsend(kIrLed);

struct dstRule StartRule = {"CEST", Last, Sun, Mar, 2, 3600}; // Central European Summer Time = UTC/GMT +2 hours
struct dstRule EndRule = {"CET", Last, Sun, Oct, 2, 0}; // Central European Time = UTC/GMT +1 hour

Ticker ticker1;
Ticker ticker2;
int32_t tick;
bool readyForNtpUpdate = false;
simpleDSTadjust dstAdjusted(StartRule, EndRule);
Dusk2Dawn dusk2dawn(LAT, LON, 1);

asyncHTTPrequest httpRequest;

bool shouldDisplayTimeScreen = true;

int _weather_id;
int _weather_temp;
int _weather_wind_speed;
int _weather_wind_dir;
String _weather_phrase;
bool _weather_has_precipitation = false;
int _weather_humidity;
int _weather_dewpoint;
String _weather_barometertrend;
tm _weather_rain_time;
float _weather_amount_of_rain;
byte _next_rain_idx;

/** custom chars **/
byte degreeCelciusChar[] = { 0x8, 0x14, 0x8, 0x3, 0x4, 0x4, 0x3, 0x0 };

byte odoubleacuteChar[] = {  0x05,  0x0A,  0x00,  0x0E,  0x11,  0x11,  0x0E,  0x00 };
byte udoubleacuteChar[] = {  0x05,  0x0A,  0x00,  0x11,  0x11,  0x13,  0x0D,  0x00 };
byte aacuteChar[] = {  0x02,  0x04,  0x0E,  0x01,  0x0F,  0x11,  0x0F,  0x00 };
byte iacuteChar[] = {  0x02,  0x04,  0x00,  0x0C,  0x04,  0x04,  0x0E,  0x00 };
byte uacuteChar[] = {  0x02,  0x04,  0x00,  0x11,  0x11,  0x13,  0x0D,  0x00 };
byte oacuteChar[] = {  0x02,  0x04,  0x00,  0x0E,  0x11,  0x11,  0x0E,  0x00 };
byte eacuteChar[] = {  0x02,  0x04,  0x0E,  0x11,  0x1F,  0x10,  0x0E,  0x00 };

String uuml = String("\xf5");
String ouml = String("\xef");
String odoubleacute = String("\x01");
String udoubleacute = String("\x02");
String aacute = String("\x03");
String iacute = String("\x04");
String uacute = String("\x05");
String oacute = String("\x06");
String eacute = String("\x07");

int weather_age = WEATHER_UPDATE_SECS - 2;
byte screen = 0;

void setup() {
  Serial.begin(SERIAL_BAUD_RATE);

  Serial.print("Connecting to WiFi");

  WiFi.disconnect();
  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_STA);
  WiFi.hostname(DHCP_CLIENTNAME);
  
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  Serial.print(" connected: ");
  Serial.println(WiFi.localIP());


  httpUpdater.setup(&httpServer);
  httpServer.begin();

  NBNS.begin(DHCP_CLIENTNAME);
  lcd.begin(16, 2);
  lcd.createChar(0, degreeCelciusChar);
  lcd.createChar(1, odoubleacuteChar);
  lcd.createChar(2, udoubleacuteChar);
  lcd.createChar(3, aacuteChar);
  lcd.createChar(4, iacuteChar);
  lcd.createChar(5, uacuteChar);
  lcd.createChar(6, oacuteChar);
  lcd.createChar(7, eacuteChar);

  udp.begin(LISTEN_PORT);
  irsend.begin();

  updateNTP();

  tick = NTP_UPDATE_INTERVAL_SEC;
  ticker1.attach(1, secTicker);
  
  setupAsyncHWinfoClient();
  setupAsyncWeather();
}

void updateNTP() {
  
  configTime(timezone * 3600, 0, NTP_SERVERS);

  delay(500);
  while (!time(nullptr)) {
    Serial.print("#");
    delay(1000);
  }
}


void secTicker()
{
  tick--;
  if(tick<=0)
   {
    readyForNtpUpdate = true;
    tick= NTP_UPDATE_INTERVAL_SEC; // Re-arm
   }

   weather_age++;

  if(! shouldDisplayTimeScreen) return;

  if(weather_age > WEATHER_UPDATE_SECS) {
    weather_age = 0;
    requestWeather();
  }

  char *dstAbbrev;
  time_t t = dstAdjusted.time(&dstAbbrev);
  struct tm *timeinfo = localtime (&t);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.printf("%d", _weather_temp);
  lcd.write(byte(0));

  lcd.setCursor(11, 0);
  lcd.printf("%02d%c%02d", 
    timeinfo->tm_hour,
    timeinfo->tm_sec % 2 ? ':' : ' ',
    timeinfo->tm_min);

  lcd.setCursor(4, 0);
  lcd.print(_weather_phrase);

  lcd.setCursor(0, 1);
  if(_weather_has_precipitation) {
    int rain_in_minutes = round((mktime(&_weather_rain_time) - t) / 60);
    if(_next_rain_idx == 0) {
      lcd.print("M\x07g");
    } else {
      lcd.print("Es\x01");
    }
    lcd.printf(": %dp %.2fmm",
      rain_in_minutes,
      _weather_amount_of_rain);
  } else {
    if(tick % 3 == 0) screen++;
    screen = screen % 3;
    switch(screen) {
      case 0:
        lcd.printf("Sz\x07l: %dkm, %d\xdf",
          _weather_wind_speed,
          _weather_wind_dir);
        break;
      case 1:
        lcd.printf("P\x03ra: %d%%, %d",
          _weather_humidity,
          _weather_dewpoint);
        lcd.write(byte(0));
        break;
      case 2:
        lcd.print("L\x07gny.: ");
        lcd.print(_weather_barometertrend);
        break;
      default:
        break;
    }
  }
}

void loop() {
  if(readyForNtpUpdate)
  {
    readyForNtpUpdate = false;
    updateNTP();
  }
  
  handleWolPackets();
  httpServer.handleClient();
}
