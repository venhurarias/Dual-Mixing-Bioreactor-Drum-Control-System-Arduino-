#include <Streaming.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Chrono.h>

#define AC_MOTOR_A_PIN 36
#define AC_MOTOR_B_PIN 37
#define AC_MOTOR_PIN_2 35
#define AC_HEATER_PIN 8
#define PILOT_RED_PIN 49     //red
#define PILOT_YELLOW_PIN 48  //yellow - GREEN 1
#define PILOT_GREEN1_PIN 47  //green 46
#define PILOT_GREEN2_PIN 46  //green 45
#define PILOT_GREEN3_PIN 45  //green 44
#define PILOT_GREEN4_PIN 44  //green - YELLOW   48
#define BUTTON_UP_PIN 3
#define BUTTON_DOWN_PIN 4
#define BUTTON_ENTER_PIN 5
#define BUTTON_BACK_PIN 6
#define BUZZER_STOP_PIN 7
#define FAN_PIN 38
#define BUZZER_PIN 34
#define EMERGENCY_PIN 2
#define RE_PIN 10
#define DE_PIN 11

#define MIN_TEMP 45
#define MAX_TEMP 55
#define NIT_MIN 0.15
#define NIT_MAX 0.5
#define PHO_MIN 0.2
#define PHO_MAX 0.8
#define POT_MIN 0.25
#define POT_MAX 0.5
#define PH_MIN 6.5
#define PH_MAX 7.5

LiquidCrystal_I2C lcd(0x27, 20, 4);
Chrono buzzerChrono, npkChrono, timeChrono;

bool isBuzzerOn = false;

byte values[11];

byte c1[8] = { B10000, B10100, B01110, B10101, B01110, B10100, B10000, B00000 };

int mode = 0;
int displayMode = -10;

int choose = 0;
bool onPressed = false;
float myNit, myPhos, myPot, myPh, myTemp;
int second, minute, hour;

Chrono myChrono, adminChrono;

void setup() {

  pinMode(AC_MOTOR_A_PIN, OUTPUT);
  pinMode(AC_MOTOR_B_PIN, OUTPUT);
  pinMode(AC_MOTOR_PIN_2, OUTPUT);
  pinMode(AC_HEATER_PIN, OUTPUT);
  pinMode(PILOT_RED_PIN, OUTPUT);
  pinMode(PILOT_YELLOW_PIN, OUTPUT);
  pinMode(PILOT_GREEN1_PIN, OUTPUT);
  pinMode(PILOT_GREEN2_PIN, OUTPUT);
  pinMode(PILOT_GREEN3_PIN, OUTPUT);
  pinMode(PILOT_GREEN4_PIN, OUTPUT);
  pinMode(BUTTON_UP_PIN, INPUT_PULLUP);
  pinMode(BUTTON_UP_PIN, INPUT_PULLUP);
  pinMode(BUTTON_DOWN_PIN, INPUT_PULLUP);
  pinMode(BUTTON_ENTER_PIN, INPUT_PULLUP);
  pinMode(BUTTON_BACK_PIN, INPUT_PULLUP);
  pinMode(BUZZER_STOP_PIN, INPUT_PULLUP);

  pinMode(FAN_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(EMERGENCY_PIN, INPUT_PULLUP);
  pinMode(RE_PIN, OUTPUT);
  pinMode(DE_PIN, OUTPUT);
  allOff();
  Serial.begin(9600);
  Serial.setTimeout(100);
  Serial1.begin(4800);

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0), lcd << F("====================");
  lcd.setCursor(0, 1), lcd << F("     BIOREACTOR     ");
  lcd.setCursor(0, 2), lcd << F("                    ");
  lcd.setCursor(0, 3), lcd << F("====================");
  lcd.createChar(0, c1);

  delay(2000);
}

void loop() {
  // Serial << "UP :: " << isUpPressed() << "\t\tDOWN :: " << isDownPressed() << "\t\tENTER :: " << isEnterPressed() << "\t\tBACK :: " << isBackPressed() << "\t\tBuzzer Stop :: " << isBuzzerStopPressed() << "\t\tEmergency :: " << isEmergencyPressed() << endl;
  // testing();
  normalProccess();
}

void normalProccess() {
  if (mode != 100) {
    if (isEmergencyPressed()) {
      mode = 100;
      allOff();
      delay(1000);
      lcd.setCursor(0, 0), lcd << F("====================");
      lcd.setCursor(0, 1), lcd << F("   EMERGENCY STOP   ");
      lcd.setCursor(0, 2), lcd << F("     TRIGGERED      ");
      lcd.setCursor(0, 3), lcd << F("====================");

      delay(1000);
      pilotYellowOn();
      delay(1000);
    }
  }


  if (mode >= 10) {
    if (isSetPointReach()) {
      lcd.setCursor(9, 3), lcd << F("H:ON  F:OFF");
      mode = 40;
    }
  }

  if (timeChrono.hasPassed(1000)) {
    timeChrono.restart();
    second++;
    if (second >= 60) {
      second = 0;
      minute++;
      if (minute >= 60) {
        minute = 0;
        hour++;
      }
    }
    if (mode >= 10 && mode < 40) {
      lcd.setCursor(0, 0);
      lcd << F("Time: ")
          << hour << F(":")
          << (minute < 10 ? F("0") : F("")) << minute << F(":")
          << (second < 10 ? F("0") : F("")) << second << F("  ");
    }
  }
  switch (mode) {
    case 0:
      if (displayMode != 0) {
        pilotRedOn();
        delay(100);
        lcd.setCursor(1, 0), lcd << F("AGITATOR ONLY      ");
        lcd.setCursor(1, 1), lcd << F("AGITATOR+DRUM CW   ");
        lcd.setCursor(1, 2), lcd << F("AGITATOR+DRUM CCW  ");
        lcd.setCursor(1, 3), lcd << F("DRUM MANUAL        ");
        displayMode = 0;
      }

      if (isUpPressed()) {
        if (!onPressed) {
          choose++;
          onPressed = true;
          if (choose > 3) {
            choose = 0;
          }
        }
      } else if (isDownPressed()) {
        if (!onPressed) {
          --choose;
          onPressed = true;
          if (choose < 0) {
            choose = 3;
          }
        }
      } else {
        onPressed = false;
      }

      if (choose == 0) {
        lcd.setCursor(0, 0), lcd.print(char(0));
        lcd.setCursor(0, 1), lcd.print(" ");
        lcd.setCursor(0, 2), lcd.print(" ");
        lcd.setCursor(0, 3), lcd.print(" ");

      } else if (choose == 1) {
        lcd.setCursor(0, 0), lcd.print(" ");
        lcd.setCursor(0, 1), lcd.print(char(0));
        lcd.setCursor(0, 2), lcd.print(" ");
        lcd.setCursor(0, 3), lcd.print(" ");

      } else if (choose == 2) {
        lcd.setCursor(0, 0), lcd.print(" ");
        lcd.setCursor(0, 1), lcd.print(" ");
        lcd.setCursor(0, 2), lcd.print(char(0));
        lcd.setCursor(0, 3), lcd.print(" ");

      } else if (choose == 3) {
        lcd.setCursor(0, 0), lcd.print(" ");
        lcd.setCursor(0, 1), lcd.print(" ");
        lcd.setCursor(0, 2), lcd.print(" ");
        lcd.setCursor(0, 3), lcd.print(char(0));
      }
      if (isEnterPressed()) {
        if (choose == 3) {
          mode = 230;
        } else {
          mode = 1;
          lcd.clear();
        }
      }
      break;

    case 1:
      if (!isEnterPressed()) {
        mode = 2;
      }

      break;

    case 2:
      if (displayMode != 1) {
        displayMode = 1;
        lcd.setCursor(0, 0), lcd << F("====================");
        lcd.setCursor(0, 1), lcd << F("CONTINUE            ");
        lcd.setCursor(0, 3), lcd << F("ENTER or BACK       ");
        if (choose == 0) {
          lcd.setCursor(0, 2), lcd << F("AGITATOR ONLY?      ");
        } else if (choose == 1) {
          lcd.setCursor(0, 2), lcd << F("AGITATOR+DRUM CW?   ");
        } else if (choose == 2) {
          lcd.setCursor(0, 2), lcd << F("AGITATOR+DRUM CCW?  ");
        }
      }
      if (isEnterPressed()) {
        hour = 0;
        minute = 0;
        second = 0;
        mode = 10;
        lcd.clear();
        delay(100);
      } else if (isBackPressed()) {
        mode = 0;
      }


      break;

    case 10:
      displayMode = 10;
      pilotGreen1On();
      pilotRedOff();
      DCMotor2On();
      if (choose == 0) {
        pilotGreen2On();
      } else if (choose == 1) {
        DCMotorForward();
        pilotGreen3On();
      } else if (choose == 2) {
        DCMotorReverse();
        pilotGreen4On();
      }
      mode = 20;
      break;

    case 20:

      if (displayMode != 20) {
        displayMode = 20;
        heaterOn();
      }
      if (myTemp >= MAX_TEMP) {
        heaterOff();
        mode = 30;
      }
      break;

    case 30:
      if (displayMode != 30) {
        displayMode = 30;
        fanOn();
      }

      if (myTemp <= MIN_TEMP) {
        fanOff();
        mode = 20;
      }
      break;

    case 40:

      if (displayMode != 40) {

        displayMode = 40;
        DCMotor2Off();
        DCMotorOff();
      }

      buzzerAlarm();
      if (isBuzzerStopPressed()) {
        mode = 0;
        buzzerOff();
        pilotGreen1Off();
        pilotGreen2Off();
        pilotGreen3Off();
        pilotGreen4Off();
        delay(1000);
        lcd.setCursor(0, 0), lcd << F("====PROCESS DONE====");
        lcd.setCursor(0, 1), lcd << F("     BIOREACTOR     ");
        lcd.setCursor(0, 2), lcd << F("                    ");
        lcd.setCursor(0, 3), lcd << F("====================");
        delay(5000);
      }
      break;

    case 100:
      displayMode = 100;
      if (!isEmergencyPressed()) {
        pilotYellowOff();
        mode = 0;
        delay(1000);
      }
      break;



    case 230:
      if (displayMode != 230) {
        lcd.setCursor(0, 0), lcd << F("========DRUM========");
        lcd.setCursor(0, 1), lcd << F("UP -> CC            ");
        lcd.setCursor(0, 2), lcd << F("DOWN -> CCW         ");
        lcd.setCursor(0, 3), lcd << F("ENTER -> OFF        ");
        displayMode = 230;
      }

      if (isUpPressed()) {
        DCMotorForward();
      } else if (isDownPressed()) {
        DCMotorReverse();
      } else if (isEnterPressed()) {
        DCMotorOff();
      }
      if (isBackPressed()) {
        mode = 0;
      }
      break;
  }
}

void buzzerAlarm() {
  if (buzzerChrono.hasPassed(1000)) {
    buzzerChrono.restart();
    if (isBuzzerOn) {
      buzzerOff();
      isBuzzerOn = false;
    } else {
      buzzerOn();
      isBuzzerOn = true;
    }
  }
}

bool isSetPointReach() {
  if (npkChrono.hasPassed(2000)) {
    npkChrono.restart();
    myNit = nitrogen();
    myPhos = phosphorous();
    myPot = potassium();
    myPh = ph();
    myTemp = temp();
    if (mode >= 10 && mode < 40) {
      lcd.setCursor(0, 0);
      lcd << F("Time: ")
          << hour << F(":")
          << (minute < 10 ? F("0") : F("")) << minute << F(":")
          << (second < 10 ? F("0") : F("")) << second << F("  ");
      lcd.setCursor(0, 1), lcd << F("N: ") << String(myNit, 2) << F("%");
      lcd.setCursor(9, 1), lcd << F("P: ") << String(myPhos, 2) << F("%");
      lcd.setCursor(0, 2), lcd << F("K: ") << String(myPot, 2) << F("%");
      lcd.setCursor(9, 2), lcd << F("Ph: ") << String(myPh, 2);
      lcd.setCursor(0, 3), lcd << F("T: ") << String(myTemp, 2) << F("C");
      if (mode == 20)
        lcd.setCursor(9, 3), lcd << F("H:ON  F:OFF");
      if (mode == 30)
        lcd.setCursor(9, 3), lcd << F("H:OFF F:ON ");
    }
  }

  return myNit >= NIT_MIN
         && myNit <= NIT_MAX
         && myPhos >= PHO_MIN
         && myPhos <= PHO_MAX
         && myPot >= POT_MIN
         && myPot <= POT_MAX
         && myPh >= PH_MIN
         && myPh <= PH_MAX
         && myTemp >= MIN_TEMP
         && myTemp <= MAX_TEMP;
}


float nitrogen() {
  // return (float)scanRegister3(0x06) / 0.05 / 10000;
  return 100;
}

float phosphorous() {
  return (float)scanRegister3(0x05) / 0.05 / 10000;
}
float potassium() {
  // return scanRegister3(0x07);
  return (float)scanRegister3(0x05) * 1.9 / 0.05 / 10000;
}

float ph() {
  return (float)scanRegister3(0x03) / 10;
}

float temp() {
  return (float)scanRegister3(0x01) / 10;
}


int scanRegister3(uint16_t registerAddress) {
  byte request[8];
  request[0] = 0x01;
  request[1] = 0x03;
  request[2] = highByte(registerAddress);
  request[3] = lowByte(registerAddress);
  request[4] = 0x00;
  request[5] = 0x01;

  uint16_t crc = calculateCRC(request, 6);
  request[6] = lowByte(crc);
  request[7] = highByte(crc);

  while (Serial1.available()) Serial1.read();

  digitalWrite(DE_PIN, HIGH);
  digitalWrite(RE_PIN, HIGH);
  delay(2);

  Serial1.write(request, 8);
  Serial1.flush();

  digitalWrite(DE_PIN, LOW);
  digitalWrite(RE_PIN, LOW);
  delay(50);

  byte response[10];
  byte i = 0;
  unsigned long start = millis();
  while (Serial1.available() && i < 10 && millis() - start < 100) {
    response[i++] = Serial1.read();
  }


  if (i >= 5 && response[2] == 2) {
    int val = (response[3] << 8) | response[4];
    return val;
  }

  return -1;
}



void testing() {
  if (Serial.available()) {
    String reading = Serial.readString();
    reading.trim();
    Serial << reading << endl;
    if (reading == "0") {
      Serial << "All OFF" << endl;
      allOff();
    } else if (reading == "q") {
      Serial << "AC MOTOR FORWARD" << endl;
      DCMotorForward();
    } else if (reading == "w") {
      Serial << "AC MOTOR REVERSE" << endl;
      DCMotorReverse();
    } else if (reading == "e") {
      Serial << "AC MOTOR 2" << endl;
      DCMotor2On();
    } else if (reading == "r") {
      Serial << "HEATER ON" << endl;
      heaterOn();
    } else if (reading == "1") {
      Serial << "PILOT RED ON" << endl;
      pilotRedOn();
    } else if (reading == "2") {
      Serial << "PILOT YELLOW ON" << endl;
      pilotYellowOn();
    } else if (reading == "3") {
      Serial << "PILOT GREEN1 ON" << endl;
      pilotGreen1On();
    } else if (reading == "4") {
      Serial << "PILOT GREEN2 ON" << endl;
      pilotGreen2On();
    } else if (reading == "5") {
      Serial << "PILOT GREEN3 ON" << endl;
      pilotGreen3On();
    } else if (reading == "6") {
      Serial << "PILOT GREEN4 ON" << endl;
      pilotGreen4On();
    } else if (reading == "7") {
      Serial << "FAN ON" << endl;
      fanOn();
    } else if (reading == "8") {
      Serial << "BUZZER ON" << endl;
      buzzerOn();
    }
  }
}


void allOff() {
  DCMotorOff();
  DCMotor2Off();
  heaterOff();
  pilotRedOff();
  pilotYellowOff();
  pilotGreen1Off();
  pilotGreen2Off();
  pilotGreen3Off();
  pilotGreen4Off();
  fanOff();
  buzzerOff();
}

void DCMotorForward() {
  digitalWrite(AC_MOTOR_A_PIN, LOW);
  digitalWrite(AC_MOTOR_B_PIN, HIGH);
}
void DCMotorReverse() {
  digitalWrite(AC_MOTOR_A_PIN, HIGH);
  digitalWrite(AC_MOTOR_B_PIN, LOW);
}
void DCMotorOff() {
  digitalWrite(AC_MOTOR_A_PIN, HIGH);
  digitalWrite(AC_MOTOR_B_PIN, HIGH);
}

void DCMotor2On() {
  digitalWrite(AC_MOTOR_PIN_2, LOW);
}

void DCMotor2Off() {
  digitalWrite(AC_MOTOR_PIN_2, HIGH);
}

void heaterOn() {
  digitalWrite(AC_HEATER_PIN, HIGH);
}

void heaterOff() {
  digitalWrite(AC_HEATER_PIN, LOW);
}

void pilotRedOn() {
  digitalWrite(PILOT_RED_PIN, LOW);
}

void pilotRedOff() {
  digitalWrite(PILOT_RED_PIN, HIGH);
}

void pilotYellowOn() {
  digitalWrite(PILOT_YELLOW_PIN, LOW);
}

void pilotYellowOff() {
  digitalWrite(PILOT_YELLOW_PIN, HIGH);
}

void pilotGreen1On() {
  digitalWrite(PILOT_GREEN1_PIN, LOW);
}

void pilotGreen1Off() {
  digitalWrite(PILOT_GREEN1_PIN, HIGH);
}

void pilotGreen2On() {
  digitalWrite(PILOT_GREEN2_PIN, LOW);
}

void pilotGreen2Off() {
  digitalWrite(PILOT_GREEN2_PIN, HIGH);
}

void pilotGreen3On() {
  digitalWrite(PILOT_GREEN3_PIN, LOW);
}

void pilotGreen3Off() {
  digitalWrite(PILOT_GREEN3_PIN, HIGH);
}

void pilotGreen4On() {
  digitalWrite(PILOT_GREEN4_PIN, LOW);
}

void pilotGreen4Off() {
  digitalWrite(PILOT_GREEN4_PIN, HIGH);
}

void fanOn() {
  digitalWrite(FAN_PIN, LOW);
}

void fanOff() {
  digitalWrite(FAN_PIN, HIGH);
}

void buzzerOn() {
  digitalWrite(BUZZER_PIN, LOW);
}

void buzzerOff() {
  digitalWrite(BUZZER_PIN, HIGH);
}


bool isUpPressed() {
  return !digitalRead(BUTTON_UP_PIN);
}
bool isDownPressed() {
  return !digitalRead(BUTTON_DOWN_PIN);
}
bool isEnterPressed() {
  return !digitalRead(BUTTON_ENTER_PIN);
}
bool isBackPressed() {
  return !digitalRead(BUTTON_BACK_PIN);
}
bool isBuzzerStopPressed() {
  return !digitalRead(BUZZER_STOP_PIN);
}

bool isEmergencyPressed() {
  return !digitalRead(EMERGENCY_PIN);
}

uint16_t calculateCRC(byte *data, byte length) {
  uint16_t crc = 0xFFFF;
  for (byte i = 0; i < length; i++) {
    crc ^= data[i];
    for (byte j = 0; j < 8; j++) {
      if (crc & 0x0001) {
        crc >>= 1;
        crc ^= 0xA001;
      } else {
        crc >>= 1;
      }
    }
  }
  return crc;
}
