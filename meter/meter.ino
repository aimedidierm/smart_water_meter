#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#define SS_PIN 10
#define RST_PIN 9
MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.

int Interrupt = 0;
int sensorPin       = 2;
#define Valve A2
#define red A1
float calibrationFactor = 90; 
volatile byte pulseCount =0;  
float flowRate = 0.0;
unsigned int flowMilliLitres =0;
unsigned long totalMilliLitres = 270;
unsigned long oldTime = 0;

LiquidCrystal_I2C lcd(0x27,20,4);  // set the LCD address to 0x27 for a 16 chars and 2 line display

int drinkvolume=2000;
String tagID = "";
void setup() 
{
  lcd.init();                      // initialize the lcd 
  lcd.init();
  SPI.begin();  
  Serial.begin(115200);   // Initiate a serial communication
  SPI.begin();      // Initiate  SPI bus
  mfrc522.PCD_Init();   // Initiate MFRC522
  pinMode(Valve , OUTPUT);
  digitalWrite(Valve, HIGH);
  pinMode(sensorPin, INPUT);
  digitalWrite(sensorPin, HIGH);
  attachInterrupt(Interrupt, pulseCounter, FALLING);
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Smart water");
  lcd.setCursor(0,1);
  lcd.print("meter");
  delay(5000);

}

void loop() 
{
  if (getID()){
    //Read data on card
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Reading data");
    delay(2000);
   // readdata();
  }/*
  digitalWrite(Valve, HIGH);
    while(drinkvolume>20){
      if((millis() - oldTime) > 1000)    // Only process counters once per second
  { 
    detachInterrupt(Interrupt);
    flowRate = ((1000.0 / (millis() - oldTime)) * pulseCount) / calibrationFactor;
    oldTime = millis();
    flowMilliLitres = (flowRate / 60) * 1000;
    drinkvolume -= flowMilliLitres;
 
    unsigned int frac;
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(drinkvolume);
    pulseCount = 0;
    attachInterrupt(Interrupt, pulseCounter, FALLING);
  }
      }
    digitalWrite(Valve,LOW);*/
}
/*
readdata(){
  
  }
  */

void pulseCounter()
{
  pulseCount++;
}
boolean getID(){
  if(!mfrc522.PICC_IsNewCardPresent()){
    return false;
    }
  if(!mfrc522.PICC_ReadCardSerial()){
    return false;
    }
    tagID = "";
    for (uint8_t i = 0; i < 4; i++){
      tagID.concat(String(mfrc522.uid.uidByte[i], HEX));
      }
      tagID.toUpperCase();
      mfrc522.PICC_HaltA();
      return true;
  }
