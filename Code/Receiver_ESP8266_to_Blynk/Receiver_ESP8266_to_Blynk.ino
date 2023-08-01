#include <SoftwareSerial.h>

SoftwareSerial mySerial(D7, D8);

int i;
bool parsing = false;
String sData,data[10];

void setup(){
  Serial.begin(115200);
  mySerial.begin(115200);
  while(!mySerial){
  }
}

void loop(){
  while(mySerial.available()){
    char inChar = mySerial.read();
    sData += inChar;
    if (inChar == '$'){
  }
    if (parsing){
      int q = 0;
      for(int i=0; i < sData.length(); i++){
        if(sData[i] == '#'){
          q++;
          data[q] = "";
        } else {
          data[q] += sData[i];
        }
      }
      Serial.println(data[1]);
      Serial.println(data[2]);
      Serial.println(data[3]);
      Serial.println();
      parsing = false;
      sData = "";
    }
}
}
