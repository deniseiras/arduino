// TODO 
// 1 - fazer pular / voltar rampa
// 2 - excluir rampa
// 3 - salvar rampa eprom
// 4 - Simbolo grau - lcd.write(223);
//

// ================= Wire Config ========================
// cristal liq:
// A4 - verde
// A5 - amarelo
//
// rele bomba
// D6 - azul

// rele panela
// D7 - laranja
//
// D8  - botao cancel
// D9  - botao ok
// D10 - botao sub
// D11 - botao add
//
// sens. temp
// D12 - amarelo

#include <OneWire.h>
#include <DallasTemperature.h>


// ================== System variables ==================
#define LCD_OUT 0
#define SERIAL_OUT 1
bool system_out = LCD_OUT;
String lcd_serial[2];
String programPhase;
String lastPhase;

int rampLast = -1;
int rampTemp[8];
int rampMinutes[8];
int tempOffset = 1;       // graus para cima/baixo de offset
int buttCancel = 8;       //Botão para cancelar
int buttOk = 9;           //Botão para confirmar
int buttSub = 10;         //Botão para subtrair valor
int buttAdd = 11;         //Botão para somar valor
bool backHigh = true;


// =================== Sensors cfg =====================
// *** Crystal Liq. ***
#include <LiquidCrystal_I2C.h>
// initialize the library with the numbers of the interface pins
//LiquidCrystal_I2C lcd(0x3F,2,1,0,4,5,6,7,3, POSITIVE);
LiquidCrystal_I2C lcd(0x27,2,1,0,4,5,6,7,3, POSITIVE);

// *** SENSOR ***
// Porta do pino de sinal do DS18B20
#define ONE_WIRE_BUS 12

// Define uma instancia do oneWire para comunicacao com o sensor
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress sensor1;
float tempMin = 999;
float tempMax = 0;

// *** RELE PANELA ***
int porta_rele_panela = 7;

// *** RELE BOMBA ***
int porta_rele_bomba = 6;


// =================== Define API ======================
// printing
void cls();
void cls_line(byte lin);
void print_out(String messg, byte lin);
void print_del(String messg, byte lin);
void print_out(String messg, byte lin, byte col);
void print_del(String messg, byte lin, byte col);
void print_lcd(String messg, byte lin, byte col);
void print_ser(String messg);

// button comands
bool pressed(int butt);
int getValue(String strVarName, String unity, int minValue, int maxValue );
int getButtValue(int butt, int value, int incr);
bool getOk();
String menuSelect(String menu[], int menuSize);

// sys functions
char brew();
char circulate();
float readTemp();

// ===================== Setup ========================
void setup() { 
  Serial.begin(9600);
  programPhase = '*';
  lastPhase = programPhase;
  lcd_serial[0] = "";
  lcd_serial[1] = "";

  pinMode(porta_rele_bomba, OUTPUT);
  digitalWrite(porta_rele_bomba, HIGH); //Desliga bomba
  
  pinMode(porta_rele_panela, OUTPUT);
  digitalWrite(porta_rele_panela, HIGH); //Desliga rele

 
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
//  void setBacklightPin ( uint8_t value, t_backlighPol pol );
  lcd.setBacklight(HIGH);
  sensors.begin();
  Serial.print(sensors.getDeviceCount(), DEC);
  Serial.println(" sensores.");
  if (!sensors.getAddress(sensor1, 0)) 
     Serial.println("Sensores nao encontrados !"); 
  
  print_del("Bem vindo!", 0);
  cls();
 
}

// ===================== Loop ========================
void loop() {

  char inputChar;
  int inputByte = 0;
  String str_aux;

  if(lastPhase != programPhase) {
//    print_out("Selecionado: ", 0);
//    print_del(String(programPhase), 1);
    lastPhase = programPhase;
  }

  Serial.println("Ramp Min Init" + String(rampMinutes[rampLast]));
  
  if (programPhase == "*") {
    String menu[] = {"Configurar", "Brassagem", "Resfriar", "Luz de Fundo" };
    programPhase = menuSelect(menu, 4);
  
  
  } else if ( programPhase == "Configurar") {
    String menu[] = { "Nova rampa", "Ver rampas", "Excluir rampa" };
    programPhase = menuSelect(menu, 3);
    
    if (programPhase == "Nova rampa") {
      int tempC = getValue("Temperatura", "C", -20, +50);
      int diasAux = getValue("Tempo", "dias", 0, 180);
      int horasAux = getValue("Tempo", "horas", 0, 24);
      int minAux = getValue("Tempo", "min", 0, 60 );
      if ( ! getOk()) {
        programPhase = '*';
      } else {
        rampLast++;
        rampTemp[rampLast] = tempC;
        rampMinutes[rampLast] = diasAux*24*60 + horasAux*60 + minAux;
        
        Serial.println("Ramp Minutes Config" + String(rampMinutes[rampLast]));
        Serial.println("Ramp Temp Config" + String(rampTemp[rampLast]));
        
        print_out("Rampa: ", 0);
        print_out(String(rampLast), 0, 8);
        print_del("Incluída !", 1);
        programPhase = "Configurar";
      }
    
    } else if (programPhase == "Ver rampas") {
      for (byte i = 0; i <= rampLast; i++) {
        cls();
        print_out("Rampa: ", 0);
        print_out(String(i), 0, 8);
        print_del(String(rampTemp[i]) + " C " + String(rampMinutes[i]) + " minutos", 1, 0);
      }
      programPhase = "Configurar";
    }


  } else if (programPhase == "Brassagem") {
    Serial.println("Ramp Min Brass" + String(rampMinutes[rampLast]));
    cls();
    print_out("Iniciar Brass.?", 0);
    print_out("Canc) Ok)", 1);
    if (getOk()) {
      char tempChar[4];
      char tempChar2[4];
      float tempNow;
      int rampNow = 0;
      while (rampNow <= rampLast) {
        
        print_out("Ok) Rampa " + String(rampNow), 0);
        print_out("Canc) ", 1);
        if (!getOk()) {
          // desisitiu no comeco
          if (rampNow == 0) {
            rampNow = 999;
            break;
          }
          print_out("Ok) pula rampa", 0);   
          print_out("Canc) rampa ant", 1);
          if (getOk()) {
            rampNow ++;
          } else {
            rampNow --;
          }
        }

        // loop até atingir a temp da rampa
        bool rampChanged = false;
        do {
          tempNow = readTemp();
          setPanelaOnOff(tempNow, rampNow);
          dtostrf(tempNow, 2, 1, tempChar);
          dtostrf(rampTemp[rampNow], 2, 0, tempChar2);
          
          print_out("Atingindo Rampa " + String(rampNow), 0);
          print_del("Temp: " + String(tempChar2) + " / " + String(tempChar), 1);
          if(pressed(buttCancel)) {
            print_out("Ok) pula rampa", 0);   
            print_out("Canc) rampa ant", 1);
            if (getOk()) {
              rampNow ++;
            } else {
              rampNow --;
            }
            break;
            rampChanged = true;
          }
        } while(tempNow < rampTemp[rampNow]);
        if (rampChanged) break;
        print_out("Rampa " + String(rampNow) + " Temp: " + String(rampTemp[rampNow]), 0);
        print_del("Temp. atingida!", 1);

        // loop rampa
        rampChanged = false;
        int secondsRamp;
        int secondsInitRamp = (int) ((float) millis() / 1000.0);
        int secondsPrint, minutesPrint, hoursPrint;
        String relePanelaStr;
        do {
          tempNow = readTemp();
          setPanelaOnOff(tempNow, rampNow);



          // ver pa esta trocando a hora e minutos depois da primeiro loop



          hoursPrint = (int) ((float)rampMinutes[rampNow] / 60.0);
          minutesPrint = (int) (rampMinutes[rampNow] % 60);
          print_out("R: " + String(rampNow) + " " + String(rampTemp[rampNow]) + "C " + String(hoursPrint) + ":" + String(minutesPrint) + ":00" , 0);
          dtostrf(tempNow, 2, 1, tempChar);
          secondsRamp = (int) ((float) millis() / 1000.0) - secondsInitRamp;
          hoursPrint = (int) ((float) secondsRamp / 3600.0);
          minutesPrint = (int) (((float) (secondsRamp % 3600) / 60.0));
          secondsPrint = (int) (secondsRamp % 3600) % 60;
          if(digitalRead(porta_rele_panela) == LOW) {
            relePanelaStr = "ON";
          } else {
            relePanelaStr = "OFF";
          }
          print_out(relePanelaStr + " " +String(tempChar) + "C " + String(hoursPrint) + ":" + String(minutesPrint) + ":" + String(secondsPrint), 1);
          delay(990);

          if(pressed(buttCancel)) {
            rampChanged = true;
            print_out("Ok) pula rampa", 0);   
            print_out("Canc) rampa ant", 1);
            if (getOk()) {
              Serial.println("Avançando rampa ...");
              rampNow ++;
              break;
            } else {
              Serial.println("Voltando rampa ...");
              rampNow --;
              break;
            }
            
          }
          Serial.println("SecondsRamp = " + String(secondsRamp));
          Serial.println("RampNow = " + String(rampNow));
          Serial.println(rampMinutes[rampNow]);
          Serial.println("rampMinutes[rampNow] * 60 " + String(rampMinutes[rampNow]));
        } while (secondsRamp < rampMinutes[rampNow] * 60 );

        if (rampChanged) break;
        print_out("Rampa " + String(rampNow), 0);
        print_del("Tempo atingido!", 1);
        rampNow++;
      }
      digitalWrite(porta_rele_panela, HIGH);  //DesLiga rele
    }
    programPhase = "*";


  } else if (programPhase == "Resfriar") {
    print_out("Iniciar Resfr.?", 0);
    print_out("Canc / Ok", 1);
    if (getOk()) {
      char tempChar[4];
      float tempNow;
      char keypressed;
      print_out("Coloque o sensor", 0);
      print_out("no fermentador", 1);
      if (getOk()) {
        do {
          tempNow = readTemp();
          dtostrf(tempNow, 2, 1, tempChar);
          print_out("Temp: " + String(tempChar), 0);
          print_out("Canc) Fin. Resfr", 1);
        } while(!pressed(buttCancel));
      }
    }
    programPhase = "*";


  } else if (programPhase == "Luz de Fundo") {
    do {
      delay(200);
    } while(!pressed(buttOk));
    
    backHigh = !backHigh;
    if(backHigh) {
      lcd.setBacklight(HIGH);
    } else {
      lcd.setBacklight(LOW);
    }

    programPhase = "*";
  }
  
}


// ===================== API ==========================

void setPanelaOnOff(int tempNowCheck, int rampNowCheck) {

  Serial.println(" ======== "); 
  if(tempNowCheck > rampTemp[rampNowCheck] && digitalRead(porta_rele_panela) == LOW) {
    Serial.println(" DESligando <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<");
    digitalWrite(porta_rele_panela, HIGH);  //DesLiga rele
  } else if(tempNowCheck < rampTemp[rampNowCheck] - tempOffset && digitalRead(porta_rele_panela) == HIGH) {
    Serial.println(" >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> ligando");
    digitalWrite(porta_rele_panela, LOW);  //Liga rele
  }
}


String menuSelect(String menu[], int menuSize) {
  int itemLine = 0;
  int itemLineAux = -1;

  delay(500); //avoid entering button ok pressed again
  do {
    itemLine = getButtIncr(buttAdd, itemLine, -1);
    itemLine = getButtIncr(buttSub, itemLine, +1);

    if (itemLine >= 0 && itemLine < menuSize) {
      if (itemLine != itemLineAux) {
        itemLineAux = itemLine;
        if (itemLine % 2 == 0) {
          print_out("* " + menu[itemLine], 0);
          print_out("  " + menu[itemLine + 1], 1);
        } else {
          print_out("  " + menu[itemLine - 1], 0);
          print_out("* " + menu[itemLine], 1);
        }
      }
    } else {
      itemLine = itemLineAux;
    }
  } while (! (pressed(buttCancel) || pressed(buttOk) ) );

  if (pressed(buttCancel)) return "*";
  return menu[itemLine];
}


bool pressed(int butt) {
  return digitalRead(butt) == HIGH;
}


int getButtIncr(int butt, int value, int incr) {
  int valButt = value;
  if (pressed(butt)) {
    valButt += incr;
    delay(400);
    while (pressed(butt)) {
      valButt += incr;
      delay(200);
    }
  }
  return valButt;
}


int getButtValue(int butt, int value, int incr, bool printValue, String unity, int minValue, int maxValue) {
  int valButt = value;
  if (pressed(butt)) {
    if(valButt + incr >= minValue && valButt + incr <= maxValue)
      valButt += incr;
      if (printValue) {
        print_out(String(valButt) + " " + unity, 1);
      }
      delay(400);
      while (pressed(butt)) {
        if(valButt + incr >= minValue && valButt + incr <= maxValue) {
          valButt += incr;
          if (printValue) {
            print_out(String(valButt) + " " + unity, 1);
          }
          delay(200);
        }
      }
    }

  return valButt;

}


int getValue(String strVarName, String unity, int minValue, int maxValue) {

  int value = 0;
  bool cancelled = false;
  bool ok = false;

  cls();
  print_out("Entre com " + strVarName + ":", 0);
  print_out(String(value) + " " + unity, 1);
  delay(300);

  do {

    value = getButtValue(buttAdd, value, 1, true, unity, minValue, maxValue);
    value = getButtValue(buttSub, value, -1, true, unity, minValue, maxValue);
    cancelled = pressed(buttCancel);
    ok = pressed(buttOk);

  } while (! (cancelled || ok) );

  if (cancelled) {
    return -9999;
  } else {
    Serial.println("get = " + String(value));
    return value;
  }

}

bool getOk() {

  bool ok = false;
  bool cancelled = false;

  delay(500); //avoid entering button ok pressed again
  do {
    cancelled = pressed(buttCancel);
    ok = pressed(buttOk);
  } while (! (cancelled || ok) );

  return ok;

}

void cls() {
  cls_line(0);
  cls_line(1);
}


void cls_line(byte lin) {
  if(system_out == LCD_OUT) {
    print_lcd("                ", lin, 0);
  } else {
    print_ser("                ");
  }
}


void print_out(String messg, byte lin) {
  cls_line(lin);
  print_out(messg, lin, 0);
}


void print_del(String messg, byte lin) {
  cls_line(lin);
  print_del(messg, lin, 0);
}


void print_out(String messg, byte lin, byte col) {
  if(system_out == LCD_OUT) {
    print_lcd(messg, lin, col);
  } else {
    print_ser("\n****************");
    lcd_serial[lin] = messg;
    print_ser(lcd_serial[0]);
    print_ser(lcd_serial[1]);
    print_ser("****************");
  }
}


void print_del(String messg, byte lin, byte col) {
  print_out(messg, lin, col);
  delay(1000);
}


void print_lcd(String messg, byte lin, byte col ) {
  // (note: line 1 is the second row, since counting begins with 0):
  lcd.setCursor(col, lin);
  lcd.print(messg);
}


void print_ser(String messg) {
  Serial.println(messg); 
}


char circulate(char phase) {
//  return getKeyChar();
}


float readTemp() {
  char tempChar[4];
  //String tempStr; // , tempStrMax, tempStrMin;

  // real
  sensors.requestTemperatures();
  float tempC = sensors.getTempC(sensor1);

//  if (tempC < tempMin) {
//    tempMin = tempC;
//  }
//  if (tempC > tempMax) {
//    tempMax = tempC;
//  }

//  Serial.println(tempC);
//  dtostrf(tempC, 2, 1, tempChar);
//  Serial.println(tempChar);
//  tempStr = String(tempChar);
//  Serial.println(tempStr);
  
//  dtostrf(tempMax, 2, 1, tempChar);
//  tempStrMax = String(tempChar);
//  dtostrf(tempMin, 2, 1, tempChar);
//  tempStrMin = String(tempChar);
//  print_out("Temp: " + tempStr, 0);
//  print_del(tempStrMin + " to " + tempStrMax, 1);
  return tempC;
}
