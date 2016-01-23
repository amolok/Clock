#include <LiquidCrystal.h>
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
int readkey;
void setup() {
  lcd.begin(16, 2);
  lcd.print("Fathers day");
  delay(1000);
}
void loop() {
  delay(500);
  lcd.clear();
  lcd.setCursor(0, 0);
  readkey=analogRead(0);
  {
    lcd.print("key: ");
    lcd.print(readkey, HEX);
  }
  lcd.setCursor(0, 1);
  
  if (readkey<50) {
    lcd.print("Button Right");
  }
  else if(readkey<195) {
    lcd.print("Button Up");
  }
  else if(readkey<380) {
    lcd.print("Button Down");
  }
  else if(readkey<790) {
    lcd.print("Button Left");
  }else{
    lcd.print("It doesn't?");
  }
}

void loop_() {
  // set the cursor to (0,0):
  lcd.setCursor(0, 1);
  readkey=analogRead(0);
  lcd.print("key = ");
  lcd.print(readkey);
//  lcd.clear();
  // print from 0 to 9:
  for (int thisChar = 0; thisChar < 10; thisChar++) {
    lcd.print(thisChar);
    delay(500);
  }
//
//  // set the cursor to (16,1):
//  lcd.setCursor(16, 1);
//  // set the display to automatically scroll:
//  lcd.autoscroll();
//  // print from 0 to 9:
//  for (int thisChar = 0; thisChar < 10; thisChar++) {
//    lcd.print(thisChar);
//    delay(500);
//  }
//  // turn off automatic scrolling
//  lcd.noAutoscroll();
//
  // clear screen for the next loop:
  lcd.clear();
}
