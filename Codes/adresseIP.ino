/* 
 *  Abderrazek SAIDANE - EI2I-4
 *  Wassim ALLAL
 *  Date du 01/2021
 *  affichage de l'adresse IP de l'ESP32
 */
 
#include <WiFi.h>
#include <WiFiUdp.h>

const char* ssid     = "Freebox-36F519";
const char* password = "rfhtrqrtkbqbcbtrqcxbv5";

void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}
  
void loop() {
}
  
