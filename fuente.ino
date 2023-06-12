#include <ThingSpeak.h>
#include <DFRobot_RGBLCD1602.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiServer.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

//sensor interno ESP32
#ifdef __cplusplus
extern "C" {
#endif
uint8_t temprature_sens_read();
#ifdef __cplusplus
}
#endif#include <ThingSpeak.h>
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
      



// Usuarios y claves para la conexión WiFi
char ssid[] = "Livebox6-767D"; //SSID - Red WiFi a la que me conecto
char pass[] = "dvK6G3QnFLbz"; // Passowrd de la red WiFi

WiFiClient  client;

//Mientras no creo app, lo accionaremos por telegram
//Token de Telegram BOT se obtenienen desde Botfather en telegram
#define BOT_TOKEN "6290093904:AAEH4SRhlyax4rqqXLPtmgNWrlDeYB9HD_4"
#define ID_Chat "5928346727"//ID_Chat se obtiene de telegram
const unsigned long tiempo = 1000; //tiempo medio entre escaneo de mensajes
String datos;
String chat_id;
WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);
unsigned long tiempoAnterior; //última vez que se realizó el análisis de mensajes


// Variables para definir la conexión con ThingSpeak
unsigned long myChannelNumber = 2177896; //Código de canal de Things Speak
const char * myWriteAPIKey = "XU5MUD4ZN8IJ1A94"; // Indicar aquí el código de escritura de ThingSpeak

//variables del esp32
const int ledPin = 34;
const int pulPin = 15;
const int bombPin = 2; 
//int actual = (digitalRead(bombPin));
//int tiempo_activo = 0;
float temp = ((temprature_sens_read() - 32) / 1.8);
int bombaStatus = 0;
int estadoM = 1;
int inicio = 1;





  

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
        digitalWrite(bombPin, LOW);     
        pinMode(pulPin, INPUT);
         
       // pinMode(ledPin, OUTPUT);
        //digitalWrite(ledPin, LOW);
   

       
  
       
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

     secured_client.setCACert(TELEGRAM_CERTIFICATE_ROOT); //Agregar certificado raíz para api.telegram.org
  

//Minetras se conecta imprimirá ...
while(WiFi.status() != WL_CONNECTED){
  delay(5000);
  
  
//Ya que se estableció la conexión al Wifi se imprime conexión establecida
  lcd.clear();
       lcd.setCursor(1, 0);
    lcd.print("Conectado");
   
 delay (2000);
} 
ThingSpeak.begin(client); //Iniciar el servidor de ThingSpeak
if(inicio == 1){
  
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Sistema listo");
    delay (2000);
    bot.sendMessage(ID_Chat, "Sistema preparado!!!, escribe /Ayuda para ver las opciones", "");//Enviamos un mensaje a telegram para informar que el sistema está listo
    inicio = 0;

}
}


void loop()
{/*
   estadoBomba();
    actual = (digitalRead(bombPin));*/
    
     breath(lcd.REG_ONLY);

      //Lectura del pulsador
 int lecturaPin15 = digitalRead(pulPin);
 if(lecturaPin15 == LOW){
  lcd.clear();
   lcd.setCursor(0, 0);
  lcd.print("Boton activado");
  delay (2000);
  lcd.clear();
 
 }
  //Verifica si hay datos nuevos en telegram cada 1 segundo
  if (millis() - tiempoAnterior > tiempo)
  {
    int numerosMensajes = bot.getUpdates(bot.last_message_received + 1);
 
    while (numerosMensajes)
    { 
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Comando recibido");
      mensajesNuevos(numerosMensajes);
      numerosMensajes = bot.getUpdates(bot.last_message_received + 1);
    }
 
    tiempoAnterior = millis();
  }
   


 /*     
  if ((digitalRead(pulPin) == HIGH) && (actual == 0)){
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
   
  
   */



 Serial.print("Temp CPU: ");
  
  // Convert raw temperature in F to Celsius degrees
   
  
  Serial.print((temprature_sens_read() - 32) / 1.8);
 
  delay(50);

// check_flujo(); //si la bomba esta encendida revisar que exista flujo a su salida

//arrancar o apagar bomba
int activar = digitalRead(pulPin);
 if ((bombPin == LOW ) && (activar == HIGH)){
 bombaManual();}
 write_datos();
}

/*FUNCIONES*/

/*
void estadoBomba(){
 actual = (digitalRead(bombPin));
  if(actual == HIGH){ 
 
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
  
   
     digitalWrite(bombPin, HIGH); 
     digitalWrite(ledPin, HIGH);
    digitalWrite(actual, HIGH);
         lcd.clear();
       lcd.setCursor(0, 0);
    lcd.print("ON!!");
   
    delay (5000);
  
    
  }


  void  parar(){
    actual = (digitalRead(bombPin));
    if ((actual == 1) && (pulPin == LOW))
       digitalWrite(bombPin, LOW); 
     digitalWrite(ledPin, LOW);
   
 
          lcd.clear();
       lcd.setCursor(0, 0);
    lcd.print("STOP!!");
    lcd.setCursor(1, 1);
    delay (5000);
    
    
  }
  
 */

//Funciona para escribir datos en la nube

void write_datos(){
   //actual = (digitalRead(bombPin));
  //LCD
  lcd.clear();
       lcd.setCursor(0, 0);
    lcd.print("Enviando...");
    delay (2000);

    
  // Hacemos un primer intento de subir los datos. Al usar una cuenta gratuita tenemos limitación sobre la velocidad a la que podemos subir información. 
  // En el caso de que falle, probamos pasados 2 segundos.
  int mensaje = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);

  ThingSpeak.setField(3, temp); 
  // ThingSpeak.setField(2, tiempo_activo);
   ThingSpeak.setField(1, bombaStatus);
  
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
                                                                                       
    

}
/*
//sensor de flujo

float calibrationFactor = 7;//7.5;
volatile byte pulseCount;
 
float flowRate = 0;
 
unsigned long oldTime = 0;
/*
  Insterrupt Service Routine

void IRAM_ATTR pulseCounter()
{
  // Increment the pulse counter
  pulseCount++;
}
void check_flujo() {

 actual = (digitalRead(bombPin));
  if (actual == HIGH) {
    if ((millis() - oldTime) > 1000)   // Only process counters once per second
    {
 
      detachInterrupt(sensorPin);
      flowRate = ((1000.0 / (millis() - oldTime)) * pulseCount) / calibrationFactor;//flujo en litros por minuto
      oldTime = millis();
 
      //    unsigned int frac;
 
      // Print the flow rate for this second in litres / minute
      Serial.print("Flow rate: ");
      Serial.print(int(flowRate));  // Print the integer part of the variable
      Serial.println("L/min");
 
      lcd.setCursor(0, 0);
      lcd.print(flowRate, 0);
 
      // Reset the pulse counter so we can start incrementing again
      pulseCount = 0;
 
      // Enable the interrupt again now that we've finished sending output
      attachInterrupt(sensorPin, pulseCounter, FALLING);
    }
    // se chequea que el flujo no sea menor que 5
    if (int(flowRate) <= 5) { //no esta bombeando eficientemente o no lo esta haciendo
     
      bombaFail_counter ++;
      if (bombaFail_counter > 5) {
        digitalWrite(bombPin, LOW);
        lcd.clear();
        lcd.print("Error de bomba");
        lcd.display();
        while (1) {}
      }
 
    }
  }
  else { //si la bomba esta apagada
    lcd.setCursor(0, 0);
    lcd.print("  ");
    lcd.setCursor(0, 0);
    lcd.print("0");//no hay flujo, esta apagada la bomba
  }
}

*/

void bombaManual(){
  int lecturaPin2 = digitalRead(bombPin);//Leemos el estado del pin de la bomba de agua
  if(lecturaPin2 == LOW ){
    digitalWrite(bombPin, HIGH);//Activamos la bomba de agua
    bombaStatus = 1;
    delay(200);
    bot.sendMessage(ID_Chat, "Activada manualmente", "");
    }
      
 if(lecturaPin2 == HIGH ){
      digitalWrite(bombPin, LOW);//Desactiva la bomba de agua
      bombaStatus = 0;
      delay(200);
      bot.sendMessage(ID_Chat, "Bomba de agua desactivada manualmente", ""); 
}
}

void mensajesNuevos(int numerosMensajes)
{
  for (int i = 0; i < numerosMensajes; i++)
  {
    chat_id = bot.messages[i].chat_id;
    String text = bot.messages[i].text;
//////////Activa la Bomba de agua activada indefinidamente //////
    if (text == "/Activar")
    {
      digitalWrite(bombPin, HIGH); 
      bombaStatus = 1;
      bot.sendMessage(chat_id, "Bomba de agua activada indefinidamente", "");
    }

   
 
///////////Desactiva la bomba de agua///////////////////////////
 
    if (text == "/Apagar")
    {
      digitalWrite(bombPin, LOW);
      bombaStatus = 0;
      bot.sendMessage(chat_id, "Bomba de agua apagada", "");
    }  
 
////////Estado de la bomba de agua/////////     
 
    if (text == "/Estado")
    {
      if (bombaStatus)
      {
        bot.sendMessage(chat_id, "Estado actual: Bomba de agua encendida", "");
      }
      else
      {
        bot.sendMessage(chat_id, "Estado actual: Bomba de agua apagada", "");
      }
    }
////////Imprime el menú de ayuda//////////
    if (text == "/Ayuda")
    {
      String ayuda = "Fuente Jardín " ".\n";
      ayuda += "Menú:\n\n";
      ayuda += "/Activar: Activa el la bomba de agua. \n";
      ayuda += "/Apagar: Desactiva el la bomba de agua. \n";
      ayuda += "/Estado: de la bomba\n";
      ayuda += "/Ayuda: Imprime este menú \n";
      ayuda += "Recuerda el sistema distingue entre mayuculas y minusculas \n";
      bot.sendMessage(chat_id, ayuda, "");
    }    
  }
}
  
