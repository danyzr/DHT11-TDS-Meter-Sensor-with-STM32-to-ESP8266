#define TdsSensorPin PA4
#define VREF 5.0              // analog reference voltage(Volt) of the ADC
#define SCOUNT  30            // sum of sample point
#define dht_apin PA5
#include <dht.h>
#include <LiquidCrystal_I2C.h>
#include <HardwareSerial.h>

HardwareSerial Serial2(PA3,PA2);

dht DHT;
LiquidCrystal_I2C lcd(0x27, 16, 2);

String str;
int analogBuffer[SCOUNT];     // store the analog value in the array, read from ADC
int analogBufferTemp[SCOUNT];
int analogBufferIndex = 0;
int copyIndex = 0;

float averageVoltage = 0;
float tdsValue = 0;
float temperature = 16;       // current temperature for compensation


// median filtering algorithm
int getMedianNum(int bArray[], int iFilterLen){
  int bTab[iFilterLen];
  for (byte i = 0; i<iFilterLen; i++)
  bTab[i] = bArray[i];
  int i, j, bTemp;
  for (j = 0; j < iFilterLen - 1; j++) {
    for (i = 0; i < iFilterLen - j - 1; i++) {
      if (bTab[i] > bTab[i + 1]) {
        bTemp = bTab[i];
        bTab[i] = bTab[i + 1];
        bTab[i + 1] = bTemp;
      }
    }
  }
  if ((iFilterLen & 1) > 0){
    bTemp = bTab[(iFilterLen - 1) / 2];
  }
  else {
    bTemp = (bTab[iFilterLen / 2] + bTab[iFilterLen / 2 - 1]) / 2;
  }
  return bTemp;
}

void setup(){
  lcd.init();
  lcd.backlight();
  Serial.begin(115200);
  Serial2.begin(115200); //to esp
  pinMode(TdsSensorPin,INPUT);
  pinMode(buzzer,OUTPUT);
}

void loop(){
  static unsigned long analogSampleTimepoint = millis();
  if(millis()-analogSampleTimepoint > 40U){     //every 40 milliseconds,read the analog value from the ADC
    analogSampleTimepoint = millis();
    analogBuffer[analogBufferIndex] = analogRead(TdsSensorPin);    //read the analog value and store into the buffer
    analogBufferIndex++;
    if(analogBufferIndex == SCOUNT){ 
      analogBufferIndex = 0;
    }
  }   
  
  static unsigned long printTimepoint = millis();
  if(millis()-printTimepoint > 800U){
    printTimepoint = millis();
    for(copyIndex=0; copyIndex<SCOUNT; copyIndex++){
      analogBufferTemp[copyIndex] = analogBuffer[copyIndex];
      
      // read the analog value more stable by the median filtering algorithm, and convert to voltage value
      averageVoltage = getMedianNum(analogBufferTemp,SCOUNT) * (float)VREF / 1024.0;
      
      //temperature compensation formula: fFinalResult(25^C) = fFinalResult(current)/(1.0+0.02*(fTP-25.0)); 
      float compensationCoefficient = 1.0+0.02*(temperature-25.0);
      //temperature compensation
      float compensationVoltage=averageVoltage/compensationCoefficient;
      
      //convert voltage value to tds value
      tdsValue=(133.42*compensationVoltage*compensationVoltage*compensationVoltage - 255.86*compensationVoltage*compensationVoltage + 857.39*compensationVoltage)*0.5;
      
      //Serial.print("voltage:");
      //Serial.print(averageVoltage,2);
      //Serial.print("V   ");
      
      Serial.print("TDS Value:");
      Serial.print(tdsValue);
      Serial.println("ppm");
      lcd.setCursor(0,0);
      lcd.print("TDS:");
      lcd.print(tdsValue);
      lcd.print(" ");
      lcd.print("ppm");
    }
  }
  DHT.read11(dht_apin);
  lcd.setCursor(0,1);
  lcd.print("H:");
  lcd.print(DHT.humidity);
  lcd.print("%");
  lcd.setCursor(9,1);
  lcd.print("T:");
  lcd.print(DHT.temperature); 
  lcd.println("C  ");


str = String("*")+String(tdsValue)+String(",")+String(DHT.humidity)+String(",")+String(DHT.temperature)+String("#");
Serial2.println(str);
}
