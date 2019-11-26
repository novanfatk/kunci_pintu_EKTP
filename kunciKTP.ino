#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

WiFiClient client;


//NodeMCU        MFRC522
//D1        RST
//D5        SCK
//D6        MISO
//D7        MOSI
//D8        SDA(SS)


#define SS_PIN D0
#define RST_PIN D3
#define PIR_PIN D1

long int
ledOn = 10000,
lastTime = -ledOn;
int state_pir = LOW; 
int value = 0;

MFRC522 rfid(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;


String request_string;
//const char* host = "managementpintu.smitphb.org";
const char* host = "192.168.43.240";
int index1, index2;
String no_id, dat2;
int buzzer = D2;
int sensor = PIR_PIN; 

HTTPClient http;
Servo myservo;
void setup() {
  // put your setup code here, to run once:
  SPI.begin();
  rfid.PCD_Init();
  Serial.begin(9600);
  Serial.println("menunggu untuk scan kartu...");
  myservo.attach(D8);//punya servo
  myservo.write(0);
  pinMode(PIR_PIN, INPUT);
 // digitalWrite(PIR_PIN,0);
  pinMode(buzzer,OUTPUT);
   WiFi.disconnect();
  WiFi.begin("andaisaja", "dicoding");
  

  //WiFi.begin("sin","87654321");
  while ((!(WiFi.status() == WL_CONNECTED))) {
    delay(300);
    Serial.print(".");
  }
  Serial.println(F("Connected !"));
  SPI.begin();
  rfid.PCD_Init();
  pinMode(D1, INPUT);
  pinMode(D4, OUTPUT);
  digitalWrite(D4, 0);
  Serial.println(F("Ready...."));
}
String strID, val, data;
unsigned long timenow = 0;
boolean state_buzzer = 0;
boolean state_kartu = 0;


void baca_serial() {
  while (client.available() > 0) {
    delay(10);
    char c = client.read();
    data += c;
  }

  if (data.length() > 0) {
    Serial.println(data);
    index1 = data.indexOf('%');
    index2 = data.indexOf('%', index1 + 1);

    no_id = data.substring(index1 + 1, index2);
    //    saldo = dat1.toInt();
    Serial.print("No ID: ");
    Serial.println(no_id);
    if ( state_buzzer == 0 && no_id == "none") {
      Serial.println("Dilarang Masuk !");
      tone(buzzer,500000);
      delay(1000);
      digitalWrite(D4, 0);
      delay(300);
      digitalWrite(D4, 1);
      delay(400);
      digitalWrite(D4, 0);
      delay(1000);
      digitalWrite(D4, 1);
      delay(400);
      delay(300);
      digitalWrite(D4, 1);
      delay(400);
      digitalWrite(D4, 0);
      delay(1000);
      digitalWrite(D4, 1);
      delay(400);
      noTone(buzzer);
      delay(1000);

      state_buzzer = 1 ;
    }
    else {
      if (state_kartu == 0) {
        Serial.println("Silahkan Masuk !");
        myservo.write(180);
        delay(10000);
        myservo.write(0);
        state_buzzer = 0 ;
        state_kartu = 1;
      }
    }
    data = "";
  }
}
void loop() {
//   if (digitalRead(PIR_PIN)){
//   lastTime = millis();
//    Serial.println("adanya pergerakan");
//   delay(100);
//   digitalWrite(D4,HIGH);
//   delay(1000);
//   }
//   else{
//   Serial.println("tidak ada pergerakan");
//   delay(100);
//   digitalWrite(D4,LOW);
//   delay(100);    
//   }
value = digitalRead(PIR_PIN); 
  if (value == HIGH) {
    digitalWrite(D4, HIGH);
    delay(100); 
    if (state_pir== LOW) {
      Serial.println("Gerakan Terdteksi!"); 
      state_pir = HIGH; 
    } if (!client.connect(host, 80)) {
    Serial.println("Gagal Konek");
    return;
  }

  request_string = "/TA/simpandata.php?data1=";
  //request_string += "&data2=";
  request_string += sensor;
 
  Serial.print("requesting URL: ");
  Serial.println(request_string);
  client.print(String("GET ") + request_string + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n");

  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      Serial.println(">>> Client Timeout !");
      client.stop();
      return;
    }
  }
   
  }   else {
      digitalWrite(D4, LOW); 
      delay(200);
      
      if (state_pir == HIGH){
        Serial.println("Tidak Ada Gerakan!");
        state_pir = LOW;
    }
  }
   
  // put your main code here, to run repeatedly:
if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial())
    return;
  // Serial.print(F("PICC type: "));
  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
  // Serial.println(rfid.PICC_GetTypeName(piccType));

//  //id kartu dan yang akan dikirim ke database
  strID = "";
  for (byte i = 0; i <rfid.uid.size; i++) {
    strID +=
      (rfid.uid.uidByte[i] < 0x10 ? "0" : "") +
      String(rfid.uid.uidByte[i], HEX) +
      (i != rfid.uid.size-1 ? ":" : "");

}

strID.toUpperCase();
 Serial.print("ID kartu anda: " );
  Serial.println(strID);
  delay(100);
  myservo.write(0);
  masuk(strID);
  baca_serial();
  state_buzzer = 0;
  state_kartu = 0;
  delay(1000);
  Serial.println("SCAN KARTU ANDA = ");

}
  
void masuk(String id)
{
  if (!client.connect(host, 80)) {
    Serial.println("Gagal Konek");
    return;
  }

  request_string = "/TA/rfidupdatemasuk.php?idcard=";
  request_string += id;
 
  Serial.print("requesting URL: ");
  Serial.println(request_string);
  client.print(String("GET ") + request_string + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n");

  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      Serial.println(">>> Client Timeout !");
      client.stop();
      return;
    }
  }
}

 
