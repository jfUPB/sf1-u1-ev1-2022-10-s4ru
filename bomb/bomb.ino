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

  static uint8_t counter;

  static uint8_t clave[7];
  static uint8_t claveContador =0;
  static uint8_t password[7] = {UP_BTN, UP_BTN, DOWN_BTN, DOWN_BTN, UP_BTN, DOWN_BTN, ARM_BTN};

  uint32_t currentMillis = millis();


  switch (bombState) {
    case BombStates::INIT: {

        pinMode(14, OUTPUT);
        pinMode(BOMB_OUT, OUTPUT);
        pinMode(LED_COUNT, OUTPUT);

        digitalWrite(14, LOW);
        digitalWrite(LED_COUNT, HIGH);
        digitalWrite(BOMB_OUT, LOW);

        display.init();
        display.setContrast(255);
        display.clear();
        display.setTextAlignment(TEXT_ALIGN_LEFT);
        display.setFont(ArialMT_Plain_16);

        counter = 20;
        display.clear();
        display.drawString(10, 5, String(counter));
        display.display();
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
            display.drawString(10, 5, String(counter));
            display.display();
          }

          if(evButtonsData == ARM_BTN) {
            bombState = BombStates::COUNTING;         
          }






          // En la condición armado no olvidar preparar todas las variables para el siguiente estado.
          // por ejemplo claveContador = 0;


        }

        break;
      }

    case BombStates::COUNTING: {
       
        if (evButtons == true) {
          evButtons = false;

          clave[claveContador] = evButtonsData;
          claveContador++;
          if (claveContador == 7) {
            if(password[UP_BTN] ==  clave[UP_BTN]){
              if(password[UP_BTN] ==  clave[UP_BTN]){
                if(password[DOWN_BTN] ==  clave[DOWN_BTN]){
                  if(password[DOWN_BTN] ==  clave[DOWN_BTN]){
                    if(password[UP_BTN] ==  clave[UP_BTN]){
                      if(password[DOWN_BTN] ==  clave[DOWN_BTN]){
                        if(password[ARM_BTN] ==  clave[ARM_BTN]){
                           display.clear();
                           display.drawString(10, 5, "Bomba Desarmada");
                           display.display();
                           bombState =  BombStates::WAITING_CONFIG;
                           }
                         }
                       }
                     }
                   }
                 }
               }
             }
                 if(password[UP_BTN] !=  clave[UP_BTN]){
                 display.clear();
                 display.drawString(10, 5, "!Error¡");
                 display.display();
                 display.clear();
                 display.drawString(10, 5, String(counter));
                 display.display();
                 }
                 if(password[UP_BTN] !=  clave[UP_BTN]){
                 display.clear();
                 display.drawString(10, 5, "!Error¡");
                 display.display();
                 display.clear();
                 display.drawString(10, 5, String(counter));
                 display.display();
                 }
                 if(password[DOWN_BTN] !=  clave[DOWN_BTN]){
                 display.clear();
                 display.drawString(10, 5, "!Error¡");
                 display.display();
                 display.clear();
                 display.drawString(10, 5, String(counter));
                 display.display();
                 }
                 if(password[DOWN_BTN] !=  clave[DOWN_BTN]){
                 display.clear();
                 display.drawString(10, 5, "!Error¡");
                 display.display();
                 display.clear();
                 display.drawString(10, 5, String(counter));
                 display.display();
                 }
                 if(password[UP_BTN] !=  clave[UP_BTN]){
                 display.clear();
                 display.drawString(10, 5, "!Error¡");
                 display.display();
                 display.clear();
                 display.drawString(10, 5, String(counter));
                 display.display();
                 }
                 if(password[DOWN_BTN] !=  clave[DOWN_BTN]){
                 display.clear();
                 display.drawString(10, 5, "!Error¡");
                 display.display();
                 display.clear();
                 display.drawString(10, 5, String(counter));
                 display.display();
                 }
                 if(password[ARM_BTN] !=  clave[ARM_BTN]){
                 display.clear();
                 display.drawString(10, 5, "!Error¡");
                 display.display();
                 display.clear();
                 display.drawString(10, 5, String(counter));
                 display.display();
                 }

                 


                 
        if(counter = 0) {
          digitalWrite(BOMB_OUT,HIGH);
          display.clear();
          display.drawString(10, 5, "BOOOOM");
          display.display();
          if(counter = 0) {
            bombState =  BombStates::WAITING_CONFIG;
          }
        }        
     
          
        // Si ya pasó un 1 segundo --> decremento el counter
        // Cuando el contador llegue a cero --> BOOM!. Debo esperar un tiempo para que el usario pueda ver el mensaje
        // y el LED de la bomba se vea activa y luego paso de nuevo a configurar. OJO Cómo deben estar las cosas inicializadas
        // antes de entrar al estado de configuración?
        //

        // Si ya pasó 500 ms --> cambio el estado del LED de conteo


        break;
      }
  
      break;
  }

}
}
