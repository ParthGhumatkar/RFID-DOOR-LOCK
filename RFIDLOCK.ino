#include <MFRC522.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <SoftwareSerial.h>
#include <Servo.h>
#include <SPI.h>
SoftwareSerial SIM900(3, 4); 
MFRC522 mfrc522(10, 9); 
LiquidCrystal_I2C lcd(0x27, 16, 2);
Servo servo;
constexpr uint8_t greenLed = 7;
constexpr uint8_t redLed = 6;
constexpr uint8_t servoPin = 8;
constexpr uint8_t buzzerPin = 5;
char initial_password[4] = {'1', '9', '7', '3'};  
String tagUID = "";  
char password[4];  
boolean RFIDMode = true; 
boolean NormalMode = true; 
char key_pressed = 0; 
uint8_t i = 0; 
const byte rows = 4;
const byte columns = 4;
// Keypad pin map
char hexaKeys[rows][columns] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte row_pins[rows] = {A0, A1, A2, A3};
byte column_pins[columns] = {2, 1, 0};

Keypad keypad_key = Keypad( makeKeymap(hexaKeys), row_pins, column_pins, rows, columns);
void setup() {
  // Arduino Pin configuration
  pinMode(buzzerPin, OUTPUT);
  pinMode(redLed, OUTPUT);
  pinMode(greenLed, OUTPUT);
  servo.attach(servoPin);  
  servo.write(0);
  lcd.begin();  
  lcd.backlight();
  SPI.begin();     
  mfrc522.PCD_Init();   
  SIM900.begin(19200);
  SIM900.print("AT+CMGF=1\r");
  delay(100);
  SIM900.print("AT+CNMI=2,2,0,0,0\r");
  delay(100);
  lcd.clear();
}
void loop() {
  if (NormalMode == false) {
    receive_message();
  }
  else if (NormalMode == true) {
    if (RFIDMode == true) {
      receive_message();
      lcd.setCursor(0, 0);
      lcd.print("   Door Lock");
      lcd.setCursor(0, 1);
      lcd.print(" Scan Your Tag ");
      if ( ! mfrc522.PICC_IsNewCardPresent()) {
        return;
      }
      if ( ! mfrc522.PICC_ReadCardSerial()) {
        return;
      }
      //Reading from the card
      String tag = "";
      for (byte j = 0; j < mfrc522.uid.size; j++)
      {
        tag.concat(String(mfrc522.uid.uidByte[j] < 0x10 ? " 0" : " "));
        tag.concat(String(mfrc522.uid.uidByte[j], HEX));
      }
      tag.toUpperCase();
  
      if (tag.substring(1) == tagUID)
      {
  
        lcd.clear();
        lcd.print("Tag Matched");
        digitalWrite(greenLed, HIGH);
        delay(3000);
        digitalWrite(greenLed, LOW);
        lcd.clear();
        lcd.print("Enter Password:");
        lcd.setCursor(0, 1);
        RFIDMode = false; 
      }
      else
      {
     
        lcd.print("Access Denied");
        digitalWrite(buzzerPin, HIGH);
        send_message("");
        delay(3000);
        digitalWrite(buzzerPin, LOW);
        lcd.clear();
      }
    }
    if (RFIDMode == false) {
      key_pressed = keypad_key.getKey();
      if (key_pressed)
      {
        password[i++] = key_pressed; 
        lcd.print("*");
      }
      if (i == 4)
      {
        delay(200);
        if (!(strncmp(password, initial_password, 4))) 
        {
          lcd.clear();
          lcd.print("Pass Accepted");
          servo.write(90); // Door Opened
          digitalWrite(greenLed, HIGH);
          send_message("");
          delay(3000);
          digitalWrite(greenLed, LOW);
          servo.write(0); 
          lcd.clear();
          i = 0;
          RFIDMode = true; 
        }
        else    
        {
          lcd.clear();
          lcd.print("Wrong Password");
          digitalWrite(buzzerPin, HIGH);
          send_message("");
          delay(3000);
          digitalWrite(buzzerPin, LOW);
          lcd.clear();
          i = 0;
          RFIDMode = true; 
        }
      }
    }
  }
}

void receive_message()
{
  char incoming_char = 0; 
  String incomingData;   
  
  if (SIM900.available() > 0)
  {
    incomingData = SIM900.readString(); 
    delay(10);
  }
  if (incomingData.indexOf("open") >= 0)
  {
    servo.write(90);
    NormalMode = true;
    send_message("Opened");
    delay(10000);
    servo.write(0);
  }
  if (incomingData.indexOf("close") >= 0)
  {
    NormalMode = false;
    send_message("Closed");
  }
  incomingData = "";
}
void send_message(String message)
{
  SIM900.println("AT+CMGF=1");    
  delay(100);
  SIM900.println("AT+CMGS=\"+\"");
  delay(100);
  SIM900.println(message);   
  delay(100);
  SIM900.println((char)26);
  delay(100);
  SIM900.println();
  delay(1000);
}
