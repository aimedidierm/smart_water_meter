#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
const uint8_t RST_PIN = 9;
const uint8_t SS_PIN = 10;
LiquidCrystal_I2C lcd(0x27, 20, 4);
MFRC522 mfrc522(SS_PIN, RST_PIN);


void(* resetFunc) (void) = 0;

const byte ROWS = 4; //four rows
const byte COLS = 3; //four columns
//define the cymbols on the buttons of the keypads
char newNum[12] = "";
String water = "";
//define the cymbols on the buttons of the keypads
char keys[ROWS][COLS] = {

  {'1', '2', '3'},

  {'4', '5', '6'},

  {'7', '8', '9'},

  {'1', '0', '#'}

};
byte rowPins[ROWS] = {2, 3, 4, 5}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {6, 7, 8};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);
MFRC522::MIFARE_Key key;
int blockNum = 4;
byte block_data[16];
byte bufferLen = 18;
byte readBlockData[18];
MFRC522::StatusCode status;
const int green =  A2;
const int red =  A3;
void setup()
{
  Serial.begin(9600);
  SPI.begin();
  mfrc522.PCD_Init();
  Serial.println("Scan a MIFARE 1K Tag to write data...");
  Serial.begin(9600);
  pinMode(green, OUTPUT);
  pinMode(red, OUTPUT);
  digitalWrite(green, LOW);
  digitalWrite(red, LOW);
  lcd.init();
  lcd.init();
  lcd.backlight();
  SPI.begin();
  mfrc522.PCD_Init();
  lcd.setCursor(0, 0);
  lcd.print("Recharging");
  lcd.setCursor(0, 1);
  lcd.print("Machine");
  delay(3000);
  lcd.clear();
  lcd.print("Tap your card");
  delay(1000);
}
void loop()
{
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return;
  }
  /* Select one of the cards */
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;
  }
  Serial.print("\n");
  lcd.clear();
  lcd.print("Card Detected");
  delay(1000);
  Serial.println("**Card Detected**");
  Serial.print(F("Card UID:"));
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
  }
  Serial.print("\n");
  /* Print type of card (for example, MIFARE 1K) */
  Serial.print(F("PICC type: "));
  MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
  Serial.println(mfrc522.PICC_GetTypeName(piccType));
  int i = 0, j = 0, m = 0, x = 0, s = 0, k = 0;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Enter amount");
  lcd.setCursor(0, 1);
  lcd.print("and press#");
  delay(2000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Enter amount");
  lcd.setCursor(0, 1);
  lcd.print("and press#");
  delay(2000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Enter amount");

  for (i = 2; i > 0; i++)
  {
    lcd.setCursor(0, 1);
    int key = keypad.getKey();

    if (key != NO_KEY && key != '#' && key != '*')
    {
      newNum[j] = key;
      newNum[j + 1] = '\0';
      j++;
      lcd.clear();
      lcd.setCursor(0, 1);
      lcd.print(newNum);
    }

    if (key == '#' && j > 0)
    {
      j = 0;
      break;
    }
    delay(100);
  }
  int amount = atoi(newNum);
  blockNum = 4;
  toBlockDataArray(String(amount));
  WriteDataToBlock(blockNum, block_data);
  ReadDataFromBlock(blockNum, readBlockData);
  dumpSerial(blockNum, readBlockData);
}
/****************************************************************************************************
   Writ() function
 ****************************************************************************************************/
void WriteDataToBlock(int blockNum, byte blockData[])
{
  Serial.print("Writing data on block ");
  Serial.print(blockNum);
  //------------------------------------------------------------------------------
  /* Authenticating the desired data block for write access using Key A */
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockNum, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Authentication failed for Write: ");
    lcd.clear();
    lcd.print("Fail");
    digitalWrite(red, HIGH);
    delay(4000);
    resetFunc();
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
  //------------------------------------------------------------------------------
  else {
    Serial.println("Authentication success");
  }
  //------------------------------------------------------------------------------
  /* Write data to the block */
  status = mfrc522.MIFARE_Write(blockNum, blockData, 16);
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Writing to Block failed: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
  else
  { Serial.println("Data was written into Block successfully");
    lcd.clear();
    lcd.print("Money loaded");
    digitalWrite(green, HIGH);
    delay(4000);
    resetFunc();
  }
  //------------------------------------------------------------------------------
}
/****************************************************************************************************
   ReadDataFromBlock() function
 ****************************************************************************************************/
void ReadDataFromBlock(int blockNum, byte readBlockData[])
{
  Serial.print("Reading data from block ");
  Serial.println(blockNum);
  //------------------------------------------------------------------------------
  /* Authenticating the desired data block for Read access using Key A */
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockNum, &key, &(mfrc522.uid));
  //------------------------------------------------------------------------------
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Authentication failed for Read: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
  else {
    Serial.println("Authentication success");
  }
  //------------------------------------------------------------------------------
  /* Reading data from the Block */
  status = mfrc522.MIFARE_Read(blockNum, readBlockData, &bufferLen);
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Reading failed: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
  else {
    Serial.println("Block was read successfully");
  }
  //------------------------------------------------------------------------------
}



/****************************************************************************************************
   dumpSerial() function
 ****************************************************************************************************/
void dumpSerial(int blockNum, byte blockData[])
{
  Serial.print("\n");
  Serial.print("Data in Block:");
  Serial.print(blockNum);
  Serial.print(" --> ");
  for (int j = 0 ; j < 16 ; j++) {
    Serial.write(readBlockData[j]);
  }
  Serial.print("\n"); Serial.print("\n");
}


/****************************************************************************************************
   dumpSerial() function
 ****************************************************************************************************/
void toBlockDataArray(String str)
{
  byte len = str.length();
  if (len > 16)
    len = 16;
  for (byte i = 0; i < len; i++)
    block_data[i] = str[i];
  for (byte i = len; i < 16; i++)
    block_data[i] = ' ';
}