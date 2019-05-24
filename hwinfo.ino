char hello[] = { /* Packet 4 */
0x43, 0x52, 0x57, 0x48, 0x01, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

char request[] = { /* Packet 8 */
0x43, 0x52, 0x57, 0x48, 0x02, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

uint8_t hello_magic[] = {0x50, 0x52, 0x57, 0x48, 0x1};
uint8_t magic[] = {0x50, 0x52, 0x57, 0x48, 0x02};

static os_timer_t intervalTimer;

static int received_num = 0;



typedef union{
  long value;
  uint8_t bytes[8];
} _long;

typedef union{
  uint value;
  uint8_t bytes[4];
} _uint;

typedef union{
  double value;
  uint8_t bytes[8];
} _double;

typedef struct{
  uint group;
  uint id;
  double value;
} _hw_sensor;

static   _long polling_time;
static   _uint offset_of_readings;
static   _uint size_of_reading;
static   _uint number_of_readings;

static   _double doubleBuf;
static   _uint uintBuf;

static uint16_t idx = 0;
static bool magic_received = false;

static _hw_sensor reading;

static void parse(void *data, size_t len) {
  if(!magic_received) return;

  for(uint16_t i=0;i<len;i++) {
    uint8_t ch = *(uint8_t*)(data+i);

     if(idx >= 24 && idx < 32) {
       polling_time.bytes[idx-24] = ch; // reading time
     } else if(idx >= 44 && idx < 48 ) {
       offset_of_readings.bytes[idx-44] = ch; // readings offset
     } else if(idx >= 48 && idx < 52 ) {
       size_of_reading.bytes[idx-48] = ch; // reading element size
     } else if(idx >= 52 && idx < 56 ) {
       number_of_readings.bytes[idx-52] = ch; // num of reading elements
     }

     if(size_of_reading.value>0) {

     if(idx >= (12 + 4 + offset_of_readings.value)) {
       if(((idx - (12 + 4 + offset_of_readings.value)) % size_of_reading.value) == 0) {
         //Serial.print("[");
       }
       if(((idx - (12 + 4 + offset_of_readings.value)) % size_of_reading.value) < 4) {
         uintBuf.bytes[((idx - (12 + 4 + offset_of_readings.value)) % size_of_reading.value)] = ch;
       }
       if(((idx - (12 + 4 + offset_of_readings.value)) % size_of_reading.value) == 4) {
         //Serial.print(uintBuf.value);
         reading.group = uintBuf.value;
         //Serial.print(":");
       }
     }
     if(idx >= (12 + 8 + offset_of_readings.value)) {
       if(((idx - (12 + 8 + offset_of_readings.value)) % size_of_reading.value) < 4) {
         uintBuf.bytes[((idx - (12 + 8 + offset_of_readings.value)) % size_of_reading.value)] = ch;
       }
       if(((idx - (12 + 8 + offset_of_readings.value)) % size_of_reading.value) == 4) {
         //Serial.print(uintBuf.value);
         reading.id = uintBuf.value;
         //Serial.print("] ");
       }
     }
/*
     if(idx >= (12 + 12 + 128 + offset_of_readings.value)) {
       if(((idx - (12 + 12 + 128  + offset_of_readings.value)) % size_of_reading.value) < 128) {
         if(ch) Serial.print((char)ch);
       }
     }
 */
/*
     if(idx >= (12 + 12 + 128 + 128 + offset_of_readings.value)) {
       if(((idx - (12 + 12 + 128 + 128 + offset_of_readings.value)) % size_of_reading.value) == 0) {
         Serial.print(" (");
       }
       if(((idx - (12 + 12 + 128 + 128 + offset_of_readings.value)) % size_of_reading.value) < 16) {
         if(ch) Serial.print((char)ch);
       }
       if(((idx - (12 + 12 + 128 + 128 + offset_of_readings.value)) % size_of_reading.value) == 16) {
         Serial.print("): ");
       }
     }
*/
     if(idx >= (12 + 12 + 128 + 128 + 16 + offset_of_readings.value)) {
       if(((idx - (12 + 12 + 128 + 128 + 16 + offset_of_readings.value)) % size_of_reading.value) < 8) {
         doubleBuf.bytes[((idx - (12 + 12 + 128 + 128 + 16 + offset_of_readings.value)) % size_of_reading.value)] = ch;
       }
       if(((idx - (12 + 12 + 128 + 128 + 16 + offset_of_readings.value)) % size_of_reading.value) == 8) {
         //Serial.println(doubleBuf.value);

         reading.value = doubleBuf.value;

         onReadComplete();
       }
     }
     }

    idx++;
  }
}

static void requestPacket(void* arg) {
  AsyncClient* client = reinterpret_cast<AsyncClient*>(arg);
  client->add(request, sizeof request);
  client->send();
}

/* event callbacks */
static void handleData(void* arg, AsyncClient* client, void *data, size_t len) {
  if(memcmp(hello_magic, data, sizeof hello_magic) == 0) {
    Serial.println("hello received, sending request");
    lcd.clear();
    shouldDisplayTimeScreen = false;
    os_timer_arm(&intervalTimer, 2000, true); // schedule for reply to server at next 2s
  }else if(memcmp(magic, data, sizeof magic) == 0) {
    magic_received = true;
    idx=0;

    parse(data, len);
  } else {
    parse(data, len);
  }
}

void onError2(void* arg, AsyncClient* client) {
  client->close(true);
}

void onConnect(void* arg, AsyncClient* client) {
  Serial.printf("\n client has been connected to %s on port %d \n", SERVER_HOST_NAME, TCP_PORT);

  magic_received = false;
  client->add(hello, sizeof hello);
  client->send();
}
void onDisconnect(void* arg, AsyncClient* client) {
  Serial.println("disconnected");

  os_timer_disarm(&intervalTimer);

  shouldDisplayTimeScreen = true;

  client->close(true);
  client->connect(SERVER_HOST_NAME, TCP_PORT);
}

void onReadComplete() {

  if(reading.group == 3 && reading.id == 83886087) { /* CPU+SoC Power (SVI2 TFN) (W) */
    lcd.setCursor(12, 0);
    lcd.printf("%3d", (int)reading.value);
    lcd.print("W");
  } else if(reading.group == 3 && reading.id == 16777216) { /* CPU (Tctl/Tdie) (C) */
    lcd.setCursor(13, 1);
    lcd.printf("%2d", (int)reading.value);
    lcd.write(byte(0));
  } else if(reading.group == 10 && reading.id == 117440513) { /* GPU D3D Usage (%) */
    lcd.setCursor(0, 1);
    lcd.print("G");
    lcd.printf("%3d", (int)reading.value);
    lcd.print("%");
  } else if(reading.group == 1 && reading.id == 117440521) { /* Total CPU Usage (%) */
    lcd.setCursor(0, 0);
    lcd.print("C");
    lcd.printf("%3d", (int)reading.value);
    lcd.print("%");
  } else if(reading.group == 0 && reading.id == 134217731) { /* Physical Memory Used (MB) */
    lcd.setCursor(6, 0);
    lcd.printf("%4d", (int)reading.value);
    lcd.print("M");
  } else if(reading.group == 10 && reading.id == 134217730) { /* GPU D3D Memory Dynamic (MB) */
    lcd.setCursor(6, 1);
    lcd.printf("%4d", (int)reading.value);
    lcd.print("M");
  }
}

void setupAsyncHWinfoClient() {
  AsyncClient* client = new AsyncClient;
  client->onData(&handleData, client);
  client->onConnect(&onConnect, client);
  client->onDisconnect(&onDisconnect, client);
  client->close(true);
  client->connect(SERVER_HOST_NAME, TCP_PORT);
  
  os_timer_disarm(&intervalTimer);
  os_timer_setfn(&intervalTimer, &requestPacket, client);
}
