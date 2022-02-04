#include <Wire.h>
#include "MFRC522_I2C.h"

#define RST_PIN 5 // Arduino UNO Pin
// #define RST_PIN 14 // D5 Pin on NodeMCU
const uint8_t addr_1 = 0x35;
const uint8_t addr_2 = 0x2f;
// 0x28 is i2c address on SDA. Check your address with i2cscanner if not match.
MFRC522_I2C mfrc522(addr, RST_PIN);   // Create MFRC522 instance.
MFRC522_I2C::MIFARE_Key key;
void setup() {
  Serial.begin(115200);           // Initialize serial communications with the PC
  Wire.begin();                   // Initialize I2C
  mfrc522.PCD_Init();             // Init MFRC522
  ShowReaderDetails();            // Show details of PCD - MFRC522 Card Reader details
  Serial.println(F("Scan PICC to see UID, type, and data blocks..."));

  Serial.println(F("Scan a MIFARE Ultralight PICC to demonstrate read and write."));
  Serial.println();

  Serial.println(F("BEWARE: Data will be written to the PICC, in sector #1"));

}

void loop() {
  // Look for new cards, and select one if present
  if ( ! mfrc522.PICC_IsNewCardPresent() || ! mfrc522.PICC_ReadCardSerial() ) {
    delay(50);
    return;
  }
  
  // Now a card is selected. The UID and SAK is in mfrc522.uid.
  
  // Dump UID
  Serial.print(F("Card UID:"));
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
  } 
  Serial.println();
    // In this sample we use the second sector,
    // that is: sector #1, covering block #4 up to and including block #7
    //byte sector         = 1;
    byte pageAddr      = 10;
    byte dataBlock[]    = {
        0x0a, 0x0b, 0x0c, 0x0d, //  1,  2,   3,  4,
    };
    byte trailerBlock   = 7;
    MFRC522_I2C::StatusCode status;
    byte buffer[18]; //buffer size is at least 18
    byte size = sizeof(buffer);

    // Show the whole sector as it currently is
    Serial.println(F("Current data in Tag:"));
    mfrc522.PICC_DumpMifareUltralightToSerial();
    Serial.println();

    // Read data from the block
    Serial.print(F("Reading data from page ")); Serial.print(pageAddr);
    Serial.println(F(" ..."));
    status = (MFRC522_I2C::StatusCode) mfrc522.MIFARE_Read(pageAddr, buffer, &size);
    if (status != MFRC522_I2C::STATUS_OK) {
        Serial.print(F("MIFARE_Read() failed: "));
        Serial.println(mfrc522.GetStatusCodeName(status));
    }
    Serial.print(F("Data in page ")); Serial.print(pageAddr); Serial.println(F(":"));
    dump_byte_array(buffer, 4); Serial.println();
    Serial.println();

    // Write data to the block
    Serial.print(F("Writing data into page ")); Serial.print(pageAddr);
    Serial.println(F(" ..."));
    dump_byte_array(dataBlock, 4); Serial.println();
    status = (MFRC522_I2C::StatusCode) mfrc522.MIFARE_Ultralight_Write(pageAddr, dataBlock, 4);
    if (status != MFRC522_I2C::STATUS_OK) {
        Serial.print(F("MIFARE_Write() failed: "));
        Serial.println(mfrc522.GetStatusCodeName(status));
    }
    Serial.println();

    // Read data from the block (again, should now be what we have written)
    Serial.print(F("Reading data from page ")); Serial.print(pageAddr);
    Serial.println(F(" ..."));
    status = (MFRC522_I2C::StatusCode) mfrc522.MIFARE_Read(pageAddr, buffer, &size);
    if (status != MFRC522_I2C::STATUS_OK) {
        Serial.print(F("MIFARE_Read() failed: "));
        Serial.println(mfrc522.GetStatusCodeName(status));
    }
    Serial.print(F("Data in page ")); Serial.print(pageAddr); Serial.println(F(":"));
    dump_byte_array(buffer, 4); Serial.println();

    // Check that data in block is what we have written
    // by counting the number of bytes that are equal
    Serial.println(F("Checking result..."));
    byte count = 0;
    for (byte i = 0; i < 4; i++) {
        // Compare buffer (= what we've read) with dataBlock (= what we've written)
        if (buffer[i] == dataBlock[i])
            count++;
    }
    Serial.print(F("Number of bytes that match = ")); Serial.println(count);
    if (count == 4) {
        Serial.println(F("Success :-)"));
    } else {
        Serial.println(F("Failure, no match :-("));
        Serial.println(F("  perhaps the write didn't work properly..."));
    }
    Serial.println();

    // Dump the sector data
    Serial.println(F("Current data in Tag:"));
    mfrc522.PICC_DumpMifareUltralightToSerial();
    Serial.println();

    // Halt PICC
    mfrc522.PICC_HaltA();
    // Stop encryption on PCD
    mfrc522.PCD_StopCrypto1();
  // Dump debug info about the card; PICC_HaltA() is automatically called
  //mfrc522.PICC_DumpToSerial(&(mfrc522.uid));
}

void ShowReaderDetails() {
  // Get the MFRC522 software version
  byte v = mfrc522.PCD_ReadRegister(mfrc522.VersionReg);
  Serial.print(F("MFRC522 Software Version: 0x"));
  Serial.print(v, HEX);
  if (v == 0x91)
    Serial.print(F(" = v1.0"));
  else if (v == 0x92)
    Serial.print(F(" = v2.0"));
  else
    Serial.print(F(" (unknown)"));
  Serial.println("");
  // When 0x00 or 0xFF is returned, communication probably failed
  if ((v == 0x00) || (v == 0xFF)) {
    Serial.println(F("WARNING: Communication failure, is the MFRC522 properly connected?"));
  }
}
/**
 * Helper routine to dump a byte array as hex values to Serial.
 */
void dump_byte_array(byte *buffer, byte bufferSize) {
    for (byte i = 0; i < bufferSize; i++) {
        Serial.print(buffer[i] < 0x10 ? " 0" : " ");
        Serial.print(buffer[i], HEX);
    }
}
