#include "EPD_IO.h"

EPD_IO epd_io;
SPIClass epd_spi;  //default using HSPI, not VSPI or FSPI
static const int spiClk = 12000000;  // 12 MHz

EPD_IO::EPD_IO() {
  //IO初始化
  pinMode(RST_PIN, OUTPUT);
  pinMode(EPD_CS_M, OUTPUT);
  pinMode(EPD_CS_S, OUTPUT);
  pinMode(EPD_POWER_CTL, OUTPUT);
  pinMode(DCDC_MODE_CTL, OUTPUT);
  pinMode(BUSY_PIN, INPUT);
  digitalWrite(DCDC_MODE_CTL, 0);
  digitalWrite(EPD_POWER_CTL, 0);
  digitalWrite(EPD_CS_M, 0);
  digitalWrite(EPD_CS_S, 0);
  digitalWrite(RST_PIN, 0);
}

EPD_IO::~EPD_IO() {
}


void EPD_IO::EPD_IO_Initialize(void) {
  //IO初始化-SPI
  epd_spi.begin(SPI_SCLK, SPI_MISO, SPI_MOSI, -1);  //SCLK, MISO, MOSI, SS
  epd_spi.setHwCs(false);
  epd_spi.beginTransaction(SPISettings(spiClk, MSBFIRST, SPI_MODE0));
  //IO初始化
  pinMode(RST_PIN, OUTPUT);
  pinMode(EPD_CS_M, OUTPUT);
  pinMode(EPD_CS_S, OUTPUT);
  pinMode(EPD_POWER_CTL, OUTPUT);
  pinMode(DCDC_MODE_CTL, OUTPUT);
  pinMode(BUSY_PIN, INPUT_PULLUP);
  digitalWrite(DCDC_MODE_CTL, 0);
  digitalWrite(EPD_POWER_CTL, 0);
  digitalWrite(EPD_CS_M, 0);
  digitalWrite(EPD_CS_S, 0);
  digitalWrite(RST_PIN, 0);


}

void EPD_IO::EPD_IO_Deinitialize(void) {
  epd_spi.endTransaction();
  epd_spi.end();
  pinMode(BUSY_PIN, INPUT);
}

void EPD_IO::EPD_IO_Power_On(void) {
  //切换DCDC工作模式为高性能
  digitalWrite(DCDC_MODE_CTL, 1);
  DelayMs(10);
  //开启EPD供电
  digitalWrite(EPD_POWER_CTL, 1);
  digitalWrite(EPD_CS_M, 1);
  digitalWrite(EPD_CS_S, 1);
  DelayMs(10);
}


void EPD_IO::EPD_IO_Power_Off(void) {
  //所有IO口拉低，避免漏电
  digitalWrite(RST_PIN, 0);
  digitalWrite(EPD_CS_M, 0);
  digitalWrite(EPD_CS_S, 0);
  // DelayMs(10);
  //关闭EPD供电
  digitalWrite(EPD_POWER_CTL, 0);
  DelayMs(10);
  //切换DCDC工作模式为低功耗
  digitalWrite(DCDC_MODE_CTL, 0);
  DelayMs(10);
}

void EPD_IO::DelayMs(unsigned int delaytime) {
  delay(delaytime);
}

void EPD_IO::EPD_IO_CS_M_Ctrl(unsigned int status) {
  if (status)
    digitalWrite(EPD_CS_M, 1);
  else
    digitalWrite(EPD_CS_M, 0);
}

void EPD_IO::EPD_IO_CS_S_Ctrl(unsigned int status) {
  if (status)
    digitalWrite(EPD_CS_S, 1);
  else
    digitalWrite(EPD_CS_S, 0);
}

void EPD_IO::EPD_IO_Write_byte(const unsigned char data) {
  epd_spi.transferBytes(&data, NULL, 1);
}

void EPD_IO::EPD_IO_WriteDataBytes(const unsigned char* data, unsigned int count) {
  epd_spi.transferBytes(data, NULL, count);
}


/**
 *  @brief: module reset.
 *          often used to awaken the module in deep sleep,
 *          see Epd::Sleep();
 */
void EPD_IO::EPD_IO_Reset(void) {
  digitalWrite(RST_PIN, HIGH);
  DelayMs(20);
  digitalWrite(RST_PIN, LOW);  //module reset
  DelayMs(30);
  digitalWrite(RST_PIN, HIGH);
  DelayMs(30);
  digitalWrite(RST_PIN, LOW);  //module reset
  DelayMs(30);
  digitalWrite(RST_PIN, HIGH);
  DelayMs(30);
}




void EPD_IO::EPD_IO_WriteCommandData_2CH(const unsigned char cmd, const unsigned char* data, unsigned int data_length, unsigned int cs_mask) {
  if (cs_mask == CS_MASK_MASTER_SLAVE) {
    EPD_IO_CS_M_Ctrl(0);
    EPD_IO_CS_S_Ctrl(0);
  } else if (cs_mask == CS_MASK_MASTER)
    EPD_IO_CS_M_Ctrl(0);
  else if (cs_mask == CS_MASK_SLAVE)
    EPD_IO_CS_S_Ctrl(0);

  EPD_IO_Write_byte(cmd);
  EPD_IO_WriteDataBytes(data, data_length);

  if (cs_mask == CS_MASK_MASTER_SLAVE) {
    EPD_IO_CS_M_Ctrl(1);
    EPD_IO_CS_S_Ctrl(1);
  } else if (cs_mask == CS_MASK_MASTER)
    EPD_IO_CS_M_Ctrl(1);
  else if (cs_mask == CS_MASK_SLAVE)
    EPD_IO_CS_S_Ctrl(1);
}


#define BUSY_CHECK_MAX_LOOP 60000

/**
 *  @brief: Wait until the BUSY_PIN goes LOW
 */
void EPD_IO::EPD_IO_CheckBusy_L(void) {
  int loop_cnt = 0;
  while (digitalRead(BUSY_PIN) == 1) {  //1: busy, 0: idle
    DelayMs(1);
    if ((loop_cnt++) > BUSY_CHECK_MAX_LOOP) {
      Serial.println("ERROR: BUSY CHECK Timeout!");
      break;
    }
  }
}

/**
 *  @brief: Wait until the BUSY_PIN goes HIGH
 */
void EPD_IO::EPD_IO_CheckBusy_H(void) {
  int loop_cnt = 0;
  while (digitalRead(BUSY_PIN) == 0) {  //0: busy, 1: idle
    DelayMs(1);
    if ((loop_cnt++) > BUSY_CHECK_MAX_LOOP) {
      Serial.println("ERROR: BUSY CHECK Timeout!");
      break;
    }
  }
}
