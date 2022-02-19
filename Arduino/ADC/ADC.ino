//►►►►►►►►►►►►►►►►►►►►►►►►  ◄◄◄◄◄◄◄◄◄◄◄◄◄◄◄◄◄◄◄◄◄◄◄◄
//►►►►►►►►►►►►►►►►►►►►► Start of Code ◄◄◄◄◄◄◄◄◄◄◄◄◄◄◄◄◄◄◄


//♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦Header Files♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦
//                                    ▼▼▼
#define BYTE_NUM 32                   // Standard mode 32+32 
#include <SPI.h>
#include <DueTimer.h>             // Sample Time setting Library
#include "EasyCAT.h"             // EasyCAT SPI Library

//♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦Instances Definition♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦
//                                    ▼▼▼
EasyCAT EASYCAT(9);      // Define EasyCAT Instance and specify EasyCAT SPI ChipSelect Pin in paranthesis

//♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦Global Variables♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦
//                                    ▼▼▼

const int Debug_LED = 13;   // LED used for debuging purposes

//--->>>ADC Variables<<<<
const int CS_ADC = 53;   // ADC SPI ChipSelect
byte M1ADC[8];          // eight 8bit differential ADC variables
byte Garbage[2];
int loopCount[] = {1, 2, 3, 0};
unsigned long t;        // variable to store time in MicroSecs, for loop execution time monitoring
unsigned int count = 0;

//♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦Setup♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦
//                                    ▼▼▼
void setup()
{
  Serial.begin(115200);    //Serial communication Arduino Mega with PC for debugging
  delay(1000);
  Serial.println("hello world");
  pinMode(CS_ADC, OUTPUT);  //Declare ADC ChipSelect Pin as Output
  pinMode(Debug_LED, OUTPUT);  //Declare ADC ChipSelect Pin as Output
  digitalWrite(Debug_LED, LOW);//Turn LED off
  digitalWrite(CS_ADC, LOW);//Disable ADC (Active LOW)

//▼▬▬EasyCAT Initialization
  Serial.println ("\n EasyCAT Initialization In Process");   // Print the banner
  if    (EASYCAT.Init() == true)
  {
    Serial.println ("EasyCAT Initialized");
  }
  else
  { Serial.print ("EasyCAT Initialization Failed");
    //while (1);
  } // Stop Here if EtherCAT Init fails
  //▬▬▬▲


}


//♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦Main Loop♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦
//                                     ▼▼▼
void loop()
{

  t = micros();             // Monitor Controller internal time at loop start
  heartbeat();              // Blink LED to prove it is alive!

  EASYCAT.MainTask();       // UDF for EasyCAT Trigger
  read_ADC();               // UDF for ADC reading
  EtherCAT_Frame_Update();  // UDF for EtherCAT data Frame Updating

  Serial.print((micros() - t) / 1000.); // Loop execution time
  Serial.print("\t");
  Serial.println(M1ADC[0]);
}

//♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦User Defined Functions♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦
//                                        ▼▼▼


//▓▒▒▒╠═══ ADC Read ══════╣▒▒▒▓
void read_ADC()
{
  digitalWrite(CS_ADC, HIGH);
  delayMicroseconds(50);
  SPI.begin(CS_ADC);
    digitalWrite(CS_ADC, LOW);
    byte x1 = 0x44;
    byte x2 = 0x00;
    Garbage[0] = SPI.transfer(x1);
    Garbage[1] = SPI.transfer(x2);
    digitalWrite(CS_ADC, HIGH);
  
  for (word i = 0; i <= 3; i++)
  {
    digitalWrite(CS_ADC, LOW);
    byte x1 = loopCount[i] << 4 | 0x44;
    byte x2 = 0x00;
    M1ADC[2 * i + 1] = SPI.transfer(x1);
    M1ADC[2 * i] = SPI.transfer(x2);
    digitalWrite(CS_ADC, HIGH);
  }
    digitalWrite(CS_ADC, LOW); 
  SPI.end(CS_ADC); 

}

//▓▒▒▒╠═══ EtherCAT Data Update ╣▒▒▒▓
void EtherCAT_Frame_Update()
{
  for (byte a = 0; a <= 7; a++)
  {
    EASYCAT.BufferIn.Byte[a] = M1ADC[a];  // two bytes for each channel ADC
  }
}

//▓▒▒▒╠═══ Toggle LED to prove alive ╣▒▒▒▓
void heartbeat()
{
  count++;
  if (count == 1) {
    digitalWrite(Debug_LED, HIGH); //Turn LED on
  }
  else if (count == 100) {
    digitalWrite(Debug_LED, LOW); //Turn LED off
  }
  else if (count == 200) {
    count = 0;
  }
}

//►►►►►►►►►►►►►►►►►►►►►►►► END of CODE ◄◄◄◄◄◄◄◄◄◄◄◄◄◄◄◄◄◄◄◄◄◄◄◄
//►►►►►►►►►►►►►►►►►►►►►►►►►►►► ◄◄◄◄◄◄◄◄◄◄◄◄◄◄◄◄◄◄◄◄◄◄◄◄◄◄◄◄
