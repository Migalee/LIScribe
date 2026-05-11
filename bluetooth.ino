#include <ArduinoBLE.h>

// ATTENZIONE: Abbiamo cambiato un numero finale (da 4 a 9) per ingannare Windows!
BLEService liscribeService("19B10000-E8F2-537E-4F6C-D104768A1219"); 
BLEByteCharacteristic letterChar("19B10001-E8F2-537E-4F6C-D104768A1219", BLERead | BLENotify);

unsigned long tempoPrecedente = 0;
bool alternaLettera = false;

void setup() {
  Serial.begin(9600);
  if (!BLE.begin()) {
    Serial.println("Errore BLE!");
    while (1);
  }

  // NUOVO NOME per bypassare la memoria di Windows
  BLE.setLocalName("LIScribe_V2");
  BLE.setAdvertisedService(liscribeService);
  liscribeService.addCharacteristic(letterChar);
  BLE.addService(liscribeService);
  
  letterChar.writeValue(32); 
  BLE.advertise();
  Serial.println("Bluetooth V2 attivo! LIScribe_V2 pronto.");
}

void loop() {
  BLEDevice central = BLE.central();

  if (central) {
    Serial.print("Connesso al PC: ");
    Serial.println(central.address());

    while (central.connected()) {
      unsigned long tempoAttuale = millis();
      
      if (tempoAttuale - tempoPrecedente >= 2000) {
        tempoPrecedente = tempoAttuale;
        if (alternaLettera) {
          letterChar.writeValue('A');
        } else {
          letterChar.writeValue('B');
        }
        alternaLettera = !alternaLettera;
      }
      BLE.poll(); 
    }
    Serial.println("PC Disconnesso.");
  }
}