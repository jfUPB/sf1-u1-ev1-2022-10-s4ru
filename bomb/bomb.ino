#include "SSD1306Wire.h"

#define BOMB_OUT 25
#define LED_COUNT 26
#define UP_BTN 13
#define DOWN_BTN 32
#define ARM_BTN 33

// Selecciona uno según tu display.
SSD1306Wire display(0x3c, SDA, SCL, GEOMETRY_128_32);
//SSD1306Wire display(0x3c, SDA, SCL, GEOMETRY_64_48);

void taskButtons();
void taskBomb();

void setup() {
  taskButtons();
  taskBomb();
}

bool evButtons = false;
uint8_t evButtonsData = 0;


void loop() {
  taskButtons();
  taskBomb();
}

void taskButtons() {
  enum class ButtonsStates {INIT, WAITING_PRESS, WAITING_STABLE, WAITING_RELEASE};
  static ButtonsStates buttonsState =  ButtonsStates::INIT;
  static uint32_t referenceTime;
  const uint32_t STABLETIMEOUT = 100;

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
          referenceTime = millis();
          buttonsState = ButtonsStates::WAITING_STABLE;
        }
        break;
      }

    case ButtonsStates::WAITING_STABLE: {
        if (digitalRead(UP_BTN) == HIGH) {
          buttonsState = ButtonsStates::WAITING_PRESS;
        }
        else if ( (millis() - referenceTime) >= STABLETIMEOUT) {
          buttonsState = ButtonsStates::WAITING_RELEASE;
        }

        break;
      }

    case ButtonsStates::WAITING_RELEASE: {
        if (digitalRead(UP_BTN) == HIGH) {

          evButtons = true;
          evButtonsData = UP_BTN;
          
          buttonsState = ButtonsStates::WAITING_PRESS;
        }
        break;
      }
    default:
      break;



  }

}

void taskBomb() {

}
