// Programa : Sensor de temperatura DS18B20
// Autor : Denis Eiras


// *** Crystal Liq. ***
#include <LiquidCrystal_I2C.h>
// initialize the library with the numbers of the interface pins
LiquidCrystal_I2C lcd(0x27,2,1,0,4,5,6,7,3, POSITIVE);

 
// *** DS18B20 ***
#include <OneWire.h>
#define ONE_WIRE_BUS 12
OneWire oneWire(ONE_WIRE_BUS);
// Define uma instancia do oneWire para comunicacao com o sensor

#include <DallasTemperature.h> 
DallasTemperature sensors(&oneWire);
DeviceAddress sensor1;

// *** System ***
// Armazena temperaturas minima e maxima
float tempMin = 999;
float tempMax = 0;
int buttCancel = 8;       //Botão para cancelar
int buttOk = 9;           //Botão para confirmar
bool backHigh = true;

  
void setup(void)
{
  Serial.begin(9600);
  sensors.begin();
  // Localiza e mostra enderecos dos sensores
  Serial.println("Localizando sensores DS18B20...");
  Serial.print("Foram encontrados ");
  Serial.print(sensors.getDeviceCount(), DEC);
  Serial.println(" sensores.");
  if (!sensors.getAddress(sensor1, 0)) 
     Serial.println("Sensores nao encontrados !"); 
  // Mostra o endereco do sensor encontrado no barramento
  Serial.print("Endereco sensor: ");
  mostra_endereco_sensor(sensor1);
  Serial.println();
  Serial.println();
  lcd.begin(16, 2);
  lcd.setBacklight(HIGH);
   
}
 
void mostra_endereco_sensor(DeviceAddress deviceAddress)
{
  for (uint8_t i = 0; i < 8; i++)
  {
    // Adiciona zeros se necessário
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
}
 
void loop()
{
  // Le a informacao do sensor
  sensors.requestTemperatures();
  float tempC = sensors.getTempC(sensor1);

  if(pressed(buttOk)) {
    backHigh = !backHigh;
    if(backHigh) {
      lcd.setBacklight(HIGH);
    } else {
      lcd.setBacklight(LOW);
    }
  }
  // Atualiza temperaturas minima e maxima
  if (tempC < tempMin)
  {
    tempMin = tempC;
  }
  if (tempC > tempMax)
  {
    tempMax = tempC;
  }
  // Mostra dados no serial monitor
  Serial.print("Temp C: ");
  Serial.print(tempC);
  Serial.print(" Min : ");
  Serial.print(tempMin);
  Serial.print(" Max : ");
  Serial.println(tempMax);
   
  // Mostra dados no LCD  
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Temp.:       ");
  //Simbolo grau
  lcd.write(223);
  lcd.print("C");
  lcd.setCursor(7,0);
  lcd.print(tempC);
  lcd.setCursor(0,1);
  lcd.print("L: ");
  lcd.setCursor(3,1);
  lcd.print(tempMin,1);
  lcd.setCursor(8,1);
  lcd.print("H: ");
  lcd.setCursor(11,1);
  lcd.print(tempMax,1);
  delay(5000);
}

bool pressed(int butt) {
  return digitalRead(butt) == HIGH;
}
