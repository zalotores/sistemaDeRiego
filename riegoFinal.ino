/*
 * Controlador para sistema de riego automatico.
 * Muestra la informacion en un lcd a traves de protocolo i2c 
 * y utiliza un reloj para evitar riego en horas de mayor expocision solar.
 * Aprovecha las funciones de fecha, hora y temperatura del reloj para mostrarlas en display
 * Mide la humedad del suelo con sensores FC-28, y habilita señal de riego 
 */

//#include <Wire.h>  ya incluida en liquid Cristal
#include <LiquidCrystal_I2C.h>
#include <RTClib.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);    //declara lcd,  direccion 20 , display 16x2
RTC_DS3231 rtc;     //declara reloj

//array de dias de la semana para mostrar el dia (0 a 6)
char diaSemana [7][10] = { "Domingo", "Lunes", "Martes", "Miercole", "Jueves", "Viernes", "Sabado"};   
int dia, hora;
int temp;

int sensor1 = A2;                                     // sensores de humedad
int sensor2 = A3;
int umbralUno = A0;     //potenciometros para regular umbral
int umbralDos = A1;
int sensado1, sensado2, u1, u2;   // variables para guardar valores de sensor
float porcentajeSensado;
byte actuador1 = 8;   //salida para actuador 1
byte actuador2 = 9;

void setup() {
  pinMode(actuador1, OUTPUT);
  pinMode(actuador2, OUTPUT);

  digitalWrite(actuador1, LOW);
  digitalWrite(actuador2, LOW);
  
  Serial.begin(9600);                             // lectura serie

  // initializar reloj
  if(!rtc.begin()) {
      Serial.println("No se encuentra RTC!");
      Serial.flush();
      while (1) delay(10);
  }
 
  // inicializa con fecha y hora de compilacion
  if(rtc.lostPower()) {
        // carga fecha y hora de compilacion cuando se queda sin bat
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }

  //desabilita puerto 32k
  rtc.disable32K();
  // deshabilita señal clock write
  rtc.writeSqwPinMode(DS3231_OFF);

  lcd.clear();                        //borra lcd
  lcd.init();                        //arranca lcd
  lcd.backlight();                  //enciende backlight

  //debug
  //rtc.adjust(DateTime(2020,11,2,20,21,0));

  delay(1000);

}

void loop() {

  // lectura de umbral humedad para el sensor 1
  u1 = analogRead(umbralUno);                      
  u2 = analogRead(umbralDos); 

  //convierto a porcentaje, cambiando medida de 50 a 1000 para evitar ruido del potenciometro
  if (u1 > 999) {
    porcentajeSensado = 100;
  }
  else if ((u1 <= 999) && (u1 >= 50)) {
     porcentajeSensado = (u1 / 10.0);
  }
  else {
    porcentajeSensado = 0;
  }
  
  u1 = porcentajeSensado;     //vuelvo a int para tener numeros redondos

  if (u2 > 999) {
    porcentajeSensado = 100;
  }
  else if ((u2 <= 999) && (u2 >= 50)) {
     porcentajeSensado = (u2 / 10.0);
  }
  else {
    porcentajeSensado = 0;
  }
  
  u2 = porcentajeSensado;

  sensado1 = analogRead(sensor1);   // lectura sensor, siendo 0 humedad maxima y 1023 sequia maxima
  porcentajeSensado = 100* (1 - (sensado1 / 1023.0));
  sensado1 = porcentajeSensado;

  sensado2 = analogRead(sensor2);
  porcentajeSensado = 100* (1 - (sensado2 / 1023.0));
  sensado2 = porcentajeSensado;

  char date[10] = "hh:mm:ss";   //carga hora en formato correcto
  char dateYear[10] = "DD/MM/YY";   //carga dia, mes y año
  rtc.now().toString(date);
  rtc.now().toString(dateYear);
  dia = rtc.now().dayOfTheWeek();   //dias de la semana: 0 = domingo, 1 = lunes, etc 
  hora = rtc.now().hour();    //capturo la hora para riego

  temp = rtc.getTemperature();

  lcd.clear(); 
  lcd.setCursor(0,0);                        
  lcd.print(diaSemana[dia]);
  lcd.setCursor(8,0);
  lcd.print(dateYear);

  lcd.setCursor(0,1);
  lcd.print(temp);
  lcd.setCursor(3,1);
  lcd.print((char)223);   //simbolo de grados
  lcd.print("C");
  lcd.setCursor(8,1);
  lcd.print(date);
  delay(3000);

  lcd.clear(); 
  lcd.setCursor(0,0);
  lcd.print("S1");
  lcd.setCursor(3,0);
  lcd.print(sensado1);
  lcd.setCursor(6,0);
  lcd.print("%");
  lcd.setCursor(9,0);
  lcd.print("S2");
  lcd.setCursor(12,0);
  lcd.print(sensado2);
  lcd.setCursor(15,0);
  lcd.print("%");
  
  lcd.setCursor(0,1);
  lcd.print("U1");
  lcd.setCursor(3,1);
  lcd.print(u1);
  lcd.setCursor(6,1);
  lcd.print("%");
  lcd.setCursor(9,1);
  lcd.print("U2");
  lcd.setCursor(12,1);
  lcd.print(u2);
  lcd.setCursor(15,1);
  lcd.print("%");
  delay(3000);

  //riega si falta humedad, y entre las 8 y las 10, o entre las 19 y 21 hs
  if ((sensado1 < u1) 
  && (((hora >=8) && (hora < 10)) 
  || ((hora >=19) && (hora < 21)))) {  
    digitalWrite(actuador1, HIGH);
    lcd.clear(); 
    lcd.setCursor(0,0);
    lcd.print("REGANDO");
    lcd.setCursor(0,1);
    lcd.print("BOMBA 1");
    delay(10000);
    digitalWrite(actuador1, LOW);
  }

  if ((sensado2 < u2) 
  && (((hora >=8) && (hora < 10)) 
  || ((hora >=19) && (hora < 21)))) {  
    digitalWrite(actuador2, HIGH);
    lcd.clear(); 
    lcd.setCursor(0,0);
    lcd.print("REGANDO");
    lcd.setCursor(0,1);
    lcd.print("BOMBA 2");
    delay(10000);
    digitalWrite(actuador2, LOW);
  }
}
