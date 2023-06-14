#include <ThingSpeak.h>
#include <DFRobot_RGBLCD1602.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiServer.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <Wire.h>
#include <dummy.h>



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
//const int ledPin = 34;
const int pulPin = 15;
const int bombPin = 4; 
//int actual = (digitalRead(bombPin));
//int tiempo_activo = 0;
float temp_cpu = ((temprature_sens_read() - 32) / 1.8);
int bombaStatus = 0;
int estadoM = 1;
int inicio = 1;
int RSSI;


//waterflow
//#define LED_BUILTIN 
#define SENSOR  2
long currentMillis = 0;
long previousMillis = 0;
int interval = 1000;
boolean ledState = LOW;
float calibrationFactor = 4.5;
volatile byte pulseCount;
byte pulse1Sec = 0;
float flowRate;
unsigned int flowMilliLitres;
unsigned long totalMilliLitres;
int flujo = flowMilliLitres;
int caudal_total = totalMilliLitres;





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

//waterflow
volatile double waterFlow;
void IRAM_ATTR pulseCounter()
{
  pulseCount++;
}




void setup()
{

  //configuramos modo pines
       pinMode(bombPin, OUTPUT);
        digitalWrite(bombPin, LOW);     
        pinMode(pulPin, INPUT);
         
      // pinMode(ledPin, OUTPUT);
      //  digitalWrite(ledPin, LOW);


   
//waterflow
 Serial.begin(115200);

  //pinMode(LED_BUILTIN, OUTPUT);
  pinMode(SENSOR, INPUT);

  pulseCount = 0;
  flowRate = 0.0;
  flowMilliLitres = 0;
  totalMilliLitres = 0;
  previousMillis = 0;

  attachInterrupt(digitalPinToInterrupt(SENSOR), pulseCounter, FALLING);



       
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
    valor_wifi();
   
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
{
     valor_wifi();
    
     breath(lcd.REG_ONLY);
    
 

      //Lectura del pulsador
 int lecturaPin15 = digitalRead(pulPin);
 if(lecturaPin15 == LOW){
 
  Serial.print("Boton activado");
  }


//waterflow
 currentMillis = millis();
  if (currentMillis - previousMillis > interval) {
    
    pulse1Sec = pulseCount;
    pulseCount = 0;

    // Because this loop may not complete in exactly 1 second intervals we calculate
    // the number of milliseconds that have passed since the last execution and use
    // that to scale the output. We also apply the calibrationFactor to scale the output
    // based on the number of pulses per second per units of measure (litres/minute in
    // this case) coming from the sensor.
    flowRate = ((1000.0 / (millis() - previousMillis)) * pulse1Sec) / calibrationFactor;
    previousMillis = millis();

    // Divide the flow rate in litres/minute by 60 to determine how many litres have
    // passed through the sensor in this 1 second interval, then multiply by 1000 to
    // convert to millilitres.
    flowMilliLitres = (flowRate / 60) * 1000;

    // Add the millilitres passed in this second to the cumulative total
    totalMilliLitres += flowMilliLitres;
    
    // Print the flow rate for this second in litres / minute
    lcd.clear();
    lcd.print("Flow rate: ");
    lcd.setCursor(2,2);
    lcd.print(int(flowRate));  // Print the integer part of the variable
    lcd.print("L/min");
 
    Serial.print("\t");       // Print tab space
    
    // Print the cumulative total of litres flowed since starting
   
    Serial.print("Output Liquiq: ");
       

    Serial.print(totalMilliLitres);
    Serial.print("mL / ");
    Serial.print(totalMilliLitres / 1000);
    Serial.print("L");
   
  }
  //Verifica si hay datos nuevos en telegram cada 1 segundo
  if (millis() - tiempoAnterior > tiempo)
  {
    int numerosMensajes = bot.getUpdates(bot.last_message_received + 1);
 
    while (numerosMensajes)
    { 
     
      Serial.print("Comando recibido");
      mensajesNuevos(numerosMensajes);
      numerosMensajes = bot.getUpdates(bot.last_message_received + 1);
    }
 
    tiempoAnterior = millis();
  }
   



 Serial.print("Temp CPU: ");
  
  // Convert raw temperature in F to Celsius degrees
   
  
  Serial.print((temprature_sens_read() - 32) / 1.8);
 
  delay(50);



//arrancar o apagar bomba
int activar = digitalRead(pulPin);
 if ((bombPin == LOW ) && (activar == HIGH)){
 bombaManual();

 }



 
}

/*FUNCIONES*/

//valores conexion wifi
void valor_wifi(){
  lcd.clear();
 lcd.setCursor(0,0);
    lcd.print(WiFi.localIP()); 
    delay (5000);
 lcd.clear();
  lcd.setCursor(0,0);
 lcd.print("Signal Strength: ");
  lcd.setCursor(2,2);
 lcd.print(WiFi.RSSI()); 
 int RSSI = (WiFi.RSSI()) * -1;
 delay (2000);
  write_datos();
}

//Funciona para escribir datos en la nube

void write_datos(){
  
    
  // Hacemos un primer intento de subir los datos. Al usar una cuenta gratuita tenemos limitación sobre la velocidad a la que podemos subir información. 
  // En el caso de que falle, probamos pasados 2 segundos.
  int mensaje = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);

  ThingSpeak.setField(3, temp_cpu); 
   ThingSpeak.setField(4, caudal_total);
   ThingSpeak.setField(1, bombaStatus);
    ThingSpeak.setField(2, flujo);
      ThingSpeak.setField(7, RSSI);
   
  
  /*Escribir datos con marca de tiempo  
tStamp = datetime('now');
ThingSpeak.writeFields(2177896,[2.3,1.2,3.2,0.1],'WriteKey','23ZLGOBBU9TWHG2H','TimeStamp',tStamp);*/

  if (mensaje == 200){
   
    //LCD
 
    Serial.print("Canal Update");
   
   
     

  }
  else{
    delay(2000);
    if (mensaje == 200){
      Serial.print("Canal Update");
    }
    else{
      Serial.print("ERROR: " + String(mensaje));

    Serial.print("ERROR envio");


    }
  }
 }



void bombaManual(){
  int lecturaPin2 = digitalRead(bombPin);//Leemos el estado del pin de la bomba de agua
  if(lecturaPin2 == LOW ){
    digitalWrite(bombPin, HIGH);//Activamos la bomba de agua
    bombaStatus = 1;
    //digitalWrite(ledPin, HIGH);

    
    bot.sendMessage(ID_Chat, "Activada manualmente", "");
    }
      
 if(lecturaPin2 == HIGH ){
      digitalWrite(bombPin, LOW);//Desactiva la bomba de agua
      bombaStatus = 0;
      digitalWrite(bombPin, LOW);
      
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



  
