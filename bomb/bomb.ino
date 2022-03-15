#include "SSD1306Wire.h"

#define BOMB_OUT 25
#define LED_COUNT 26
#define LED_1 27
#define LED_WRONG_PASSWORD 14
#define UP_BTN 13
#define DOWN_BTN 32
#define ARM_BTN 33

// Selecciona uno según tu display.
//SSD1306Wire display(0x3c, SDA, SCL, GEOMETRY_128_32);
SSD1306Wire display(0x3c, SDA, SCL, GEOMETRY_64_48);

void taskButtons();
void taskBomb();
void taskSerial();

void setup() {
  taskSerial();
  taskButtons();
  taskBomb();
}

bool evButtons = false;
uint8_t evButtonsData = 0;


void loop() {
  taskSerial();
  taskButtons();
  taskBomb();
}

void taskSerial() {
  enum class SerialStates {INIT, WAITING_COMMANDS};
  static SerialStates serialState =  SerialStates::INIT;
  switch (serialState) {
    case SerialStates::INIT: {
        Serial.begin(115200);
        serialState = SerialStates::WAITING_COMMANDS;
        break;
      }
    case SerialStates::WAITING_COMMANDS: {

        if (Serial.available() > 0 ) {
          int dataRx = Serial.read();
          if (dataRx == 'u') {
            evButtons = true;
            evButtonsData = UP_BTN;
          }
          else if (dataRx == 'd') {
            evButtons = true;
            evButtonsData = DOWN_BTN;
          }
          else if (dataRx == 'a') {
            evButtons = true;
            evButtonsData = ARM_BTN;
          }
        }
        break;
      }

    default:
      break;
  }

}

void taskButtons() {
  enum class ButtonsStates {INIT, WAITING_PRESS, WAITING_STABLE, WAITING_RELEASE};
  static ButtonsStates buttonsState =  ButtonsStates::INIT;
  static uint32_t referenceTime;
  const uint32_t STABLETIMEOUT = 100;
  static uint8_t lastButton = 0;

  switch (buttonsState) {
    case ButtonsStates::INIT: {
        pinMode(UP_BTN, INPUT_PULLUP);
        pinMode(DOWN_BTN, INPUT_PULLUP);
        pinMode(ARM_BTN, INPUT_PULLUP);
        buttonsState = ButtonsStates::WAITING_PRESS;
        break;
      }

    case ButtonsStates::WAITING_PRESS: {
        if (digitalRead(UP_BTN) == LOW) {
          lastButton = UP_BTN;
          referenceTime = millis();
          buttonsState = ButtonsStates::WAITING_STABLE;
        }
        else if (digitalRead(DOWN_BTN) == LOW) {
          lastButton = DOWN_BTN;
          referenceTime = millis();
          buttonsState = ButtonsStates::WAITING_STABLE;
        }
        else if (digitalRead(ARM_BTN) == LOW) {
          lastButton = ARM_BTN;
          referenceTime = millis();
          buttonsState = ButtonsStates::WAITING_STABLE;
        }

        break;
      }

    case ButtonsStates::WAITING_STABLE: {
        if (digitalRead(lastButton) == HIGH) {
          buttonsState = ButtonsStates::WAITING_PRESS;
        }
        else if ( (millis() - referenceTime) >= STABLETIMEOUT) {
          buttonsState = ButtonsStates::WAITING_RELEASE;
        }

        break;
      }

    case ButtonsStates::WAITING_RELEASE: {
        if (digitalRead(lastButton) == HIGH) {

          evButtons = true;
          evButtonsData = lastButton;

          buttonsState = ButtonsStates::WAITING_PRESS;
        }
        break;
      }
    default:
      break;
  }

}

void taskBomb() {
  enum class BombStates {INIT, WAITING_CONFIG, COUNTING};
  static BombStates bombState =  BombStates::INIT;
  static uint8_t counter = 20;

  switch (bombState) {
    
    case BombStates::INIT: {

        pinMode(LED_WRONG_PASSWORD, OUTPUT);
        pinMode(BOMB_OUT, OUTPUT);
        pinMode(LED_COUNT, OUTPUT);
        pinMode(LED_1, OUTPUT);

        digitalWrite(LED_WRONG_PASSWORD, LOW);
        digitalWrite(LED_COUNT, HIGH);
        digitalWrite(BOMB_OUT, LOW);

        display.init();
        display.setContrast(255);
        display.clear();
        display.setTextAlignment(TEXT_ALIGN_LEFT);
        display.setFont(ArialMT_Plain_16);
       
        display.clear();
        display.drawString(10, 5, String(counter));
        display.display();

        bombState = BombStates::WAITING_CONFIG;

        break;
      }
    case BombStates::WAITING_CONFIG: {

       display.clear();
       display.drawString(10, 5, String(counter));
       display.display();

        if (evButtons == true) {
          evButtons = false;

          if (evButtonsData == UP_BTN) {
            if (counter < 60) {
              counter++;
            }
          }

          if (evButtonsData == DOWN_BTN) {
            if (counter > 10) {
              counter--;
            }
          }

          if (evButtonsData == ARM_BTN) {
            bombState = BombStates::COUNTING;
            Serial.println("BombStates::COUNTING");
          }
        }
        break;
      }
    case BombStates::COUNTING: {
        const uint8_t vecLenght = 7;
        static uint8_t  vecTryPassword[vecLenght];
        static uint8_t dataCounter = 0;
        static uint8_t  vecTruePassword[vecLenght] = {UP_BTN, UP_BTN, DOWN_BTN, DOWN_BTN, UP_BTN, DOWN_BTN, ARM_BTN};
        bool statePassword = false;
      
        const uint32_t interval = 500;
        static uint32_t previousMillis = 0;
        static uint8_t ledstate_BOMB_OUT = LOW;
        uint32_t currentMillis = millis();


        if (currentMillis - previousMillis >= interval) {
          previousMillis = currentMillis;
          if (ledstate_BOMB_OUT == LOW) {
            ledstate_BOMB_OUT = HIGH;
          }
          else {
            ledstate_BOMB_OUT = LOW;
            counter--;
            display.clear();
            display.drawString(10, 5, String(counter));
            display.display();
          }
          digitalWrite(LED_COUNT, ledstate_BOMB_OUT);
          if (counter == 0) {
            digitalWrite(LED_COUNT, LOW);
            digitalWrite(BOMB_OUT, HIGH);
            display.clear();
            display.drawString(10, 5, "BOOOOOOM");
            display.display();
            delay(3000);
            digitalWrite(LED_COUNT, HIGH);
            digitalWrite(BOMB_OUT, LOW);
            counter = 20;
            display.clear();
            display.drawString(10, 5, String(counter));
            display.display();
            bombState = BombStates::WAITING_CONFIG;
          }
        }

        if (evButtons == true) {
          evButtons = false;
          if (dataCounter > vecLenght) {
            if (evButtonsData == UP_BTN) {
              vecTryPassword[dataCounter] = evButtonsData;
            }
            if (evButtonsData == DOWN_BTN) {
              vecTryPassword[dataCounter] = evButtonsData;
            }
            if (evButtonsData == ARM_BTN) {
              vecTryPassword[dataCounter] = evButtonsData;
            }
            dataCounter++;
          }

        }
        else if (dataCounter == vecLenght) {
          Serial.println("La clave esta llena");
          disarmTask(vecTryPassword, vecTruePassword, vecLenght, &statePassword);
          if (statePassword == true) {
            Serial.println("Contraseña Correcta");
            digitalWrite(LED_COUNT, HIGH);
            digitalWrite(BOMB_OUT, LOW);
            counter = 20;
            display.clear();
            display.drawString(10, 20, String(counter));
            display.display();
            dataCounter = 0;
            for (uint8_t r = 0; r < vecLenght; r++) {
              vecTryPassword[r] = 0;
            }
            digitalWrite(LED_1, HIGH);
            delay(300);
            digitalWrite(LED_1, LOW);
            bombState = BombStates::WAITING_CONFIG;
          }
          else {
            Serial.println("Contraseña Incorrecta");
            dataCounter = 0;
            for (uint8_t i = 0; i < vecLenght; i++) {
              vecTryPassword[i] = 0;
            }
            digitalWrite(LED_WRONG_PASSWORD, HIGH);
            delay(300);
            digitalWrite(LED_WRONG_PASSWORD, LOW);
          }
        }
      }
           break;
    default: {
      }
      break;
  }
}

void disarmTask(uint8_t *vecTryData, uint8_t *vecTrueData, uint8_t vecLengthData, bool *res) {
  for (uint8_t i = 0; i < vecLengthData; i++) {
    if (vecTrueData[i] == vecTryData[i]) {
      *res = true;
    }
    else {
      *res = false;
      break;
    }
  }
}
