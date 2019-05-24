#define SERIAL_BAUD_RATE 115200

#define DHCP_CLIENTNAME         "ESP-LCD"
#define WIFI_SSID               ""
#define WIFI_PASSWORD           ""

#define SERVER_HOST_NAME        "192.168.1.2"
#define TCP_PORT                27007

#define LISTEN_PORT 9
#define WOL_INIT {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}

#define WEATHER_UPDATE_SECS 60 * 10
#define WEATHER_API_URL "http://api.weather.com/v2/turbo/vt1precipitation;vt1observation?apiKey={API_KEY}&format=json&geocode={LAT}%2C{LON}&language={LANG}&units={UNIT}"

#define LAT 12.34
#define LON 56.78

#define NTP_UPDATE_INTERVAL_SEC 5*3600
#define NTP_SERVERS "pool.ntp.org"
#define UTC_OFFSET +1
#define timezone +1

#define WOL_PC_ON {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
#define TV_ON_CODE 0xE0E040BF
