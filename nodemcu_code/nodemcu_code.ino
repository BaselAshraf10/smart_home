#include <ESP8266WiFi.h>        
#include <WebSocketsClient.h>   
#include <Servo.h>              

WebSocketsClient wsc;           
Servo servoMotor;               

const char *ssid = "-----";    
const char *pass = "****"; 

#define SERVER  "192.168.1.8"   
#define PORT    3000            
#define URL     "/"             

#define LED1_PIN D0
#define LED2_PIN D1
#define FLAME_SENSOR_PIN D2
#define FLAME_ALERT_PIN D3
#define MQ2_SENSOR_PIN A0
#define MQ2_ALERT_PIN D4
#define MQ2_FAN_PIN D5
#define DOOR_OPEN_PIN D6
#define SERVO_PIN D7

unsigned long lastTime = 0;     
const unsigned long interval = 1000; 

void websocketEvent(WStype_t type, uint8_t *data, size_t length) {
    switch (type) {
        case WStype_CONNECTED:    
            Serial.println("Connected to server");
            break;
        case WStype_DISCONNECTED: 
            Serial.println("Disconnected!");
            break;
        case WStype_TEXT:         
            Serial.printf("Message: %s\n", data); 
            if (strcmp((char*)data, "LED1on") == 0) {
                digitalWrite(LED1_PIN, LOW); 
            } else if (strcmp((char*)data, "LED1off") == 0) {
                digitalWrite(LED1_PIN, HIGH); 
            } else if (strcmp((char*)data, "LED2on") == 0) {
                digitalWrite(LED2_PIN, LOW); 
            } else if (strcmp((char*)data, "LED2off") == 0) {
                digitalWrite(LED2_PIN, HIGH); 
            }
            break;
    }
}

void checkGasLeak() {
    int gasState = digitalRead(FLAME_SENSOR_PIN);
    if (gasState == LOW) { 
        Serial.println("Fire detected! Sending notification...");
        if (wsc.isConnected()) {
            wsc.sendTXT("Fire detected!");
        }
        digitalWrite(FLAME_ALERT_PIN, HIGH);
        digitalWrite(DOOR_OPEN_PIN, HIGH);

        servoMotor.write(140); 
    } else {
        digitalWrite(FLAME_ALERT_PIN, LOW);
        digitalWrite(DOOR_OPEN_PIN, LOW);

        servoMotor.write(0); 
    }
}

void sendMQ2Data() {
    int mq2Value = analogRead(MQ2_SENSOR_PIN);
    String message = "MQ2 Value: " + String(mq2Value);
    if (wsc.isConnected()) {
        wsc.sendTXT(message.c_str());
    }

    if (mq2Value > 105) { 
        Serial.println("High gas level detected! Sending notification...");
        if (wsc.isConnected()) {
            wsc.sendTXT("High gas level detected!");
        }
        digitalWrite(MQ2_ALERT_PIN, HIGH);
        digitalWrite(MQ2_FAN_PIN, HIGH);
    } else {
        digitalWrite(MQ2_ALERT_PIN, LOW);
        digitalWrite(MQ2_FAN_PIN, LOW);
    }
}

void setup() {
    pinMode(LED1_PIN, OUTPUT);
    pinMode(LED2_PIN, OUTPUT);
    pinMode(FLAME_SENSOR_PIN, INPUT);
    pinMode(FLAME_ALERT_PIN, OUTPUT);
    pinMode(MQ2_ALERT_PIN, OUTPUT);
    pinMode(MQ2_FAN_PIN, OUTPUT);
    pinMode(DOOR_OPEN_PIN, OUTPUT);

    digitalWrite(LED1_PIN, LOW);
    digitalWrite(LED2_PIN, LOW);
    digitalWrite(FLAME_ALERT_PIN, LOW);
    digitalWrite(MQ2_ALERT_PIN, LOW);
    digitalWrite(MQ2_FAN_PIN, LOW);
    digitalWrite(DOOR_OPEN_PIN, LOW);

    servoMotor.attach(SERVO_PIN);
    servoMotor.write(0); 

    Serial.begin(115200);

    WiFi.begin(ssid, pass);
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(500);
    }

    Serial.println("\nConnected to Wi-Fi");
    Serial.println(WiFi.localIP());

    wsc.begin(SERVER, PORT, URL);
    wsc.onEvent(websocketEvent);
    wsc.setReconnectInterval(1000); 
}

void loop() {
    wsc.loop();
    checkGasLeak();

    unsigned long currentMillis = millis();
    if (currentMillis - lastTime >= interval) {
        lastTime = currentMillis;
        sendMQ2Data();
    }
}
