#ifndef __EL133UF1_H__
#define __EL133UF1_H__

#include <Arduino.h>
#include <stdlib.h>
#include "EPD_IO.h"

#define BLACK 0x00
#define WHITE 0x11
#define YELLOW 0x22
#define RED 0x33
#define BLUE 0x55
#define GREEN 0x66

const unsigned char colors[] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66 };

#define PSR 0x00
#define PWR 0x01
#define POF 0x02
#define PON 0x04
#define BTST_N 0x05
#define BTST_P 0x06
#define DTM 0x10
#define DRF 0x12
#define CDI 0x50
#define TCON 0x60
#define TRES 0x61
#define AN_TM 0x74
#define AGID 0x86
#define BUCK_BOOST_VDDN 0xB0
#define TFT_VCOM_POWER 0xB1
#define EN_BUF 0xB6
#define BOOST_VDDP_EN 0xB7
#define CCSET 0xE0
#define PWS 0xE3
#define CMD66 0xF0
#define SPIM 0xE6

// Display resolution
#define EPD_WIDTH 1200
#define EPD_HEIGHT 1600
// EL133UF1，有两个CS(Frame)每个字节包含两个像素，每个像素4bit
#define EPD_IMAGE_SIZE  (EPD_WIDTH * EPD_HEIGHT)  //1920000 - 为了存储方便，一个byte一个像素
#define EPD_FRAME_SIZE (EPD_IMAGE_SIZE / 2)  //480000 - E6的数据，一个byte两个像素

class EL133UF1 {
public:
  EL133UF1(void);
  ~EL133UF1(void);

  int EL133UF1_Init(void);
  int EL133UF1_Deinit(void);
  void EL133UF1_DisplayFrame(const unsigned char* frame_buffer_m, const unsigned char* frame_buffer_s);
  void EL133UF1_Update(void);
  void EL133UF1_Sleep(void);

  //for testing
  void EL133UF1_DisplayColor(unsigned char color, unsigned char* frame_buffer_m, unsigned char* frame_buffer_s);
  void EL133UF1_DisplayColorBar(unsigned char* frame_buffer_m, unsigned char* frame_buffer_s);

  void palette_index_to_EL133_data(uint8_t * index_buffer, uint8_t *dst_m, uint8_t *dst_s);
  void convert_image_to_frame_buffer(const uint8_t *image_data, uint8_t *frame_buffer_m, uint8_t *frame_buffer_s);
};

extern uint8_t * dst_frame_buffer_m;
extern uint8_t * dst_frame_buffer_s;
extern uint8_t * dst_image_buffer; //store image recieved
extern EL133UF1 epd;
#endif /* __EL133UF1_H__ */

/* END OF FILE */
