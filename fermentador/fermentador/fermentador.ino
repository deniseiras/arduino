// TODO 
// 4 - Simbolo grau - lcd.write(223);
//

// Configuração fios
// cristal liq:
// A4 - marron
// A5 - branco
//
// rele
// D11 - branco
//
// sens. temp
// D10 - amarelo
// 
// teclado
// D9 - marron
// D8 - vermelho
// D7 - laranja
// D6 - amaralo 
// D5 - verde
// D4 - azul
// D3 - lilas
// D2 - cinza

#include <OneWire.h>
#include <DallasTemperature.h>
#include <Keypad.h>

// *** System ***
#define LCD_OUT 0
#define SERIAL_OUT 1
bool system_out = LCD_OUT;
String lcd_serial[2];
char programPhase;
char lastPhase;
// 0 = getKeyChar
// 1 = mostura / fervura
// 2 = recirculacao
// 3 = resfriamento / despejo
int rampLast = -1;
int rampTemp[8];
int rampMinutes[8];
int tempOffset = 2; // graus para cima/baixo de offset


// *** Crystal Liq. ***
#include <LiquidCrystal_I2C.h>
// initialize the library with the numbers of the interface pins
//LiquidCrystal_I2C lcd(0x3F,2,1,0,4,5,6,7,3, POSITIVE);
LiquidCrystal_I2C lcd(0x27,2,1,0,4,5,6,7,3, POSITIVE);


// *** SENSOR ***
// Porta do pino de sinal do DS18B20
#define ONE_WIRE_BUS 10
// Define uma instancia do oneWire para comunicacao com o sensor
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress sensor1;
float tempMin = 999;
float tempMax = 0;

// *** RELE PANELA ***
int porta_rele_panela =11;

// *** KEYPad ***
const byte numRows= 4; //number of rows on the keypad
const byte numCols= 4; //number of columns on the keypad
/*keymap defines the key pressed according to the row and columns just as appears on the keypad*/
char keymap[numRows][numCols] = {
{'1', '2', '3', 'A'},
{'4', '5', '6', 'B'},
{'7', '8', '9', 'C'},
{'*', '0', '#', 'D'}
};
//Code that shows the the keypad connections to the arduino terminals
// key 1 to arduino 9; key 9 to arduino 2
byte rowPins[numRows] = {2, 3, 4, 5 }; //Rows 0 to 3
byte colPins[numCols]= {6, 7, 8, 9}; //Columns 0 to 3
//initializes an instance of the Keypad class
Keypad myKeypad= Keypad(makeKeymap(keymap), rowPins, colPins, numRows, numCols);

// Define API
void cls();
void cls_line(byte lin);

void print_out(String messg, byte lin);
void print_del(String messg, byte lin);
void print_out(String messg, byte lin, byte col);
void print_del(String messg, byte lin, byte col);
void print_lcd(String messg, byte lin, byte col);
void print_ser(String messg);
float readTemp();
char getKeyChar();
String getKeyString();


void setup() { 
  Serial.begin(9600);
  programPhase = '*';
  lastPhase = programPhase;
  lcd_serial[0] = "";
  lcd_serial[1] = "";

  digitalWrite(porta_rele_panela, HIGH); //Desliga rele
  pinMode(porta_rele_panela, OUTPUT);

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


void loop() {

  char inputChar;
  int inputByte = 0;
  String str_aux;

  if(lastPhase != programPhase) {
//    print_out("Selecionado: ", 0);
//    print_del(String(programPhase), 1);
    lastPhase = programPhase;
  }
  switch(programPhase) {
    case '*':
      print_out("A) Configurar", 0);
      print_del("B) Resfriar", 1);
      programPhase = getKeyChar();
      break;
    case 'A': {
      print_out("A) Nova rampa", 0);
      print_del("B) Ver rampas", 1);
      print_out("C) Excluir rampa", 0);
      print_del("*) Voltar", 1);
      inputChar = getKeyChar();
      switch(inputChar) {
        case '*': {
          programPhase = '*';
        } break;
        case 'A': {
          print_out("Temp: ", 0);
          print_del("*) cancela #) Ok", 1);
          cls();
          print_del("Temp:", 0);
          str_aux = getKeyString().toInt();
          if(str_aux == "*") break;
          rampLast++;
          rampTemp[rampLast] = str_aux.toInt();

          print_out("Minutos: ", 0);
          print_del("*) cancela #) Ok", 1);
          cls();
          print_del("Minutos:", 0);
          str_aux = getKeyString().toInt();
          if(str_aux == "*") { 
            rampLast--;
            break;
          }
          rampMinutes[rampLast] = str_aux.toInt();
          
          print_out("Rampa: ", 0);
          print_out(String(rampLast), 0, 8);
          print_del("Incluída !", 1);
        } break;
        case 'B': {
          for(byte i=0; i<= rampLast; i++) {
            cls();
            print_out("Rampa: ", 0);
            print_out(String(i), 0, 8);
            print_del(String(rampTemp[i]) + " C " + String(rampMinutes[i]) + " minutos", 1, 0);
          }
        } break;
      }
    } break; 
    case 'B': {
      print_out("Iniciar Resfr.?", 0);
      print_out("*)Nao / #)Sim", 1);
      inputChar = getKeyChar();
      switch(inputChar) {
        case '*': {
          programPhase = '*';
        } break;
        case '#': {
          char tempChar[4];
          float tempNow;
          byte rampNow = 0;
          while (rampNow <= rampLast) {
            // loop até atingir a temp da rampa
            tempNow = readTemp();
            if(tempNow > rampTemp[rampNow] - tempOffset) {
              digitalWrite(porta_rele_panela, LOW);  //Liga rele
            }
            do {
              tempNow = readTemp();
              dtostrf(tempNow, 2, 1, tempChar);
              print_out("Atingindo Rampa " + String(rampNow), 0);
              print_del(String(tempChar) + "C / atual: " + String(rampTemp[rampNow]) + "C", 1);
            } while(tempNow > rampTemp[rampNow]);
            
            print_out("Rampa " + String(rampNow) + " / " + String(rampTemp[rampNow]) + "C", 0);
            print_del("Temp. atingida!", 1);
            
            unsigned long secondsRamp;
            unsigned long secondsInitRamp = millis()/1000;
            unsigned long secondsPrint, minutesPrint, hoursPrint;
            do {
              tempNow = readTemp();
              Serial.println("Temp = ");
              Serial.println(rampTemp[rampNow]);
              Serial.println(tempOffset);
              Serial.println(tempNow);
              
              if(tempNow < rampTemp[rampNow] - tempOffset) {
                digitalWrite(porta_rele_panela, HIGH);  //DesLiga rele
              } else if(tempNow > rampTemp[rampNow] + tempOffset) {
                digitalWrite(porta_rele_panela, LOW);  //Liga rele
              }


              hoursPrint = rampMinutes[rampNow] / 3600;
              minutesPrint = (rampMinutes[rampNow] % 3600) / 60;
              print_out("R: " + String(rampNow) + " " + String(rampTemp[rampNow]) + "C " + String(hoursPrint) + ":" + String(minutesPrint) + ":00" , 0);
              dtostrf(tempNow, 2, 1, tempChar);
              secondsRamp = (millis()/1000)-secondsInitRamp;
              hoursPrint = secondsRamp / 3600;
              minutesPrint = (secondsRamp % 3600) / 60;
              secondsPrint = (secondsRamp % 3600) % 60;
              print_out(String(tempChar) + "C " + String(hoursPrint) + ":" + String(minutesPrint) + ":" + String(secondsPrint), 1);
              delay(1000);
              
            } while(secondsRamp < rampMinutes[rampNow] * 60);
            
            print_out("Rampa " + String(rampNow), 0);
            print_del("Tempo atingido!", 1);

            // continua sem parar a ultima rampa (==)
            if (rampNow < rampLast) rampNow ++;
          }
          digitalWrite(porta_rele_panela, HIGH);  //desLiga rele
        } break;
      }
    }
    break;      
    
//    case 'D': {
//      digitalWrite(porta_rele_panela, LOW);  //Liga rele
//      delay(2000);
//      digitalWrite(porta_rele_panela, HIGH); //Desliga rele
//      delay(2000);
//    }
  }
  //delay(2000);
}


// API

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
  delay(3000);
}


void print_lcd(String messg, byte lin, byte col ) {
  // (note: line 1 is the second row, since counting begins with 0):
  lcd.setCursor(col, lin);
  lcd.print(messg);
}


char getKeyChar() {
  char keypressed;
  do {
    keypressed = myKeypad.getKey();
  } while(keypressed == NO_KEY);
  return keypressed;
}


String getKeyString()
{
   String num = "";
   byte col = 0;
   char key1;
   while (key1 != '#' && key1 != '*') {
     key1 = myKeypad.getKey();
     switch (key1) {
       case NO_KEY:
        break;
        
      case '0': case '1': case '2': case '3': case '4':
      case '5': case '6': case '7': case '8': case '9':
        print_out(String(key1), 1, col);
        num = num + key1;
        col += 1;
        break;
  
      case '#':
          return num;
  
      case '*':
          num = "*";
          return num;
     }
   }
}

char circulate(char phase) {
  return getKeyChar();
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
