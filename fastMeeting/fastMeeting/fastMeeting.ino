#include <Arduino_JSON.h>
#include <WiFi.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

#include "GUI_Paint.h"
#include "fonts.h"

#include <Wire.h>
#include <HTTPClient.h>
#include <Arduino.h>

#include <BLEServer.h>
#include <BLE2902.h>
#include <string>
#include <Update.h>

#include <esp_heap_caps.h>

#include "GUI_Paint.h"
#include "fonts.h"

// include user defined header files --- >
#include "EPD_IO.h"
#include "EL133UF1.h"

#define I2C_SDA 10
#define I2C_SCL 11

#define SERVICE_UUID        "00000180-0000-1000-8000-00805F9B34FB"  // Custom service UUID
#define CHARACTERISTIC_UUID "0000DEAD-0000-1000-8000-00805F9B34FB"  // Custom characteristic UUID

#define BQ25792_ADDRESS 0x6B // Default I2C address

// Register Addresses
#define ICHG_REGISTER 0x03    // Charging current register (placeholder)
#define VSYSMIN_REGISTER 0x0F // Minimum system voltage register (placeholder)
#define VREG_REGISTER 0x01    // Charge voltage register (placeholder)
#define VBAT_REGISTER 0x3B    // Battery voltage ADC register (placeholder)
#define ADC_CONTROL_REGISTER 0x2E
#define Charger_Control_1_REGISTER 0x10
#define IBAT_REGISTER 0x33
#define VAC1_REGISTER 0x37
#define IBUS_REGISTER 0x31
#define VSYS_REGISTER 0x3D
#define Precharge_Control_REGISTER 0x08
#define Termination_Control_REGISTER 0x09
#define Recharge_Control_REGISTER 0x0A

// uint8_t* dst_image_buffer = (uint8_t*)malloc(EPD_FRAME_SIZE);

float vbatVoltage, vac1Voltage, vsysVoltage;
uint16_t vbat, ibus, vac1, vsys;
int ibat;
int ibatsigned;

#define Charger_Control_5_REGISTER 0x14

#define Charger_Control_2_REGISTER 0x11
float vbatlowlimit = 2.50;
uint8_t chgcon2 = 0x43;
int count_chgcon2 = 3;

// const char* ssid = "C3 LABS";
// const char* password = "c34874455";
const char* ssid = "DESKTOP-FSKQPKC 7472";
const char* password = "(O357s53";

//Your Domain name with URL path or IP address with path
// String serverName = "https://c3dss-eink-bms-webapp-kxfozwrl.azurewebsites.net/api/BMS/Status?";
const char* firmwareURL = "https://c3dss-eink-einkadmin-webapp-ppkzyipf.azurewebsites.net/binfiles/ESP32v3.0.ino.bin";
const char* serverName = "https://einkmeetingroom2mockapi-cwhdgegwctamgtd6.southeastasia-01.azurewebsites.net/meetings/";


unsigned long lastTime = 0;
unsigned long timerDelay = 10000;

BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;
uint32_t value = 0;
int Periodint = 300; 
int mtuSize;
int mtuSize2;
bool isSlideshowRunning = false;
String slideshowstatus = "S-0";
String slideTimeS = "";
int slideDelay = 30000;
float slideTimeF = 0;
bool isslideon = false;
char delimiter = '-';

//Json Data
String meetingId = "";
String title = "";
String meetingDate = "";
String startTime = "";
String endTime = "";
String organizer = "";
String description = "";
String participants = "";


#define POWER_KEY 4
#define POWER_HOLD 5
#define LED_R 17
#define LED_G 18
#define LED_B 8

class MyServerCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
        deviceConnected = true;
        Serial.println("Client connected");
    }

    void onDisconnect(BLEServer* pServer) {
        deviceConnected = false;
        // oldDeviceConnected = true;
        Serial.println("Client disconnected");
    }
};

std::pair<std::string, std::string> splitIntoTwo(const std::string& word, char delimiter) {
    std::string part1, part2;
    bool foundDelimiter = false;

    // Iterate through the string
    for (char c : word) {
        if (c == delimiter && !foundDelimiter) {
            // Mark that we've found the delimiter
            foundDelimiter = true;
        } else if (foundDelimiter) {
            // Add characters to the second part after the delimiter
            part2 += c;
        } else {
            // Add characters to the first part before the delimiter
            part1 += c;
        }
    }

    return {part1, part2}; // Return the two parts as a pair
}


class CharacteristicCallBack : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pChar) override {
        String pChar_value = pChar->getValue();  // Get the value from the characteristic (Arduino String)
        int pChar_value_int = pChar_value.toInt();  // Convert the String to an integer
        Serial.println("Received value: " + String(pChar_value_int));  // Print the received value

        // Example of handling incoming data
        if (pChar_value_int == 1) {
            // Perform action for value 1
            Serial.println("Action for value 1 executed");
        }
        // Add more conditions if needed for different values
    }
};

void enterDeepSleep() {
 // Set ESP32 into Deep Sleep mode
   Serial.println("start Deep sleep");
   Serial.println(Periodint);
   esp_sleep_enable_timer_wakeup(Periodint*1000000); 
   esp_deep_sleep_start();

}

void setup() {
  Serial.begin(115200);

  //FOR BMS STARTS-----------------------------------------------------
  Wire.begin(I2C_SDA, I2C_SCL); // Initialize I2C on ESP32-S3 default pins (SDA: GPIO21, SCL: GPIO22)

  // Configure ICHG (Charging Current)
  uint16_t ichg = 0x0096; // Example: Set charging current to 1.5A
  writeRegisterDouble(ICHG_REGISTER, ichg);
  Serial.println("ICHG configured.");

  // Configure VREG (Charge Voltage Regulation)
  uint16_t vreg = 0x01AE; // Example: Set VREG to 4.3V
  writeRegisterDouble(VREG_REGISTER, vreg);
  Serial.println("VREG configured.");

  // Configure Precharge_Control
  uint16_t precon = 0x83; // Example: Set Precharge to Fastcharge Threshold at 2.87V ( 66.7%*VREG) and Iprecharge at 120mA
  writeRegisterSingle(Precharge_Control_REGISTER, precon);
  Serial.println("Precharge_Control configured.");

  // Configure Termination_Control
  uint16_t tercon = 0x05; // Example: Set termination current at 200mA
  writeRegisterSingle(Termination_Control_REGISTER, tercon);
  Serial.println("Termination_Control configured.");

  // Configure Recharge_Control
  uint16_t rechgcon = 0x21; // Example: Set battery recharge deglich time at 1024ms and Battery Recharge Threshold Offset at 100mV below VREG
  writeRegisterSingle(Recharge_Control_REGISTER, rechgcon);
  Serial.println("Recharge_Control configured.");
  //FOR BMS ENDS-------------------------------------------------------

  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    // WiFi.setSleep(false);
  }

  BLEDevice::init("13.3-BMS");
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  
  BLEService *pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
      CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY
  );
  pCharacteristic->setCallbacks(new CharacteristicCallBack());
  BLE2902 *pBLE2902 = new BLE2902();
  pBLE2902->setNotifications(true);
  pCharacteristic->addDescriptor(pBLE2902);
  pCharacteristic->setValue(String(vbatVoltage)+"@"+String(ibatsigned)+"@"+String(ibus)+"@"+String(vsysVoltage));
  pService->start();
  pServer->getAdvertising()->addServiceUUID(SERVICE_UUID);
  pServer->getAdvertising()->setScanResponse(false);
  pServer->getAdvertising()->start();
  // Serial.println("Waiting for a client to connect...");

  getData();
  Serial.println("Data took");
  Serial.printf("Heap Caps: %d Bytes\n", heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
  dst_frame_buffer_m = (uint8_t *)heap_caps_malloc(EPD_IMAGE_SIZE, MALLOC_CAP_SPIRAM);
  dst_frame_buffer_s = (uint8_t *)heap_caps_malloc(EPD_IMAGE_SIZE, MALLOC_CAP_SPIRAM);
  WiFi.disconnect();
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Not connected");
    screenText();
  }

  enterDeepSleep();

}

void loop() {

    if (deviceConnected) {
        showBMSdata();
        pCharacteristic->setValue(String(vbatVoltage)+"@"+String(ibatsigned)+"@"+String(ibus)+"@"+String(vsysVoltage)+"@"+"S-0"); 
        Serial.println(String(vbatVoltage)+"@"+String(ibatsigned)+"@"+String(ibus)+"@"+String(vsysVoltage)); // Update the characteristic value
        pCharacteristic->notify();  // Send notification to the connected client
        delay(1000);  // Delay for 1 second to simulate periodic updates
    }

    // If the client disconnected, restart advertising to allow new connections
    if (!deviceConnected && oldDeviceConnected) {
        delay(500);
        pServer->startAdvertising();  // Restart advertising
        Serial.println("Re-starting advertising...");
        oldDeviceConnected = deviceConnected;
    }

    // getData();
    // WiFi.disconnect();
}

void recogOverdischarge(float vbat, float vbatlowlimit){
  if (vbat<=vbatlowlimit){
    count_chgcon2-= 1;
  }
  else {
    count_chgcon2 = 3;
  }
  if (count_chgcon2<=0){
    Wire.beginTransmission(BQ25792_ADDRESS);
    Wire.write(Charger_Control_2_REGISTER);
    Wire.write(chgcon2);
    Wire.endTransmission();
  }
}

void writeRegisterDouble(uint8_t reg, uint16_t value) {
  Wire.beginTransmission(BQ25792_ADDRESS);
  Wire.write(reg);
  Wire.write((value >> 8) & 0xFF); // MSB
  Wire.write(value & 0xFF);        // LSB
  Wire.endTransmission();
}

void writeRegisterSingle(uint8_t reg, uint8_t value) {
  Wire.beginTransmission(BQ25792_ADDRESS);
  Wire.write(reg);
  Wire.write(value);
  Wire.endTransmission();
}

// Function to read a 16-bit value from a register in BQ
uint16_t readRegister(uint8_t reg) {
  Wire.beginTransmission(BQ25792_ADDRESS);
  Wire.write(reg);
  Wire.endTransmission(false); // Restart for reading

  Wire.requestFrom(BQ25792_ADDRESS, 2);
  if (Wire.available() == 2) {
    uint16_t value = ((Wire.read() << 8) & 0xFFFF) | Wire.read(); //No need to "& 0xFFFF"
    return value;
  }
}

void performOTAUpdate() {
  if ((WiFi.status() == WL_CONNECTED)) {  // Check Wi-Fi connection
    HTTPClient http;

    Serial.println("Connecting to firmware server...");
    http.begin(firmwareURL);

    int httpCode = http.GET();  // Send the request

    // Check for a successful response
    if (httpCode == HTTP_CODE_OK) {
      int contentLength = http.getSize();
      bool canBegin = Update.begin(contentLength);

      if (canBegin) {
        Serial.println("Starting OTA update...");

        // Stream the data
        WiFiClient* client = http.getStreamPtr();
        size_t written = Update.writeStream(*client);

        if (written == contentLength) {
          Serial.println("OTA update successfully written!");
        } else {
          Serial.printf("OTA update failed. Written only %d/%d bytes\n", written, contentLength);
        }

        // End the update
        if (Update.end()) {
          Serial.println("OTA update complete. Restarting...");
          ESP.restart();
        } else {
          Serial.println("Error occurred during OTA update.");
        }
      } else {
        Serial.println("Not enough space to begin OTA update.");
      }
    } else {
      Serial.printf("Failed to download firmware. HTTP response code: %d\n", httpCode);
    }
    http.end();
  } else {
    Serial.println("Wi-Fi not connected");

  }
}

void showBMSdata () {
  if ((millis() - lastTime) > timerDelay) {
    
    //FOR BMS STARTS------------------------------------------------------------------------------------
    uint16_t config_Settings = 0x84; //Enabling ADC
    writeRegisterSingle(ADC_CONTROL_REGISTER, config_Settings);
    Serial.println("ADC_Control Re-configured(Enabled)");

    // Configure Charger_Control_5 Register
    uint16_t chgcon5 = 0xB7;//Enabling ShipFET Operations and OCP
    writeRegisterSingle(Charger_Control_5_REGISTER, chgcon5);
    Serial.println("Charger_Control_5_REGISTER configured.");

    int dataRetrievCount = 10; //Sample data required for Reliable readings
    while (dataRetrievCount>1){
      Serial.println("------------------------------------------------------------------");
      
      vbat = readRegister(VBAT_REGISTER); //Read BAT Voltage
      vbatVoltage = vbat * 0.001; // Convert raw ADC value to voltage (example scaling)
      Serial.print("Battery Voltage: ");
      Serial.print(vbatVoltage);
      Serial.println(" V");

      ibat = readRegister(IBAT_REGISTER); //Read IBAT Current
      //2S Complement 
      if (ibat>=8001) {
        ibatsigned=((-1)*((ibat xor 0xFFFF)+1));
      }
      else {
        ibatsigned=ibat;
      }
      Serial.print("Battery Charge-Discharge Current: ");
      Serial.print(ibatsigned);
      Serial.println(" mA");
      Serial.println("---------");

      vac1 = readRegister(VAC1_REGISTER); //VOLTAGE OF THE INPUT POWER SOURCE
      vac1Voltage = vac1 * 0.001; // Convert raw ADC value to voltage (example scaling)
      Serial.print("VAC1 Voltage: ");
      Serial.print(vac1Voltage);
      Serial.println(" V");

      ibus = readRegister(IBUS_REGISTER); //INPUT CURRENT FROM THE INPUT POWER SOURCE(USB)
      //Serial.println(ibus);
      Serial.print("Input Current through VBUS: ");
      Serial.print(ibus);
      Serial.println(" mA"); 
      Serial.println("---------");

      vsys = readRegister(VSYS_REGISTER); //VOLTAGE at the VSYS Pin
      vsysVoltage = vsys * 0.001; // Convert raw ADC value to voltage (example scaling)
      Serial.print("VSYS Voltage: ");
      Serial.print(vsysVoltage);
      Serial.println(" V");

      Serial.println("------------------------------------------------------------------");
      delay(300); //Required for the stability

      dataRetrievCount-=1; //Decrement by 1
      //FOR BMS ENDS------------------------------------------------------------------------------------
    }

    //FOR WIFI STARTS-----------------------------------------------------------------------------------
    config_Settings = 0x00; //Disabaling ADC - RESTARTING DUE TO STUCKING - ADC MEASUREMENTS
    writeRegisterSingle(ADC_CONTROL_REGISTER, config_Settings);
    Serial.println("ADC_Control Re-configured(Disabled)");

    // Configure Charger_Control_5 Register
    chgcon5 = 0x97;//Enabling ShipFET Operations and OCP
    writeRegisterSingle(Charger_Control_5_REGISTER, chgcon5);
    Serial.println("Charger_Control_5_REGISTER configured.");

    recogOverdischarge(vbatVoltage,vbatlowlimit); //Check for Over-Discharge Situation
    
    lastTime = millis();

    Serial.println("Next update is set to " +String((timerDelay/1000))+ " seconds from now");
    //FOR WIFI ENDS-------------------------------------------------------------------------------------
  }
}

void getData() {
  int maxRetries = 3; // Maximum number of retries
  int retryCount = 0;
  bool success = false;

  while (retryCount < maxRetries && !success) {
    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;

      // Start HTTP request
      http.begin(serverName);

      // Send HTTP GET request
      int httpResponseCode = http.GET();

      if (httpResponseCode > 0) {
        // If request is successful, get the response payload
        String payload = http.getString();
        Serial.print("HTTP Response Code: ");
        Serial.println(httpResponseCode);
        Serial.println("Payload: ");
        Serial.println(payload);

        // Parse the JSON response
        JSONVar jsonResponse = JSON.parse(payload);

        // Check if JSON parsing is successful
        if (JSON.typeof(jsonResponse) == "undefined") {
          Serial.println("Parsing failed!");
        } else {
          // Loop through the meeting data and print the details
          for (int i = 0; i < jsonResponse.length(); i++) {
            JSONVar meeting = jsonResponse[i];
            meetingId = JSONVar::stringify(meeting["meetingId"]).substring(1, JSONVar::stringify(meeting["meetingId"]).length() - 1);
            title = JSONVar::stringify(meeting["title"]).substring(1, JSONVar::stringify(meeting["title"]).length() - 1);
            meetingDate = JSONVar::stringify(meeting["meetingDate"]).substring(1, JSONVar::stringify(meeting["meetingDate"]).length() - 1);
            startTime = JSONVar::stringify(meeting["startTime"]).substring(1, JSONVar::stringify(meeting["startTime"]).length() - 1);
            endTime = JSONVar::stringify(meeting["endTime"]).substring(1, JSONVar::stringify(meeting["endTime"]).length() - 1);
            organizer = JSONVar::stringify(meeting["organizer"]).substring(1, JSONVar::stringify(meeting["organizer"]).length() - 1);
            description = JSONVar::stringify(meeting["description"]).substring(1, JSONVar::stringify(meeting["description"]).length() - 1);
            participants = JSONVar::stringify(meeting["participants"]).substring(1, JSONVar::stringify(meeting["participants"]).length() - 1);

            // Print the meeting details
            Serial.println("Meeting Details:");
            Serial.print("Meeting ID: ");
            Serial.println(meetingId);
            Serial.print("Title: ");
            Serial.println(title);
            Serial.print("Meeting Date: ");
            Serial.println(meetingDate);
            Serial.print("Start Time: ");
            Serial.println(startTime);
            Serial.print("End Time: ");
            Serial.println(endTime);
            Serial.print("Organizer: ");
            Serial.println(organizer);
            Serial.print("Description: ");
            Serial.println(description);
            Serial.print("Participants: ");
            Serial.println(participants);
            Serial.println("----------------------------");
          }
          success = true; // Data fetched successfully
        }
      } else {
        Serial.print("Error Code: ");
        Serial.println(httpResponseCode);
      }

      // Free resources
      http.end();
    } else {
      Serial.println("WiFi Disconnected");
    }

    if (!success) {
      retryCount++;
      if (retryCount < maxRetries) {
        Serial.println("Retrying...");
        delay(500); // Wait for 2 seconds before retrying
      } else {
        Serial.println("Max retries reached. Exiting...");
        WiFi.disconnect();
        return;
      }
    }
  }

  if (success) {
    Serial.println("Data fetched successfully.");
    WiFi.disconnect(); // Disconnect WiFi after successful operation
  }
}

void screenText() {
    // Debug("EPD_13IN3E_test Demo\r\n");
    // DEV_Module_Init();
    // Debug("e-Paper Init and Clear...\r\n");
    epd.EL133UF1_Init();


    // Allocate buffer (e.g., a static array for simplicity)
    // unsigned char buffer[1200 * 1600];

    UBYTE *Image;
    UWORD ImageSize = ((EPD_WIDTH % 2 == 0) ? (EPD_WIDTH / 2) : (EPD_WIDTH / 2 + 1)) * EPD_HEIGHT;
    Image = (UBYTE *)heap_caps_malloc(ImageSize, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);

    // Serial.printf("Heap Caps: %d Bytes\n", heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
    // Allocate memory from PSRAM (with 8-bit capability)
    Paint_NewImage(Image, 1200, 1600, 270, WHITE);
        
    //Select Image
    Paint_SelectImage(Image);
    Paint_SetScale(6);
    // epd.EL133UF1_Clear(WHITE, dst_frame_buffer_m, dst_frame_buffer_s);
    // Paint_SetRotate(270);
    Paint_Clear(WHITE);
    // 2.Drawing on the image
    // Debug("Drawing:Image\r\n");
    // delay(3000);

    // TOP LINE
    Paint_DrawRectangle(10, 5, 1590, 115, GREEN, DOT_PIXEL_1X1, DRAW_FILL_FULL);
    //LARGE BOXES
    Paint_DrawRectangle(10, 125, 700, 655, BLUE, DOT_PIXEL_1X1, DRAW_FILL_FULL);
    Paint_DrawRectangle(10, 665, 700, 1194, BLUE, DOT_PIXEL_1X1, DRAW_FILL_FULL);
    //SMALL BOXES
    Paint_DrawRectangle(710, 125, 840, 192, GREEN, DOT_PIXEL_1X1, DRAW_FILL_FULL);
    Paint_DrawRectangle(710, 202, 840, 269, GREEN, DOT_PIXEL_1X1, DRAW_FILL_FULL);
    Paint_DrawRectangle(710, 279, 840, 346, GREEN, DOT_PIXEL_1X1, DRAW_FILL_FULL);
    Paint_DrawRectangle(710, 356, 840, 423, GREEN, DOT_PIXEL_1X1, DRAW_FILL_FULL);
    Paint_DrawRectangle(710, 433, 840, 500, GREEN, DOT_PIXEL_1X1, DRAW_FILL_FULL);
    Paint_DrawRectangle(710, 510, 840, 577, GREEN, DOT_PIXEL_1X1, DRAW_FILL_FULL);
    Paint_DrawRectangle(710, 587, 840, 654, GREEN, DOT_PIXEL_1X1, DRAW_FILL_FULL);
    Paint_DrawRectangle(710, 665, 840, 732, GREEN, DOT_PIXEL_1X1, DRAW_FILL_FULL);
    Paint_DrawRectangle(710, 742, 840, 809, GREEN, DOT_PIXEL_1X1, DRAW_FILL_FULL);
    Paint_DrawRectangle(710, 819, 840, 886, GREEN, DOT_PIXEL_1X1, DRAW_FILL_FULL);
    Paint_DrawRectangle(710, 896, 840, 963, GREEN, DOT_PIXEL_1X1, DRAW_FILL_FULL);
    Paint_DrawRectangle(710, 973, 840, 1040, GREEN, DOT_PIXEL_1X1, DRAW_FILL_FULL);
    Paint_DrawRectangle(710, 1050, 840, 1117, GREEN, DOT_PIXEL_1X1, DRAW_FILL_FULL);
    Paint_DrawRectangle(710, 1127, 840, 1194, GREEN, DOT_PIXEL_1X1, DRAW_FILL_FULL);
    //LINES
    Paint_DrawRectangle(10, 118, 1590, 122, BLUE, DOT_PIXEL_1X1, DRAW_FILL_FULL);
    Paint_DrawRectangle(710, 272, 1590, 276, BLUE, DOT_PIXEL_1X1, DRAW_FILL_FULL);
    Paint_DrawRectangle(710, 658, 1590, 662, BLUE, DOT_PIXEL_1X1, DRAW_FILL_FULL);
    Paint_DrawRectangle(710, 966, 1590, 970, BLUE, DOT_PIXEL_1X1, DRAW_FILL_FULL);
    Paint_DrawRectangle(10, 1196, 1590, 1200, BLUE, DOT_PIXEL_1X1, DRAW_FILL_FULL);
    Paint_DrawString_EN(25, 50, "MEETING ID: ", &Montserrat28, GREEN, WHITE);
    Paint_DrawString_EN(500, 50, meetingId.c_str(), &Montserrat28, GREEN, WHITE);
    Paint_DrawString_EN(1150, 50, meetingDate.c_str(), &Montserrat28, GREEN, WHITE);
    Paint_DrawString_EN(850, 145, title.c_str(), &Montserrat28, WHITE, BLUE);
    Paint_DrawString_EN(25, 165, title.c_str(), &Montserrat28, BLUE, WHITE);
    Paint_DrawString_EN(25, 220, organizer.c_str(), &Montserrat28, BLUE, WHITE);
    // Paint_DrawString_EN(25, 275, description.c_str(), &Montserrat28, BLUE, WHITE);

    // int maxWidth = 600; // Maximum width of the text area
    // DrawStringWithWrapping(25, 275, description, &Montserrat28, EPD_13IN3E_BLUE, EPD_13IN3E_WHITE, maxWidth);

    Paint_DrawString_EN(850, 290, "BOARD MEETING", &Montserrat28, WHITE, BLUE);
    Paint_DrawString_EN(850, 675, "OPERATIONS MEETING", &Montserrat28, WHITE, BLUE);
    Paint_DrawString_EN(850, 985, "STAFF MEETING", &Montserrat28, WHITE, BLUE);
    Paint_DrawString_EN(730, 150, "8:30", &Font24, GREEN, WHITE);
    Paint_DrawString_EN(730, 225, "9:00", &Font24, GREEN, WHITE);
    Paint_DrawString_EN(730, 300, "9:30", &Font24, GREEN, WHITE);
    Paint_DrawString_EN(730, 375, "10:00", &Font24, GREEN, WHITE);
    Paint_DrawString_EN(730, 450, "10:30", &Font24, GREEN, WHITE);
    Paint_DrawString_EN(730, 535, "11:00", &Font24, GREEN, WHITE);
    Paint_DrawString_EN(730, 610, "11:30", &Font24, GREEN, WHITE);
    Paint_DrawString_EN(730, 685, "12:00", &Font24, GREEN, WHITE);
    Paint_DrawString_EN(730, 760, "12:30", &Font24, GREEN, WHITE);
    Paint_DrawString_EN(730, 845, "13:00", &Font24, GREEN, WHITE);
    Paint_DrawString_EN(730, 920, "13:30", &Font24, GREEN, WHITE);
    Paint_DrawString_EN(730, 995, "14:00", &Font24, GREEN, WHITE);
    Paint_DrawString_EN(730, 1070,"14:30", &Font24, GREEN, WHITE);
    Paint_DrawString_EN(730, 1145,"15:00", &Font24, GREEN, WHITE);
    epd.convert_image_to_frame_buffer(Image,dst_frame_buffer_m,dst_frame_buffer_s);

    // epd.EL133UF1_DisplayPart(Image, 0, 0, 1200, 1600, dst_frame_buffer_m, dst_frame_buffer_s);
    DEV_Delay_ms(3000);
  
   //Paint_DrawString_EN(145, 0, "IDEA ROOM - 2", &Font16,BLACK, WHITE);
  //  EPD_13IN3E_DisplayPart(Image, 400, 0, 200, 200, frame_buffer2_m, frame_buffer2_s);
  //   DEV_Delay_ms(3000);


  //  epd.EL133UF1_Sleep();
  //  Serial.println("EL081EF2_Sleep Done.");
  //  epd.EL133UF1_Deinit();
  //  Serial.println("EL081EF2_Deinit Done.");
}