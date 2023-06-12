#include <ThingSpeak.h>
#include <DFRobot_RGBLCD1602.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiServer.h>

//sensor interno ESP32
#ifdef __cplusplus
extern "C" {
#endif
uint8_t temprature_sens_read();
#ifdef __cplusplus
}
#endif



// Usuarios y claves para la conexión WiFi
char ssid[] = "Livebox6-767D"; //SSID - Red WiFi a la que me conecto
char pass[] = "dvK6G3QnFLbz"; // Passowrd de la red WiFi

WiFiClient  client;


// Variables para definir la conexión con ThingSpeak
unsigned long myChannelNumber = 2177896; //Código de canal de Things Speak
const char * myWriteAPIKey = "XU5MUD4ZN8IJ1A94"; // Indicar aquí el código de escritura de ThingSpeak

//variables del esp32
const int ledPin = 34;
const int pulPin = 15;
const int bombPin = 2;
bool estadoBot =  false; 
bool estado = (digitalRead(estadoBot));
int actual = (digitalRead(bombPin));
int tiempo_activo = 0;
float temp = ((temprature_sens_read() - 32) / 1.8);



  

//Configurar LCD
DFRobot_RGBLCD1602 lcd(/*RGBAddr*/0x6B ,/*lcdCols*/16,/*lcdRows*/2);  //16 characters and 2 lines of show

void breath(unsigned char color){
    for(int i=0; i<255; i++){
        lcd.setPWM(color, i);  
        delay(5);
    }

    delay(500);
    for(int i=254; i>=0; i--){
        lcd.setPWM(color, i);
        delay(5);
    }

    delay(500);
}



/****************************************
*Funciones principales
****************************************/

void setup()
{

  //configuramos modo pines
       pinMode(bombPin, OUTPUT);
     
        pinMode(pulPin, INPUT);
       
      
       pinMode(ledPin, OUTPUT);
  
       
// initialize LCD
    lcd.init();
    // Print a message to the LCD.
    lcd.setCursor(1, 0);
    lcd.print("Fuente Jardin");
    lcd.setCursor(1, 1);
    delay(2000);
    lcd.clear();
      lcd.setCursor(1, 0);
    lcd.print("Iniciando...");
    delay(2000);


//Imprimir mensaje de conexión a la red Wifi
WiFi.begin(ssid, pass); //Se inicia la conexión al Wifi
  WiFi.mode(WIFI_STA);
//en el LCD
  lcd.clear();
       lcd.setCursor(1, 0);
    lcd.print("Conectando...");
    
 delay (2000);

  

//Minetras se conecta imprimirá ...
while(WiFi.status() != WL_CONNECTED){
  delay(5000);
  
  
//Ya que se estableció la conexión al Wifi se imprime conexión establecida
  lcd.clear();
       lcd.setCursor(1, 0);
    lcd.print("Conectado");
   
 delay (2000);

  

   
  
    
    
ThingSpeak.begin(client); //Iniciar el servidor de ThingSpeak

}
}


void loop()
{
    actual = (digitalRead(bombPin));
     breath(lcd.REG_ONLY);
    estadoBomba();
      
  if (digitalRead(pulPin)== HIGH){
  actual = (digitalRead(bombPin));
  if (actual == 0){
    lcd.clear();
        lcd.setCursor(0, 0);
  lcd.print("Arrancando...");
  delay(2000);
    
    arrancar();
   
   
 }else {
lcd.clear();
        lcd.setCursor(1, 0);
  lcd.print("Parando...");
  delay(2000);
     parar();
 }
   }
  
   



 Serial.print("Temp CPU: ");
  
  // Convert raw temperature in F to Celsius degrees
   
  
  Serial.print((temprature_sens_read() - 32) / 1.8);
 
  delay(5000);

 

 write_datos();
}

/*FUNCIONES*/


void estadoBomba(){
 actual = (digitalRead(bombPin));
  if(actual == 1){ 
 
     lcd.clear();
       lcd.setCursor(0, 0);
    lcd.print("Estado actual:");
    lcd.setCursor(1, 1);
    lcd.print("Activa");

    delay(2000);
 
  
 }else{  
  lcd.clear();
       lcd.setCursor(0, 0);
          lcd.print("Estado actual:");
            lcd.setCursor(1, 1);
    lcd.print("Apagada");
 
    delay (2000);
 }
 }


void  arrancar(){
  actual = (digitalRead(bombPin));
   if ((actual == 0) && (pulPin== HIGH)){
     digitalWrite(bombPin, HIGH); 
     digitalWrite(ledPin, HIGH);
    digitalWrite(estadoBot, true);
         lcd.clear();
       lcd.setCursor(0, 0);
    lcd.print("ON!!");
   
    delay (5000);
   

    
  }
}

  void  parar(){
    actual = (digitalRead(bombPin));
    if ((actual == 1) && (pulPin == LOW))
       digitalWrite(bombPin, LOW); 
     digitalWrite(ledPin, LOW);
   
    digitalWrite(estadoBot, false);
          lcd.clear();
       lcd.setCursor(0, 0);
    lcd.print("STOP!!");
    lcd.setCursor(1, 1);
    delay (5000);
    
    
  }
  
 

//Funciona para escribir datos en la nube

void write_datos(){
   actual = (digitalRead(bombPin));
  //LCD
  lcd.clear();
       lcd.setCursor(0, 0);
    lcd.print("Enviando...");
    delay (2000);

    
  // Hacemos un primer intento de subir los datos. Al usar una cuenta gratuita tenemos limitación sobre la velocidad a la que podemos subir información. 
  // En el caso de que falle, probamos pasados 2 segundos.
  int mensaje = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);

  ThingSpeak.setField(3, temp); 
   ThingSpeak.setField(2, tiempo_activo);
   ThingSpeak.setField(1, actual);
  
  /*Escribir datos con marca de tiempo  
tStamp = datetime('now');
ThingSpeak.writeFields(2177896,[2.3,1.2,3.2,0.1],'WriteKey','23ZLGOBBU9TWHG2H','TimeStamp',tStamp);*/

  if (mensaje == 200){
   
    //LCD
  lcd.clear();
       lcd.setCursor(0, 0);
    lcd.print("Canal Update");
    lcd.setCursor(1, 1);
    delay (5000);
   
       lcd.clear();
       lcd.setCursor(0, 0);
    lcd.print("Fuente jardin");
    lcd.setCursor(1, 1);
    lcd.print(actual);
 delay (2000);

  }
  else{
    delay(2000);
    if (mensaje == 200){
      Serial.print("Canal Update");
    }
    else{
      Serial.print("ERROR: " + String(mensaje));
      //LCD
  lcd.clear();
       lcd.setCursor(0, 0);
    lcd.print("ERROR envio");
    lcd.setCursor(1, 1);
 delay (2000);

    }
  }
      
