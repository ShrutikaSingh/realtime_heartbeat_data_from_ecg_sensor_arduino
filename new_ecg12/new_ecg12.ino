#include <WiFi.h>
#include <HTTPClient.h>

#define PLOTT_DATA 
#define MAX_BUFFER 100

uint32_t prevData[MAX_BUFFER];
uint32_t sumData=0;
uint32_t maxData=0;
uint32_t avgData=0;
uint32_t roundrobin=0;
uint32_t countData=0;
uint32_t period=0;
uint32_t lastperiod=0;
uint32_t millistimer=millis();
double frequency;
double beatspermin=0;
uint32_t newData;

const int analogIn = A0;
int RawValue= 0;
double Voltage = 0;
double tempC = 0;
double tempF = 0;

WiFiClient client;

String MakerIFTTT_Key ;
String MakerIFTTT_Event;
char *append_str(char *here, String s) {  int i=0; while (*here++ = s[i]){i++;};return here-1;}
char *append_ul(char *here, unsigned long u) { char buf[20]; return append_str(here, ultoa(u, buf, 10));}
char post_rqst[256];char *p;char *content_length_here;char *json_start;int compi;



void freqDetec() 
{
  if (countData==MAX_BUFFER) 
  {
   if (prevData[roundrobin] < avgData*1.5 && newData >= avgData*1.5){ // increasing and crossing last midpoint
    period = millis()-millistimer;//get period from current timer value
    millistimer = millis();//reset timer
    maxData = 0;
   }
  }
 roundrobin++;
 if (roundrobin >= MAX_BUFFER) {
    roundrobin=0;
 }
 if (countData<MAX_BUFFER) {
    countData++;
    sumData+=newData;
 } else {
    sumData+=newData-prevData[roundrobin];
 }
 avgData = sumData/countData;
 if (newData>maxData) {
  maxData = newData;
 }

 
#ifdef PLOTT_DATA
  Serial.print(newData);
 Serial.print("\t");
 Serial.print(avgData);
 Serial.print("\t");
 Serial.print(avgData*1.5);
 Serial.print("\t");
 Serial.print(maxData);
 Serial.print("\t");
 Serial.println("Beats per minute=");
 Serial.println(beatspermin);
#endif
 prevData[roundrobin] = newData;//store previous value
}

void setup() {
  Serial.begin(115200);
  
WiFi.disconnect();
  delay(5);
  Serial.println("START");
  WiFi.begin("aman","aman1234");
 while((!(WiFi.status() == WL_CONNECTED)))
 {
  delay(5);
  Serial.print("..");
}
  Serial.println("Connected");
  Serial.println("Your IP is");
  Serial.println((WiFi.localIP()));
  if (client.connect("maker.ifttt.com",80)) {
    MakerIFTTT_Key ="m_p4sAiOrRqavMTr6EDZIkY3ut0rxJbpwl2FRM36hH_";
    MakerIFTTT_Event ="SMS";
    p = post_rqst;
    p = append_str(p, "POST /trigger/");
    p = append_str(p, MakerIFTTT_Event);
    p = append_str(p, "/with/key/");
    p = append_str(p, MakerIFTTT_Key);
    p = append_str(p, " HTTP/1.1\r\n");
    p = append_str(p, "Host: maker.ifttt.com\r\n");
    p = append_str(p, "Content-Type: application/json\r\n");
    p = append_str(p, "Content-Length: ");
    content_length_here = p;
    p = append_str(p, "NN\r\n");
    p = append_str(p, "\r\n");
    json_start = p;
    p = append_str(p, "{\"value1\":\"");
    p = append_str(p, "00919468946304");
    p = append_str(p, "\",\"value2\":\"");
    p = append_str(p, "alert!!! bubbly is critical");
    p = append_str(p, "\",\"value3\":\"");
    p = append_str(p, "");
    p = append_str(p, "\"}");

    compi= strlen(json_start);
    content_length_here[0] = '0' + (compi/10);
    content_length_here[1] = '0' + (compi%10);
    client.print(post_rqst);

  }
}

void loop()
{
  newData = analogRead(34); //variable to be put in the graph:Shrutika
  freqDetec();
  if (period!=lastperiod) {
     frequency = 1000/(double)period;//timer rate/period
     if (frequency*60 > 20 && frequency*60 < 200) { // supress unrealistic Data
      beatspermin=frequency*60;

        lastperiod=period;
     }
  }
  delay(5);

RawValue = analogRead(analogIn);
Voltage = (RawValue / 2048.0) * 3300; // 5000 to get millivots.
tempC = Voltage * 0.1;
tempF = (tempC * 1.8) + 32; // convert to F
Serial.println("Temperature=");
Serial.println(tempF);

HTTPClient http;

//CODE TO SEND NEW_VOLAGE_DATA
http.begin("http://13.233.111.90/receive1.php?voltage=" + String(newData) + "");
int httpCode = http.GET();   												//Send the request
String payload = http.getString();                  //Get the response payload

Serial.println("Http status code: " + String(httpCode));   //Print HTTP return code
Serial.println("Http payload: " + payload);    //Print request response payload
http.end(); //Free the resources


//CODE TO SEND BPM & TEMPERATURE
http.begin("http://13.233.111.90/receive.php?bpm=" + String(beatspermin) + "&tempf=" + String(tempF) + "");
int httpCode = http.GET();   //Send the request
String payload = http.getString();                  //Get the response payload

Serial.println("Http status code: " + String(httpCode));   //Print HTTP return code
Serial.println("Http payload: " + payload);    //Print request response payload
http.end(); //Free the resources
delay(4000);


}
