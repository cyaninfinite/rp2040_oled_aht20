/*
  ===========================================================
  RP2040 0.42" OLED with AHT20
  Version 0.1.2
  By: Cyaninfinite (https://github.com/cyaninfinite)
  ===========================================================

  > Display current temperature and relative humidity on 0.42" OLED display on microcontroller.

*/

#include <Arduino.h>
#include <U8g2lib.h>
#include <Adafruit_NeoPixel.h>
#include <DFRobot_AHT20.h>
#include "config.h"

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

U8G2_SSD1306_72X40_ER_F_HW_I2C u8g2(U8G2_R0, /* reset=*/U8X8_PIN_NONE);  // EastRising 0.42" OLED
Adafruit_NeoPixel rgb_led(LED_COUNT, LED_PIN, NEO_RGB + NEO_KHZ800);
DFRobot_AHT20 aht20(Wire1);

unsigned long mil_prev{ 0 };
float temp_cur{ 0.0 };
float humidity_cur{ 0.0 };
bool led_toggle{ 0 };

void startWire(TwoWire &tw, const int *i2c_pins) {
  tw.setSDA(i2c_pins[0]);
  tw.setSCL(i2c_pins[1]);
  tw.begin();
  if (DEBUG) Serial.println("\nWire init");
}

//(int)temp_cur - start_cold > 0 ? temp_cur - start_cold : 0, (int)start_hot - temp_cur > 0 ? start_hot - temp_cur : 0
int calIntensity(const bool &isHot, const float &currentT, const int &thresholdT, const int &led_limit) {
  auto f_val{ (isHot ? -1 : 1) * currentT - thresholdT };
  f_val = f_val > 0 ? f_val : 0;
  return static_cast<int>(constrain(f_val, 0, led_limit));
}

void setup() {
  if (DEBUG) {
    Serial.begin(9600);
    while (!Serial)
      ;  // Wait for Serial Monitor
    Serial.println("\nSerial init");
  }
  rgb_led.begin();  // RGB init

  // OLED Display (I2C1)
  startWire(Wire, I2C1_pins);
  u8g2.begin();
  u8g2.setFont(u8g2_font_ncenB10_tr);

  //I2C for AHT20 (I2C0)
  startWire(Wire1, I2C1_pins);
  uint8_t status;
  boolean sen_err{ false };
  while ((status = aht20.begin()) != 0) {
    if (!sen_err) {
      u8g2.clearBuffer();
      u8g2.drawStr(0, 14, "Sensor");
      char tmp[10];
      sprintf(tmp, "Error: %d", status);
      u8g2.drawStr(0, 28, tmp);
      u8g2.sendBuffer();
      sen_err = 1;
      if (DEBUG) Serial.println(tmp);
    }
  }
}

void loop() {
  rgb_led.clear();
  unsigned long mil_cur = millis();
  if (mil_cur - mil_prev >= interval) {
    mil_prev = mil_cur;

    if (aht20.startMeasurementReady(/* crcEn = */ true)) {
      temp_cur = aht20.getTemperature_C();
      humidity_cur = aht20.getHumidity_RH();

      char tmp_c[14];
      char tmp_h[14];

      sprintf(tmp_c, "T: %0.2f C", temp_cur);
      sprintf(tmp_h, "H: %0.2f %%", humidity_cur);

      if (DEBUG) {
        Serial.print(tmp_c);
        Serial.print("\t");
        Serial.println(tmp_h);
      }

      //Somehow the LED is connected in gbr manner
      //Enable green LED to indicate debugging, and when data is updated
      led_toggle = 1 - led_toggle;
      rgb_led.setPixelColor(0, rgb_led.Color(DEBUG ? led_toggle * LED_MAX_BRIGHTNESS / 10 : 0,
                                             calIntensity(0, temp_cur, start_cold, LED_MAX_BRIGHTNESS),
                                             calIntensity(1, temp_cur, start_hot, LED_MAX_BRIGHTNESS)));
      rgb_led.show();

      u8g2.clearBuffer();
      u8g2.drawStr(0, 14, tmp_c);
      u8g2.drawStr(0, 32, tmp_h);
      u8g2.sendBuffer();
    }
  }
}