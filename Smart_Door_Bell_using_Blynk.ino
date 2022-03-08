/*

*/
#include "esp_camera.h"
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include "DFRobotDFPlayerMini.h"

#define CAMERA_MODEL_AI_THINKER

HardwareSerial hwSerial(1);
DFRobotDFPlayerMini myDFPlayer;

#include "camera_pins.h"

#define FLASH 4
#define BUTTON 14
#define LED 15

#define TX2 13
#define RX2 12


const char* ssid = "Punto3Web";
const char* password = "Pedroda0608-Astorga72";
char auth[] = "nxFLN7HXMLYNK90u6y-aSYxB9JoEXx4b";

String my_Local_IP;

void startCameraServer();

BLYNK_WRITE(V2) // ALARMA
{
  int alarma = param.asInt();

  if (alarma == 1) {
    myDFPlayer.play(004);
  }
  else {
    myDFPlayer.stop();
  }
}

BLYNK_WRITE(V3) // NO ESTOY EN CASA
{
  int voy = param.asInt();

  if (voy == 1) {
    myDFPlayer.play(002);
  }
}

BLYNK_WRITE(V4) // VOY A LA PUERTA
{
  int no_voy = param.asInt();

  if (no_voy == 1) {
    myDFPlayer.play(001);
  }
}

BLYNK_WRITE(V5) // VOLUMEN
{
  int volumen = param.asInt();

  myDFPlayer.volume(volumen);  //Set volume value (0~30).

}

void capture()
{
  myDFPlayer.play(003);  //Play the campana largo mp3

  digitalWrite(FLASH, HIGH);
  uint32_t number = random(40000000);
  Blynk.notify("Hay alguien en la puerta...");
  Serial.println("http://" + my_Local_IP + "/capture?_cb=" + (String)number);
  Blynk.setProperty(V1, "urls", "http://" + my_Local_IP + "/capture?_cb=" + (String)number);
  delay(1000);
  digitalWrite(FLASH, LOW);

  digitalWrite(LED, LOW); delay(500);
  digitalWrite(LED, HIGH); delay(200);
  digitalWrite(LED, LOW); delay(600);
  digitalWrite(LED, HIGH); delay(300);
  digitalWrite(LED, LOW); delay(700);
  digitalWrite(LED, HIGH); delay(400);
  digitalWrite(LED, LOW); delay(800);
  digitalWrite(LED, HIGH); delay(500);
  digitalWrite(LED, LOW); delay(800);
  digitalWrite(LED, HIGH); delay(400);
  digitalWrite(LED, LOW); delay(700);
  digitalWrite(LED, HIGH); delay(300);
  digitalWrite(LED, LOW); delay(600);
  digitalWrite(LED, HIGH); delay(200);
  digitalWrite(LED, LOW); delay(500);
  digitalWrite(LED, HIGH); delay(200);

  digitalWrite(LED, HIGH);
}

void setup() {
  btStop(); // turn off bluetooth
  hwSerial.begin(9600, SERIAL_8N1, TX2, RX2);  // speed, type, TX, RX

  Serial.begin(115200);

  pinMode(FLASH, OUTPUT);
  pinMode(LED, OUTPUT);
  pinMode(33, OUTPUT);
  digitalWrite(LED, HIGH); //enciende el led
  digitalWrite(33, HIGH); //enciende el led
  //Serial.setDebugOutput(true);


  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  //init with high specs to pre-allocate larger buffers
  if (psramFound()) {
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    //Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  sensor_t * s = esp_camera_sensor_get();
  //initial sensors are flipped vertically and colors are a bit saturated
  if (s->id.PID == OV3660_PID) {
    s->set_vflip(s, 1);//flip it back
    s->set_brightness(s, 1);//up the blightness just a bit
    s->set_saturation(s, -2);//lower the saturation
  }
  //drop down frame size for higher initial frame rate
  s->set_framesize(s, FRAMESIZE_QVGA);
  //s->set_vflip(s, 1);

  //------------------------   SETUP DFPLAYER ----------------------------------------------
  Serial.println();
  Serial.println(F("DFRobot DFPlayer Mini Demo"));
  Serial.println(F("Initializing DFPlayer ... (May take 3~5 seconds)"));

  if (!myDFPlayer.begin(hwSerial)) {  //Use softwareSerial to communicate with mp3.
//    Serial.println(F("Unable to begin:"));
//    Serial.println(F("1.Please recheck the connection!"));
//    Serial.println(F("2.Please insert the SD card!"));
//    while (true) {
//      delay(0); // Code to compatible with ESP8266 watch dog.
//    }
  }
  Serial.println(F("DFPlayer Mini online."));

  myDFPlayer.setTimeOut(500); //Set serial communictaion time out 500ms

  myDFPlayer.outputDevice(DFPLAYER_DEVICE_SD);

  //------------------------------------------------------------------------------

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  if (WiFi.status() == WL_CONNECTED)
  {
    digitalWrite(33, LOW);
  }
  startCameraServer();

  Serial.print("Camera Ready! Use 'http://");
  Serial.print(WiFi.localIP());
  my_Local_IP = WiFi.localIP().toString();
  Serial.println("' to connect");
  Blynk.begin(auth, ssid, password);
}
void loop() {

  Blynk.run();
  if (digitalRead(BUTTON) == LOW) {
    capture();
  }

}
