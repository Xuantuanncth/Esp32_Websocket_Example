// Import required libraries
#include <WiFi.h>
#include <time.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Adafruit_GFX.h>
#include <PxMatrix.h>
#include "websocket.h"
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSansOblique9pt7b.h>
#include <Fonts/FreeMono9pt7b.h>

#define P_LAT 17
#define P_A 19
#define P_B 23
#define P_C 18
#define P_D 5
#define P_E 15
#define P_OE 16
hw_timer_t *timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
#define matrix_width 32
#define matrix_height 16
uint8_t display_draw_time = 10; //30-70 is usually fine

PxMATRIX display(64, 32, P_LAT, P_OE, P_A, P_B, P_C, P_D);

uint16_t myRED = display.color565(255, 0, 0);
uint16_t myGREEN = display.color565(0, 255, 0);
uint16_t myBLUE = display.color565(0, 0, 255);
uint16_t myWHITE = display.color565(255, 255, 255);
uint16_t myYELLOW = display.color565(255, 255, 0);
uint16_t myCYAN = display.color565(0, 255, 255);
uint16_t myMAGENTA = display.color565(255, 0, 255);
uint16_t myBLACK = display.color565(0, 0, 0);

uint16_t myCOLORS[8] = {myRED, myGREEN, myBLUE, myWHITE, myYELLOW, myCYAN, myMAGENTA, myBLACK};

// Replace with your network credentials
const char *ssid = "Welcome";
const char *password = "123456789";

bool ledState = 0;
const int ledPin = 2;
/*
   scroll = 0: right->left
   scroll = 1: left->right
*/
unsigned char scroll = 0;
unsigned char speed_delay = 50;
char buffDisplay[100] = "Wellcome To Viet Nam";
unsigned int color_R = 90, color_G = 90, color_B = 90;
unsigned char eff1 = 0, eff2 = 0, eff3 = 0, yPos = 9;
/*---------------------------ADD Real Time Internet------------------------------*/
const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 25200;
const int daylightOffset_sec = 25200;
unsigned char hour_on = 6, hour_off = 23, min_on = 0, min_off = 0, hour_now = 0, min_now = 0;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

void notifyClients()
{
  ws.textAll(String(ledState));
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len)
{
  AwsFrameInfo *info = (AwsFrameInfo *)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT)
  {
    data[len] = 0;
    printf("[TDX] (%s %d) =======> Data: %s\r\n", __func__, __LINE__, (char *)data);
    switchData((char *)data);
    if (strcmp((char *)data, "toggle") == 0)
    {
      ledState = !ledState;
      //notifyClients();
    }
  }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len)
{
  switch (type)
  {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\r\n", client->id(), client->remoteIP().toString().c_str());
      client->printf("1,%s", buffDisplay);
      client->printf("2,%d", scroll);
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\r\n", client->id());
      break;
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}

void initWebSocket()
{
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

String processor(const String &var)
{
  Serial.println(var);
  if (var == "STATE")
  {
    if (ledState)
    {
      return "ON";
    }
    else
    {
      return "OFF";
    }
  }
}

void IRAM_ATTR display_updater()
{
  // Increment the counter and set the time of ISR
  portENTER_CRITICAL_ISR(&timerMux);
  display.display(display_draw_time);
  portEXIT_CRITICAL_ISR(&timerMux);
}

void display_update_enable(bool is_enable)
{
  if (is_enable)
  {
    timer = timerBegin(0, 80, true);
    timerAttachInterrupt(timer, &display_updater, true);
    timerAlarmWrite(timer, 4000, true);
    timerAlarmEnable(timer);
  }
  else
  {
    timerDetachInterrupt(timer);
    timerAlarmDisable(timer);
  }
}

void DisplayLedMatrix(void *pvParam);

void setup()
{
  Serial.begin(115200);

  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  //Connect to Wi-Fi
  Serial.println("Connecting to WiFi..");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  // Print ESP Local IP Address
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  display.begin(16);
  display.clearDisplay();
  display.setTextColor(myCYAN);
  display.setCursor(2, 0);
  display.print("IP Address");
  display_update_enable(true);
  display.setCursor(0, 10);
  display.print(WiFi.localIP());
  delay(10000);
  initWebSocket();

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/html", index_html, processor);
  });

  // Start server
  server.begin();
  xTaskCreate(
    DisplayLedMatrix, "DisplayLedMatrix", 8192 // Stack size
    ,
    NULL, 1 // Priority
    ,
    NULL);
}

void loop()
{
  ws.cleanupClients();
  printLocalTime();
  digitalWrite(ledPin, ledState);
}

void DisplayLedMatrix(void *pvParam)
{
  bool isplay = false;
  while (1)
  {
    if ((hour_now == hour_on) && (hour_now == hour_off))
    {
      if ((min_now > min_on) && (min_now < min_off))
      {
        isplay = true;
      } 
      else 
      {
        isplay = false;
      }
    }
    else if (hour_now == hour_on) {
      if ((min_now > min_on))
      {
        isplay = true;
      }
      else
      {
        isplay = false;
      }
    }
    else if(hour_now == hour_off){
      if(min_now < min_off)
      {
        isplay = true;
      }
      else 
      {
        isplay = false;
      }  
    }
    else if((hour_now > hour_on) && (hour_now < hour_off))
    {
      isplay = true;
    }
    else
    {
      isplay = false;
    }
    if (isplay == true)
    {
      scroll_text(yPos, speed_delay, buffDisplay, color_R, color_G, color_B);
      display.clearDisplay();
      printf("[TDX] (%s %d) ============> Is running\r\n", __func__, __LINE__);
      printf("(%s %d) =========> Hour_now: %d, Min_now: %d\r\n", __func__, __LINE__, hour_now, min_now);
    }
    else
    {
      printf("(%s %d) Is Off now %d:%d, on %d:%d, off %d:%d\r\n", __func__, __LINE__, hour_now, min_now, hour_on, min_on, hour_off, min_off);
      vTaskDelay(1000);
    }
  }
}
void scroll_text(uint8_t ypos, unsigned long scroll_delay, String text, uint8_t colorR, uint8_t colorG, uint8_t colorB)
{
  uint16_t text_length = text.length();
  display.setTextWrap(false); // we don't wrap text so it scrolls nicely
  display.setTextSize(1);
  display.setRotation(0);
  display.setTextColor(display.color565(colorR, colorG, colorB));
  if (eff1)
  {
    display.setFont(&FreeMono9pt7b);
  }
  else if (eff2)
  {
    display.setFont(&FreeSans9pt7b);
  }
  else if (eff3)
  {
    display.setFont(&FreeSansOblique9pt7b);
  }
  else
  {
    display.setFont();
  }
  // Asuming 5 pixel average character width
  if (scroll == 0)
  {
    for (int xpos = matrix_width; xpos > -(matrix_width + text_length * 6); xpos--)
    {
      display.setTextColor(display.color565(colorR, colorG, colorB));
      display.clearDisplay();
      display.setCursor(xpos, ypos);
      display.println(text);
      delay(scroll_delay);
      yield();

      // This might smooth the transition a bit if we go slow
      // display.setTextColor(display.color565(colorR/4,colorG/4,colorB/4));
      // display.setCursor(xpos-1,ypos);
      // display.println(text);

      delay(scroll_delay / 5);
      yield();
    }
  }
  else
  {
    for (int xpos = -((matrix_width + text_length * 6)); xpos < 32; xpos++)
    {
      display.setTextColor(display.color565(colorR, colorG, colorB));
      display.clearDisplay();
      display.setCursor(xpos, ypos);
      display.println(text);
      delay(scroll_delay);
      yield();

      // This might smooth the transition a bit if we go slow
      // display.setTextColor(display.color565(colorR/4,colorG/4,colorB/4));
      // display.setCursor(xpos-1,ypos);
      // display.println(text);

      delay(scroll_delay / 5);
      yield();
    }
  }
}

void switchData(char data[])
{
  unsigned char i = 0;
  if (data[0] == '1')
  {
    printf("(%s %d) ==========> 1", __func__, __LINE__);
    memset(buffDisplay, 0, 100);
    for (i = 0; i < strlen(data) - 2; i++)
    {
      buffDisplay[i] = data[i + 2];
    }
    printf("(%s %d) buffDisplay: %s\r\n", __func__, __LINE__, buffDisplay);
  }
  else if (data[0] == '2')
  {
    printf("(%s %d) ==========> 2", __func__, __LINE__);
    parserData(data);
  }
  else if (data[0] == '3')
  {
    printf("(%s %d) ==========> 3", __func__, __LINE__);
    eff1 = 1;
    eff2 = 0;
    eff3 = 0;
    yPos = 20;
  }
  else if (data[0] == '4')
  {
    printf("(%s %d) ==========> 4", __func__, __LINE__);
    eff1 = 0;
    eff2 = 1;
    eff3 = 0;
    yPos = 20;
  }
  else if (data[0] == '5')
  {
    printf("(%s %d) ==========> 5", __func__, __LINE__);
    eff1 = 0;
    eff2 = 0;
    eff3 = 1;
    yPos = 20;
  }
  else if (data[0] == '6')
  {
    hour_on = (data[2] - '0') * 10 + (data[3] - '0');
    min_on = (data[5] - '0') * 10 + (data[6] - '0');
    printf("(%s %d) ==========> 6: %d %d \r\n", __func__, __LINE__, hour_on, min_on);
  }
  else if (data[0] == '7')
  {
    hour_off = (data[2] - '0') * 10 + (data[3] - '0');
    min_off = (data[5] - '0') * 10 + (data[6] - '0');
    printf("(%s %d) ==========> 7: %d %d \r\n", __func__, __LINE__, hour_off, min_off);
  }
  else if (data[0] == '8')
  {
    scroll = (data[2] - '0');
    printf("(%s %d) [TDX] =========> scroll_text: %d\r\n", __func__, __LINE__, scroll);
  }
  else if (data[0] == '9')
  {
    speed_delay = (data[2] - '0') * 10 + (data[3] - '0');
    printf("(%s %d) [TDX] =========> Speed delay: %d\r\n", __func__, __LINE__, speed_delay);
  }
}
void parserData(char data[])
{
  char r[5], g[5], b[5], fist = 0, j = 0, i = 0;
  if (data[0] == '2')
  {
    for (i = 0; i < strlen(data); i++)
    {
      if (data[i] == ',')
      {
        fist++;
        j = 0;
      }
      if (fist == 1 && data[i] != ',')
      {
        printf("data[%d]: %c\r\n", i, data[i]);
        r[j++] = data[i];
      }
      if (fist == 2 && data[i] != ',')
      {
        printf("data[%d]: %c\r\n", i, data[i]);
        g[j++] = data[i];
      }
      if (fist == 3 && data[i] != ',')
      {
        printf("data[%d]: %c\r\n", i, data[i]);
        b[j++] = data[i];
      }
    }
  }
  color_R = atoi(r);
  color_G = atoi(g);
  color_B = atoi(b);
  printf("(%s %d) R: %d, G: %d, B: %d \r\n", __func__, __LINE__, color_R, color_G, color_B);
}

void printLocalTime()
{
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo))
  {
    //Serial.println("Failed to obtain time");
    return;
  }
  //Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
  //Serial.println("Time variables");
  char timeHour[3], timeMin[3];
  strftime(timeHour, 3, "%H", &timeinfo);
  //Serial.println(timeHour);
  strftime(timeMin, 3, "%M", &timeinfo);
  hour_now = atoi(timeHour);
  min_now = atoi(timeMin);
  //printf("(%s %d) =========> Hour: %d, Min: %d\r\n",__func__,__LINE__,hour_now,min_now);
  delay(300);
}
