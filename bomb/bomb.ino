#include "SSD1306Wire.h"

#define BOMB_OUT 25
#define LED_COUNT 26
#define LED_T 27
#define LED_contraseña_mala 14
#define UP_BTN 13
#define DOWN_BTN 32
#define ARM_BTN 33

// Selecciona uno según tu display.
SSD1306Wire display(0x3c, SDA, SCL, GEOMETRY_128_32);
//SSD1306Wire display(0x3c, SDA, SCL, GEOMETRY_64_48);

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
          display.clear();
          display.drawString(10, 5, String(BombCounter));
          display.display();

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

  static uint8_t Bombcounter = 20;
  uint32_t currentMillis = millis();


  switch (bombState) {
    case BombStates::INIT: {

        pinMode(LED_contraseña_mala 14, OUTPUT);
        pinMode(BOMB_OUT, OUTPUT);
        pinMode(LED_COUNT, OUTPUT);
        pinMode(LED_T, OUTPUT);

        digitalWrite(LED_contraseña_mala, LOW);
        digitalWrite(LED_COUNT, HIGH);
        digitalWrite(BOMB_OUT, LOW);

        display.init();
        display.setContrast(255);
        display.clear();
        display.setTextAlignment(TEXT_ALIGN_LEFT);
        display.setFont(ArialMT_Plain_12);

        counter = 20;
        display.clear();
        display.drawString(10, 5, String(counter));
        display.display();



        display.clear();
        display.drawString(10, 5, String(BombCounter));
        display.display();
        ledBombCountState = HIGH;
        bombState = BombStates::WAITING_CONFIG;



        bombState = BombStates::WAITING_CONFIG;
        break;
      }
    case BombStates::WAITING_CONFIG: {

        if (evButtons == true) {
          evButtons = false;

          if (evButtonsData == UP_BTN) {
            if (counter < 60) {
              counter++;
            }

            display.clear();
            display.drawString(10, 5, String(counter));
            display.display();
          }

          if (evButtonsData == DOWN_BTN) {
            if (counter > 10) {
              counter--;
            }

            display.clear();
            display.drawString(10, 5, String(BombCounter));
            display.display();
          }

          if (evButtonsData == ARM_BTN) {
            display.clear();
            display.drawString(10, 5, "BOMBA ARMADA");
            display.display();
            claveContador = 0;
            bombState = BombStates::COUNTING;
          }






          // En la condición armado no olvidar preparar todas las variables para el siguiente estado.
          // por ejemplo claveContador = 0;


        }

        break;
      }

    case BombStates::COUNTING: {
        const uint8_t ClaveLargo = 7;
        static uint8_t claveIntento[ClaveBuena];
        static uint8_t claveBuena[ClaveBuena] = {UP_BTN, UP_BTN, DOWN_BTN, DOWN_BTN, UP_BTN, DOWN_BTN, ARM_BTN}
                                                static uint8_t claveContador = 0;
        const uint32_t LEDBOMBINTERVAL = 500;
        static uint8_t ledBombCountState = LOW;
        bool EstadoClave = false;


        if (currentMillis - previousMillis >= interval) {
          previousMillis = currentMillis;
          if (ledBombCountState == LOW) {
            ledBombCountState = HIGH;
          }
          else {
            ledBombCountState = LOW;
            counter--;
            display.clear();
            display.drawString(10, 5, String(counter));
            display.display();
          }
          digitalWrite(LED_COUNT, ledBombCountState);
          if (counter == 0) {
            igitalWrite(LED_COUNT, LOW);
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
            bombState = BombStates::CONFIG;
          }
        }

        if (evButtons == true) {
          evButtons = false;
          if (claveContador > ClaveLargo) {
            if (evButtonsData == UP_BTN) {
              claveIntento[ClaveBuena] = evButtonsData;
            }
            if (evButtonsData == DOWN_BTN) {
              claveIntento[ClaveBuena] = evButtonsData;
            }
            if (evButtonsData == ARM_BTN) {
              claveIntento[ClaveBuena] = evButtonsData;
            }
            claveContador++;
          }

        }
        else if (claveContador == ClaveLargo) {
          Serial.println("La clave esta llena");
          DesarmeTask(claveIntento, claveBuena, ClaveLargo, EstadoClave);
          if (EstadoClave == true) {
            Serial.println("Contraseña Correcta")
            digitalWrite(LED_COUNT, HIGH);
            digitalWrite(BOMB_OUT, LOW);
            counter = 20;
            display.clear();
            display.drawString(10, 20, String(counter));
            display.display();
            claveContador = 0;
            for (uint8_t r = 0; r < ClaveLargo; r++) {
              claveIntento[r] = 0;
            }
            digitalWrite(LED_T, HIGH);
            delay(300);
            digitalWrite(LED_T, LOW);
            bombState = BombStates::CONFIG;
          }
          else {
            Serial.println("Contraseña Incorrecta");
            claveContador = 0;
            for (uint8_t i = 0; i < ClaveLargo; i++) {
              claveIntento[i] = 0;
            }
            digitalWrite(LED_contraseña_mala, HIGH);
            delay(300);
            digitalWrite(LED_contraseña_mala, LOW);
          }
        }
      }
           break;
    default: {
      }
      break;
  }
}
