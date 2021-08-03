#include <esp32cam.h>
#include <WebServer.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>  
#include <ESP32_FTPClient.h>
#define LAMP 4

// kamera_1_20 : 7e0b7094-98b0-4706-9b2b-b86e7585b1f6
// kamera_2_21 : 57860369-b272-485f-b899-3a72632016a9
// kamera_3_22 : 2aa2cb1c-ce47-46bb-94b3-79bc1b1335ed
// kamera_4_23 : bf81ce05-d6dc-4ce2-9c9c-2b059cae7dc4
// kamera_5_24 : 179e8b24-1573-4dee-a4de-e1eb4be22aaa
// kamera_6_25 : 126bebe0-5950-4c77-bb49-c0fe3d522d9a
// kamera_7_26 : f10ada26-4f32-4f58-8732-06707c672833
// kamera_8_27 : 2bf0f95c-acbb-4b3d-b14b-1270547cb9bb
// kamera_9_28 : fda54283-8f12-4b8c-a9cc-dfbb516054e4
// kamera_10_29 :203b8fb6-9f37-47f8-af1e-e840833cc6ce
// kamera_11_30 :36899a1e-6a01-49fd-9a06-80c195c6ad40
// kamera_12_31 :dc501cf6-6a07-42b1-abe2-c1b86c893780
// kamera_13_32 :f2c50b2c-49e9-47c3-b72c-6a85726e6b5c
// kamera_14_33 :906b9ec7-074f-4f5e-a180-3e853fbb7e39
// kamera_15_34 :7fc0d0e6-1532-4365-ac6e-062bbad1fd9f
// kamera_16_35 :bdd73c96-d454-4944-8789-aa857baa7dcc
// kamera_17_36 :de36cb1a-f03b-11eb-9a03-0242ac130003
// kamera_18_37 :ea38cbb6-f03b-11eb-9a03-0242ac130003
// kamera_19_38 :f23430ee-f03b-11eb-9a03-0242ac130003
// kamera_20_39 :fcc877ae-f03b-11eb-9a03-0242ac130003
// kamera_21_40 :079940e6-f03c-11eb-9a03-0242ac130003
// kamera_22_41 :13eecd0c-f03c-11eb-9a03-0242ac130003
// kamera_23_42 :1baa0ec6-f03c-11eb-9a03-0242ac130003
// kamera_24_43 :262798fa-f03c-11eb-9a03-0242ac130003

const char* WIFI_SSID = "ESP32CAM_3D";
const char* WIFI_PASS = "12345678";

const char *TOPIC = "3d";  // Topic to subcribe to
const char *TOPIC_Report = "report_3d";  // Topic to subcribe to
const char* mqtt_server = "192.168.4.219"; 
const char* mqtt_user = "/lana:lana";
const char* mqtt_pass= "lana";

const char* CL = "kamera_3d_16";//nama alat **************yg diubah
const char* client_id= "bdd73c96-d454-4944-8789-aa857baa7dcc"; // **************yg diubah


char ftp_server[] = "192.168.4.5";
char ftp_user[]   = "iotdevice";
char ftp_pass[]   = "1234567890";

ESP32_FTPClient ftp (ftp_server,ftp_user,ftp_pass, 5000, 2);

WebServer server(80);

WiFiClient espClient;
PubSubClient client(espClient);

static auto loRes = esp32cam::Resolution::find(320, 240);
static auto hiRes = esp32cam::Resolution::find(800, 600);

// Set your Static IP address
IPAddress local_IP(192, 168, 1, 35); //**************yg diubah
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(8, 8, 8, 8);   //optional

void handleBmp()
{
  if (!esp32cam::Camera.changeResolution(loRes)) {
    Serial.println("SET-LO-RES FAIL");
  }

  auto frame = esp32cam::capture();
  if (frame == nullptr) {
    Serial.println("CAPTURE FAIL");
    server.send(503, "", "");
    return;
  }
  Serial.printf("CAPTURE OK %dx%d %db\n", frame->getWidth(), frame->getHeight(),
                static_cast<int>(frame->size()));

  if (!frame->toBmp()) {
    Serial.println("CONVERT FAIL");
    server.send(503, "", "");
    return;
  }
  Serial.printf("CONVERT OK %dx%d %db\n", frame->getWidth(), frame->getHeight(),
                static_cast<int>(frame->size()));

  server.setContentLength(frame->size());
  server.send(200, "image/bmp");
  WiFiClient client = server.client();
  frame->writeTo(client);
}

void serveJpg()
{
  auto frame = esp32cam::capture();
  if (frame == nullptr) {
    Serial.println("CAPTURE FAIL");
    server.send(503, "", "");
    return;
  }
  Serial.printf("CAPTURE OK %dx%d %db\n", frame->getWidth(), frame->getHeight(),
                static_cast<int>(frame->size()));

  server.setContentLength(frame->size());
  server.send(200, "image/jpeg");
  WiFiClient client = server.client();
  frame->writeTo(client);
}

void handleJpgLo()
{
  if (!esp32cam::Camera.changeResolution(loRes)) {
    Serial.println("SET-LO-RES FAIL");
  }
  serveJpg();
}

void handleJpgHi()
{
  if (!esp32cam::Camera.changeResolution(hiRes)) {
    Serial.println("SET-HI-RES FAIL");
  }
  serveJpg();
}

void handleJpg()
{
  server.sendHeader("Location", "/cam-hi.jpg");
  server.send(302, "", "");
}

void handleMjpeg()
{
  if (!esp32cam::Camera.changeResolution(hiRes)) {
    Serial.println("SET-HI-RES FAIL");
  }

  Serial.println("STREAM BEGIN");
  WiFiClient client = server.client();
  auto startTime = millis();
  int res = esp32cam::Camera.streamMjpeg(client);
  if (res <= 0) {
    Serial.printf("STREAM ERROR %d\n", res);
    return;
  }
  auto duration = millis() - startTime;
  Serial.printf("STREAM END %dfrm %0.2ffps\n", res, 1000.0 * res / duration);
}
int FTP_uploadImage(int64_t t , unsigned char * pdata, unsigned int size, String namefile)
{
  char filename[100] = "";

  Serial.print("FTP_uploadImage=");
  Serial.println(size);
  char loc[100] = "/iotdevice/ftpcam/3d/temp/";

  ftp.OpenConnection();
  
  sprintf(filename,"%s.jpg", CL);

  //ftp.InitFile("Type A");
  ftp.ChangeWorkDir(loc);
  ftp.InitFile("Type I");
  ftp.NewFile(filename); 
  ftp.WriteData( pdata, size );
  ftp.CloseFile();

  ftp.CloseConnection();
  
  char buf[100];
  sprintf(buf, "File has been sent to %s%s",loc,filename);
  client.publish(TOPIC_Report, buf);
  Serial.println("Send Report Done!");
  
  return 0;
}
extern int capture_ftpupload(void);

void callback(char* topic, byte* payload, unsigned int length) {
  String response;

  for (int i = 0; i < length; i++) {
    response += (char)payload[i];
  }
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.println('\n'); 
  Serial.print("] ");
  Serial.println(response);
  if(response == "on")  // Turn the light on
    {
      capture_ftpupload();
      Serial.println("[INFO] Sukses Upload FTP!");
      
    }
  else{
    Serial.println("[INFO] False Command!");
    
  }
}
void reconnect() {
  // Loop until we're reconnected
  Serial.println("In reconnect...");
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if(client.connect(CL, mqtt_user, mqtt_pass)) {
      client.subscribe(TOPIC);
      Serial.println("connected");
      Serial.print("Subcribed to: ");
      Serial.println(TOPIC);
      Serial.println('\n');
      digitalWrite(LAMP, HIGH);
      delay(550);
      digitalWrite(LAMP, LOW);
      

    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup()
{
  Serial.begin(115200);
  Serial.println();
  pinMode(LAMP, OUTPUT);

  {
    using namespace esp32cam;
    Config cfg;
    cfg.setPins(pins::AiThinker);
    cfg.setResolution(hiRes);
    cfg.setBufferCount(2);
    cfg.setJpeg(80);

    bool ok = Camera.begin(cfg);
    Serial.println(ok ? "CAMERA OK" : "CAMERA FAIL");
  }
  
  if (!WiFi.config(local_IP, gateway, subnet, primaryDNS)) {
    Serial.println("STA Failed to configure");
  }

  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  Serial.print("http://");
  Serial.println(WiFi.localIP());
  Serial.println("  /cam.bmp");
  Serial.println("  /cam-lo.jpg");
  Serial.println("  /cam-hi.jpg");
  Serial.println("  /cam.mjpeg");

  server.on("/cam.bmp", handleBmp);
  server.on("/cam-lo.jpg", handleJpgLo);
  server.on("/cam-hi.jpg", handleJpgHi);
  server.on("/cam.jpg", handleJpg);
  server.on("/cam.mjpeg", handleMjpeg);
  
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);// Initialize the callback routine
  
  server.begin();
}



void loop()
{
  if (!client.connected())  // Reconnect if connection is lost
  {
    reconnect();
  }
  client.loop();
  server.handleClient();
}
