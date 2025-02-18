#ifndef __EPD_IO_H__
#define __EPD_IO_H__

#include <Arduino.h>
#include <SPI.h>

// HW Ver 3.0/1/2
#define SPI_MISO 10
#define SPI_MOSI 14
#define SPI_SCLK 12
#define SPI_SS 15

// Pin definition
#define EPD_CS_M 16
#define EPD_CS_S 4
#define RST_PIN 6
#define BUSY_PIN 7

#define EPD_POWER_CTL 41
#define DCDC_MODE_CTL 11


// HW Ver 1.0/2.0 （without tf card slot）
/*
#define SPI_MISO 40
#define SPI_MOSI 41
#define SPI_SCLK 9
#define SPI_SS 18

// Pin definition
#define EPD_CS_M 18
#define EPD_CS_S 17
#define RST_PIN 6
#define BUSY_PIN 7

#define EPD_POWER_CTL 45
#define DCDC_MODE_CTL 3
*/

#define CS_MASK_MASTER 0x01
#define CS_MASK_SLAVE 0x10
#define CS_MASK_MASTER_SLAVE 0x11

class EPD_IO {
private:

public:
  EPD_IO(void);
  ~EPD_IO(void);

  void DelayMs(unsigned int delaytime);
  void EPD_IO_Write_byte(const unsigned char data);
  void EPD_IO_WriteDataBytes(const unsigned char* data, unsigned int count);

  void EPD_IO_Initialize(void);
  void EPD_IO_Deinitialize(void);
  void EPD_IO_Reset(void);
  void EPD_IO_Power_On(void);
  void EPD_IO_Power_Off(void);

  void EPD_IO_CS_M_Ctrl(unsigned int status);
  void EPD_IO_CS_S_Ctrl(unsigned int status);
  void EPD_IO_WriteCommandData_2CH(const unsigned char cmd, const unsigned char* data, unsigned int data_length, unsigned int cs_mask);
  void EPD_IO_CheckBusy_L(void);
  void EPD_IO_CheckBusy_H(void);
};

extern EPD_IO epd_io;
#endif
