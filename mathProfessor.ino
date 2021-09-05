//Sample using LiquidCrystal library
#include <LiquidCrystal.h>
#include <EEPROM.h>

/*******************************************************

  Mathematical Trainer Quiz by Fabian Leuthold v1.0

********************************************************/

// select the pins used on the LCD panel
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

#define btnRIGHT  0
#define btnUP     1
#define btnDOWN   2
#define btnLEFT   3
#define btnSELECT 4
#define btnNONE   5

#define modeMENU  0
#define modeGAME  1

int lcd_key     = 0;
int adc_key_in  = 0;
int app_mode = modeMENU;
bool bolMenuRendered = false;
bool bolTaskRendered = false;

// configuration
int level;
int op;
int eq;

int menuPos;
// number of menu items: 3 (levels, ops, eq)
int menuSize = 3;
int maxLevel = 4;
int maxOp = 7;
int maxEq = 2;

char* levels[] = { "L1", "L2", "L3", "L4"};
char* ops[] = { "   +", "   -", "   *", "   :", "  +-", "  *:", "+-*:"};
char* eqs[] = {"std", "eq "};

// read the buttons
int read_LCD_buttons()
{
  adc_key_in = analogRead(0);
  if (adc_key_in > 1000) return btnNONE;
  if (adc_key_in < 50)   return btnRIGHT;
  if (adc_key_in < 250)  return btnUP;
  if (adc_key_in < 450)  return btnDOWN;
  if (adc_key_in < 650)  return btnLEFT;
  if (adc_key_in < 850)  return btnSELECT;
  return btnNONE;
}

void restoreConfigFromEPROM() {
  // level is default = 1, max 9
  level = EEPROM.read(0);
  if (level < 1 || level > 9) level = 1;
  // operations is default = 1, max 31 (meaning + only)
  op = EEPROM.read(1);
  if (op < 1 || op > 31) op = 1;
  // eq is default = 1, max 2 (meaning equations disabled)
  eq = EEPROM.read(2);
  if (eq < 1 || eq > 2) eq = 1;
  Serial.print("level: ");
  Serial.print(level);
  Serial.print("op: ");
  Serial.print(op);
  Serial.print("eq: ");
  Serial.print(eq);
}

void renderMenu() {
  for (int i = 0; i < menuSize; i++) {
    switch (i) {
      case 0:
        lcd.setCursor(0, 1);
        if (menuPos == i) lcd.print("["); else lcd.print("(");
        lcd.setCursor(1, 1);
        lcd.print(levels[level - 1]);
        lcd.setCursor(3, 1);
        if (menuPos == i) lcd.print("]"); else lcd.print(")");
        break;
      case 1:
        lcd.setCursor(4, 1);
        if (menuPos == i) lcd.print("["); else lcd.print("(");
        lcd.setCursor(5, 1);
        lcd.print(ops[op - 1]);
        lcd.setCursor(9, 1);
        if (menuPos == i) lcd.print("]"); else lcd.print(")");
        break;
      case 2:
        lcd.setCursor(10, 1);
        if (menuPos == i) lcd.print("["); else lcd.print("(");
        lcd.setCursor(11, 1);
        lcd.print(eqs[eq - 1]);
        lcd.setCursor(14, 1);
        if (menuPos == i) lcd.print("]"); else lcd.print(")");
        break;
    }
  }
}

void setup()
{
  Serial.begin(9600);
  // init/reset global values
  lcd_key     = 0;
  adc_key_in  = 0;
  app_mode = modeMENU;
  menuPos = 0;
  bolMenuRendered = false;
  bolTaskRendered = false;
  app_mode = modeMENU;
  lcd.begin(16, 2);
  restoreConfigFromEPROM();
}

void loop()
{
  // render Content
  if (app_mode == modeMENU) {
    // render Menu
    if (!bolMenuRendered) {
      lcd.setCursor(0, 0);
      lcd.print("*Math-Professor*");
      renderMenu();
      bolMenuRendered = true;
    }
  } else {
    // render Game Task
    bolTaskRendered = true;
  }

  // handle input
  lcd_key = read_LCD_buttons();
  if (app_mode == modeMENU) {
    switch (lcd_key)
    {
      case btnRIGHT:
        {
          if (menuPos < menuSize-1) {
            menuPos++;
            bolMenuRendered = false;            
          }
          break;
        }
      case btnLEFT:
        {
          if (menuPos > 0) {
            menuPos--;
            bolMenuRendered = false;
           }
          break;
        }
      case btnUP:
        {
          if (menuPos == 0 && level < maxLevel) {
             level++;
             bolMenuRendered = false;
          } else if (menuPos == 1 && op < maxOp) {
             op++;
             bolMenuRendered = false;
          } else if (menuPos == 2 && eq < maxEq) {
            eq++;
            bolMenuRendered = false;
          }
          break;
        }
      case btnDOWN:
        {
          if (menuPos == 0 && level > 1) {
             level--;
             bolMenuRendered = false;
          } else if (menuPos == 1 && op > 1) {
             op--;
             bolMenuRendered = false;
          } else if (menuPos == 2 && eq > 1) {
            eq--;
            bolMenuRendered = false;
          }
          break;
        }
      case btnSELECT:
        {
          break;
        }  
     }

    // wait until key released
    while(true) {
      lcd_key = read_LCD_buttons();
      if (lcd_key ==  btnNONE) break;   
    }

     
  } else if (app_mode == modeGAME) {

  }

//  Serial.print("menuPos: ");
//   Serial.println(menuPos);

  //  lcd.setCursor(9, 1);           // move cursor to second line "1" and 9 spaces over
  //  lcd.print(millis() / 1000);    // display seconds elapsed since power-up
  //
  //
  //  lcd.setCursor(0, 1);           // move to the begining of the second line
  //  lcd_key = read_LCD_buttons();  // read the buttons
  //
  //  switch (lcd_key)               // depending on which button was pushed, we perform an action
  //  {
  //    case btnRIGHT:
  //      {
  //        lcd.print("RIGHT ");
  //        break;
  //      }
  //    case btnLEFT:
  //      {
  //        lcd.print("LEFT   ");
  //        break;
  //      }
  //    case btnUP:
  //      {
  //        lcd.print("UP    ");
  //        break;
  //      }
  //    case btnDOWN:
  //      {
  //        lcd.print("DOWN  ");
  //        break;
  //      }
  //    case btnSELECT:
  //      {
  //        lcd.print("SELECT");
  //        break;
  //      }
  //    case btnNONE:
  //      {
  //        lcd.print("NONE  ");
  //        break;
  //      }
  //  }

}
