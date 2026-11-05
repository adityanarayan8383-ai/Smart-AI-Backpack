#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 10
#define RST_PIN 9
#define BUZZER 6

MFRC522 rfid(SS_PIN, RST_PIN);

String items[10];
String itemNames[10];
int count = 0;

String getItemName(String id) {
  if (id == "F0D44652") return "WATER BOTTLE";
  if (id == "F13BFB63") return "DSA FILE";
  if (id == "21F5DC66") return "KEYBOARD";
  if (id == "99C6C411") return "ID CARD";
  if (id == "086011B9") return "POCO F6";

  return "UNKNOWN ITEM";
}

void setup() {
  Serial.begin(9600);
  SPI.begin();
  rfid.PCD_Init();

  pinMode(BUZZER, OUTPUT);

  Serial.println("SMART BACKPACK");
  Serial.println("--------------------");
  Serial.println("r = Register Item");
  Serial.println("c = Check List");
  Serial.println("s = Scan Item");
  Serial.println("t = Check TAG ID");
  Serial.println("x = Remove All Items");
  Serial.println("--------------------");
}

void loop() {

  if (Serial.available()) {

    char cmd = Serial.read();

    // REGISTER ITEM
    if (cmd == 'r' || cmd == 'R') {

      Serial.println("Scan item to register...");

      waitForCard();

      String id = getID();
      String name = getItemName(id);

      bool exists = false;

      for (int i = 0; i < count; i++) {
        if (items[i] == id) {
          exists = true;
          break;
        }
      }

      if (exists) {
        Serial.println("Item Already Registered!");
      }

      else if (count < 10) {

        if (name == "UNKNOWN ITEM") {

          Serial.println("Unknown Item Detected!");
          Serial.println("Enter Item Name:");

          while (Serial.available()) {
            Serial.read();
          }

          while (Serial.available() == 0) {
            delay(10);
          }

          name = Serial.readStringUntil('\n');
          name.trim();

          if (name.length() == 0) {
            name = "UNKNOWN ITEM";
          }
        }

        items[count] = id;
        itemNames[count] = name;
        count++;

        Serial.print("REGISTERED:");
        Serial.println(name);

        Serial.print(name);
        Serial.println(" REGISTERED SUCCESSFULLY!");

        tone(BUZZER, 1000);
        delay(300);
        noTone(BUZZER);
      }

      else {
        Serial.println("List Full!");
      }
    }

    // CHECK REGISTERED ITEMS
    else if (cmd == 'c' || cmd == 'C') {

      Serial.println("REGISTERED ITEMS:");

      if (count == 0) {
        Serial.println("No Items Registered.");
      }

      for (int i = 0; i < count; i++) {

        Serial.print("Item ");
        Serial.print(i + 1);
        Serial.print(": ");
        Serial.println(itemNames[i]);
      }

      Serial.print("TOTAL:");
      Serial.println(count);
    }

    // SCAN ITEM
    else if (cmd == 's' || cmd == 'S') {

      Serial.println("Scan item to check...");

      waitForCard();

      String id = getID();

      bool found = false;

      for (int i = 0; i < count; i++) {

        if (items[i] == id) {

          found = true;

          Serial.print("ITEM:");
          Serial.println(itemNames[i]);

          Serial.print(itemNames[i]);
          Serial.println(" FOUND!");

          tone(BUZZER, 1500);
          delay(200);
          noTone(BUZZER);

          break;
        }
      }

      if (!found) {
        Serial.println("Item Not Registered!");
      }
    }

    // NEW: CHECK TAG ID
    else if (cmd == 't' || cmd == 'T') {

      Serial.println("SCAN ANY RFID TAG TO CHECK ITS ID");

      waitForCard();

      String id = getID();

      Serial.println("--------------------");
      Serial.print("TAG ID: ");
      Serial.println(id);

      String name = getItemName(id);

      Serial.print("ITEM NAME: ");
      Serial.println(name);

      bool registered = false;

      for (int i = 0; i < count; i++) {

        if (items[i] == id) {

          registered = true;

          Serial.println("STATUS: REGISTERED");
          Serial.print("REGISTERED NAME: ");
          Serial.println(itemNames[i]);

          break;
        }
      }

      if (!registered) {
        Serial.println("STATUS: NOT REGISTERED");
      }

      Serial.println("--------------------");
    }

    // REMOVE ALL ITEMS
    else if (cmd == 'x' || cmd == 'X') {

      count = 0;

      Serial.println("CLEARED");
      Serial.println("All Items Removed!");
    }
  }
}

// WAIT FOR RFID CARD
void waitForCard() {

  while (true) {

    if (rfid.PICC_IsNewCardPresent() &&
        rfid.PICC_ReadCardSerial()) {

      break;
    }

    delay(50);
  }
}

// GET RFID TAG ID
String getID() {

  String id = "";

  for (byte i = 0; i < rfid.uid.size; i++) {

    if (rfid.uid.uidByte[i] < 0x10) {
      id += "0";
    }

    id += String(rfid.uid.uidByte[i], HEX);
  }

  id.toUpperCase();

  Serial.print("Tag ID: ");
  Serial.println(id);

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();

  return id;
}