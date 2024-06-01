#include <Arduino.h>

#define REG_LED_GPIO 33
#define WHITE_LED_GPIO 4
#define canalPWM 0

int led_intensity { 0 };

void setup()   {
  int pwm_frequence { 1000 };
  int pwm_resolution { 8 }; 

  pinMode(REG_LED_GPIO, OUTPUT); // GPIO 33 réglée en sortie
  ledcSetup(canalPWM, pwm_frequence, pwm_resolution);
  ledcAttachPin(WHITE_LED_GPIO, canalPWM);
}

void loop() {
  digitalWrite(REG_LED_GPIO, LOW);
  ledcWrite(canalPWM, 0);
  delay(1000);
  digitalWrite(REG_LED_GPIO, HIGH);
  ledcWrite(canalPWM, 3 + led_intensity * 63); // 3 - 66 - 129 - 192 - 255
  delay(1000);

  led_intensity = (led_intensity+1) % 5;
}