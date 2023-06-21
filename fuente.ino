// Programa básico bomba de agua, leds, control temp cpu y señal wifi.

#include <analogWrite.h>
#include <dummy.h>
#include <ThingSpeak.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiServer.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <Wire.h>




//sensor interno ESP32
#ifdef __cplusplus
extern "C" {
#endif
  uint8_t temprature_sens_read();

#ifdef __cplusplus
}
#endif


// Usuarios y claves para la conexión WiFi
char ssid[] = "MIWIFI_j7Ff";  //SSID - Red WiFi a la que me conecto
char pass[] = "*****";     // Passowrd de la red WiFi

WiFiClient  client;


//Mientras no creo app, lo accionaremos por telegram
//Token de Telegram BOT se obtenienen desde Botfather en telegram
#define BOT_TOKEN "*********"
#define ID_Chat "*******"        //ID_Chat se obtiene de telegram
const unsigned long tiempo = 1000;  //tiempo medio entre escaneo de mensajes
String datos;
String chat_id;
WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);
unsigned long tiempoAnterior;  //última vez que se realizó el análisis de mensajes


// Variables para definir la conexión con ThingSpeak
unsigned long myChannelNumber = 2177896;         //Código de canal de Things Speak
const char* myWriteAPIKey = "********";  // Indicar aquí el código de escritura de ThingSpeak
//Temporizador ThingSpeak XXseg
unsigned long ultimoTiempo = 0; // almacena la ultima vez que se lanzo nuestro evento
unsigned long intervaloEnvio = 20000; // 20 segundos

//variables del esp32
//const int ledPin = 25;
const int pulPin = 15;
const int bombPin = 4;
float temp_cpu = ((temprature_sens_read() - 32) / 1.8);
int bombaStatus = 0;
int estadoM = 1;
int inicio = 1;
long RSSI;




//SETUP

void setup() {
  
  Serial.begin(115200);
 

  //configuramos modo pines
  pinMode(bombPin, OUTPUT);
  digitalWrite(bombPin, LOW);
  pinMode(pulPin, INPUT);

//Inicio
  Serial.println("Fuente Jardin");
  delay(1000);
  Serial.println("Iniciando...");
  delay(2000);


  //Imprimir mensaje de conexión a la red Wifi
  WiFi.begin(ssid, pass);  //Se inicia la conexión al Wifi
  WiFi.mode(WIFI_STA);

  Serial.println("Conectando...");


  secured_client.setCACert(TELEGRAM_CERTIFICATE_ROOT);  //Agregar certificado raíz para api.telegram.org


  //Minetras se conecta imprimirá ...
  while (WiFi.status() != WL_CONNECTED) {
    delay(5000);


    //Ya que se estableció la conexión al Wifi se imprime conexión establecida

    Serial.println("Conectado");
    delay(2000);
     Serial.println(WiFi.localIP());
     delay(2000);
   
  }
  ThingSpeak.begin(client);  //Iniciar el servidor de ThingSpeak
  if (inicio == 1) {

    Serial.println("Sistema listo");
    delay(200);
    bot.sendMessage(ID_Chat, "Sistema preparado!!!, escribe /Ayuda para ver las opciones", "");  //Enviamos un mensaje a telegram para informar que el sistema está listo
    inicio = 0;
  }



  
}


void loop() {
 
   
  //Lectura del pulsador
  int lecturaPin15 = digitalRead(pulPin);
  if (lecturaPin15 == LOW) {

    Serial.println("Boton activado");
  }


  //Verifica si hay datos nuevos en telegram cada 1 segundo
  if (millis() - tiempoAnterior > tiempo) {
    int numerosMensajes = bot.getUpdates(bot.last_message_received + 1);

    while (numerosMensajes) {

      Serial.println("Comando recibido");
      mensajesNuevos(numerosMensajes);
      numerosMensajes = bot.getUpdates(bot.last_message_received + 1);
    }

    tiempoAnterior = millis();
    
  }


//arrancar o apagar bomba
  int activar = digitalRead(pulPin);
  if ((bombPin == LOW) && (activar == HIGH)) {
    
    bombaManual();//activamos bomba
  }

  
  //llamada a funciones en cadena
  cpu();
   valor_wifi();
   update();
   


}


/*FUNCIONES*/

  //actualizar estadisticas cada 20sec
 

//Temp CPU
void cpu(){
  Serial.print("Temp CPU: ");
  // Convert raw temperature in F to Celsius degrees
  Serial.println((temprature_sens_read() - 32) / 1.8);

  delay (1000);
   
    

}



//valores conexion wifi
void valor_wifi() {
 
    Serial.print("RSSI: ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");
    RSSI = ((WiFi.RSSI())* (-1)); 
    delay (2000);
      
}

//Funciona para escribir datos en la nube

void update() {


  // Hacemos un primer intento de subir los datos. Al usar una cuenta gratuita tenemos limitación sobre la velocidad a la que podemos subir información.
  // En el caso de que falle, probamos pasados 5 segundos.
  int mensaje = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);

  ThingSpeak.setField(2, temp_cpu);
  
  ThingSpeak.setField(1, bombaStatus);
 
  ThingSpeak.setField(3, RSSI);
  
   // subimos los datos a nuestro canal en ThingSpeak
    //200 es el codigo tipico de una transmision OK en el protocolo HTTP
    int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
    if (x == 200) {
      Serial.println("Update!!");
    
      delay (5000);
    }
    else {
      Serial.println("HTTP error code " + String(x));
      delay (2000);
    }

  /*Escribir datos con marca de tiempo  
tStamp = datetime('now');
ThingSpeak.writeFields(2177896,[2.3,1.2,3.2,0.1],'WriteKey','23ZLGOBBU9TWHG2H','TimeStamp',tStamp);*/

}



void bombaManual() {
  int lecturaPin2 = digitalRead(bombPin);  //Leemos el estado del pin de la bomba de agua
  if (lecturaPin2 == LOW) {
    digitalWrite(bombPin, HIGH);  //Activamos la bomba de agua
    bombaStatus = 1;
    //digitalWrite(ledPin, HIGH); //Activamos leds
   
    delay (200);


    bot.sendMessage(ID_Chat, "Activada manualmente", "");
    delay (50);
    

   
  }

  if (lecturaPin2 == HIGH) {
    digitalWrite(bombPin, LOW);  //Desactiva la bomba de agua
    bombaStatus = 0;
     //digitalWrite(ledPin, HIGH); //Desctivamos leds
     delay (200);
    

    bot.sendMessage(ID_Chat, "Bomba de agua desactivada manualmente", "");
    delay (50);
    
  }
}

void mensajesNuevos(int numerosMensajes) {
  for (int i = 0; i < numerosMensajes; i++) {
    chat_id = bot.messages[i].chat_id;
    String text = bot.messages[i].text;
    //////////Activa la Bomba de agua activada indefinidamente //////
    if (text == "/Activar") {
      digitalWrite(bombPin, HIGH);
      bombaStatus = 1;
      bot.sendMessage(chat_id, "Bomba de agua activada indefinidamente", "");
      delay (50);
     
    }



    ///////////Desactiva la bomba de agua///////////////////////////

    if (text == "/Apagar") {
      digitalWrite(bombPin, LOW);
      bombaStatus = 0;
      bot.sendMessage(chat_id, "Bomba de agua apagada", "");
       delay (50);
     
    }

    ////////Estado de la bomba de agua/////////

    if (text == "/Estado") {
      if (bombaStatus) {
        bot.sendMessage(chat_id, "Estado actual: Bomba de agua encendida", "");
         delay (50);
      update();
      delay (50);
      } else {
        bot.sendMessage(chat_id, "Estado actual: Bomba de agua apagada", "");
         delay (50);
     
      }
    }
    ////////Imprime el menú de ayuda//////////
    if (text == "/Ayuda") {
      String ayuda = "Fuente Jardín "
                     ".\n";
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
