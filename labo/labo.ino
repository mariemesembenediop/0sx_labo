#include <LCD_I2C.h> 
#define BTN_PIN 2           
#define LDR_PIN A0          
#define LED_PIN 8           
#define JOY_X_PIN A1       
#define JOY_Y_PIN A2       
#define LCD_ADDRESS 0x27    
#define LCD_COLUMNS 16     
#define LCD_ROWS 2         
#define SERIAL_BAUD 115200 
#define INIT_DELAY 3000     
#define SERIAL_INTERVAL 100 
#define LDR_THRESHOLD 50    
#define LDR_DELAY 5000      
#define MIN_SPEED -25      
#define MAX_SPEED 120       
#define MIN_ANGLE -90       
#define MAX_ANGLE 90        

LCD_I2C lcd(LCD_ADDRESS, LCD_COLUMNS, LCD_ROWS); 
unsigned long currentTime = 0;
unsigned long lastSerialTime = 0; 
unsigned long lastLDRTime = 0;    
unsigned long initStartTime = 0;  
bool pharesOn = false;
bool displayPage1 = true; 
bool buttonPressed = false;


byte customChar5[8] = {
  0b11100,  
  0b10000,  
  0b11100,  
  0b00111,  
  0b11100,  
  0b00111,  
  0b00001,  
  0b00111   
};

void setup() {
  initializePins();
  initializeLCD();
  initializeSerial();
  initStartTime = millis();
}



void initializePins() {
  pinMode(LED_PIN, OUTPUT);
  pinMode(BTN_PIN, INPUT_PULLUP);
}

void initializeLCD() {
  lcd.begin();
  lcd.backlight(); 
  lcd.createChar(0, customChar5); 
  displayInitialScreen();
}

void initializeSerial() {
  Serial.begin(SERIAL_BAUD);
}

void displayInitialScreen() {
  lcd.setCursor(0, 0);
  lcd.print("Diop");
  lcd.setCursor(0, 1);
  lcd.write(0);
 
  lcd.setCursor(14, 1);
  lcd.print("55"); 
  
}

void handleButtonPress() {
  if (digitalRead(BTN_PIN) == LOW && !buttonPressed) {
    buttonPressed = true;
    displayPage1 = !displayPage1;
  } else if (digitalRead(BTN_PIN) == HIGH && buttonPressed) {
    buttonPressed = false;
  }
}

void displayLDRPage() {
  int brightness = readLDR();
  controlPhares(brightness);


  lcd.setCursor(0, 0);
  lcd.print("Luminosite: ");
  lcd.print(brightness);
  lcd.print("% ");
  lcd.setCursor(0, 1);
  lcd.print("Phares: ");
  lcd.print(pharesOn ? "ON    " : "OFF     ");
}

int readLDR() {
  int ldrValue = analogRead(LDR_PIN);
  return map(ldrValue, 0, 1023, 0, 100);
}

void controlPhares(int brightness) {
  if (brightness < LDR_THRESHOLD && !pharesOn) {
    if (currentTime - lastLDRTime >= LDR_DELAY) {
      digitalWrite(LED_PIN, HIGH);
      pharesOn = true;
      lastLDRTime = currentTime;
    }
  } else if (brightness > LDR_THRESHOLD && pharesOn) {
    if (currentTime - lastLDRTime >= LDR_DELAY) {
      digitalWrite(LED_PIN, LOW);
      pharesOn = false;
      lastLDRTime = currentTime;
    }
  }
}

void displayJoystickPage() {
  int speed = readJoystickSpeed();
  int direction = readJoystickDirection();
  char dirChar = (direction < 0) ? 'D' : 'G';

 // lcd.clear();
 if(speed>0){
  lcd.setCursor(0, 0);
  lcd.print("Avance: ");
 }else{
  lcd.setCursor(0, 0);
  lcd.print("Recule: ");
 }
 
  lcd.print(speed);
  lcd.print(" km/h ");
  lcd.setCursor(0, 1);
  lcd.print("Direction: ");
  lcd.print(dirChar);
}

int readJoystickSpeed() {
  int yValue = analogRead(JOY_Y_PIN);
  return map(yValue, 0, 1023, MIN_SPEED, MAX_SPEED);
}

int readJoystickDirection() {
  int xValue = analogRead(JOY_X_PIN);
  return map(xValue, 0, 1023, MIN_ANGLE, MAX_ANGLE);
}

void sendSerialData() {
  int xValue = analogRead(JOY_X_PIN);
  int yValue = analogRead(JOY_Y_PIN);
  int sysValue = pharesOn ? 1 : 0;

  Serial.print("etd:2415655,x:");
  Serial.print(xValue);
  Serial.print(",y:");
  Serial.print(yValue);
  Serial.print(",sys:");
  Serial.println(sysValue);
}
void loop() {
  currentTime = millis(); 
 
  if (currentTime - initStartTime < INIT_DELAY) {
   
    return; 
  }
 

  handleButtonPress();
  
  if (displayPage1) {
    displayLDRPage(); 
  } else {
    displayJoystickPage(); 
  }

  if (currentTime - lastSerialTime >= SERIAL_INTERVAL) {
    sendSerialData();
    lastSerialTime = currentTime;
  }
}