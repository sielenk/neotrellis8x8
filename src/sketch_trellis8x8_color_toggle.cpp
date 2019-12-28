#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <WiFiManager.h>
#include <FS.h>
#include <Adafruit_NeoTrellis.h>

#include <memory>

#define INT_PIN 14
#define ANALOG_PIN A0

Adafruit_NeoTrellis trelli[4] = {
    Adafruit_NeoTrellis{NEO_TRELLIS_ADDR},
    Adafruit_NeoTrellis{NEO_TRELLIS_ADDR + 1},
    Adafruit_NeoTrellis{NEO_TRELLIS_ADDR + 2},
    Adafruit_NeoTrellis{NEO_TRELLIS_ADDR + 3}};

auto trellis = Adafruit_MultiTrellis{trelli, 2, 2};

WiFiManager wifiManager;
ESP8266WebServer server;

void setAll(uint32_t color)
{
  for (uint8_t i = 0; i < 64; i++)
  {
    trellis.setPixelColor(i, color);
    trellis.show();
    delay(5);
  }
}

uint8_t colors[64] = {};

TrellisCallback onKey(keyEvent evt)
{
  uint8_t &color = colors[evt.bit.NUM];
  ++color;

  uint32_t const pixelColor =
      (((color & 1) != 0) * 0x4f0000) |
      (((color & 2) != 0) * 0x004f00) |
      (((color & 4) != 0) * 0x00004f) |
      0;

  trellis.setPixelColor(evt.bit.NUM, pixelColor); //on rising
  trellis.show();

  return nullptr;
}

void handlerIcon()
{
  auto file = SPIFFS.open("/favicon.ico", "r");
  auto const size = file.size();
  auto const buffer = std::unique_ptr<char>{new char[size]};

  Serial.printf("size = %d\r\n", size);

  file.readBytes(buffer.get(), size);
  server.send(200, "image/x-icon", buffer.get(), size);

  file.close();
}

void setup()
{
  Serial.begin(74880);
  Serial.println("setup - start");

  wifiManager.autoConnect();

  pinMode(INT_PIN, INPUT);

  Serial.printf("trellis: %s\r\n", trellis.begin() ? "succeeded" : "failed");
  Serial.printf("SPIFFS: %s\r\n", SPIFFS.begin() ? "succeeded" : "failed");

  setAll(0x00004f);
  setAll(0x004f00);
  setAll(0x4f0000);
  setAll(0x000000);

  for (uint8_t i = 0; i < 64; ++i)
  {
    trellis.activateKey(i, SEESAW_KEYPAD_EDGE_RISING);
    trellis.registerCallback(i, &onKey);
  }

  server.on("/favicon.ico", HTTP_GET, handlerIcon);
  server.begin();
  Serial.println("HTTP server started");

  Serial.println("setup - end");
}

unsigned long secondsOld;

void loop()
{
  unsigned long const secondsNew = millis() / 1000;

  if (secondsOld != secondsNew)
  {
    auto const val = analogRead(ANALOG_PIN);
    Serial.printf("%lu\t%d\r\n", secondsNew, val);

    secondsOld = secondsNew;
  }

  if (!digitalRead(INT_PIN))
  {
    trellis.read();
  }

  server.handleClient();

  delay(2);
}
