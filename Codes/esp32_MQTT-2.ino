/*
 * Abderrazek SAIDANE
 * Wassim ALLAL 
 * Date 12/2020
 */

#include <PubSubClient.h> //Bibliothèque pour utiliser MQTT
#include "DHT.h" //Bibliothèque pour utiliser tous DHT 
#include <WiFi.h> //Bibliothèque pour se connecter au réseau Wifi

// Décommentez l’une des lignes ci-dessous pour le type de capteur DHT que vous utilisez !
#define DHTTYPE DHT11   // DHT 11
//#define DHTTYPE DHT21   // DHT 21 (AM2301)
//#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321

// Modifiez les identifiants ci-dessous afin que votre ESP32 se connecte à votre routeur
const char* ssid     = "NomDuRéseauWifi"; //"Freebox-36F519";
const char* password = "MotDePasse"; //"rfhtrqrtkbqbcbtrqcxbv5";

// Changez la variable à votre adresse IP Raspberry Pi, pour qu’elle se connecte à votreMQTT
const char* mqtt_server = "AdresseIpRaspberry"; //"192.168.1.14";//"192.168.1.137";//"192.168.1.31";

// Initialise l’espClient. Vous devez changer le nom de l’espClient si plusieurs ESPs sont en cours d’exécution dans votre système domotique
WiFiClient espClient;
PubSubClient client(espClient);

// DHT Sera broncher sur la pâte 5 
const int DHTPin = 5;

// La lampe sera sur la pâte 4
const int lamp = 4;

// initialisation du DHT
DHT dht(DHTPin, DHTTYPE);

// Timers auxiliar variables
long now = millis();
long lastMeasure = 0;

// Ne modifiez pas la fonction ci-dessous. Cette fonction connecte votre ESP32 à votre routeur
void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("WiFi connected - ESP IP address: ");
  Serial.println(WiFi.localIP());
}

// Cette fonction est exécutée lorsque certains appareils publient un message sur un sujet auquel votre ESP32 est abonné.
void callback(String topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  // Feel free to add more if statements to control more GPIOs with MQTT

  // If a message is received on the topic room/lamp, you check if the message is either on or off. Turns the lamp GPIO according to the message
  if(topic=="room/lamp"){ //Donner toujours le même topic entre node-red et votre code
      Serial.print("Changing Room lamp to ");
      if(messageTemp == "on"){ // Vérification du contenu du payload depuis node-red
        digitalWrite(lamp, HIGH);
        Serial.print("On");
      }
      else if(messageTemp == "off"){ // Vérification du contenu du payload depuis node-
        digitalWrite(lamp, LOW);
        Serial.print("Off");
      }
  }
  Serial.println();
}

// Cette fonction reconnecte votre ESP32 à votre courtier MQTT
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP32Client")) {
      Serial.println("connected");  
      // S’abonner ou se réabonner à un sujet
      // Vous pouvez vous abonner à plus de sujets (pour contrôler plus de LED dans cet exemple)
      client.subscribe("room/lamp");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

// La fonction de configuration positionne vos ESP GPPO sur Sorties, démarre la communication série à un taux en baud de 115200
// Définit votre mqtt et définit la fonction callback
// La fonction callback est ce qui reçoit les messages et contrôle réellement les LED
void setup() {
  pinMode(lamp, OUTPUT);
  dht.begin();
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  if(!client.loop())
    client.connect("ESP32Client");
  now = millis();
// Publie la nouvelle température et l’humidité toutes les 30 minutes
    if (now - lastMeasure > 3000000) {
      lastMeasure = now;
      // Depuis la datasheet du capteur on sait qu'il lui faut au moins deux secondes pour la lecture
      float h = dht.readHumidity();
      float t = dht.readTemperature();
      // Retour de température en Fahrenheit 
      float f = dht.readTemperature(true);
  
      // Vérifiez si les lectures ont échoué et quittez le système plus tôt que prévu (pour réessayer).
      if (isnan(h) || isnan(t) || isnan(f)) {
        Serial.println("Failed to read from DHT sensor!");
        return;
      }
  
      // Converti la température en Celcus 
      float hic = dht.computeHeatIndex(t, h, false);
      static char temperatureTemp[7];
      dtostrf(hic, 6, 2, temperatureTemp);
      
      // Uncomment to compute temperature values in Fahrenheit 
      // float hif = dht.computeHeatIndex(f, h);
      // static char temperatureTemp[7];
      // dtostrf(hif, 6, 2, temperatureTemp);
      
      static char humidityTemp[7];
      dtostrf(h, 6, 2, humidityTemp);
  
      // Publishes Temperature and Humidity values
      client.publish("room/temperature", temperatureTemp);
      client.publish("room/humidity", humidityTemp);
      
      Serial.print("Humidity: ");
      Serial.print(h);
      Serial.print(" %\t Temperature: ");
      Serial.print(t);
      Serial.print(" *C ");
      //Serial.print(f);
      //Serial.print(" *F\t Heat index: ");
      Serial.print(hic);
      Serial.println(" *C ");
      client.setCallback(callback);

  }
} 
