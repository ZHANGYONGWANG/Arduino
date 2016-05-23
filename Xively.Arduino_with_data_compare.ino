#include <SPI.h>
#include <ESP8266WiFi.h>
#include <HttpClient.h>
#include <Xively.h>
String comdata = "";
String t_outside = "";
String h_room = "";
String t_room = "";
float f_t_outside;
float f_h_room;
float f_t_room;
float old_t_outside, old_h_room, old_t_room;
float new_t_outside, new_h_room, new_t_room;
float diff_t_outside, diff_h_room, diff_t_room;
float diffLimit = 5;
boolean AfterFirstDataReceived = false;
boolean bigDataDeviation = false;
const char* ssid     = "Home_300M";
const char* password = "";

// Your Xively key to let you upload data
char xivelyKey[] ="" 
//your xively feed ID
#define xivelyFeed 1604025287
//datastreams
char TempOutsideID[] = "Temp.Outside";
char HumidityRoomID[] = "Humidity.In.Room";
char TempRoomID[] = "Temp.In.Room";
// Define the strings for our datastream IDs
XivelyDatastream datastreams[] = {
  XivelyDatastream(TempOutsideID, strlen(TempOutsideID), DATASTREAM_FLOAT),
  XivelyDatastream(HumidityRoomID, strlen(HumidityRoomID), DATASTREAM_FLOAT),
  XivelyDatastream(TempRoomID, strlen(TempRoomID), DATASTREAM_FLOAT)
};
// Finally, wrap the datastreams into a feed
XivelyFeed feed(xivelyFeed, datastreams, 3 /* number of datastreams */);

WiFiClient client;
XivelyClient xivelyclient(client);

void printWifiStatus() {

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}


void setup() {

  Serial.begin(9600);
  delay(10);
  Serial.println("Starting multiple datastream upload to Xively...");
  Serial.println();
  // We start by connecting to a WiFi network

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  printWifiStatus();

}

void loop() {
  t_outside = "";
  h_room = "";
  t_room = "";
  comdata = "";
  Serial.println("Wailting for the data comming in");
  while (Serial.available() <= 0 ) {
    //Serial.println("communication error");
  }
  while (Serial.available() > 0  && Serial.find("DataBegin")) {
    delay(100); //waiting for the data fill up
    char inChar = Serial.read();
    //Serial.print(inChar);
    while (inChar != '\n') {
      comdata += inChar;
      inChar = Serial.read();
      delay(10);
    } 
  }
  if (comdata != "") {
    Serial.println("Recieved comdata .............");
    Serial.println(comdata);
    int firstIndex = comdata.indexOf('D');
    int lastIndex = comdata.indexOf('T');
    lastIndex = lastIndex + 6;
    Serial.print("First index value: ");
    Serial.println(firstIndex);
    Serial.print("Last index Value: ");
    Serial.println(lastIndex);
    String validComData = comdata.substring(firstIndex, lastIndex);
    Serial.print("valid comdata is ");
    Serial.println(validComData);
    comdata = validComData;

    for (int i = 1; i <= 5; i++) {
      t_outside += comdata[i];
    }
    for (int i = 7; i <= 11; i++) {
      h_room += comdata[i];
    }
    for (int i = 13; i <= 17; i++) {
      t_room += comdata[i];
    }
    if (t_outside.indexOf('D') == -1 && h_room.indexOf('H') == -1 && t_room.indexOf('T') == -1) {
      f_t_outside = t_outside.toFloat();
      f_h_room = h_room.toFloat();
      f_t_room = t_room.toFloat();
      new_t_outside = f_t_outside;
      new_h_room = f_h_room;
      new_t_room = f_t_room;
      datastreams[0].setFloat(f_t_outside);
      datastreams[1].setFloat(f_h_room);
      datastreams[2].setFloat(f_t_room);
      //print the sensor valye
      Serial.print("Temp at outside:  ");
      Serial.println(datastreams[0].getFloat());
      Serial.print("Humidity in the room:  ");
      Serial.println(datastreams[1].getFloat());
      Serial.print("Temp in the room:  ");
      Serial.println(datastreams[2].getFloat());
      if (AfterFirstDataReceived) {
        Serial.println("Old Values and New Values:");
        Serial.print(old_t_outside);
        Serial.print(old_h_room);
        Serial.println(old_t_room);
        Serial.print(new_t_outside);
        Serial.print(new_h_room);
        Serial.println(new_t_room);
        diff_t_outside = abs(new_t_outside - old_t_outside);
        diff_h_room = abs(new_h_room - old_h_room);
        diff_t_room = abs(new_t_room - old_t_room);
        Serial.print("Diff values: ");
        Serial.print(diff_t_outside);
        Serial.print(diff_h_room);
        Serial.println(diff_t_room);
        if (diff_t_outside >= diffLimit || diff_h_room >= diffLimit || diff_t_room >= diffLimit) {
          Serial.println("Big deviation detected, data dropped!");
          bigDataDeviation = true;
        }
        else {
          bigDataDeviation = false;
          Serial.println("No big deviation, data uploading proceed!");
        }
      }
      if ( !bigDataDeviation) {
        //send value to xively
        Serial.println("Uploading it to Xively");
        int ret = xivelyclient.put(feed, xivelyKey);
        //return message
        Serial.print("xivelyclient.put returned ");
        Serial.println(ret);
        Serial.println("");
        AfterFirstDataReceived = true;
        old_t_outside = f_t_outside;
        old_h_room = f_h_room;
        old_t_room = f_t_room;
        Serial.println("The 1st Round Data Passed And the Old Values Are: ");
        Serial.print(old_t_outside);
        Serial.print(old_h_room);
        Serial.println(old_t_room);
        //delay between calls
        delay(5000);
      }
    }
  }
  else {
    Serial.println("Data Error");
    Serial.println(comdata);
  }
}

