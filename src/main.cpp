#include <memory>

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <WiFiManager.h>
#include <FS.h>
#include <Adafruit_NeoTrellis.h>
#include <Schedule.h>

#include "Game.hpp"

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

Game game;

void showGame()
{
  for (uint8_t x = 0; x < 8; ++x)
  {
    for (uint8_t y = 0; y < 8; ++y)
    {
      auto const pixelColor = game.cellColor(x, y);

      trellis.setPixelColor(x, y, pixelColor);
    }
  }

  trellis.show();
}

TrellisCallback onKey(keyEvent evt)
{
  bool isRising;
  switch (evt.bit.EDGE)
  {
  case SEESAW_KEYPAD_EDGE_FALLING:
    isRising = false;
    break;
  case SEESAW_KEYPAD_EDGE_RISING:
    isRising = false;
    break;
  default:
    return nullptr;
  }

  if (game.onKey(evt.bit.NUM & 7, (evt.bit.NUM >> 3) & 7, isRising))
  {
    showGame();
  }

  return nullptr;
}

double getBatteryVoltage()
{
  // adjusted according to measured values on my device
  return (analogRead(ANALOG_PIN) * 3.87) / 834.0;
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
      buffer += "<td style='width:20px; height:20px; background-color: ";
      buffer += game.cellColorHtml(x, y);
      buffer += ";'/>";
    }
    buffer += "</tr>";
  }
  buffer += "</table><p>Battery voltage: ";
  buffer += getBatteryVoltage();
  buffer += "V</p></body></html>";

  server.send(200, "text/html", buffer.c_str(), buffer.length());
}

void handlerIcon()
{
  auto file = SPIFFS.open("/favicon.ico", "r");
  auto const size = file.size();
  auto const buffer = std::unique_ptr<char>{new char[size]};

  file.readBytes(buffer.get(), size);
  server.send(200, "image/x-icon", buffer.get(), size);

  file.close();
}

void setup()
{
  Serial.begin(74880);
  Serial.println("setup - start");

  pinMode(INT_PIN, INPUT);

  Serial.printf("trellis: %s\r\n", trellis.begin() ? "succeeded" : "failed");

  setAll(0x00004f);

  wifiManager.autoConnect();

  setAll(0x004f00);

  Serial.printf("SPIFFS: %s\r\n", SPIFFS.begin() ? "succeeded" : "failed");

  setAll(0x4f0000);

  server.on("/", HTTP_GET, handlerRoot);
  server.on("/favicon.ico", HTTP_GET, handlerIcon);
  server.begin();
  Serial.println("HTTP server started");

  setAll(0x000000);

  for (uint8_t i = 0; i < 64; ++i)
  {
    trellis.activateKey(i, SEESAW_KEYPAD_EDGE_RISING);
    trellis.registerCallback(i, &onKey);
  }

  Serial.println("setup - end");

  showGame();
}

unsigned long secondsOld;

void loop()
{
  if (digitalRead(INT_PIN) == 0)
  {
    trellis.read();
  }

  unsigned long const secondsNew = millis() / 1000;

  if (secondsOld != secondsNew)
  {
    auto const val = getBatteryVoltage();
    Serial.printf("%lu\t%05.3fV\r\n", secondsNew, val);

    secondsOld = secondsNew;
  }

  server.handleClient();

  delay(2);
}
