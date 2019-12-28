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
  color = (color + 1) & 7;

  uint32_t const pixelColor =
      (((color & 1) != 0) * 0x4f0000) |
      (((color & 2) != 0) * 0x004f00) |
      (((color & 4) != 0) * 0x00004f) |
      0;

  trellis.setPixelColor(evt.bit.NUM, pixelColor); //on rising
  trellis.show();

  return nullptr;
}

void handlerRoot()
{
  String buffer;

  buffer += "<!DOCTYPE html><html><head><meta http-equiv='Content-Type' content='text/html; charset=utf-8'></head><body><table border='1'>";
  for (uint8_t y = 0; y < 8; ++y)
  {
    buffer += "<tr>";
    for (uint8_t x = 0; x < 8; ++x)
    {
      auto const i((y << 3) | x);
      auto const c(colors[i]);

      buffer += "<td style='background-color: #";
      buffer += (c & 1) ? "ff" : "00";
      buffer += (c & 2) ? "ff" : "00";
      buffer += (c & 4) ? "ff" : "00";
      buffer += ";'>";
      buffer += i;
      buffer += " ";
      buffer += static_cast<int>(c);
      buffer += "</td>";
    }
    buffer += "</tr>";
  }
  buffer += "</table></body></html>";

  //buffer << analogRead(ANALOG_PIN);

  server.send(200, "text/html", buffer.c_str(), buffer.length());
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

  server.on("/", HTTP_GET, handlerRoot);
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
