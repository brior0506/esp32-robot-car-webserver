#include <WiFi.h>
#include <ArduinoOTA.h>
#include <WebServer.h>
#include <secret.h>

//  web server setup
WebServer server(80);

//html for root page

String htmlPage = R"rawliteral(
  <html>
  <head>
  <title>ESP32 Car Control</title>
  </head>
  <body>
  <h1>Car Control Panel</h1>
  <button onmousedown="fetch('/forward?value=' + currentSpeed)" onmouseup="fetch('/stop')" ontouchstart="fetch('/forward?value=' + currentSpeed)" ontouchend="fetch('/stop')">Forward</button>
  <button onmousedown="fetch('/backward?value=' + currentSpeed)" onmouseup="fetch('/stop')" ontouchstart="fetch('/backward?value=' + currentSpeed)" ontouchend="fetch('/stop')">Backward</button>
  <button onmousedown="fetch('/right?value=' + currentSpeed)" onmouseup="fetch('/stop')" ontouchstart="fetch('/right?value=' + currentSpeed)" ontouchend="fetch('/stop')">Right</button>
  <button onmousedown="fetch('/left?value=' + currentSpeed)" onmouseup="fetch('/stop')" ontouchstart="fetch('/left?value=' + currentSpeed)" ontouchend="fetch('/stop')">Left</button>
  <button onclick="fetch('/stop'); sendSpeed(0)">Stop</button>
  <br><br>
  Speed: <input type='range' min='0' max='255' value='0' oninput="sendSpeed(this.value)">
  <span id = "speedValue">0</span>
  </body>
  </html>

  <script>
  let currentSpeed = 0;

  function sendSpeed(value) {
    currentSpeed = value;
    document.getElementById("speedValue").innerText = value;
    fetch('/setspeed?value=' + value);
  }
  </script>
)rawliteral";


//L298N pins for motor control
#define backLeftForward 13
#define backLeftBackward 27
#define frontLeftForward 25
#define frontLeftBackward 26
#define backRightForward 2
#define backRightBackward 4
#define frontRightForward 18
#define frontRightBackward 19
#define frontLeftSpeed 33
#define backLeftSpeed 32
#define frontRightSpeed 23
#define backRightSpeed 21

//Hertz and Bits for Pulse With Modulation control
#define hz 1000
#define bits 8

//global last direction variable
String lastDirection = "stop";

//create web server route functions

//webpage root
void handleRoot() {
  server.send(200, "text/html", htmlPage);
}

//webpage stop
void handleStop(){
  ledcWrite(frontLeftSpeed, 0);
  ledcWrite(frontRightSpeed, 0);
  ledcWrite(backLeftSpeed, 0);
  ledcWrite(backRightSpeed, 0);

  digitalWrite(backLeftForward, LOW);
  digitalWrite(backRightForward, LOW);
  digitalWrite(frontLeftForward, LOW);
  digitalWrite(frontRightForward, LOW);
  
  digitalWrite(backLeftBackward, LOW);
  digitalWrite(backRightBackward, LOW);
  digitalWrite(frontLeftBackward, LOW);
  digitalWrite(frontRightBackward, LOW);
}

//webpage forward
void handleForward() {
  //stops car before switching direction if going backwards when pressed
  handleStop();
  lastDirection = "forward";

  //if going backwards before pressed, gets the speed to use while going forward
  int speed = 0;
  if(server.hasArg("value")) {
    speed = server.arg("value").toInt();

    if (speed<0){
      speed = 0;
    }
    if(speed>255){
      speed = 255;
    }
  }
  
  // delay(1000);
  //set motors facing forward direction
  digitalWrite(backLeftBackward, LOW);
  digitalWrite(backRightBackward, LOW);
  digitalWrite(frontRightBackward, LOW);
  digitalWrite(frontLeftBackward, LOW);
  
  digitalWrite(backLeftForward, HIGH);
  digitalWrite(backRightForward, HIGH);
  digitalWrite(frontLeftForward, HIGH);
  digitalWrite(frontRightForward, HIGH);
  //set to speed that was defined before clicking forward button
  ledcWrite(frontLeftSpeed, speed);
  ledcWrite(frontRightSpeed, speed);
  ledcWrite(backLeftSpeed, speed);
  ledcWrite(backRightSpeed, speed);
}

//webpage backward
void handleBackward() {
  handleStop();
  lastDirection = "backward";

  int speed = 0;
  if (server.hasArg("value")){
    speed = server.arg("value").toInt();

    if(speed<0){
      speed=0;
    }
    if(speed>255){
      speed = 255;
    }
  }

 

  digitalWrite(backLeftBackward, HIGH);
  digitalWrite(backRightBackward, HIGH);
  digitalWrite(frontRightBackward, HIGH);
  digitalWrite(frontLeftBackward, HIGH);
  
  digitalWrite(backLeftForward, LOW);
  digitalWrite(backRightForward, LOW);
  digitalWrite(frontLeftForward, LOW);
  digitalWrite(frontRightForward, LOW);


  ledcWrite(backRightSpeed, speed);
  ledcWrite(backLeftSpeed, speed);
  ledcWrite(frontLeftSpeed, speed);
  ledcWrite(frontRightSpeed, speed);
}

//webpage right
void handleRight() {
  handleStop();

  if (lastDirection == "forward"){
     digitalWrite(backLeftBackward, LOW);
  digitalWrite(backRightBackward, LOW);
  digitalWrite(frontRightBackward, LOW);
  digitalWrite(frontLeftBackward, LOW);
  
  digitalWrite(backLeftForward, HIGH);
  digitalWrite(backRightForward, HIGH);
  digitalWrite(frontLeftForward, HIGH);
  digitalWrite(frontRightForward, HIGH);
  }
  else if(lastDirection == "backward"){
     digitalWrite(backLeftBackward, HIGH);
  digitalWrite(backRightBackward, HIGH);
  digitalWrite(frontRightBackward, HIGH);
  digitalWrite(frontLeftBackward, HIGH);
  
  digitalWrite(backLeftForward, LOW);
  digitalWrite(backRightForward, LOW);
  digitalWrite(frontLeftForward, LOW);
  digitalWrite(frontRightForward, LOW);
  }
  else{
    handleStop();
  }

  ledcWrite(frontLeftSpeed, 255);
  ledcWrite(backLeftSpeed, 255);
  ledcWrite(backRightSpeed, 100);
  ledcWrite(frontRightSpeed, 100);

 

}

//webpage left
void handleLeft() {
  

  
  //checks previous direction to turn in same direction
  if (lastDirection == "forward"){
     digitalWrite(backLeftBackward, LOW);
  digitalWrite(backRightBackward, LOW);
  digitalWrite(frontRightBackward, LOW);
  digitalWrite(frontLeftBackward, LOW);
  
  digitalWrite(backLeftForward, HIGH);
  digitalWrite(backRightForward, HIGH);
  digitalWrite(frontLeftForward, HIGH);
  digitalWrite(frontRightForward, HIGH);
  }
  else if(lastDirection == "backward"){
     digitalWrite(backLeftBackward, HIGH);
  digitalWrite(backRightBackward, HIGH);
  digitalWrite(frontRightBackward, HIGH);
  digitalWrite(frontLeftBackward, HIGH);
  
  digitalWrite(backLeftForward, LOW);
  digitalWrite(backRightForward, LOW);
  digitalWrite(frontLeftForward, LOW);
  digitalWrite(frontRightForward, LOW);
}
  else{
    handleStop();
  }

  //sets speed at hard coded value to turn left    
  ledcWrite(frontLeftSpeed, 100);
  ledcWrite(backLeftSpeed, 100);
  ledcWrite(backRightSpeed, 255);
  ledcWrite(frontRightSpeed, 255);
 
}

//webpage speed
void handleSetSpeed() {
  if(server.hasArg("value")) {
    int speed = server.arg("value").toInt();
  
  //Ensures speeds are between 0 and 255
  if (speed < 0) {
    speed = 0;
  }
  if (speed > 255) {
    speed = 255;
  }

  ledcWrite(frontLeftSpeed, speed);
  ledcWrite(frontRightSpeed, speed);
  ledcWrite(backLeftSpeed, speed);
  ledcWrite(backRightSpeed, speed);

  lastSpeed = speed;
  }

  //tells sever esp32 got the value to make sure speed is sent once
  server.send(200, "text/plain", "Speed Updated");
}




void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  // wifi connection
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);


  //root page of server

  server.on("/", handleRoot);

  //forward button on webpage
  server.on("/forward", handleForward);

  //stop button on webpage
  server.on("/stop", handleStop);

  //backward button on webpage
  server.on("/backward", handleBackward);

  //right button on webpage
  server.on("/right", handleRight);

  //left button on webpage
  server.on("/left", handleLeft);

  //speed slider fetch on webpage
  server.on("/setspeed", handleSetSpeed);

 
  //checks if wifi is connected and lets user know when connected in the serial monitor
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");

  }

  Serial.println("\nConneted to WiFi");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  //MAC Address
  Serial.print("MAC Address: ");
  Serial.println(WiFi.macAddress());

  //ota setup
  //allows user to upload code over the air
  ArduinoOTA.setHostname("RCcar");
  ArduinoOTA.setPassword("1492");
  ArduinoOTA.onStart([]() {
    Serial.println("Start updating...");
  });

  ArduinoOTA.onEnd([]() {
    Serial.println("\nUpdate Complete.");
  });

  //lets user know when over the air upload is ready
  ArduinoOTA.begin();
  Serial.println("OTA Ready");


  //begin server
  server.begin();
  

   // Set motor direction pins to OUTPUT
  pinMode(backLeftBackward, OUTPUT);
  pinMode(backLeftForward, OUTPUT);
  pinMode(backRightBackward, OUTPUT);
  pinMode(backRightForward, OUTPUT);
  pinMode(frontLeftBackward, OUTPUT);
  pinMode(frontLeftForward, OUTPUT);
  pinMode(frontRightBackward, OUTPUT);
  pinMode(frontRightForward, OUTPUT);
  pinMode(frontLeftSpeed, OUTPUT);
  pinMode(frontRightSpeed, OUTPUT);
  pinMode(backRightSpeed, OUTPUT);
  pinMode(backLeftSpeed, OUTPUT);

  // Sets all motors to no direction initially
  digitalWrite(backLeftBackward, LOW);
  digitalWrite(backRightBackward, LOW);
  digitalWrite(frontRightBackward, LOW);
  digitalWrite(frontLeftBackward, LOW);
  digitalWrite(backLeftForward, LOW);
  digitalWrite(backRightForward, LOW);
  digitalWrite(frontLeftForward, LOW);
  digitalWrite(frontRightForward, LOW);
  //sets up PWM
  ledcAttach(frontLeftSpeed, hz, bits);
  ledcAttach(frontRightSpeed, hz, bits);
  ledcAttach(backLeftSpeed, hz, bits);
  ledcAttach(backRightSpeed, hz, bits);
}

void loop() {
  // put your main code here, to run repeatedly:
  ArduinoOTA.handle();
  server.handleClient();
 
  delay(50);
  

}
