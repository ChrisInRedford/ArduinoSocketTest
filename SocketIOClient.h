/*
socket.io-arduino-client: a Socket.IO client for the Arduino
Based on the Kevin Rohling WebSocketClient & Bill Roy Socket.io Lbrary
Copyright 2015 Florent Vidal
Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:
JSON support added using https://github.com/bblanchon/ArduinoJson
The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
*/
#include "Arduino.h"

#include <Ethernet.h>
#include "SPI.h"			

// Length of static data buffers
//#define DATA_BUFFER_LEN 120
constexpr auto DATA_BUFFER_LEN = 120;
constexpr auto SID_LEN = 24;

// prototype for 'on' handlers
// only dealing with string data at this point
typedef void (*functionPointer)(String data); //This is trying to cast a (String) to a pointer type. 
typedef void (*FunctionCallBack) (String);


// Maxmimum number of 'on' handlers
constexpr auto MAX_ON_HANDLERS = 8;
constexpr auto PING_INTERVAL = 5000;

class SocketIOClient {
public:
	SocketIOClient();
	bool connect(char hostname[], char hostnameshort[], int port = 80);
	bool connectHTTP(char hostname[], char hostnameshort[], int port = 80);
	bool connected();
	void disconnect();
	bool reconnect(char hostname[], char hostnameshort[], int port = 80);
	bool monitor();
	void sendMessage(String message = "");
	void emit(String id, String data);
	void on(String id, functionPointer f);
	void sendNSP();
	void sendJSON(String RID, String JSON);
	void heartbeat(int select);
	void clear();
	void getREST(String path);
	void postREST(String path, String type, String data);
	void putREST(String path, String type, String data);
	void deleteREST(String path);
private:
	void parser(int index);
	void eventHandler(int index);
	void sendHandshake(char hostname[]);
	EthernetClient client;	

	bool readHandshake();
	void readLine();
	char *dataptr;
	char databuffer[DATA_BUFFER_LEN];
	char sid[SID_LEN];
	char key[28];
	char *hostname;
	char *nsp;
	int port;

	void findColon(char which);
	void terminateCommand(void);
	bool waitForInput(void);
	void eatHeader(void);

	FunctionCallBack onFunction[MAX_ON_HANDLERS];
    String onId[MAX_ON_HANDLERS];
    uint8_t onIndex = 0;
};
