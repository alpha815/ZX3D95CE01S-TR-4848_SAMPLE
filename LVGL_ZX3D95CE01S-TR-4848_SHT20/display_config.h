#ifndef DISPLAY_CONFIG_H
#define DISPLAY_CONFIG_H

#include <lvgl.h>
#include <Wire.h>
#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include <FT6X36.h>

#define SPI_FREQUENCY 15000000
#define BACKLIGHT_FREQUENCY 1000
#define I2C_TOUCH_FREQUENCY 400000

#define TFT_WIDTH 480
#define TFT_HEIGHT 480

#define TFT_BCKL 5

#define TFT_CS 38
#define TFT_SCLK 45
#define TFT_MOSI 48

#define TFT_DE 40
#define TFT_VSYNC 41
#define TFT_HSYNC 42
#define TFT_PCLK 39

#define TFT_D0 45
#define TFT_D1 48
#define TFT_D2 47
#define TFT_D3 0
#define TFT_D4 21
#define TFT_D5 14
#define TFT_D6 13
#define TFT_D7 12
#define TFT_D8 11
#define TFT_D9 16
#define TFT_D10 17
#define TFT_D11 18
#define TFT_D12 8
#define TFT_D13 3
#define TFT_D14 46
#define TFT_D15 10

#define GPIO_SDA 15
#define GPIO_SCL 6
#define TOUCH_IRQ 4
#define TOUCH_RST -1

struct Point
{
  uint16_t x;
  uint16_t y;
};

Arduino_DataBus *bus;
Arduino_ESP32RGBPanel *panel;
Arduino_RGB_Display *gfx;
FT6X36 ts(&Wire, TOUCH_IRQ);

void init_display_driver();
void IRAM_ATTR flush_pixels(lv_disp_drv_t* disp, const lv_area_t* area, lv_color_t* color_p);
void IRAM_ATTR touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data);
void on_touch(TPoint p, TEvent e);

Point point;
static lv_disp_draw_buf_t draw_buf;
static lv_color_t *buf = (lv_color_t *)heap_caps_malloc(TFT_WIDTH * 50 * sizeof(lv_color_t), MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);

/* Update TFT */
void IRAM_ATTR flush_pixels(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);

#if (LV_COLOR_16_SWAP != 0)
  gfx->draw16bitBeRGBBitmap(area->x1, area->y1, (uint16_t *)&color_p->full, w, h);
#else
  gfx->draw16bitRGBBitmap(area->x1, area->y1, (uint16_t *)&color_p->full, w, h);
#endif

  lv_disp_flush_ready(disp);
}

/* Read the touchpad */
void IRAM_ATTR touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data)
{
  if (ts.touched())
  {
    ts.processTouch();

    data->point.x = point.x;
    data->point.y = point.y;
    data->state = LV_INDEV_STATE_PR;
  }
  else
  {
    data->state = LV_INDEV_STATE_REL;
  }
}

void on_touch(TPoint p, TEvent e)
{
  point.x = p.x;
  point.y = p.y;
}

void init_display_driver()
{
  bus = new Arduino_SWSPI(
    GFX_NOT_DEFINED, /* DC */
    TFT_CS,          /* CS */
    TFT_SCLK,        /* SCK or SCLK */
    TFT_MOSI,        /* MOSI */
    GFX_NOT_DEFINED  /* MISO */
  );
  panel = new Arduino_ESP32RGBPanel(
    TFT_DE,    /* DE */
    TFT_VSYNC, /* VSYNC */
    TFT_HSYNC, /* HSYNC */
    TFT_PCLK,  /* PCLK */
    TFT_D11,   /* R0 */
    TFT_D12,   /* R1 */
    TFT_D13,   /* R2 */
    TFT_D14,   /* R3 */
    TFT_D15,   /* R4 */
    TFT_D5,    /* G0 */
    TFT_D6,    /* G1 */
    TFT_D7,    /* G2 */
    TFT_D8,    /* G3 */
    TFT_D9,    /* G4 */
    TFT_D10,   /* G5 */
    TFT_D0,    /* B0 */
    TFT_D1,    /* B1 */
    TFT_D2,    /* B2 */
    TFT_D3,    /* B3 */
    TFT_D4,    /* B4 */
    1,         /* hsync_polarity */
    8,         /* hsync_front_porch */
    10,        /* hsync_pulse_width */
    50,        /* hsync_back_porch */
    1,         /* vsync_polarity */
    8,         /* vsync_front_porch */
    10,        /* vsync_pulse_width */
    20         /* vsync_back_porch */
  );
  gfx = new Arduino_RGB_Display(
    TFT_WIDTH,  /* width */
    TFT_HEIGHT, /* height */
    panel,
    0,    /* rotation */
    true, /* auto_flush (IPS, in makerfabs project it is IPS) */
    bus,
    41, /* RST */
    gc9503v_type1_init_operations,
    sizeof(gc9503v_type1_init_operations));

  Serial.println("display driver configuration done");
  pinMode(TFT_BCKL, OUTPUT);
  digitalWrite(TFT_BCKL, HIGH);
  gfx->begin();
  gfx->fillScreen(BLACK);
  Serial.println("display driver init done");

  Wire.begin(GPIO_SDA, GPIO_SCL, I2C_TOUCH_FREQUENCY);

  ts.begin();
  ts.registerTouchHandler(on_touch);

#if LV_USE_LOG != 0
  lv_log_register_print_cb(my_print); /* register print function for debugging */
#endif

  lv_init();
  lv_disp_draw_buf_init(&draw_buf, buf, NULL, TFT_WIDTH * 20);

  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = TFT_WIDTH;
  disp_drv.ver_res = TFT_HEIGHT;
  disp_drv.flush_cb = flush_pixels;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register(&disp_drv);

  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = touchpad_read;
  lv_indev_drv_register(&indev_drv);

}

#endif //DISPLAY_CONFIG_H
