/* Includes ------------------------------------------------------------------*/
#include <DEV_Config.h>
#include <EPD.h>
#include <GUI_Paint.h>
#include <fonts.h>
#include "imagedata.h"
#include <stdlib.h>
#include <Arduino.h>
#include <cstring>
#include <vector>
#include <string>
//#include "Communications.h"      
#include <map>

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h> 
 
char nametest[50];
char part1[100];
char part2[100];  

String inputBuffer;
int l = 0;

#define SERVICE_UUID        "00000180-0000-1000-8000-00805F9B34FB"
#define CHAR_UUID "0000DEAD-0000-1000-8000-00805F9B34FB"

BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
BLECharacteristic* pCharacteristic_2 = NULL;
BLEDescriptor *pDescr;
BLE2902 *pBLE2902;

bool deviceConnected = false; 
bool oldDeviceConnected = false;
bool wasConnected = false;
// uint32_t value = 0;

void qr1();
void splitString(const char *input, char *part1, char *part2);
void qr();
void sendDataToMobile();

struct CharacterInfo {
    char character;
    int width;
};

int x ;
String bleAddressArduino;
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      Serial.println("Device connected.");
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      Serial.println("Device disconnected.");
    }
};

class CharacteristicCallBack: public BLECharacteristicCallbacks {
void onWrite(BLECharacteristic *pChar) override {
    std::string pChar_value = std::string(pChar->getValue().c_str());  // Get the characteristic value as a std::string
    String pChar_value_string = String(pChar_value.c_str());  // Convert std::string to Arduino String
    int pChar_value_int = pChar_value_string.toInt();  // Convert the string to an integer
    Serial.println("pChar: " + String(pChar_value_int));  // Print the integer value
    x = pChar_value_int;
     
     for (char name1 : pChar_value) {
         
        Serial.println(name1);

        nametest[l] = name1;
        l++;


        if (l >= sizeof(nametest) - 1 || name1 == '\n') {
          break;
        }

      }
    
     //  sendDataToMobile();
      nametest[l] = '\0';  // Null-terminate the name array

      // Print the entered name
      Serial.print("Your name is: ");
      Serial.println(nametest);
    splitString(nametest, part1, part2);

    // Print the parts
    Serial.print("Part 1: ");
    Serial.println(part1);
    Serial.print("Part 2: ");
    Serial.println(part2);

      qr();
      delay(15000);
      qr1();

      l=0;                    // TO start a new name 
      inputBuffer.clear();

  }
};


/* Entry point ----------------------------------------------------------------*/
void setup() {
  // Initialize BLE
  BLEDevice::init("8.14QR");

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create the BLE Characteristic
  pCharacteristic = pService->createCharacteristic(
                      CHAR_UUID,
                      BLECharacteristic::PROPERTY_READ |
                      BLECharacteristic::PROPERTY_WRITE |
                      BLECharacteristic::PROPERTY_NOTIFY
                    );

  // Create and set the callback for handling incoming data
  pCharacteristic->setCallbacks(new CharacteristicCallBack());
  
  // Add descriptor
  pDescr = new BLEDescriptor(BLEUUID((uint16_t)0x2902));
  pCharacteristic->addDescriptor(pDescr);

  // Add BLE2902 Descriptor for notifications
  pBLE2902 = new BLE2902();
  pBLE2902->setNotifications(true);
  pCharacteristic->addDescriptor(pBLE2902);

  // Start the service
  pService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x0);
  BLEDevice::startAdvertising();
  Serial.println("Waiting for a client connection to notify...");

  // Initialize display
  printf("EPD_4IN2B_V2_test Demo\r\n");
  DEV_Module_Init();

  printf("e-Paper Init and Clear...\r\n");
  EPD_4IN2B_V2_Init();
  EPD_4IN2B_V2_Clear();
  DEV_Delay_ms(500);

  // Create and initialize the display image
  UBYTE *BlackImage;
  UWORD Imagesize = ((EPD_4IN2B_V2_WIDTH % 8 == 0) ? (EPD_4IN2B_V2_WIDTH / 8) : (EPD_4IN2B_V2_WIDTH / 8 + 1)) * EPD_4IN2B_V2_HEIGHT;
  if ((BlackImage = (UBYTE *)malloc(Imagesize)) == NULL) {
    printf("Failed to apply for black memory...\r\n");
    while(1);
  }

  printf("NewImage:BlackImage and RYImage\r\n");
  Paint_NewImage(BlackImage, EPD_4IN2B_V2_WIDTH, EPD_4IN2B_V2_HEIGHT, 270, WHITE);

  // Select Image
  Paint_SelectImage(BlackImage);
  Paint_Clear(WHITE);
  delay(100);

  Paint_DrawImage(gImage_QRNEW,0,0,400,300);

  uint16_t maxWidth = 265;         // Available width for text (20 to 280)
  uint16_t startX = 20;           // Starting x for each line
  uint16_t startY = 29;           // Starting y for text
  uint16_t lineHeight = 30;
  std::string str2 = "HEMAS PHARMACEUTICALS";
  uint16_t currentY = startY;

  std::vector<std::string> lines;  // Store all lines for later rendering
  std::vector<int> lineWidths;

  std::string currentLine;         // Holds the current line of text
  int currentLineWidth = 0;

  std::string word;                // To accumulate characters of the current word
  for (size_t i = 0; i <= str2.length(); ++i) {
      char c = (i < str2.length()) ? str2[i] : ' '; // Add trailing space for the last word
      if (c == ' ' || c == '\0') {
          if (!word.empty()) {
              int wordWidth = 0;

              // Calculate the total width of the word
              for (char wc : word) {
                  wordWidth += getCharacterWidth(wc);
              }

              int spaceWidth = getCharacterWidth(' ');
              // If the word cannot fit in the current line, move to the next line
              if (!currentLine.empty() && currentLineWidth + spaceWidth + wordWidth > maxWidth) {
                  // Store the completed line and its width
                  lines.push_back(currentLine);
                  lineWidths.push_back(currentLineWidth);

                  // Start a new line
                  currentLine.clear();
                  currentLineWidth = 0;
              }

              // Add the word to the current line
              if (!currentLine.empty()) {
                  currentLine += ' ';
                  currentLineWidth += spaceWidth;
              }
              currentLine += word;
              currentLineWidth += wordWidth;

              word.clear(); // Reset the word
          }
      } else {
          word += c; // Accumulate characters for the word
      }
  }

  // Store the last line
  if (!currentLine.empty()) {
      lines.push_back(currentLine);
      lineWidths.push_back(currentLineWidth);
  }

  // Draw all lines, centering each one
  for (size_t i = 0; i < lines.size(); ++i) {
      const std::string& line = lines[i];
      int lineWidth = lineWidths[i];

      // Calculate the starting X position to center the line
      uint16_t lineStartX = startX + ((maxWidth - lineWidth) / 2);

      // Draw the line
      for (char ch : line) {
          std::string temp(1, ch);
          const char* charToDraw = temp.c_str();
          Paint_DrawString_EN(lineStartX, currentY, charToDraw, &Barlow22, WHITE, BLACK);
          lineStartX += getCharacterWidth(ch);
      }

      // Move to the next line
      currentY += lineHeight;
  }

  printf("EPD_Display\r\n");
  EPD_4IN2B_V2_Display(BlackImage);
  DEV_Delay_ms(2000);

  printf("Goto Sleep...\r\n");
  EPD_4IN2B_V2_Sleep();
  free(BlackImage);
  BlackImage = NULL;

  // Get BLE address
  bleAddressArduino = BLEDevice::getAddress().toString();
  bleAddressArduino.toUpperCase();
  Serial.println(String(bleAddressArduino));
}

/* The main loop -------------------------------------------------------------*/
void loop()
{
  // if (deviceConnected) {
  //   pCharacteristic->setValue(value);
  //   pCharacteristic->notify();
  //   value++;
  //   delay(1000);
  // }

  if (!deviceConnected && oldDeviceConnected) {
    delay(500); // Give the Bluetooth stack time to get things ready
    pServer->startAdvertising(); // Restart advertising
    Serial.println("start advertising");
    oldDeviceConnected = deviceConnected;
  }

  if (deviceConnected && !oldDeviceConnected) {
    // Do stuff here on connecting
    delay(500); // Give the BLE stack time to get ready
    sendDataToMobile();
    oldDeviceConnected = deviceConnected;
  }
}

// void Paint_DrawString_EN(int x, int y, char c, void* font, int color, int background) {
// }

std::vector<char> splitString2(const std::string& str) {
    std::vector<char> characters;
    for (char c : str) {
        characters.push_back(c);
    }
    return characters;
}

int getCharacterWidth(char c) {
    if (c == ' ') return 6;
    if (c == '.') return 6;  // Example: space width
    if (c == ',') return 6;
    if (c == '(') return 8;
    if (c == ')') return 8;
    return 14;               // Example: default width for other characters
}

int getCharacterWidth2(char c) {
    if (c == ' ') return 6;
    if (c == '.') return 9;  // Example: space width
    if (c == ',') return 9;
    return 15;               // Example: default width for other characters
}

std::vector<CharacterInfo> splitStringWithWidths(const std::string& str) {
    std::vector<CharacterInfo> characterData;
    for (char c : str) {
        CharacterInfo info = {c, getCharacterWidth(c)};
        characterData.push_back(info);
    }
    return characterData;
}

std::vector<CharacterInfo> splitStringWithWidths2(const std::string& str) {
    std::vector<CharacterInfo> characterData2;
    for (char c : str) {
        CharacterInfo info = {c, getCharacterWidth2(c)};
        characterData2.push_back(info);
    }
    return characterData2;
}

int calculateTotalTextWidth(const std::string& str) {
    std::vector<CharacterInfo> charactersWithWidths = splitStringWithWidths2(str);
    int totalTextWidth = 0; 
    for (const auto& charInfo2 : charactersWithWidths) {
        totalTextWidth += charInfo2.width;
    }
    return totalTextWidth;
}

void qr(){
 printf("e-Paper Init and Clear...\r\n");
  EPD_4IN2B_V2_Init();
 // EPD_4IN2B_V2_Clear();
  //DEV_Delay_ms(500);

  //Create a new image cache named IMAGE_BW and fill it with white
  UBYTE *BlackImage; // Red or Yellow
  UWORD Imagesize = ((EPD_4IN2B_V2_WIDTH % 8 == 0) ? (EPD_4IN2B_V2_WIDTH / 8 ) : (EPD_4IN2B_V2_WIDTH / 8 + 1)) * EPD_4IN2B_V2_HEIGHT;
  if ((BlackImage = (UBYTE *)malloc(Imagesize)) == NULL) {
    printf("Failed to apply for black memory...\r\n");
    while(1);
  }
   
  printf("NewImage:BlackImage and RYImage\r\n");
  Paint_NewImage(BlackImage, EPD_4IN2B_V2_WIDTH, EPD_4IN2B_V2_HEIGHT, 270, BLACK);

  //Select Image
  Paint_SelectImage(BlackImage);
  Paint_Clear(WHITE);
  delay(100);

  Paint_DrawImage(gImage_QR3,0,0,400,300);

  // #########################################
  uint16_t maxWidth = 265;         // Available width for text (20 to 280)
  uint16_t startX = 20;           // Starting x for each line
  uint16_t startY = 35;           // Starting y for text
  uint16_t lineHeight = 30;
  std::string str2 = "HEMAS PHARMACEUTICALS";
  uint16_t currentY = startY;

  std::vector<std::string> lines;  // Store all lines for later rendering
  std::vector<int> lineWidths;

  std::string currentLine;         // Holds the current line of text
  int currentLineWidth = 0;

  std::string word;                // To accumulate characters of the current word
  for (size_t i = 0; i <= str2.length(); ++i) {
      char c = (i < str2.length()) ? str2[i] : ' '; // Add trailing space for the last word
      if (c == ' ' || c == '\0') {
          if (!word.empty()) {
              int wordWidth = 0;

              // Calculate the total width of the word
              for (char wc : word) {
                  wordWidth += getCharacterWidth(wc);
              }

              int spaceWidth = getCharacterWidth(' ');
              // If the word cannot fit in the current line, move to the next line
              if (!currentLine.empty() && currentLineWidth + spaceWidth + wordWidth > maxWidth) {
                  // Store the completed line and its width
                  lines.push_back(currentLine);
                  lineWidths.push_back(currentLineWidth);

                  // Start a new line
                  currentLine.clear();
                  currentLineWidth = 0;
              }

              // Add the word to the current line
              if (!currentLine.empty()) {
                  currentLine += ' ';
                  currentLineWidth += spaceWidth;
              }
              currentLine += word;
              currentLineWidth += wordWidth;

              word.clear(); // Reset the word
          }
      } else {
          word += c; // Accumulate characters for the word
      }
  }

  // Store the last line
  if (!currentLine.empty()) {
      lines.push_back(currentLine);
      lineWidths.push_back(currentLineWidth);
  }

  // Draw all lines, centering each one
  for (size_t i = 0; i < lines.size(); ++i) {
      const std::string& line = lines[i];
      int lineWidth = lineWidths[i];

      // Calculate the starting X position to center the line
      uint16_t lineStartX = startX + ((maxWidth - lineWidth) / 2);

      // Draw the line
      for (char ch : line) {
          std::string temp(1, ch);
          const char* charToDraw = temp.c_str();
          Paint_DrawString_EN(lineStartX, currentY, charToDraw, &Barlow22, WHITE, BLACK);
          lineStartX += getCharacterWidth(ch);
      }

      // Move to the next line
      currentY += lineHeight;
  }


  int totalTextWidth2 = calculateTotalTextWidth(part2);  
  uint16_t EPD_4IN2B_V2_WIDTH_2 = 300;
  uint16_t x = (EPD_4IN2B_V2_WIDTH_2 - totalTextWidth2) / 2;

  Paint_DrawString_EN(98, 285, "R", &Barlow22, WHITE, BLACK);
  Paint_DrawString_EN(111, 285, "E", &Barlow22, WHITE, BLACK);
  Paint_DrawString_EN(124, 285, "C", &Barlow22, WHITE, BLACK);
  Paint_DrawString_EN(137, 285, "E", &Barlow22, WHITE, BLACK);
  Paint_DrawString_EN(150, 285, "I", &Barlow22, WHITE, BLACK);
  Paint_DrawString_EN(163, 285, "V", &Barlow22, WHITE, BLACK);
  Paint_DrawString_EN(176, 285, "E", &Barlow22, WHITE, BLACK);
  Paint_DrawString_EN(189, 285, "D", &Barlow22, WHITE, BLACK);
  Paint_DrawString_EN(94, 349, "T", &Barlow22, WHITE, BLACK);
  Paint_DrawString_EN(107, 349, "H", &Barlow22, WHITE, BLACK);
  Paint_DrawString_EN(120, 349, "A", &Barlow22, WHITE, BLACK);
  Paint_DrawString_EN(133, 349, "N", &Barlow22, WHITE, BLACK);
  Paint_DrawString_EN(146, 349, "K", &Barlow22, WHITE, BLACK);
  Paint_DrawString_EN(168, 349, "Y", &Barlow22, WHITE, BLACK);
  Paint_DrawString_EN(181, 349, "O", &Barlow22, WHITE, BLACK);
  Paint_DrawString_EN(194, 349, "U", &Barlow22, WHITE, BLACK);

  std::string str = part2;
  size_t len = str.size();
  uint16_t xb = x;
  std::vector<char> characters = splitString2(str);

  for (uint16_t i = 0; i < len; i++) {
      char charToDraw = characters[i];  // Access character directly
      int charWidth = 15;  // Default width for characters

      // Adjust width and alignment for special characters
      if (charToDraw == '.' || charToDraw == ',') {
          xb -=3;
          charWidth = 10;   // Reduced width for dots and commas
      }

      // Convert char to string for Paint_DrawString_EN
      std::string temp(1, charToDraw);
      Paint_DrawString_EN(xb, 314, temp.c_str(), &Barlow27, WHITE, BLACK);

      // Update X-coordinate for next character
      xb += charWidth;
    }

  printf("EPD_Display\r\n");
  EPD_4IN2B_V2_Display(BlackImage);
  DEV_Delay_ms(2000);

  printf("Goto Sleep...\r\n");
  EPD_4IN2B_V2_Sleep();
  free(BlackImage);
 
  BlackImage = NULL;

  // sendDataToMobile();
}

uint16_t calculateTextWidth(const char *text, const sFONT *font) {
  uint16_t width = 0;
  for (int i = 0; text[i] != '\0'; i++) {
    width += font->Width;
  }
  return width;
}

void formatWithCommas(char *str) {
  char* decimalPos = strchr(str, '.');
  char integerPart[100] = {0};
  char fractionalPart[100] = {0};

  if (decimalPos != NULL) {
    size_t integerLength = decimalPos - str;
    strncpy(integerPart, str, integerLength);
    strcpy(fractionalPart, decimalPos + 1);
  } else {
    strcpy(integerPart, str);
    fractionalPart[0] = '\0';
  }
  // Format the integer part with commas
    int len = strlen(integerPart);
    int commaCount = 0;
    char formattedInteger[200] = {0};  // To hold the formatted integer part
    int formattedIndex = 0;
    
    // Add commas for every 3 digits from the right
    for (int i = len - 1; i >= 0; i--) {
        formattedInteger[formattedIndex++] = integerPart[i];
        commaCount++;

        // Insert a comma after every 3 digits, except at the start
        if (commaCount == 3 && i != 0) {
            formattedInteger[formattedIndex++] = ',';
            commaCount = 0;
        }
    }
    
    // Null-terminate the formatted integer part
    formattedInteger[formattedIndex] = '\0';

    // Reverse the formatted string to restore the correct order
    int start = 0;
    int end = formattedIndex - 1;
    while (start < end) {
        char temp = formattedInteger[start];
        formattedInteger[start] = formattedInteger[end];
        formattedInteger[end] = temp;
        start++;
        end--;
    }

    // Reassemble the final string
    if (fractionalPart[0] != '\0') {
        // If there is a fractional part, concatenate it with the formatted integer part
        sprintf(str, "%s.%s", formattedInteger, fractionalPart);
    } else {
        // No fractional part, just copy the formatted integer part back
        strcpy(str, formattedInteger);
    }
}

void splitString(const char *input, char *part1, char *part2) {
  const char *delimiter = "@";
  char *delimiterPos = strchr(input, delimiter[0]);

  if (delimiterPos != NULL) {
    size_t part1Length = delimiterPos - input;
    strncpy(part1, input, part1Length);
    part1[part1Length] = '\0';  // Null-terminate part1

    strcpy(part2, delimiterPos + 1);  // Copy the part after the delimiter to part2
    convertToUppercase(part1);

    formatWithCommas(part2);

    char tempPart2[100];
    strcpy(tempPart2, "RS.");
    strcat(tempPart2, part2);

    strcpy(part2, tempPart2);

  } else {
    // If no delimiter is found, copy the entire input to part1 and leave part2 empty
    strcpy(part1, input);
    part2[0] = '\0';
    convertToUppercase(part1);
  }
}

void convertToUppercase(char *str) {
  for (int i = 0; str[i] != '\0'; i++) {
    str[i] = toupper(str[i]);
  }
}

void qr1(){
 printf("e-Paper Init and Clear...\r\n");
  EPD_4IN2B_V2_Init();
  //Create a new image cache named IMAGE_BW and fill it with white
  UBYTE *BlackImage; // Red or Yellow
  UWORD Imagesize = ((EPD_4IN2B_V2_WIDTH % 8 == 0) ? (EPD_4IN2B_V2_WIDTH / 8 ) : (EPD_4IN2B_V2_WIDTH / 8 + 1)) * EPD_4IN2B_V2_HEIGHT;
  if ((BlackImage = (UBYTE *)malloc(Imagesize)) == NULL) {
    printf("Failed to apply for black memory...\r\n");
    while(1);
  }
   
  printf("NewImage:BlackImage and RYImage\r\n");
  Paint_NewImage(BlackImage, EPD_4IN2B_V2_WIDTH, EPD_4IN2B_V2_HEIGHT, 270, BLACK);
  //Paint_NewImage(RYImage, EPD_4IN2B_V2_WIDTH, EPD_4IN2B_V2_HEIGHT, 180, WHITE);

  //Select Image
  Paint_SelectImage(BlackImage);
  Paint_Clear(WHITE);
  delay(100);

  Paint_DrawImage(gImage_QRNEW,0,0,400,300);

  uint16_t maxWidth = 265;         // Available width for text (20 to 280)
  uint16_t startX = 20;           // Starting x for each line
  uint16_t startY = 29;           // Starting y for text
  uint16_t lineHeight = 30;
  std::string str2 = "HEMAS PHARMACEUTICALS";
  uint16_t currentY = startY;

  std::vector<std::string> lines;  // Store all lines for later rendering
  std::vector<int> lineWidths;

  std::string currentLine;         // Holds the current line of text
  int currentLineWidth = 0;

  std::string word;                // To accumulate characters of the current word
  for (size_t i = 0; i <= str2.length(); ++i) {
      char c = (i < str2.length()) ? str2[i] : ' '; // Add trailing space for the last word
      if (c == ' ' || c == '\0') {
          if (!word.empty()) {
              int wordWidth = 0;

              // Calculate the total width of the word
              for (char wc : word) {
                  wordWidth += getCharacterWidth(wc);
              }

              int spaceWidth = getCharacterWidth(' ');
              // If the word cannot fit in the current line, move to the next line
              if (!currentLine.empty() && currentLineWidth + spaceWidth + wordWidth > maxWidth) {
                  // Store the completed line and its width
                  lines.push_back(currentLine);
                  lineWidths.push_back(currentLineWidth);

                  // Start a new line
                  currentLine.clear();
                  currentLineWidth = 0;
              }

              // Add the word to the current line
              if (!currentLine.empty()) {
                  currentLine += ' ';
                  currentLineWidth += spaceWidth;
              }
              currentLine += word;
              currentLineWidth += wordWidth;

              word.clear(); // Reset the word
          }
      } else {
          word += c; // Accumulate characters for the word
      }
  }

  // Store the last line
  if (!currentLine.empty()) {
      lines.push_back(currentLine);
      lineWidths.push_back(currentLineWidth);
  }

  // Draw all lines, centering each one
  for (size_t i = 0; i < lines.size(); ++i) {
      const std::string& line = lines[i];
      int lineWidth = lineWidths[i];

      // Calculate the starting X position to center the line
      uint16_t lineStartX = startX + ((maxWidth - lineWidth) / 2);

      // Draw the line
      for (char ch : line) {
          std::string temp(1, ch);
          const char* charToDraw = temp.c_str();
          Paint_DrawString_EN(lineStartX, currentY, charToDraw, &Barlow22, WHITE, BLACK);
          lineStartX += getCharacterWidth(ch);
      }

      // Move to the next line
      currentY += lineHeight;
  }

  printf("EPD_Display\r\n");
  EPD_4IN2B_V2_Display(BlackImage);
  DEV_Delay_ms(2000);
 

  //printf("Clear...\r\n");
  //EPD_4IN2B_V2_Clear();

  printf("Goto Sleep...\r\n");
  EPD_4IN2B_V2_Sleep();
  free(BlackImage);
 
  BlackImage = NULL;
}

void sendDataToMobile() {
    if (deviceConnected) {
        String dataToSend = "Part1: " + String(part1) + ", Part2: " + String(part2);
        pCharacteristic->setValue(dataToSend.c_str());
        pCharacteristic->notify();
        Serial.println("Data sent to mobile: " + dataToSend);
        delay(100);
    } else {
        Serial.println("Device not connected. Unable to send data.");
    }
}