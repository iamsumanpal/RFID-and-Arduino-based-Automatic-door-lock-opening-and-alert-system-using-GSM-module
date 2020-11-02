#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>
// Create instances
LiquidCrystal_I2C lcd(0x27, 16, 2);
MFRC522 mfrc522(10, 9); // MFRC522 mfrc522(SS_PIN, RST_PIN)
Servo sg90;
// Initialize Pins for led's, servo and buzzer
// Blue LED is connected to 5V
constexpr uint8_t greenLed = 7;
constexpr uint8_t redLed = 6;
constexpr uint8_t servoPin = 8;
constexpr uint8_t buzzerPin = 5;
String tagUID = "29 B9 ED 23";  // String to store UID of tag. 
boolean RFIDMode = true;

void setup() {
  // Arduino Pin configuration
  pinMode(buzzerPin, OUTPUT);
  pinMode(redLed, OUTPUT);
  pinMode(greenLed, OUTPUT);
  
  sg90.attach(servoPin);  //Declare pin 9 for servo
  sg90.write(0); // Set initial position at 90 degrees
  lcd.begin();   // LCD screen
  lcd.backlight();
  SPI.begin();      // Init SPI bus
  mfrc522.PCD_Init();   // Init MFRC522
  

   // Arduino communicates with SIM900 GSM shield at a baud rate of 19200
  // Make sure that corresponds to the baud rate of your module
  SIM900.begin(19200);

  // AT command to set SIM900 to SMS mode
  SIM900.print("AT+CMGF=1\r");
  delay(100);
  // Set module to send SMS data to serial out upon receipt
  SIM900.print("AT+CNMI=2,2,0,0,0\r");
  delay(100);
  lcd.clear(); // Clear LCD screen
}



void loop() {

  //System will first look for mode
  if (RFIDMode == true) {
  // Function to receive message
      receive_message();

  lcd.setCursor(0, 0);
  lcd.print("   Door Lock");
  lcd.setCursor(0, 1);
  lcd.print(" Scan Your Tag ");
  lcd.setCursor(0, 0);
  lcd.print(" RFID Door Lock");
  lcd.setCursor(0, 1);
  lcd.print(" Show Your Tag ");
  // Look for new cards
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return;
  }
  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;
  }
  //Reading from the card
  String tag = "";
  for (byte i = 0; i < mfrc522.uid.size; i++)
  {
    tag.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
    tag.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  tag.toUpperCase();
  //Checking the card
  if (tag.substring(1) == tagUID) //change here the UID of the card/cards that you want to give access
  {
    // If UID of tag is matched.
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Access Granted");
    lcd.setCursor(0, 1);
    lcd.print("Door Opened");
    sg90.write(90);
    digitalWrite(greenLed, HIGH);
    send_message("Door Opened \nIf it was't you, type 'close' to halt the system.");
    delay(3000);
    digitalWrite(greenLed, LOW);
    sg90.write(0);
    lcd.clear();
    i = 0;
    RFIDMode = true; // Make RFID mode true
  }
  else
  {
    // If UID of tag is not matched.
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Wrong Tag Shown");
    lcd.setCursor(0, 1);
    lcd.print("Access Denied");
    digitalWrite(buzzerPin, HIGH);
    digitalWrite(redLed, HIGH);
    send_message("Someone Tried with the wrong Password \nType 'close' to halt the system.");
    delay(3000);
    digitalWrite(buzzerPin, LOW);
    digitalWrite(redLed, LOW);
    lcd.clear();
    i = 0;
    RFIDMode = true;  // Make RFID mode true
  }
}

// Receiving the message
void receive_message()
{
  char incoming_char = 0; //Variable to save incoming SMS characters
  String incomingData;   // for storing incoming serial data
  
  if (SIM900.available() > 0)
  {
    incomingData = SIM900.readString(); // Get the incoming data.
    delay(10);
  }
  // if received command is to open the door
  if (incomingData.indexOf("open") >= 0)
  {
    sg90.write(90);
    NormalMode = true;
    send_message("Opened");
    delay(10000);
    sg90.write(0);
  }
  // if received command is to halt the system
  if (incomingData.indexOf("close") >= 0)
  {
    NormalMode = false;
    send_message("Closed");
  }
  incomingData = "";
}
// Function to send the message
void send_message(String message)
{
  SIM900.println("AT+CMGF=1");    //Set the GSM Module in Text Mode
  delay(100);
  SIM900.println("AT+CMGS=\"+XXXXXXXXXXXX\"");
  delay(100);
  SIM900.println(message);   // The SMS text you want to send
  delay(100);
  SIM900.println((char)26);  // ASCII code of CTRL+Z
  delay(100);
  SIM900.println();
  delay(1000);
}
