#include <Arduino.h>
#include "one_wire.h"

// required for otaWeb
#include "otaWeb.h"
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Update.h>

const char *host = "esp32";
const char *ssid = "EMV3_HW_Test_AP";
const char *password = "thisisfine";

WebServer server(80);

#define MAX_WIFI_SSID_LENGTH 64
#define MAX_WIFI_PASSWORD_LENGTH 64

static char wifi_ssid[MAX_WIFI_SSID_LENGTH+1]         = "";
static char wifi_password[MAX_WIFI_PASSWORD_LENGTH+1] = "";

static uint8_t one_wire_pin = 0xff;

// forward declaration
void i2c_begin(), i2c_scan(), blink_start(int, int, int, int), wifi_begin(), wifi_scan(), wifi_info(), esp_output(), wifi_disconnect(), wifi_connect(const char*, const char*), adc_test(), tft_test(int);
void input_wifi_credentials();
void input_one_wire_pin();
bool wifi_connected();
void otaSetup();
void onJavaScript();

void output_menu() {
  Serial.println("1. Show ESP information");
  Serial.println("2. Enter wifi credentials");

  if(wifi_connected())
    Serial.printf("3. Disconnect from wifi network %s\n", wifi_ssid);
  else
    Serial.printf("3. Connect to wifi network %s\n", strlen(wifi_ssid) > 0 ? wifi_ssid : "(enter SSID to be able to connect)");
    
  Serial.println("4. Current wifi info");
  Serial.println("5. Scan wifi networks");
  Serial.println("6. Scan I2C devices");
  Serial.println("7. Test Digital IO");
  Serial.println("8. Scan 1 Wire devices");
  Serial.println("9. Test Analog Inputs");
  Serial.println("0. Restart");
  Serial.println("t/T. TFT Test");
  Serial.println("c/C. Calibrate Touchscreen and TFT Test");
  Serial.println("");
  Serial.println("h, H, ? <cr>. This menu");

} // end output_menu


void setup(){

  Serial.begin(115200);

  // setup ota stuff
  otaSetup();

  Serial.println();
  Serial.println();
  Serial.println("Engine Monitor HW V3 Test");
  delay(5000);

  // enable I2C for test
  i2c_begin();

  // run the output menu at the end of setup
  output_menu();
} // end setup

// main processing loop
void loop() {

  // check for otaWeb stuff
  server.handleClient();

  // parse serial test comamnds 
  if (Serial.available() > 0) {
    char cmd[2];
    Serial.println("Char input");
    Serial.readBytes(cmd, 1);
    Serial.println(cmd);
    
    switch(cmd[0]) {
    case '1':
      esp_output();
      break;

    case '2':
      input_wifi_credentials();
      break;

    case '3':
      if(wifi_connected())
	      wifi_disconnect();
      else {
        if(strlen(wifi_ssid) > 0)
          wifi_connect(wifi_ssid, wifi_password);
        else
          Serial.println("You must enter a Wifi SSID first");
      } // end if
      break;

    case '4':
      if(strlen(wifi_ssid) > 0) {
        Serial.printf("Entered SSID: %s\n", wifi_ssid);
        Serial.printf("Entered password: %s\n", wifi_password);
      } else {
      	Serial.println("No SSID entered");
      } // end if

      Serial.println();
      wifi_info();
      Serial.println();
      Serial.println();
      break;

    case '5':
      wifi_scan();
      break;

    case '6':
      i2c_scan();
      break;

    case '7':
      #define Out1 26 // Relay Output1
      #define Out2 19 // Relay Output2
      #define In1 14 // Relay Output1
      #define In2 13 // Relay Output2
      blink_start(Out1, Out2, In1, In2);
      break;

    case '8':
      one_wire_scan(one_wire_pin);
      break;

    case '9':
      adc_test();
      break;

    case '0':
      Serial.println("RESTART ESP");
      ESP.restart();
      break;

    case 'T':
    case 't':
      // tft test, no calibration
      tft_test(0);
      break;

    case 'C':
    case 'c':
      // tft test, ts calibration
      tft_test(1);
      break;

    case 'H':
    case 'h':
    case '?':
    case '\n':
      output_menu();
      break;

    default:
      Serial.printf("Unknown command '%c'\n", cmd[0]);
      output_menu();
    } // end case
  } // end if
} // end loop

/* basic line reading function which handles backspace and delete
 * can't believe I had to write this; I must be missing something...
 */
int serial_read_line(char* buf, int length) {
  int n = 0;
  
  //  wait for input
  while(1) {
    if(Serial.available() == 0) {
      yield();
      continue;
    }

    char c = Serial.read();

        Serial.printf("%d", (int)c);

    if(c == -1 || c == '\n') {
      yield();
      continue;
    }

    if(c == '\b' || c == 0x7f) {
      n--;
      buf[n] = '\0';
      Serial.print('\b');
      continue;
    }

    if(n == length) {
      Serial.print("\07");
      continue;
    }
    
    if(c == '\r') {
      Serial.println();
      return n;
    }

    Serial.print(c);

    buf[n++] = c;
    buf[n] = '\0';
  } // end while
} // end serial_read_line


void input_wifi_credentials() {
  Serial.println("Enter wifi SSID");

  serial_read_line(wifi_ssid, MAX_WIFI_SSID_LENGTH);

  Serial.println("Enter wifi password");

  serial_read_line(wifi_password, MAX_WIFI_PASSWORD_LENGTH);

  Serial.println();
  Serial.println();

  Serial.printf("SSID %s\n", wifi_ssid);
  Serial.printf("Password %s\n", wifi_password);

  Serial.println();
  Serial.println();
}

void input_one_wire_pin() {
  char pin_number[5];

  Serial.println("Enter pin number for OneWire data");

  serial_read_line(pin_number, 4);

  Serial.println();
  Serial.println();

  one_wire_pin = atoi(pin_number);
  Serial.printf("OneWire pin number %d\n", one_wire_pin);
  Serial.println("Remember to connect a 4.7K resistor between VCC and OneWire data!\n");

  Serial.println();
  Serial.println();
} // end input_one_wire_pin

/* otaWeb setup function */
void otaSetup(void)
{
    IPAddress local_ip(192, 168, 1, 1);
    IPAddress local_mask(255,255,255,0);
    IPAddress gateway(192, 168, 1, 1);

    WiFi.softAP(ssid,password);
    WiFi.softAPConfig(local_ip,gateway,local_mask);
    Serial.println("");
    Serial.print("AP set to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.softAPIP());

    // serial debug stuff
    Serial.println("");
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    /*use mdns for host name resolution*/
    if (!MDNS.begin(host))
    { // http://esp32.local
        Serial.println("Error setting up MDNS responder!");
        while (1)
        {
            delay(1000);
        } // end while
    } // end if
    Serial.println("mDNS responder started");

    /*return javascript jquery */
    server.on("/jquery.min.js", HTTP_GET, onJavaScript);

    /*return index page which is stored in serverIndex */
    server.on("/", HTTP_GET, []()
              {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", loginIndex); });
    server.on("/serverIndex", HTTP_GET, []()
              {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", serverIndex); });
    /*handling uploading firmware file */
    server.on(
        "/update", HTTP_POST, []()
        {
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    ESP.restart(); },
        []()
        {
            HTTPUpload &upload = server.upload();
            if (upload.status == UPLOAD_FILE_START)
            {
                Serial.printf("Update: %s\n", upload.filename.c_str());
                if (!Update.begin(UPDATE_SIZE_UNKNOWN))
                { // start with max available size
                    Update.printError(Serial);
                }
            }
            else if (upload.status == UPLOAD_FILE_WRITE)
            {
                /* flashing firmware to ESP*/
                if (Update.write(upload.buf, upload.currentSize) != upload.currentSize)
                {
                    Update.printError(Serial);
                }
            }
            else if (upload.status == UPLOAD_FILE_END)
            {
                if (Update.end(true))
                { // true to set the size to the current progress
                    Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
                }
                else
                {
                    Update.printError(Serial);
                }
            }
        });
    server.begin();
} // end otaSetup

// otaWeb related
void onJavaScript(void) {
    Serial.println("onJavaScript(void)");
		server.setContentLength(jquery_min_js_v3_2_1_gz_len);
		server.sendHeader(F("Content-Encoding"), F("gzip"));
    server.send_P(200, "text/javascript", jquery_min_js_v3_2_1_gz, jquery_min_js_v3_2_1_gz_len);
} // end onJavaScript

