//NodeMCU RGB-Controller for Homebridge & HomeKit (Siri)
/* References
    http://www.esp8266.com/viewtopic.php?f=11&t=12259#sthash.ZNowLh9q.dpuf
    http://arcfn.com/2009/08/multi-protocol-infrared-remote-library.html
    https://kaistech.wordpress.com/2016/10/09/wifi-infrared-ir-remote-extension-using-esp8266/
    https://github.com/markszabo/IRremoteESP8266
*/

#include <ESP8266WiFi.h>
#include <math.h>

/*******************IR Libraries********************/
#include <IRremoteESP8266.h>
#include <IRutils.h>
#include "IR_Controller_Codes.h"
/***************************************************/

#define redPin 13       //D7 - Red channel
#define grnPin 12       //D6 - Green channel
#define bluPin 14       //D5 - Blue channel

#define RECV_PIN 5 // Pino do Receptor IR
#define CAPTURE_BUFFER_SIZE 1024
#define TIMEOUT 15U

String readString;           //String to hold incoming request
String hexString = "ff0000"; //Define inititial color here (hex value)

int state;

int r;
int g;
int b;

// IR Brighness Iteration Variables //
int i_red;
int i_green;
int i_blue;

int prevR = r;
int prevG = g;
int prevB = b;
//---------------------//remover caso funcione//

float R;
float G;
float B;

int x;
int V;

unsigned long lastCode; // stores last IR code received

IRrecv irrecv(RECV_PIN, CAPTURE_BUFFER_SIZE, TIMEOUT, true);
decode_results results;  // Somewhere to store the results

// MARK: - Setup
void setup() {
  // set pin modes
  pinMode(redPin, OUTPUT);
  pinMode(grnPin, OUTPUT);
  pinMode(bluPin, OUTPUT);

  Serial.begin(115200);
  delay(1);
  setHex(); //Set initial color after booting. Value defined above
  WiFi.mode(WIFI_STA);
  WiFiStart();
  //showValues(); //Uncomment for serial output
  irrecv.enableIRIn(); // Start the receiver
}

///// WiFi SETTINGS - Replace with your values /////////////////
const char* ssid = "CASE";
const char* password = "U.S._Marine_Corps";
IPAddress ip(192, 168, 1, 10);   // set a fixed IP for the NodeMCU
IPAddress gateway(192, 168, 1, 1); // Your router IP
IPAddress subnet(255, 255, 255, 0); // Subnet mask
WiFiServer server(80); //Set server port
////////////////////////////////////////////////////////////////////

void WiFiStart() {
  printf("Connecting to %s", String(ssid));
  WiFi.begin(ssid, password);
  WiFi.config(ip, gateway, subnet);
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    printf("_");
  }
  printf("Done");
  printf("IP address: ");
  printf(WiFi.localIP());
  printf("\n");

  server.begin();
}

// MARK: Control functions

void allOff() {
  state = 0;
  analogWrite(redPin, 0);
  analogWrite(grnPin, 0);
  analogWrite(bluPin, 0);
}

// nao testado - troquei '=' pra '==' antes que dê merda
void setBrightnessIR(long pressedKey) {
  if (pressedKey == IR_BPlus) {

  }
  if (pressedKey == IR_BMinus) {

  }

}

// talvez funcione, deu erro antes, naqueles ifs
int incrementAndLimit(int &val, int increment) {
  val += increment; // This will sum or subtract the value of increment from val. So far only +1 and -1 are being used.
  
  // Defensive driving: making sure val stays in the range 0-1023
  if (val > 1023) { val = 1023; }
  else if (val < 0) { val = 0; }
  
  return val;
}

// testar
void setRGBBrightnessIR(long pressedKey) {
  printf("\n");
  
  if (pressedKey == IR_UPR) {
    printf("IR_UPR\n");
    incrementAndLimit(r, +1);

  } else if (pressedKey == IR_UPG) {
    printf("IR_UPG\n");
    incrementAndLimit(g, +1);

  } else if (pressedKey == IR_UPB) {
    printf("IR_UPB\n");
    incrementAndLimit(b, +1);

  } else if (pressedKey == IR_DOWNR) {
    printf("IR_DOWNR\n");
    incrementAndLimit(r, -1);

  } else if (pressedKey == IR_DOWNG) {
    printf("IR_DOWNG\n");
    incrementAndLimit(g, -1);

  } else if (pressedKey == IR_DOWNB) {
    printf("IR_DOWNB\n");
    incrementAndLimit(b, -1);
  }
  getHex(r, g, b);
}

// MARK: Color functions

// depende da função anterior para saber se funciona haha
void getHex(int red, int green, int blue) {
  red = map(red, 0, 1023, 0, 255);
  green = map(green, 0, 1023, 0, 255);
  blue = map(blue, 0, 1023, 0, 255);

  long rgb = 0;
  rgb = ((long)red << 16) | ((long)green << 8 ) | (long)blue;

  printf("R: " + String(red) + " | " + "G: " + String(green) + " | " + "B: " + String(blue) + "\n");
  printf("Hex: 0x" + String(rgb, HEX) + "\n");
  //hexString = String(rgb, HEX)
  //setHex();
}

//Write requested hex-color to the pins
void setHex() {
  state = 1;
  long number = (long) strtol( &hexString[0], NULL, 16);
  r = number >> 16;
  g = number >> 8 & 0xFF;
  b = number & 0xFF;
  r = map(r, 0, 255, 0, 1023);  //added for 10bit pwm
  g = map(g, 0, 255, 0, 1023);  //added for 10bit pwm
  b = map(b, 0, 255, 0, 1023);  //added for 10bit pwm
  analogWrite(redPin, (r));
  analogWrite(grnPin, (g));
  analogWrite(bluPin, (b));
}

int rainbowState = -1;
unsigned long lastColorChangeTime = 0;
unsigned long colorDelay = 500;
const char* rainbowHex [6] = {"FF0000", "FF7F00", "FFFF00", "00FF00", "0000FF", "8B00FF"};
void rainbow() {
  if ((millis() - lastColorChangeTime) > colorDelay) {
    hexString = rainbowHex[rainbowState]
    setHex()
    lastColorChangeTime = millis()
    rainbowState = (rainbowState+1)%6
  }
}


//Compute current brightness value
int getV() { // Acho que dá pra melhorar o nome dessa classe
  R = roundf(r / 10.23); //for 10bit pwm, was (r/2.55);
  G = roundf(g / 10.23); //for 10bit pwm, was (g/2.55);
  B = roundf(b / 10.23); //for 10bit pwm, was (b/2.55);
  x = _max(R, G);
  V = _max(x, B); // (+) e dessa variável
  return V;
}

// MARK: Debug functions

//For serial debugging only
void showValues() {
  printf("%s %d\n", "Status on/off:", state);
  printf("%s %d | %d | %d\n", "RGB color:", r, g, b);
  printf("%s %s\n", "Hex color:", hexString);
  printf("%s %s\n\n", "Brightness:", getV());
}


void getIR() {
  /*Blink Built-in LED after receiving a code
    digitalWrite(LED_BUILTIN, HIGH);
    delay(20);
    digitalWrite(LED_BUILTIN, LOW);
  */

  // funciona
  if (results.value == IR_REPEAT && results.value != IR_OnOff) {  // if repeat command (button held down)
    results.value = lastCode;      // replace FFFF with last good code
  } else {
    lastCode = 0;
  }

  if (results.value == IR_R) {    //when button '-' is pressed
    printf("\nVermelho");
    hexString = "FF0000";
    lastCode = results.value;     // record this as last good command
    setHex();
  } else if (results.value == IR_G) {    //when button '+' is pressed
    printf("\nVerde");
    hexString = "00FF00";
    lastCode = results.value;     // record this as last good command
    setHex();
  } else if (results.value == IR_B) {    //when button '|<<' is pressed
    printf("\nAzul");
    hexString = "0000FF";
    lastCode = results.value;     // record this as last good command
    setHex();
  } else if (results.value == ) {
    printf("\nArco-íris");
    if rainbowState == -1 {
      rainbowState = 0
      rainbow()
    } else {
      rainbowState = -1
    }

  } else if (results.value == IR_OnOff) {    //when button 'ON/OFF' is pressed
    lastCode = results.value;      // record this as last good command
    if (state == 0) { // se estiver desligado, liga com a ultima cor ainda guardada na variavel hexString
      printf("\nLiga");
      setHex();
    } else { // se estiver ligado, executa o procedimento para desligar todos as cores
      printf("\nDesliga");
      allOff();
    }
  }

  // ainda nao desenvolvido - skip
  if (results.value == IR_BPlus)    //when button '>>|' is pressed
  {
    lastCode = results.value;      // record this as last good command

  }
  // ainda nao desenvolvido - skip
  if (results.value == IR_BMinus)    //when button '>>|' is pressed
  {
    lastCode = results.value;      // record this as last good command

  }
  // goal
  if (results.value == IR_UPR || results.value == IR_UPG || results.value == IR_UPB || results.value == IR_DOWNR || results.value == IR_DOWNG || results.value == IR_DOWNB)    //when button '>>|' is pressed
  {
    lastCode = results.value;      // record this as last good command
    setRGBBrightnessIR(results.value);
  }

  /*
    if(results.value == 0xFFC23D)     //when button '>>|' is pressed
    {
     lastCode = results.value;      // record this as last good command
     pos += 15;                     // adds 15º to variable pos
     myservo.write(pos);            // moves servo to new pos position
     delay(5);
    }
  */
}

// MARK: Operational function

void loop() {
  //Reconnect on lost WiFi connection (superfluous - will reconnect anyway)
  /*if (WiFi.status() != WL_CONNECTED) {
    WiFiStart();
    }
  */

  WiFiClient client = server.available();

  
  if (irrecv.decode(&results)) {
    getIR();
    showValues();
  }

  if (rainbowState != -1) {
    rainbow()
  }
  
  if (!client) { return; }
  
  while (client.connected() && !client.available()) {
    delay(1);
  }

  //Respond on certain Homebridge HTTP requests
  if (client) {
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        if (readString.length() < 100) {
          readString += c;
        }
        //Serial.println(readString); //REMOVER!!!
        if (c == '\n') {
          Serial.println("Request: " + readString); //Uncomment for serial output

          //Send reponse
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          //client.println("Connection: close");
          client.println();

          if (readString.indexOf("on") > 0) { // On
            setHex();
            //showValues();

          } else if (readString.indexOf("off") > 0) { // Off
            allOff();
            //showValues();

          } else if (readString.indexOf("set") > 0) { // Set color
            hexString = "";
            hexString = (readString.substring(9, 15));
            setHex();
            //showValues();

          } else if (readString.indexOf("status") > 0) { // Status on/off
            client.println(state);

          } else if (readString.indexOf("color") > 0) { // Status color (hex)
            client.println(hexString);

          } else if (readString.indexOf("bright") > 0) { // Status brightness (%)
            client.println(getV());
          }

          showValues(); //REMOVER!!!
          delay(1);
          client.stop();
          readString = "";
        }
      }
    }
  }
}
