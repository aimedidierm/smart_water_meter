#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <MFRC522.h>

int lcdColumns = 16;
int lcdRows = 2;
byte block;
byte len;
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);
int Interrupt = 0;
int sensorPin = 2;
#define Valve A3
#define red 
#define green 5
#define buzzer 3
float calibrationFactor = 90;
volatile byte pulseCount = 0;
float flowRate = 0.0;
unsigned int flowMilliLitres = 0;
unsigned long totalMilliLitres = 270;
unsigned long oldTime = 0;
byte buffer1[18];
#define SS_PIN 10
#define RST_PIN 9
MFRC522 mfrc522(SS_PIN, RST_PIN); // Create MFRC522 instance

void (*resetFunc)(void) = 0;

MFRC522::MIFARE_Key key;
MFRC522::StatusCode status;
uint32_t cardValue = 0;
int drinkvolume = 0; // Declare drinkvolume variable here

void setup()
{
  SPI.begin(); // Init SPI bus
  mfrc522.PCD_Init();
  pinMode(Valve, OUTPUT);
  digitalWrite(Valve, HIGH);
  pinMode(sensorPin, INPUT);
  digitalWrite(sensorPin, HIGH);
  attachInterrupt(Interrupt, pulseCounter, FALLING);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Prepayment water");
  lcd.setCursor(0, 1);
  lcd.print("meter");
  delay(3000);
  Serial.begin(9600);
}

void loop()
{
  for (byte i = 0; i < 6; i++)
    key.keyByte[i] = 0xFF;
  byte block;
  byte len;
  if (!mfrc522.PICC_IsNewCardPresent())
  {
    return;
  }
  if (!mfrc522.PICC_ReadCardSerial())
  {
    return;
  }
  lcd.clear();
  lcd.print("Card Detected");
  delay(1000);
  byte buffer1[18];
  block = 4;
  len = 18;
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 4, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK)
  {
    Serial.print(F("Authentication failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
  status = mfrc522.MIFARE_Read(block, buffer1, &len);
  if (status != MFRC522::STATUS_OK)
  {
    lcd.clear();
    lcd.print("Reading failed: ");
    delay(1000);
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
  
  uint32_t cardValue = 0;
  for (uint8_t i = 0; i < 4; i++) {
    cardValue |= (uint32_t)buffer1[i] << (8 * (3 - i));
  }

  lcd.clear();

  // Convert the value to an integer and assign it to drinkvolume
  drinkvolume = cardValue / 5; // Assuming conversion factor 5, adjust as needed
  lcd.setCursor(0, 0);
  lcd.print(drinkvolume);
  lcd.print("Rwf");
  lcd.setCursor(0, 1);
  lcd.print("Money loaded");
  delay(3000);

  // Write 0 data onto the card
  int newAmount = 0;

  // Convert the new amount back to byte array
  byte newBlockData[16];
  for (int i = 0; i < 16; i++) {
    if (i % 2 == 0) {
      newBlockData[i] = (newAmount >> 8) & 0xFF;
    } else {
      newBlockData[i] = newAmount & 0xFF;
    }
  }

  // Write data to the block
  status = mfrc522.MIFARE_Write(blockNum, newBlockData, 16);
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Writing to Block failed: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  } else {
    Serial.println("Data was written into Block successfully");
    lcd.clear();
    lcd.print("Money added");
    digitalWrite(green, HIGH);
    delay(4000);
    resetFunc();
  }
  
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1(); 
  waterout();
}

void waterout()
{
  digitalWrite(Valve, LOW);
  while (drinkvolume > 20)
  {
    if ((millis() - oldTime) > 1000) // Only process counters once per second
    {
      detachInterrupt(Interrupt);
      flowRate = ((1000.0 / (millis() - oldTime)) * pulseCount) / calibrationFactor;
      oldTime = millis();
      flowMilliLitres = (flowRate / 60) * 1000;
      drinkvolume -= flowMilliLitres;

      unsigned int frac;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(drinkvolume);
      lcd.print("ml");
      pulseCount = 0;
      attachInterrupt(Interrupt, pulseCounter, FALLING);
    }
  }
  digitalWrite(Valve, HIGH);
  drinkvolume = 0;
}

void pulseCounter()
{
  pulseCount++;
}

