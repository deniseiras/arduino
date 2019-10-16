/* 
Implementacao de rotina para pegar valor de numero de um botao


*/

#include <OneWire.h>

// *** Crystal Liq. ***
#include <LiquidCrystal_I2C.h>
// initialize the library with the numbers of the interface pins
//LiquidCrystal_I2C lcd(0x3F,2,1,0,4,5,6,7,3, POSITIVE);
LiquidCrystal_I2C lcd(0x27,2,1,0,4,5,6,7,3, POSITIVE);


// *** System ***

#define LCD_OUT 0
#define SERIAL_OUT 1
bool system_out = LCD_OUT;
String lcd_serial[2];

int buttCancel = 8;       //Botão para cancelar
int buttOk = 9;           //Botão para confirmar
int buttSub = 10;          //Botão para subtrair valor
int buttAdd = 11;          //Botão para somar valor


// Define API
void cls();
void cls_line(byte lin);
void print_out(String messg, byte lin);
void print_del(String messg, byte lin);
void print_out(String messg, byte lin, byte col);
void print_del(String messg, byte lin, byte col);
void print_lcd(String messg, byte lin, byte col);
bool pressed(int butt);
int getValue(String strVarName);
int getButtValue(int butt, int value, int incr);

void setup(){
  Serial.begin(9600);
  pinMode(buttSub, INPUT);    //Pino 7 (botão) selecionado como entrada

  lcd_serial[0] = "";
  lcd_serial[1] = "";

  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
//  void setBacklightPin ( uint8_t value, t_backlighPol pol );
  lcd.setBacklight(HIGH);
  
  print_del("Bem vindo!", 0);
  cls();
}


void loop()
{

  int tempVal = getValue("Temp");
  cls();
  print_out("Temp informada:", 0);
  print_del(String(tempVal), 1);

}


//
// API ==========================
//

bool pressed(int butt) {
  return digitalRead(butt) == HIGH;
}

int getValue(String strVarName) {

  int value = 0;
  bool cancelled = false;
  bool ok = false;
  int dispPos = 1;

  print_out("Entre com " + strVarName + ":", 0);
  do {

    value = getButtValue(buttAdd, value, 1);
    value = getButtValue(buttSub, value, -1);
    cancelled = pressed(buttCancel);
    ok = pressed(buttOk);
    
  } while (! (pressed(buttCancel) || pressed(buttOk) ) );   

  if(cancelled) {
    return -1;
  } else {
    return value;
  }

}

int getButtValue(int butt, int value, int incr) {
  int valButt = value;
  if (pressed(butt)) {
      valButt += incr;
      print_out(String(valButt), 1);
      delay(400);
      while (pressed(butt)) {
        valButt += incr;
        print_out(String(valButt), 1);
        delay(200);
      }
  }
  return valButt;
    
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
