#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <SPI.h>
#include <MFRC522.h>
#include <HTTPSRedirect.h>
#include<Wire.h>
//const int D1 = 2;
const char *GScriptId = "AKfycbw7FBL3icwL72n24DExQfKpkXT1fK7R4o4Y_6J7qG2_RiimdRAYLYlpqjp51gb79AubLw";
String gate_number = "Imiot_HQ";

const char* ssid     = "TBI Private Wi-Fi";
const char* password = "DND@TBI#2k23";
//const char* ssid     = "Phobi";
//const char* password = "97479088211";

String payload_base =  "{\"command\": \"insert_row\", \"sheet_name\": \"Sheet1\", \"values\": ";
String payload = "";

const char* host        = "script.google.com";
const int   httpsPort   = 443;
const char* fingerprint = "";
String url = String("/macros/s/") + GScriptId + "/exec";
HTTPSRedirect* client = nullptr;

String student_id;

int blocks[] = {4,5,6,8,9};
#define total_blocks  (sizeof(blocks) / sizeof(blocks[0]))
#define RST_PIN  0  //D3
#define SS_PIN   2  //D4
const int D8 = 15; // LED pin
const int BUZZER = 4; // Buzzer pin
MFRC522 mfrc522(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;  
MFRC522::StatusCode status;
int blockNum = 2;  
byte bufferLen = 18;
byte readBlockData[18];
void setup() {
  pinMode(D8, OUTPUT); // Initialize LED pin
  pinMode(BUZZER, OUTPUT); // Initialize buzzer pin
  digitalWrite(BUZZER, LOW); // Turn off the buzzer initiallys
  Serial.begin(115200);        
  delay(10);
  Serial.println('\n');
  SPI.begin();
  Serial.print("Connecting to");
  Serial.print("WiFi...");
  WiFi.begin(ssid, password);             
  Serial.print("Connecting to ");
  Serial.print(ssid); Serial.println(" ...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println('\n');
  Serial.println("WiFi Connected!");
  Serial.println(WiFi.localIP());
  client = new HTTPSRedirect(httpsPort);
  client->setInsecure();
  client->setPrintResponseBody(true);
  client->setContentTypeHeader("application/json");
  Serial.print("Connecting to ");
  Serial.print("Google ");
  delay(5000);
  Serial.print("Connecting to ");
  Serial.println(host);
  bool flag = false;
  for(int i=0; i<5; i++){ 
    int retval = client->connect(host, httpsPort);
    if (retval == 1){
      flag = true;
      String msg = "Connected. OK";
      Serial.println(msg);
      delay(2000);
      break;
    }
    else
      Serial.println("Connection failed. Retrying...");
  }
  if (!flag){
    Serial.print("Connection fail");
    Serial.print("Could not connect to server: ");
    Serial.println(host);
    delay(5000);
    return; 
  }
  delete client;    // delete HTTPSRedirect object
  client = nullptr; // delete HTTPSRedirect object
}
void loop() { 
  static bool flag = false;
  if (!flag){
    client = new HTTPSRedirect(httpsPort);
    client->setInsecure();
    flag = true;
    client->setPrintResponseBody(true);
    client->setContentTypeHeader("application/json");
  }
  if (client != nullptr){
    if (!client->connected()){
      int retval = client->connect(host, httpsPort);
      if (retval != 1){
        Serial.println("Disconnected. Retrying...");
        Serial.print("Disconnected.");
        Serial.print("Retrying...");
        return; //Reset the loop
      }
    }
  }
  else{Serial.println("Error creating client object!"); Serial.println("else");}

  Serial.print("Scan your Tag");
  mfrc522.PCD_Init();
  if ( ! mfrc522.PICC_IsNewCardPresent()) 
  {
    digitalWrite(D8, HIGH);
    //digitalWrite(BUZZER, HIGH); // Turn on buzzer
    delay(200); // Delay to keep LED and buzzer on
    digitalWrite(D8,LOW);
    //digitalWrite(BUZZER, LOW);
    return;}
  
  if ( ! mfrc522.PICC_ReadCardSerial()) {
//    digitalWrite(BUZZER, HIGH);
//    delay(2000);
//    digitalWrite(BUZZER, LOW);
    return;}
  /* Read data from the same block */
  Serial.println();
  Serial.println(F("Reading last data from RFID..."));  
  String values = "", data;
  for (byte i = 0; i < total_blocks; i++) {
    ReadDataFromBlock(blocks[i], readBlockData);
    if(i == 0){
      data = String((char*)readBlockData);
      data.trim();
      student_id = data;
      values = "\"" + data + ",";
    }
    else{
      data = String((char*)readBlockData);
      data.trim();
      values += data + ",";
    }
  }
  if (data != 0){
    digitalWrite(BUZZER, HIGH);
    delay(1000);
    digitalWrite(BUZZER, LOW);
  }
  values += gate_number + "\"}";
  payload = payload_base + values;
  Serial.print("Publishing Data");
  Serial.print("Please Wait...");
  Serial.println("Publishing data...");
  Serial.println(payload);
  if(client->POST(url, host, payload)){ 
    digitalWrite(D8, HIGH); // Turn on LED
    digitalWrite(BUZZER, HIGH); // Turn on buzzer
    delay(1000); // Delay to keep LED and buzzer on
    digitalWrite(D8, LOW);
    digitalWrite(BUZZER, LOW);
    delay(200);
    Serial.println("[OK] Data published.");
  }
  else{ 
    Serial.println("Error while connecting");
    Serial.print("Failed.");
    Serial.print("Try Again");
  }   
  Serial.println("[TEST] delay(5000)");
  delay(5000);
}
void ReadDataFromBlock(int blockNum, byte readBlockData[]) 
{ 
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockNum, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK){
     Serial.print("Authentication failed for Read: ");
     Serial.println(mfrc522.GetStatusCodeName(status));
     return;
  }
  else {
    //Serial.println("Authentication success");
  }
  status = mfrc522.MIFARE_Read(blockNum, readBlockData, &bufferLen);
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Reading failed: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
  else {
    readBlockData[16] = ' ';
    readBlockData[17] = ' ';
    //Serial.println("Block was read successfully");  
  }
}
