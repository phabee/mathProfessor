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
bool bolMenuRendered = false;
bool bolTaskRendered = false;
bool bolShowResultScreen = false;
int waitKeyPress = 350;
int waitResultScreen = 3500;
int waitResultScreenSuccess = 2000;

// configuration
int level;
int op;
int eq;

// number of menu entries in the main menu
int menuPos;
// number of menu items: 3 (levels, ops, eq)
int menuSize = 3;
// number of task menu entries (chooseable results)
int taskMenuSize = 4;
// number of level options
int maxLevel = 4;
// number of operations options
int maxOp = 7;
// number of equation options
int maxEq = 2;

char* levels[] = { "L1", "L2", "L3", "L4"};
char* ops[] = { "   +", "   -", "   *", "   :", "  +-", "  *:", "+-*:"};
char* eqs[] = {"std", "eq "};
char* sigs[] = {"+", "-", "*", ":"};

// solution variables
String results[4] = {"   ", "   ", "   ", "   "};
int trueResult;
String solution;

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
        if (menuPos == i) lcd.print(">"); else lcd.print(" "); len++;
        lcd.setCursor(len, 1);
        lcd.print(results[0]); len += results[0].length();
        break;
      case 1:
        lcd.setCursor(len, 1);
        if (menuPos == i) lcd.print(">"); else lcd.print(" "); len++;
        lcd.setCursor(len, 1);
        lcd.print(results[1]); len += results[1].length();
        break;
      case 2:
        lcd.setCursor(len, 1);
        if (menuPos == i) lcd.print(">"); else lcd.print(" "); len++;
        lcd.setCursor(len, 1);
        lcd.print(results[2]); len += results[2].length();
        break;
      case 3:
        lcd.setCursor(len, 1);
        if (menuPos == i) lcd.print(">"); else lcd.print(" "); len++;
        lcd.setCursor(len, 1);
        lcd.print(results[3]); len += results[3].length();
        break;
    }
  }
}

int getLowerRandom(int a, int notC) {
  int retVal;
  if (a == notC) return a-1;
  return (int) random(a, notC);  
}

int getHigherRandom(int notC, int b) {
  int retVal;
  if (b == notC) return notC+1;
  return (int) random(notC+1, b);  
}

int getRandomExcludeNumber(int a, int b, int notC) {
  int retVal;
  do {
    retVal = (int)random(a, b);  
  } while (retVal == notC);
  return retVal;  
}

int getAdditionalExclusiveRandom(int a, int b, int q, int r, int c) {
  int retVal;
  do {
    retVal = (int)random(a, b);      
  } while (retVal == q || retVal == r || retVal == c);
  return retVal;
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
        Serial.print("a:"); Serial.println(a);
        Serial.print("b:"); Serial.println(b);
        c = a * b;
        if (curOp == opDIV) {
          // now convert to div calc: i.e. c / a = b
          if (a != 0) {
            b = a;
            a = c;
            c = a / b;
            Serial.println("case1");
            Serial.print("a:"); Serial.println(a);
            Serial.print("b:"); Serial.println(b);
          } else if (b != 0) {
            a = c;
            c = a / b;
            Serial.println("case2");
            Serial.print("a:"); Serial.println(a);
            Serial.print("b:"); Serial.println(b);
          } else {
            b = (int)random(1, levelBound[level - 1]);
            c = 0;
            Serial.println("case3");
            Serial.print("a:"); Serial.println(a);
            Serial.print("b:"); Serial.println(b);
          }
        }
        break;
      }
  }

  // now let's render the result options and the task-output, depending on wheter 
  // equations are enabled or not
  String task = "";
  // determine the position of the menu we want to place the correct result
  trueResult = (int)random(0, 4);
  // determine whether to render an ordinary task or an equation
  // in 30% of tasks we want an equation, if equations are enabled
  if (eq == 1 && (int)random(0,10) > 6) {
    // we render an equation of the form a op ? = c (b is the answer)
    // generate result options  
    int q = getLowerRandom(0, c);
    int r = getHigherRandom(c, levelBound[level - 1]);
    int p = getAdditionalExclusiveRandom(0, levelBound[level - 1], q, r, c);  
    switch (trueResult) {
      case 0:
        results[0] = String(c);
        results[1] = String(p);
        results[2] = String(q);
        results[3] = String(r);
        break;
      case 1:
        results[0] = String(p);
        results[1] = String(c);
        results[2] = String(q);
        results[3] = String(r);
        break;
      case 2: 
        results[0] = String(p);
        results[1] = String(q);
        results[2] = String(c);
        results[3] = String(r);
        break;
      case 3: 
        results[0] = String(p);
        results[1] = String(q);
        results[2] = String(r);
        results[3] = String(c);
        break;              
    }
  
    task.concat(a);
    task.concat(" ");
    task.concat(sigs[curOp]);
    task.concat(" ");
    task.concat(b);
    task.concat(" ");
    task.concat("= ?");
    solution = String(task);
    solution.replace("?", String(c));
  } else {
    // we render an ordinary task of the form a OP b = ? (c is the answer)
    // generate result options  
    int q = getLowerRandom(0, b);
    int r = getHigherRandom(b, levelBound[level - 1]);
    int p = getAdditionalExclusiveRandom(0, levelBound[level - 1], q, r, b);  

    switch (trueResult) {
      case 0:
        results[0] = String(b);
        results[1] = String(p);
        results[2] = String(q);
        results[3] = String(r);
        break;
      case 1:
        results[0] = String(p);
        results[1] = String(b);
        results[2] = String(q);
        results[3] = String(r);
        break;
      case 2: 
        results[0] = String(p);
        results[1] = String(q);
        results[2] = String(b);
        results[3] = String(r);
        break;
      case 3: 
        results[0] = String(p);
        results[1] = String(q);
        results[2] = String(r);
        results[3] = String(b);
        break;              
    }
    
    task.concat(a);
    task.concat(" ");
    task.concat(sigs[curOp]);
    task.concat(" ");
    task.concat("?");
    task.concat(" ");
    task.concat("= ");
    task.concat(c);
    solution = String(task);
    solution.replace("?", String(b));
  }
  
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

bool renderResultScreen() {
  bool retVal = false;
  lcd.clear();
  if (menuPos == trueResult) {
    lcd.print("*You are great!*");
    retVal = true;
  } else {
    lcd.print("*Oh, not quite!*"); 
  }  
  lcd.setCursor(0, 1);
  lcd.print(solution);
  return retVal;
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
  bolShowResultScreen = false;
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
      lcd.clear();
      lcd.print("*Math-Professor*");
      renderMenu();
      bolMenuRendered = true;
      delay(waitKeyPress);
    }
  } else {
    // check, whether result screen must be rendered
    if (bolShowResultScreen) {
      bool bolSuccess = renderResultScreen();
      bolShowResultScreen = false;
      menuPos = 0;
      if (bolSuccess) {
        delay(waitResultScreenSuccess);      
      } else {
        delay(waitResultScreen);              
      }
    } else {
      // render Game Task
      if (!bolTaskRendered) {
         renderTask();
         bolTaskRendered = true;
      }
      if (!bolMenuRendered) {
        renderTaskMenu();
        bolMenuRendered = true;
        delay(waitKeyPress);
      }
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
            EEPROM.write(menuPos, level);
          } else if (menuPos == 1 && op < maxOp) {
            op++;
            bolMenuRendered = false;
            EEPROM.write(menuPos, op);
          } else if (menuPos == 2 && eq < maxEq) {
            eq++;
            bolMenuRendered = false;
            EEPROM.write(menuPos, eq);
          }
          break;
        }
      case btnDOWN:
        {
          if (menuPos == 0 && level > 1) {
            level--;
            bolMenuRendered = false;
            EEPROM.write(menuPos, level);
          } else if (menuPos == 1 && op > 1) {
            op--;
            bolMenuRendered = false;
            EEPROM.write(menuPos, op);
          } else if (menuPos == 2 && eq > 1) {
            eq--;
            bolMenuRendered = false;
            EEPROM.write(menuPos, eq);
          }
          break;
        }
      case btnSELECT:
        {
          app_mode = modeGAME;
          bolMenuRendered = false;
          bolTaskRendered = false;
          menuPos = 0;
          break;
        }
    }
  } else if (app_mode == modeGAME) {
    // we are in game mode, user gets presented a task and can select the result
    switch (lcd_key)
    {
      case btnRIGHT:
        {
          if (menuPos < taskMenuSize - 1) {
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
      case btnSELECT:
        {
          bolTaskRendered = false;
          bolMenuRendered = false;
          bolShowResultScreen = true;
          break;
        }
    }
  }
}
