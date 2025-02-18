#include "EL133UF1.h"

EL133UF1 epd;
uint8_t * dst_frame_buffer_m;
uint8_t * dst_frame_buffer_s;
uint8_t * dst_image_buffer;

const unsigned char PSR_V[2] = {
  0xDF, 0x69    //0x09 -> 0x69 20240319 V1 update.
};
const unsigned char PWR_V[6] = {
  0x0F, 0x00, 0x28, 0x2C, 0x28, 0x38
};
const unsigned char POF_V[1] = {
  0x00
};
const unsigned char DRF_V[1] = {
  0x00
};
const unsigned char CDI_V[1] = {
  0xF7
};
const unsigned char TCON_V[2] = {
  0x03, 0x03
};
const unsigned char TRES_V[4] = {
  0x04, 0xB0, 0x03, 0x20
};
const unsigned char CMD66_V[6] = {
  0x49, 0x55, 0x13, 0x5D, 0x05, 0x10
};
const unsigned char EN_BUF_V[1] = {
  0x07
};
const unsigned char CCSET_V[1] = {
  0x01
};
const unsigned char PWS_V[1] = {
  0x22
};
const unsigned char AN_TM_V[9] = {
  0xC0, 0x1C, 0x1C, 0xCC, 0xCC, 0xCC, 0x15, 0x15, 0x55
};

const unsigned char AGID_V[1] = {
  0x10
};

const unsigned char BTST_P_V[2] = {
  0xE8, 0x28
};
const unsigned char BOOST_VDDP_EN_V[1] = {
  0x01
};
const unsigned char BTST_N_V[2] = {
  0xE8, 0x28
};
const unsigned char BUCK_BOOST_VDDN_V[1] = {
  0x01
};
const unsigned char TFT_VCOM_POWER_V[1] = {
  0x02
};

const unsigned char SPIM_V[1] = {
  0x10 //0 - Single SPI (Default) 1 - Quad SPI
};

EL133UF1::EL133UF1() {
}

EL133UF1::~EL133UF1() {
}

int EL133UF1::EL133UF1_Deinit(void) {
  epd_io.EPD_IO_Power_Off();
  // Serial.println("EL133UF1 Power Off.");
  epd_io.EPD_IO_Deinitialize();
  // Serial.println("EL133UF1 Deinitialize.");
  return 0;
}

int EL133UF1::EL133UF1_Init(void) {
  epd_io.EPD_IO_Initialize();
  // Serial.println("EL133UF1 Initialize.");
  epd_io.EPD_IO_Power_On();
  // Serial.println("EL133UF1 Power On.");
  epd_io.EPD_IO_Reset();

  epd_io.EPD_IO_CheckBusy_H();

  epd_io.EPD_IO_WriteCommandData_2CH(AN_TM, AN_TM_V, sizeof(AN_TM_V), CS_MASK_MASTER);

  epd_io.EPD_IO_WriteCommandData_2CH(CMD66, CMD66_V, sizeof(CMD66_V), CS_MASK_MASTER_SLAVE);
  epd_io.EPD_IO_WriteCommandData_2CH(PSR, PSR_V, sizeof(PSR_V), CS_MASK_MASTER_SLAVE);
  epd_io.EPD_IO_WriteCommandData_2CH(CDI, CDI_V, sizeof(CDI_V), CS_MASK_MASTER_SLAVE);
  epd_io.EPD_IO_WriteCommandData_2CH(TCON, TCON_V, sizeof(TCON_V), CS_MASK_MASTER_SLAVE);
  epd_io.EPD_IO_WriteCommandData_2CH(AGID, AGID_V, sizeof(AGID_V), CS_MASK_MASTER_SLAVE);
  epd_io.EPD_IO_WriteCommandData_2CH(PWS, PWS_V, sizeof(PWS_V), CS_MASK_MASTER_SLAVE);
  epd_io.EPD_IO_WriteCommandData_2CH(CCSET, CCSET_V, sizeof(CCSET_V), CS_MASK_MASTER_SLAVE);
  epd_io.EPD_IO_WriteCommandData_2CH(TRES, TRES_V, sizeof(TRES_V), CS_MASK_MASTER_SLAVE);

  epd_io.EPD_IO_WriteCommandData_2CH(PWR, PWR_V, sizeof(PWR_V), CS_MASK_MASTER);
  epd_io.EPD_IO_WriteCommandData_2CH(EN_BUF, EN_BUF_V, sizeof(EN_BUF_V), CS_MASK_MASTER);
  epd_io.EPD_IO_WriteCommandData_2CH(BTST_P, BTST_P_V, sizeof(BTST_P_V), CS_MASK_MASTER);
  epd_io.EPD_IO_WriteCommandData_2CH(BOOST_VDDP_EN, BOOST_VDDP_EN_V, sizeof(BOOST_VDDP_EN_V), CS_MASK_MASTER);
  epd_io.EPD_IO_WriteCommandData_2CH(BTST_N, BTST_N_V, sizeof(BTST_N_V), CS_MASK_MASTER);
  epd_io.EPD_IO_WriteCommandData_2CH(BUCK_BOOST_VDDN, BUCK_BOOST_VDDN_V, sizeof(BUCK_BOOST_VDDN_V), CS_MASK_MASTER);
  epd_io.EPD_IO_WriteCommandData_2CH(TFT_VCOM_POWER, TFT_VCOM_POWER_V, sizeof(TFT_VCOM_POWER_V), CS_MASK_MASTER);

  
  Serial.println("EL133UF1 Initialize OK.");
  /* EL133UF1 hardware init end */
  return 0;
}

void EL133UF1::EL133UF1_DisplayFrame(const unsigned char* frame_buffer_m, const unsigned char* frame_buffer_s) {
  // epd_io.EPD_IO_WriteCommandData_2CH(SPIM, SPIM_V, sizeof(SPIM_V), CS_MASK_MASTER_SLAVE);
  //ATTENTION: 原本是在一个CS下拉周期里完成命令和数据的发送。此处待测试
  epd_io.EPD_IO_CS_M_Ctrl(0);
  epd_io.EPD_IO_Write_byte(DTM);
  epd_io.EPD_IO_WriteDataBytes(frame_buffer_m, EPD_WIDTH * EPD_HEIGHT / 4);
  epd_io.EPD_IO_CS_M_Ctrl(1);

  epd_io.EPD_IO_CS_S_Ctrl(0);
  epd_io.EPD_IO_Write_byte(DTM);
  epd_io.EPD_IO_WriteDataBytes(frame_buffer_s, EPD_WIDTH * EPD_HEIGHT / 4);
  epd_io.EPD_IO_CS_S_Ctrl(1);

  Serial.println("EL133UF1_DisplayFrame Ready.");
  EL133UF1_Update();
}

void EL133UF1::EL133UF1_Update(void) {
  epd_io.EPD_IO_CS_M_Ctrl(0);
  epd_io.EPD_IO_CS_S_Ctrl(0);
  epd_io.EPD_IO_Write_byte(PON);
  epd_io.EPD_IO_CS_M_Ctrl(1);
  epd_io.EPD_IO_CS_S_Ctrl(1);
  epd_io.EPD_IO_CheckBusy_H();
  //ATTENTION: check busy 原本放在CS拉高之前。此为同一般SPI接口屏幕的区别，需要测试是否兼容。

  epd_io.DelayMs(30);
  epd_io.EPD_IO_WriteCommandData_2CH(DRF, DRF_V, sizeof(DRF_V), CS_MASK_MASTER_SLAVE);
  epd_io.EPD_IO_CheckBusy_H();
  //ATTENTION: check busy 原本放在CS拉高之前。此为同一般SPI接口屏幕的区别，需要测试是否兼容。
  epd_io.EPD_IO_WriteCommandData_2CH(POF, POF_V, sizeof(POF_V), CS_MASK_MASTER_SLAVE);
  epd_io.DelayMs(1000);
}

/**
 *  @brief: After this command is transmitted, the chip would enter the 
 *          deep-sleep mode to save power. 
 *          The deep sleep mode would return to standby by hardware reset. 
 *          You can use Init() to awaken
 */
void EL133UF1::EL133UF1_Sleep(void) {
  // Serial.println("EL133UF1_Sleep.");
}
/* END OF FILE */


void EL133UF1::EL133UF1_DisplayColor(unsigned char color, unsigned char* frame_buffer_m, unsigned char* frame_buffer_s) {
  unsigned int i, j;

  Serial.println("EL133UF1_DisplayColor Prepare.");
  for (i = 0; i < EPD_FRAME_SIZE; i++) {
      frame_buffer_m[i] = color;
      frame_buffer_s[i] = color;
  }
  Serial.println("EL133UF1_DisplayColor Ready.");
  EL133UF1_DisplayFrame(frame_buffer_m, frame_buffer_s);
}


void EL133UF1::EL133UF1_DisplayColorBar(unsigned char* frame_buffer_m, unsigned char* frame_buffer_s) {
  unsigned int i, j;

  for (i = 0; i < EPD_HEIGHT / 8; i++) {
    for (j = 0; j < EPD_WIDTH / 4; j++) {
      frame_buffer_m[i * EPD_WIDTH / 4 + j] = WHITE;
      frame_buffer_s[i * EPD_WIDTH / 4 + j] = WHITE;

      frame_buffer_m[i * EPD_WIDTH / 4 + j + EPD_WIDTH * EPD_HEIGHT / 32] = BLACK;
      frame_buffer_s[i * EPD_WIDTH / 4 + j + EPD_WIDTH * EPD_HEIGHT / 32] = BLACK;

      frame_buffer_m[i * EPD_WIDTH / 4 + j + 2 * EPD_WIDTH * EPD_HEIGHT / 32] = WHITE;
      frame_buffer_s[i * EPD_WIDTH / 4 + j + 2 * EPD_WIDTH * EPD_HEIGHT / 32] = WHITE;

      frame_buffer_m[i * EPD_WIDTH / 4 + j + 3 * EPD_WIDTH * EPD_HEIGHT / 32] = RED;
      frame_buffer_s[i * EPD_WIDTH / 4 + j + 3 * EPD_WIDTH * EPD_HEIGHT / 32] = RED;

      frame_buffer_m[i * EPD_WIDTH / 4 + j + 4 * EPD_WIDTH * EPD_HEIGHT / 32] = YELLOW;
      frame_buffer_s[i * EPD_WIDTH / 4 + j + 4 * EPD_WIDTH * EPD_HEIGHT / 32] = YELLOW;

      frame_buffer_m[i * EPD_WIDTH / 4 + j + 5 * EPD_WIDTH * EPD_HEIGHT / 32] = BLUE;
      frame_buffer_s[i * EPD_WIDTH / 4 + j + 5 * EPD_WIDTH * EPD_HEIGHT / 32] = BLUE;

      frame_buffer_m[i * EPD_WIDTH / 4 + j + 6 * EPD_WIDTH * EPD_HEIGHT / 32] = GREEN;
      frame_buffer_s[i * EPD_WIDTH / 4 + j + 6 * EPD_WIDTH * EPD_HEIGHT / 32] = GREEN;

      frame_buffer_m[i * EPD_WIDTH / 4 + j + 7 * EPD_WIDTH * EPD_HEIGHT / 32] = BLACK;
      frame_buffer_s[i * EPD_WIDTH / 4 + j + 7 * EPD_WIDTH * EPD_HEIGHT / 32] = BLACK;
    }
  }

  EL133UF1_DisplayFrame(frame_buffer_m, frame_buffer_s);
}

void EL133UF1::palette_index_to_EL133_data(uint8_t * index_buffer, uint8_t *dst_m, uint8_t *dst_s) {
  for(int j=0; j < EPD_HEIGHT; j++) {
    for(int i = 0; i< EPD_WIDTH/4; i++) {
      *dst_m++  = index_buffer[i + j*EPD_WIDTH/2];
    }

    for(int i = EPD_WIDTH/4; i< EPD_WIDTH/2; i++) {
      *dst_s++  = index_buffer[i + j*EPD_WIDTH/2];
    }
  }
}

#define EPD_WIDTH 1200
#define EPD_HEIGHT 1600
// #define EPD_FRAME_SIZE (EPD_WIDTH * EPD_HEIGHT / 2)

void EL133UF1:: convert_image_to_frame_buffer(const uint8_t *image_data, uint8_t *frame_buffer_m, uint8_t *frame_buffer_s) {
    // Assuming 'image_data' is an array of image data (e.g., RGB565 or indexed format)
    // and it needs to be converted to a specific format for the display
    for (int i = 0; i < EPD_HEIGHT; i++) {
        for (int j = 0; j < EPD_WIDTH / 4; j++) {
            // Process the first part of the frame buffer (frame_buffer_m)
            frame_buffer_m[i * EPD_WIDTH / 4 + j] = image_data[i * EPD_WIDTH / 2 + j];

            // Process the second part of the frame buffer (frame_buffer_s)
            frame_buffer_s[i * EPD_WIDTH / 4 + j] = image_data[i * EPD_WIDTH / 2 + j + (EPD_WIDTH / 4)];
        }
    }

    epd.EL133UF1_DisplayFrame(frame_buffer_m, frame_buffer_s);  
}