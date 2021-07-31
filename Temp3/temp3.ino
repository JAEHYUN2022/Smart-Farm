#include <DHT.h>

DHT dht(7, DHT22);

void setup() {
  Serial.begin(115200);  
  Serial1.begin(115200);
  dht.begin();
}

void loop() {

  int f = dht.readTemperature();
  Serial.print(f);
  Serial1.write(f);

}
