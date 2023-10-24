#include "display_config.h"
#include "Wire.h"
#include "SHT2x.h"

SHT2x sht(&Wire);

lv_timer_t* timer = NULL;
lv_obj_t *label1;
lv_obj_t *tempLabel;
lv_obj_t *humidityLabel;


void setup() {

  Serial.begin(115200);
  init_display_driver();
  if (!sht.begin(GPIO_SDA, GPIO_SCL)) {
    Serial.println("Sensor Init failed");
  }
  uint8_t stat = sht.getStatus();
  Serial.print(stat, HEX);
  Serial.println();
  labels();
  timer = lv_timer_create(updateLabel, 2000, NULL);

}

void loop() {

  // put your main code here, to run repeatedly:
  lv_timer_handler();
  delay(5);

}

void labels() {

  lv_obj_t *label1 = lv_label_create(lv_scr_act());
  lv_label_set_text(label1, "SHT-20 Sensor");
  lv_obj_align(label1, LV_ALIGN_TOP_MID, 0, 10);
  lv_obj_set_style_text_font(label1, &lv_font_montserrat_24, 0);

  tempLabel = lv_label_create(lv_scr_act());
  lv_label_set_text(tempLabel, "Temperature:00.00°C");
  lv_obj_align_to(tempLabel, label1, LV_ALIGN_OUT_BOTTOM_MID, -10, 15);

  humidityLabel = lv_label_create(lv_scr_act());
  lv_label_set_text(humidityLabel, "Humidity:00.00%");
  lv_obj_align_to(humidityLabel, tempLabel, LV_ALIGN_OUT_BOTTOM_MID, -10, 10);

}

void updateLabel(lv_timer_t* timer)
{

  sht.read();
  float temperature = sht.getTemperature();
  float humidity = sht.getHumidity();

  char tempBuffer[30];
  char humidityBuffer[30];
  snprintf(tempBuffer, sizeof(tempBuffer), "Temperature: %.2f°C", temperature);
  snprintf(humidityBuffer, sizeof(humidityBuffer), "Humidity: %.2f%%", humidity);

  lv_label_set_text(tempLabel, tempBuffer);
  lv_label_set_text(humidityLabel, humidityBuffer);

}
