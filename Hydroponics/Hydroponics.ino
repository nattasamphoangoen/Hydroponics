//ส่วนที่เชื่อมต่ออินเทอร์เน็ต
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
const char* ssid = "BANK";
const char* password = "bank4227";
String apiKey = "H41MV1K5IN3NRPD4";
const char* server = "api.thingspeak.com";
char auth[] = "dd54e26f8833408fb8cbc1bc48fc3ee7";
WiFiClient client;
#define BLYNK_PRINT Serial
//ส่วนของระบบsensor
#define SensorPin A0
#define samplingInterval 20
#define printInterval 800
#define ArrayLenth  40
#define analogInPin A3
int pHArray[ArrayLenth];
int pHArrayIndex = 0;
int sensorValue = 0;
int outputValue = 0;
int p1 = 26;
int p2 = 27;
int ECL = 1200;
int ECH = 1800;
float pHL = 5.5 ;
float pHH = 7.0 ;
int Sta1 = 0 ;
int Sta2 = 0 ;
void setup() {
  Serial.begin(9600);
  delay(5);
  Serial.println(); Serial.println();
  Serial.print("Connecting to "); Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(5);
    Serial.print(".");
    Blynk.begin(auth, ssid, password);
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  //สาวนของระบบ
  pinMode(p1, OUTPUT);
  pinMode(p2, OUTPUT);
}
void loop()
{
  digitalWrite(p1, LOW);
  sensorValue = analogRead(A3) ;
  outputValue = map(sensorValue, 0, 4094, 0, 5000);
  delay(5);
  Serial.print("EC = " );
  Serial.print(sensorValue);
  Serial.print("\n ");
  static unsigned long samplingTime = millis();
  static unsigned long printTime = millis();
  static float pHValue, voltage;
  if (millis() - samplingTime > samplingInterval)
  {
    pHArray[pHArrayIndex++] = analogRead(A0);
    if (pHArrayIndex == ArrayLenth)pHArrayIndex = 0;
    voltage = avergearray(pHArray, ArrayLenth) * 5.0 / 4095 * 0.9421265141;
    pHValue = 3.5 * voltage;
    samplingTime = millis();
  }
  Serial.print("Voltage:");
  Serial.print(voltage, 2);
  Serial.print("    pH value: ");
  Serial.println(pHValue, 2);
  //ควบคุมp2
  if
  (sensorValue < ECL || sensorValue > ECH)
  {
    digitalWrite(p2, LOW );
  } else
  {
    digitalWrite(p2, HIGH);
  }
  if (pHValue < pHL || pHValue > pHH ) {
    digitalWrite(p2, LOW);
  } else {
    digitalWrite(p2, HIGH);
  }
  //ขึ้นBlynk
  Blynk.run();
  BLYNK_READ(V0);
  {
    Blynk.virtualWrite(V0, pHValue);
  }
  BLYNK_READ(V1);
  {
    Blynk.virtualWrite(V1, sensorValue);
  }
  //ส่วนขึ้นthingspeak
  Sta1 = digitalRead(p1);
  Sta2 = digitalRead(p2);
  Serial.println("------------------------------------");
  Serial.println("Requesting EC&PH ");
  if (client.connect(server, 80))
  { String postStr = apiKey;
    postStr += "&field1="; // Fields 1
    postStr += String(sensorValue);
    postStr += "&field2="; // Fields 2
    postStr += String(pHValue);
    postStr += "&field3=";  // Fields 3
    postStr += String(Sta1);
    postStr += "&field4=";  // Fields 4
    postStr += String(Sta2);
    postStr += "\r\n\r\n";
    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: " + apiKey + "\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(postStr.length());
    client.print("\n\n");
    client.print(postStr);
  }
  client.stop();
  Serial.println("Waiting...");
  delay(5);
}
double avergearray(int* arr, int number) {
  int i;
  int max, min;
  double avg;
  long amount = 0;
  if (number <= 0) {
    Serial.println("Error number for the array to avraging!/n");
    return 0;
  }
  if (number < 5) {
    for (i = 0; i < number; i++) {
      amount += arr[i];
    }
    avg = amount / number;
    return avg;
  } else {
    if (arr[0] < arr[1]) {
      min = arr[0]; max = arr[1];
    }
    else {
      min = arr[1]; max = arr[0];
    }
    for (i = 2; i < number; i++) {
      if (arr[i] < min) {
        amount += min;
        min = arr[i];
      } else {
        if (arr[i] > max) {
          amount += max;
          max = arr[i];
        } else {
          amount += arr[i];
        }
      }//if
    }//for
    avg = (double)amount / (number - 2);
  }//if
  return avg;
}

