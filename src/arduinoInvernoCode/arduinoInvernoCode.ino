#include <LiquidCrystal_I2C.h>
#include <Adafruit_NeoPixel.h>

//LEDS
#define UV_PIN 5
#define UV_NUMPIXELS 20

#define WATER_LEVEL_PIN 6
#define WATER_LEVEL_NUMPIXELS 10

//Configuracion ajustable de profundidad del
//recipiente para sistema de riego
int waterContainerDepth = 100;

//Leds config
int delayLEDS = 50;
int UVLedsBrightness = 200;
int selectedUVLedMode;
int desiredUVBirghtness;
Adafruit_NeoPixel UV_pixels = Adafruit_NeoPixel(UV_NUMPIXELS, UV_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel WATER_LEVEL_pixels = Adafruit_NeoPixel(WATER_LEVEL_NUMPIXELS, WATER_LEVEL_PIN, NEO_GRB + NEO_KHZ800);


//Create an LCD
LiquidCrystal_I2C lcd(0x20,16,2);

//BUTTONS
int leftButtonValue = 0;//definir los botones como 
int middleButtonValue = 0;//variables enteras
int rightButtonValue = 0;
int leftButtonPin = 2;//configurar a qué pin
int middleButtonPin = 3;//esta conectado cada
int rightButtonPin = 4;//botón

//Actuators and sensors PINS
int waterSensorPin = A0;//sensor de humedad o agua
int photoresistorSensorPin = A1;//fotorresistencia
int motorControllerPin = 7;//motor pin
int trigUltrasonicSensor = 8; 
int echoUltrasonicSensor = 9; 

int waterSensorValue;//inicializar sensor de agua
int photoresistorSensorValue;//inicializar sensor de luz

//Variables para sensor ultrasonico
long duration = 0;  
int cm;
int mappedUltrasonicSensorValue;

int selectedMenuNumber = 2;
int randNumber;

void setup(){
  //Iniciamos LCD
  lcd.init();
  lcd.backlight();
  
  //Iniciamos leds
  UV_pixels.begin();
  WATER_LEVEL_pixels.begin();

  //Llamamos al menu con imagen de carga
  menuLCD(1);
    
  //Pump motor setup
  pinMode(motorControllerPin, OUTPUT);
  
  //Every pin configuration
  pinMode(leftButtonPin, INPUT_PULLUP);
  pinMode(middleButtonPin, INPUT_PULLUP);
  pinMode(rightButtonPin, INPUT_PULLUP);
  pinMode(trigUltrasonicSensor, OUTPUT);  
  pinMode(echoUltrasonicSensor, INPUT);  

  Serial.begin(9600);

}

void menuLCD(int screenMode){
  /*ScreenMode presets
  
  1 = Loading screen
  2 = Welcome to Inverno!
  3 = Soil moist
  4 = Light information
  5.- Water level*/
  
  //Custom symbol Setup
    byte customCharLeft[] = {
      B10010,
      B01001,
      B00100,
      B10010,
      B01001,
      B00100,
      B00010,
      B00001
    };
   //Left part of upper symbol
   lcd.createChar(0, customCharLeft);

    byte customCharRight[] = {
      B01001,
      B10010,
      B00100,
      B01001,
      B10010,
      B00100,
      B01000,
      B10000
    };
  	//Right part of upper symbol
    lcd.createChar(1, customCharRight);
  
    switch (screenMode){
      case 1://Mostrar "loading..." en pantalla
        delay(300);  
      	setUVLedMode(1);
        lcd.setCursor(0,0);
        lcd.noAutoscroll();
        lcd.print("Loading...");
        setUVLedMode(1);
        delay(300);

        lcd.setCursor(0,1);

        for (int i = 0; i <= 16; ++i) {
          lcd.print("/");
          randNumber = random(100, 500);
          delay(randNumber);
        }
      
        lcd.clear();
        lcd.noDisplay();
        lcd.setCursor(0,0);
        lcd.print("All systems go!");

        lcd.setCursor(0,1);
        lcd.print("- - - - - - - - ");

        for (int i = 0; i <= 5; ++i){
          lcd.display();
          delay(200);
          lcd.noDisplay();
          delay(200);
         }
        lcd.clear();
        lcd.display();
        delay(1000);
      break;

      case 2://Mostrar "Welcome to Inverno" en pantalla
        lcd.setCursor(0,0);
        lcd.print("   Welcome to   ");
        lcd.setCursor(0,1);
        lcd.print("    Inverno!    ");
      
        lcd.setCursor(0,1);
        lcd.print("<");
        lcd.setCursor(15,1);
        lcd.print(">");
      break;

      case 3://Mostrar nivel de humedad de la tierra
              //de acuerdo a los datos del sensor
        lcd.setCursor(0,0);
        lcd.print(" Soil moist: " + String(getWaterSensorReadings()) + "% ");


        //Write left part of upper symbol
        lcd.setCursor(7,1);
        lcd.write(0);
        //Write right part of upper symbol
        lcd.setCursor(8,1);
        lcd.write(1);

        //símbolos decorativos en pantalla
        lcd.setCursor(0,1);
        lcd.print("<");
        lcd.setCursor(15,1);
        lcd.print(">");

      break;

      case 4://Mostrar nivel de luz de acuerdo
             //con la fotorresistencia
        lcd.setCursor(0,0);
        lcd.print(" Light info: " + String(getPhotoresistorReadings()) + "% ");


        //Write left part of upper symbol
        lcd.setCursor(7,1);
        lcd.write(0);
        //Write right part of upper symbol
        lcd.setCursor(8,1);
        lcd.write(1);

        //símbolos decorativos en pantalla
        lcd.setCursor(0,1);
        lcd.print("<");
        lcd.setCursor(15,1);
        lcd.print(">");
        lcd.setCursor(0,0);
      break;
      
      case 5: //Mostrar el nivel de agua restante 
      		  //para sistema de riego
      	lcd.setCursor(0,0);
      
        if(mappedUltrasonicSensorValue<=0){
           lcd.print("Water level: 00%");
        }
        else{
             lcd.print("Water level: " + String(mappedUltrasonicSensorValue*10) + "%");
        }

        //símbolos decorativos en pantalla
        lcd.setCursor(0,1);
        lcd.print("<");
        lcd.setCursor(15,1);
        lcd.print(">");
      break;
      }
  }

//Funci'on que configura el status del led UV
void setUVLedMode(int ledMode){
  if(getPhotoresistorReadings() <= 60 || middleButtonValue == LOW && selectedMenuNumber == 4){
    desiredUVBirghtness = 200-((200*getPhotoresistorReadings())/60);
    
    UV_pixels.setBrightness(desiredUVBirghtness);
    selectedUVLedMode = 2;
    
    if(middleButtonValue == LOW && selectedMenuNumber == 4){
    	UV_pixels.setBrightness(200);
    }
   }
   else{
    selectedUVLedMode = 3;
   }
  
  /*UVled presets
  
  1 = Bouncing effect
  2 = Solid purple
  3 = Off*/  
  
  switch(ledMode){
    case 1: //Bouncing effect (para efecto de inicio)
        UV_pixels.clear();
        for(int i=-1; i<=UV_NUMPIXELS; i++){
          UV_pixels.setPixelColor(i, UV_pixels.Color(160, 32, 240));
          UV_pixels.show();
          delay(delayLEDS); 
        }
        UV_pixels.clear();

        for(int e=UV_NUMPIXELS; e>=0; e--){
          UV_pixels.setPixelColor(e, UV_pixels.Color(160, 32, 240));
          UV_pixels.show();
          delay(delayLEDS); 
        }
        UV_pixels.clear();
      break;
    
  	case 2: //Color morado solido
      for(int i=0; i<UV_NUMPIXELS; i++) {

        UV_pixels.setPixelColor(i, UV_pixels.Color(160, 32, 240));
        UV_pixels.show();
        }
    	break;
    
    case 3: //Sin color
     for(int i=0; i<UV_NUMPIXELS; i++) {

        UV_pixels.setPixelColor(i, UV_pixels.Color(0, 0, 0));
        UV_pixels.show();
        }
    	break;
  }
}

//Configuracion para tira led medidor de agua para riego
void setWaterLevelLed(){
  //Sacamos el valor del sensor y lo ponemos a proporcion de 1 al 10
  mappedUltrasonicSensorValue = 10 - (map (getUltrasonicSensor(), 16, waterContainerDepth, 0, 10));
  
      for(int i=0; i<mappedUltrasonicSensorValue; i++) {
        WATER_LEVEL_pixels.setPixelColor(i, UV_pixels.Color(0, 25, 255));
        WATER_LEVEL_pixels.show();
       }
  
      for(int i=10; i>mappedUltrasonicSensorValue; i--) {
        WATER_LEVEL_pixels.setPixelColor(i, UV_pixels.Color(0, 0, 0));
        WATER_LEVEL_pixels.show();
       }

  Serial.println(mappedUltrasonicSensorValue);
}


//Función para mover el motor de la bomba de agua
void moveMotor(){//Si la humedad es menor o igual a 45, se activa
  
  if(getWaterSensorReadings() <= 45 && getUltrasonicSensor() <= waterContainerDepth || middleButtonValue == LOW && selectedMenuNumber == 3){
    digitalWrite(motorControllerPin, HIGH);
  }
  else{//Si la humedad es mayor a 45, no se activa
     digitalWrite(motorControllerPin, LOW);
  }
  
}
//Leer el valor del nivel de humedad
int getWaterSensorReadings(){
  waterSensorValue = map (analogRead(waterSensorPin), 0, 876, 0,100);
  return waterSensorValue;
}
//ler el valor del nivel de luz
int getPhotoresistorReadings(){
  photoresistorSensorValue = map (analogRead(photoresistorSensorPin), 6, 679, 0, 100);
  return photoresistorSensorValue;
}

//Leer el sensor ultrasonico
int getUltrasonicSensor(){
  digitalWrite(trigUltrasonicSensor,LOW); 
  digitalWrite(trigUltrasonicSensor,HIGH); 
  digitalWrite(trigUltrasonicSensor,LOW); 
  
  int duration = pulseIn (echoUltrasonicSensor, HIGH);  
  cm = duration * 0.0344 / 2;
  return cm;
}

void loop()
{
  //Menu button config.
  leftButtonValue = digitalRead(leftButtonPin);
  middleButtonValue = digitalRead(middleButtonPin);
  rightButtonValue = digitalRead(rightButtonPin);
  
  //Configuracion para usar los botones de navegacion lcd
  if(middleButtonValue == LOW && selectedMenuNumber != 2 && selectedMenuNumber != 5){
  	lcd.noDisplay();
    delay(120);
    lcd.display();
  }
  if(rightButtonValue == LOW){
  	selectedMenuNumber = selectedMenuNumber + 1;
    lcd.clear();
  }
  if(leftButtonValue == LOW){
  	selectedMenuNumber = selectedMenuNumber - 1;
    lcd.clear();
  }
  
 //We set the menu button limits
  if(selectedMenuNumber >= 6){
  	selectedMenuNumber = 2;
  }
  if(selectedMenuNumber <= 1){
  	selectedMenuNumber = 5;
  }
  
  //Llamamos la funcion del motor
  moveMotor();
 
  //Llamamos la funcion de actualizar la tira led en base
  //al nivel del agua para riego
  setWaterLevelLed();
  
  //Llamamos a la funcion para actualizar el estado de la tira led UV
  setUVLedMode(selectedUVLedMode);  
  menuLCD(selectedMenuNumber);  // Ahora puedes cambiar el valor que pasas a menuLCD()
}