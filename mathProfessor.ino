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

#define opADD     0
#define opSUB     1
#define opMUL     2
#define opDIV     3

#define modeMENU  0
#define modeGAME  1

int lcd_key     = 0;
int adc_key_in  = 0;
int app_mode = modeMENU;
bool bolScreenRendered = false;
int waitKeyPress = 350;

// configuration
int level;
int op;
int eq;

int menuPos;
// number of menu items: 3 (levels, ops, eq)
int menuSize = 3;
int taskMenuSize = 4;
int maxLevel = 4;
int maxOp = 7;
int maxEq = 2;

char* levels[] = { "L1", "L2", "L3", "L4"};
char* ops[] = { "   +", "   -", "   *", "   :", "  +-", "  *:", "+-*:"};
char* eqs[] = {"std", "eq "};
char* sigs[] = {"+", "-", "*", ":"};

// solution variables
String results[4];
int selectedResult;
int trueResult;

int levelBound[] = {10, 20, 100, 1000};

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

void renderTaskMenu() {
  int len = 0;
  for (int i = 0; i < taskMenuSize; i++) {
    
    switch (i) {
      case 0:
        lcd.setCursor(0, 1);
        if (menuPos == i) lcd.print("["); else lcd.print("("); len++;
        lcd.setCursor(1, 1);
        lcd.print(results[0]); len+=results[0].length();
        lcd.setCursor(len, 1);
        if (menuPos == i) lcd.print("]"); else lcd.print(")"); len++;
        break;
      case 1:
        lcd.setCursor(len, 1); len++;
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
      case 3:
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

void renderTask() {
  randomSeed(millis());
  int a, b, c, curOp;
  bool isEq = random(10) >= 6;
  if (op == 5) {
    // randomly choose one op of +/-
    curOp = (int)random(0, 2);
  } else if (op == 6) {
    // randomly choose one op of */:
    curOp = (int)random(2, 4);
  } else if (op == 7) {
    // randomly choose one op of +-*:
    curOp = (int)random(0, 4);
  } else {
    // choose chosen operation (0=+, 1=-, 2=*, 3=:)
    curOp = op - 1;
  }

  switch (curOp) {
    case opADD:
      {
        a = (int)random(0, levelBound[level - 1]);
        c = (int)random(0, levelBound[level - 1]);
        // swap values, if necessary
        if (a > c) {
          int tmp = c;
          c = a;
          a = tmp;
        }
        b = c - a;
        break;
      }
    case opSUB:
      {
        a = (int)random(0, levelBound[level - 1]);
        c = (int)random(0, levelBound[level - 1]);
        // swap values, if necessary
        if (a < c) {
          int tmp = c;
          c = a;
          a = tmp;
        }
        b = a - c;
        break;
      }
    case opMUL:
    case opDIV:
      {
        // random gen very frequently gives 0! :P thus exclude zero here, generate zeros later
        a = (int)random(1, (int)sqrt(levelBound[level - 1]));
        b = (int)random(1, (int)(levelBound[level - 1] / a));
        // swap a and b sometimes, to not always have a be the smaller factor
        if (random(0, 2) == 0) {
          int tmp = a;
          a = b;
          b = tmp;
        }
        // now generate zero
        if (random(1, 13) == 12) {
          b = 0;
        }
        Serial.print("a:");Serial.println(a);
        Serial.print("b:");Serial.println(b);
        c = a * b;
        if (curOp == opDIV) {
          // now convert to div calc: i.e. c / a = b
          if (a != 0) {
            b = a;
            a = c;
            c = a / b;
            Serial.println("case1");
            Serial.print("a:");Serial.println(a);
            Serial.print("b:");Serial.println(b);
          } else if (b != 0) {
            a = c;
            c = a / b;
            Serial.println("case2");
            Serial.print("a:");Serial.println(a);
            Serial.print("b:");Serial.println(b);
          } else {
            b = (int)random(1, levelBound[level - 1]);
            c = 0;
            Serial.println("case3");
            Serial.print("a:");Serial.println(a);
            Serial.print("b:");Serial.println(b);
          }
        }
        break;
      }

      // generate result options
      results[0] = "229";
      results[1] = "372";
      results[2] = "  "+c;
      results[3] = "249";

      lcd.setCursor(0, 1);
      selectedResult = 0;
      trueResult = 2;

  }

  String task = "";
  task.concat(a);
  task.concat(" ");
  task.concat(sigs[curOp]);
  task.concat(" ");
  task.concat(b);
  task.concat(" ");
  task.concat("= ?");

  lcd.clear();
  lcd.print(task);

  

  Serial.println("..");
  Serial.print("a:");
  Serial.println(a);
  Serial.print("b:");
  Serial.println(b);
  Serial.print("c:");
  Serial.println(c);

}

void setup()
{
  Serial.begin(9600);
  // init/reset global values
  lcd_key     = 0;
  adc_key_in  = 0;
  app_mode = modeMENU;
  menuPos = 0;
  bolScreenRendered = false;
  app_mode = modeMENU;
  lcd.begin(16, 2);
  restoreConfigFromEPROM();
}

void loop()
{
  // render Content
  if (app_mode == modeMENU) {
    // render Menu
    if (!bolScreenRendered) {
      lcd.setCursor(0, 0);
      lcd.print("*Math-Professor*");
      renderMenu();
      bolScreenRendered = true;
      delay(waitKeyPress);
    }
  } else {
    // render Game Task
    if (!bolScreenRendered) {
      renderTask();
      renderTaskMenu();
      bolScreenRendered = true;
      delay(waitKeyPress);
    }
  }

  // handle input
  lcd_key = read_LCD_buttons();
  if (app_mode == modeMENU) {
    switch (lcd_key)
    {
      case btnRIGHT:
        {
          if (menuPos < menuSize - 1) {
            menuPos++;
            bolScreenRendered = false;
          }
          break;
        }
      case btnLEFT:
        {
          if (menuPos > 0) {
            menuPos--;
            bolScreenRendered = false;
          }
          break;
        }
      case btnUP:
        {
          if (menuPos == 0 && level < maxLevel) {
            level++;
            bolScreenRendered = false;
          } else if (menuPos == 1 && op < maxOp) {
            op++;
            bolScreenRendered = false;
          } else if (menuPos == 2 && eq < maxEq) {
            eq++;
            bolScreenRendered = false;
          }
          break;
        }
      case btnDOWN:
        {
          if (menuPos == 0 && level > 1) {
            level--;
            bolScreenRendered = false;
          } else if (menuPos == 1 && op > 1) {
            op--;
            bolScreenRendered = false;
          } else if (menuPos == 2 && eq > 1) {
            eq--;
            bolScreenRendered = false;
          }
          break;
        }
      case btnSELECT:
        {
          app_mode = modeGAME;
          bolScreenRendered = false;
          break;
        }
    }

  } else if (app_mode == modeGAME) {

  }

}
