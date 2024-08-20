// Code For LCD
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);
//End Code For LCD

//Code For Keypad
#include <Keypad.h>

// Define the number of rows and columns of the keypad
const byte ROWS = 4; 
const byte COLS = 4; 

// Define the keymap
char keys[ROWS][COLS] = {
  {'D','#','0','*'},
  {'C','9','8','7'},
  {'B','6','5','4'},
  {'A','3','2','1'}
};

// Connect keypad ROW0, ROW1, ROW2, ROW3 to these Arduino pins.
byte rowPins[ROWS] ={8, 9, 10, 11};  


// Connect keypad COL0, COL1, COL2, COL3 to these Arduino pins.
byte colPins[COLS] = {4, 5, 6, 7}; 

// Create the keypad object
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);
//End Code For Keypad

//Code For Servo
#include <Servo.h>

Servo myServo;  // Membuat objek Servo
//End Code For Servo

//Code For Load Cell
#include "HX711.h"
#define DOUT 2
#define CLK 3

float calibration_factor = -95; //Nilai awal perkiraan
int units;
float ounces;
int beratAwal;
int bebanKeluar;

HX711 scale; 

//End Code For Load Cell

char key;
String stringKey = "";
int iterasiKeluaran = 0;
int iterasi = 0;
int akhirIterasi = 0;
int ledPin = 12;
int pbButton = 13;
int button_state;
bool wasBelowThreshold = true;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();
  Serial.begin(9600);
  lcd.setCursor(3,0);
  lcd.print("KKN UNDIP");
  lcd.setCursor(6,1);
  lcd.print("2024");
  delay(2000);
  lcd.clear();

  myServo.attach(A0);
  myServo.write(0);

  scale.begin(DOUT, CLK);
  scale.set_scale(calibration_factor);
  scale.tare();

}

void loop() {
  // put your main code here, to run repeatedly:
  key = keypad.getKey();

  lcd.setCursor(0, 0);
  lcd.print("Set : ");
  lcd.print(stringKey);
  lcd.print(" gram         ");
  Serial.println(key);
  
  if(iterasi==0){
    stringKey="0";
  } else if(iterasi==1){
    stringKey = stringKey.substring(1);
    iterasi++;
  }

  if(key >= '0' && key <= '9'){
    iterasi++;
    stringKey += key;
  }

  if(key == '*'){
    iterasi = 0;
  }
  
  if(key == '#'){
    lcd.clear();
    readyToPack();
  }
}

void readyToPack(){
  scale.set_scale(calibration_factor);
  key = keypad.getKey();
  button_state = digitalRead(pbButton);
  units = scale.get_units(), 4;

  
  //Serial.println(key);
  lcd.setCursor(0, 0);
  lcd.print("Berat : ");
  lcd.print(units);
  lcd.print(" gram     ");
  lcd.setCursor(0, 1);
  lcd.print("Iterasi : ");
  lcd.print(iterasiKeluaran);
  //lcd.setCursor(0, 1);
  //lcd.print(" (");
  //lcd.print(akhirIterasi);
  //lcd.print(") ");
  //lcd.print(" gram     ");

  if(key == '*'){
    myServo.write(0);
    lcd.clear();
    return;
  }

  akhirIterasi = stringKey.toInt()/100;

  if (iterasiKeluaran == akhirIterasi+1){
    lcd.clear();
    closeTheDoor();
  }

  if(stringKey.toInt() > 100){
    myServo.write(60);
    if (iterasiKeluaran == akhirIterasi){
      lcd.clear();
      lastIterasi();
    }
    
    if(stringKey.toInt()>100){
      if(units >= 100){
        lcd.clear();
        repeatKeluaran();
      }
    }

  }

  if(stringKey.toInt() <= 100){
    myServo.write(60);
    if(units >= stringKey.toInt()){
      lcd.clear();
      closeTheDoor();
    }
  }

  readyToPack();
}

void closeTheDoor(){
  units = scale.get_units(), 4;
  key = keypad.getKey();

  delay(500);
  myServo.write(0);
  lcd.setCursor(4, 0);
  lcd.print("SELESAI!");
  lcd.setCursor(3, 1);
  lcd.print("MAU LAGI?");

  if(key == '#'){
    if(units <= 2 && units >= -5){
      lcd.clear();
      iterasiKeluaran = 0;
      wasBelowThreshold = true;
      return;
    }
  }

  closeTheDoor();
}

void repeatKeluaran(){
  units = scale.get_units(), 4;
  delay(500);
  myServo.write(0);
  if(wasBelowThreshold==true){
    iterasiKeluaran += 1;
    wasBelowThreshold = false;
  }
  lcd.setCursor(0, 0);
  lcd.print("Berat : ");
  lcd.print(units);
  lcd.print(" gram     ");
  lcd.setCursor(0, 1);
  lcd.print("Iterasi : ");
  lcd.print(iterasiKeluaran);
  if(units <= 2 && units >= -5){
    delay(1000);
    wasBelowThreshold = true;
    lcd.clear();
    return;
  }
  
  repeatKeluaran();
}

void lastIterasi(){
  units = scale.get_units(), 4;
  if(wasBelowThreshold==true){
    iterasiKeluaran += 1;
    wasBelowThreshold = false;
  }
  

  int sisa = stringKey.toInt()%100;

  lcd.setCursor(0, 0);
  lcd.print("Berat : ");
  lcd.print(units);
  lcd.print(" gram     ");
  lcd.setCursor(0, 1);
  lcd.print("Berat Sisa : ");
  lcd.print(sisa);
  //lcd.setCursor(0, 1);
  //lcd.print(iterasiKeluaran);

  if(units >= stringKey.toInt()%100){
    delay(500);
    myServo.write(0);
    return;
  }

  lastIterasi();
}

