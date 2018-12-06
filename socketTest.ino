#include <EthernetUdp.h>
#include <EthernetServer.h>
#include <EthernetClient.h>
#include <Dns.h>
#include <Dhcp.h>
#define W5100
#include "SocketIOClient.h"
#include <Ethernet.h>
#include "SPI.h"

/*
 * This is a test to connect an arduino uno with a W5100 ethernet shield to a NodeJS server with SocketIO.
 * The corresponding circuit is just an led on pin 7 and a button on pin 8. When the button is clicked,
 * the light toggles on and off, and emits to the server which updates it's UI accordingly. The server can
 * emit to the board, which will also light up the led. The hostname is set to 'https://witty-cat-82.localtunnel.me',
 * which is a subdomain i use with localtunnel for testing. This should be changed accordingly.
**/

constexpr auto ledPin = 13; //#define ledPin 13
constexpr auto inPin = 8; //#define inPin 8

SocketIOClient client;

bool ledOn = false;
bool clicked = false;
int state1 = HIGH;      // the current state of the output pin
int reading;           // the current reading from the input pin
int previous = LOW;    // the previous reading from the input pin

long time = 0;         // the last time the output pin was toggled
long debounce = 600;   // the debounce time, increase if the output flickers
long lastping = 0;

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };


//TODO: make subdomain its own variable, im too stupid for this.
String subdomain = "witty-cat-82";
//TODO: CWA: Does the subdomain ever change? Can we make this a const char?
String _seperator = ".";
String domain = "localtunnel.me";
String encoding = "https://";
char _c_HostName[48];
char _c_HostNameShort[48];

char hostname[] = "https://witty-cat-82.localtunnel.me";
char hostnameshort[40];
//Use the stl contianer String as it's easier to work with than char arrays. (Sue me, I'm lazy).
String _HostName = encoding + subdomain + _seperator + domain;
String _HostNameShort = subdomain + _seperator + domain;
//localtunnel uses 80
const int port = 80;

// Set the static IP address to use if the DHCP fails to assign
IPAddress ip(192, 168, 0, 177);
IPAddress myDns(192, 168, 0, 1);


//Pretty sure i dont need these anymore.
extern String RID;
extern String Rname;
extern String Rcontent;

unsigned long previousMillis = 0;
long interval = 10000;
void setup() {
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
  pinMode(inPin, INPUT);
  _HostName.toCharArray(_c_HostName, 48); //Convert the String object to a character array so the SocketIO doesn't bitch.
  _HostNameShort.toCharArray(_c_HostNameShort, 48);
  Serial.begin(115200);
  Serial.println("...");
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // start the Ethernet connection:
  Serial.println("Initialize Ethernet with DHCP:");
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // Check for Ethernet hardware present
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
      Serial.print("Also, I am connecting to " );
      Serial.println(_c_HostName);
      Serial.print("Here's the short host name: ");
      Serial.print(_c_HostNameShort);
      Serial.println();
      while (true) {
        delay(1); // do nothing, no point running without Ethernet hardware
        ERROR_LED(ledPin); //Error indicator
      }
    }
    if (Ethernet.linkStatus() == LinkOFF) {
      Serial.println("Ethernet cable is not connected.");
    }
    // try to congifure using IP address instead of DHCP:
    Ethernet.begin(mac, ip, myDns);
  }
  else {
    Serial.print("DHCP assigned IP: ");
    Serial.println(Ethernet.localIP());
  }

  //Define the socketIO 'on' listener with funciton. "light" is the event, and light is the function to call...
  client.on("light", light);


  // give the Ethernet shield a second to initialize:
  delay(1000);
  Serial.print("connecting to ");
  Serial.print(hostname);
  Serial.println("...");

  if (client.connect(_c_HostName, hostnameshort, port))
  {
    Serial.println("DONE CONNECTING");
    //On first connect, toggle off.
    client.emit("toggleFromBoard", "false");
    lastping = millis();
  }
  else
  {
    Serial.println("CONNECTION ERROR");
    while (1);
  }
  delay(1000);
}

void loop() {
  //Get current value of button
  reading = digitalRead(inPin);

  // if the input just went from LOW and HIGH and we've waited long enough
  // to ignore any noise on the circuit, toggle the output pin and remember
  // the time
  if (reading == HIGH && previous == LOW && millis() - time > debounce) {
    Serial.println("click");
    if (state1 == HIGH)
      state1 = LOW;
    else
      state1 = HIGH;

    time = millis();
    if (!ledOn) {
      digitalWrite(ledPin, HIGH);
      client.emit("toggleFromBoard", "true");
      lastping = millis();
      ledOn = true;
    }
    else {
      client.emit("toggleFromBoard", "false");
      lastping = millis();
      digitalWrite(ledPin, LOW);
      ledOn = false;
    }

  }
  client.monitor();

  if (millis() - lastping >= 50000) {
    client.emit("heartbeat", "true");
    lastping = millis();
  }
}

//TODO: why the fuck does this read 'ru' and 'als'...
//TODO: CWA: This is because you're casting a String to a function pointer.
//  You need to send the event handler an address to a function, and not a pointer to a String.
void light(String state) {
  Serial.println("[light] " + state);
  if (state == "ru") {
    Serial.println("[light] ON");
    ledOn = true;
    digitalWrite(7, HIGH);
  }
  else {
    Serial.println("[light] off");
    ledOn = false;
    digitalWrite(7, LOW);
  }
}
void someThingElse(String NeatObject)
{
  return;
}

void ERROR_LED(int LEDPIN)
{
  digitalWrite(LEDPIN, LOW); //Blink the LED rapidly when there's an error
  delay(100);
  digitalWrite(LEDPIN, HIGH);
  delay(100);
  return;
}
const static struct {
  const char *name;
  void(*func)(String);
} function_map[] = {
  {"light", light },
  {"someThingElse", someThingElse }
};

int call_function(const char *name, String data)
{
  int i;

  for (i = 0; i < (sizeof(function_map) / sizeof(function_map[0])); i++) {
    if (!strcmp(function_map[i].name, name) && function_map[i].func) {
      function_map[i].func(data);
      return 0;
    }
  }

  return -1;
}
