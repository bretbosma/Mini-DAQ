//►►►►►►►►►►►►►►►►►►►►►►►►  ◄◄◄◄◄◄◄◄◄◄◄◄◄◄◄◄◄◄◄◄◄◄◄◄
//►►►►►►►►►►►►►►►►►►►►► Start of Code ◄◄◄◄◄◄◄◄◄◄◄◄◄◄◄◄◄◄◄


//♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦Header Files♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦
//                                    ▼▼▼
#define BYTE_NUM 32                   // Standard mode 32+32 
#include <SPI.h>
#include <dmtimer.h>             // Sample Time setting Library
#include "EasyCAT.h"             // EasyCAT SPI Library   
#include <Wire.h>

//♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦Instances Definition♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦
//                                    ▼▼▼
EasyCAT EASYCAT(9);      // Define EasyCAT Instance and specify EasyCAT SPI ChipSelect Pin in paranthesis
DMTimer myTimer(10000);   // Define myTimer Instance and specify Sample Time in Micro Seconds, 10 msec

//♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦Global Variables♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦
//                                    ▼▼▼

const int Debug_LED = 48;   // LED used for debuging purposes

//--->>>ADC Variables<<<<
const int CS_ADC = 53;   // ADC SPI ChipSelect
word M1ADC[4];          // four 16bit differential ADC variables
byte M2ADC[8];
unsigned long t;        // variable to store time in MicroSecs, for loop execution time monitoring

//--->>>IMU Commands<<<<

byte Put2_Config_Mode[5] = {250,  255,  48,   0,  209};  // Command to put IMU in configuration mode
//                         0xFA  0xFF  0x30  0x00  0xD1  // Hex form
byte SetOutputConfiguration[17] = {250,  255,  192,   12,  32,  48,   0, 100,  64,  48,   0, 100,   8,  16, 0, 100, 49};  //
//                                0xFA  0xFF  0xC0  0x0C 0x20 0x30 0x00 0x64 0x40 0x30 0x00 0x64 0x08 0x10 0x00 0x64 0x31 // Hex form
//                                                       [ Euler ]           [ Accel ]           [ Temp  ]
byte SetSyncSettings[17] = {250, 255, 44,  12,  8,   6,   3,   1,   0,   0,   0,   0,   0,   0,   0,   0,   183};  //Set IMU so only sends data when requested
//                          0xFA 0xFF 0x2C 0x0C 0x08 0x06 0x03 0x01 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0xB7
byte Put2_Measure_Mode[5] = {250,  255,  16,   0,  241}; // Command to put IMU in Measurement mode
//                         0xFA  0xFF  0x10  0x00 0xF1   // Hex form
byte Request_IMU_Packet[5] = {250,  255,  52,   0,  205}; // Command to Request Data from IMU in Measument mode
//                         0xFA  0xFF  0x34  0x00 0xCD   // Hex form

const int Len = 34;           //IMU Packet Length  4*3_Euler + 4*3_Acc + 4_Temp
byte IMUPacket[Len];          // IMU Data Capture Array
unsigned int N = 0;             // Number of IMU bytes captured each sample time in IMUPacket[] array
unsigned int count = 0;

float r,p,y;      //3*Eulers
float ax,ay,az;               //3* Acc
float T;                      // Temperature
//♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦Setup♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦
//                                    ▼▼▼
void setup()
{
  Serial.begin(115200);    //Serial communication Arduino Mega with PC for debugging
  Serial1.begin(115200);   //Serial communication Arduino Mega with IMU
  Serial1.setTimeout(3);   //maximum wait period in milli seconds for data from IMU
  delay(1000);             //Pause for the above Serial Ports configuration to take effect

  pinMode(CS_ADC, OUTPUT);  //Declare ADC ChipSelect Pin as Output
  pinMode(Debug_LED, OUTPUT);  //Declare ADC ChipSelect Pin as Output

  digitalWrite(Debug_LED, LOW);//Turn LED off
  digitalWrite(CS_ADC, HIGH);//Disable ADC (Active LOW)

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

  //▼▬▬IMU Initialization
  Serial.print ("\n IMU Initialization In Process");           // Print the banner
  Initialize_IMU();    // Stop Here if IMU Init fails
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
  read_IMU();               // UDF for IMU reading
  EtherCAT_Frame_Update();  // UDF for EtherCAT data Frame Updating


  Serial.print(N);
  Serial.print("\t");
  Serial.println((micros() - t) / 1000.); // Loop execution time
  while (!myTimer.isTimeReached());  // Wait here Till 10 msec sample time Tick
}

//♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦User Defined Functions♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦
//                                        ▼▼▼

//▓▒▒▒╠═══ ADC Read ══════╣▒▒▒▓
void read_ADC()
{
  for (word add = 0; add <= 3; add++)
  {
    digitalWrite(CS_ADC, LOW);
    word x = add << 12 | 0x4400;
    M1ADC[add] = SPI.transfer16( x );
    digitalWrite(CS_ADC, HIGH);
    digitalWrite(CS_ADC, LOW);
    M1ADC[add] = SPI.transfer16( x );
    digitalWrite(CS_ADC, HIGH);
    M2ADC[2*add] = highByte(M1ADC[add]);
    M2ADC[2*add +1] = lowByte(M1ADC[add]);
  }
}

//▓▒▒▒╠═══ Initialize IMU ══╣▒▒▒▓
void Initialize_IMU(void)
{
  byte temp[5] = {0, 0, 0, 0, 0};
  int i = 0;
  while (i <= 10) // Ten attempts to put IMU in Measure Mode
  {
    delay(500);     //Needed to let the IMU wake up before serial communication
    Serial1.write(Put2_Config_Mode, 5);
    //delay(5);
    while (Serial1.available() > 0) Serial1.read();
    delay(5);

    Serial1.write(SetOutputConfiguration, 17);
    delay(100);                     // This delay is needed to prevent fail
    while (Serial1.available() > 0) Serial1.read();
    
    Serial1.write(SetSyncSettings, 17);
    delay(100);                     // This delay is needed to prevent fail
    while (Serial1.available() > 0) Serial1.read();
    
    Serial1.write(Put2_Measure_Mode, 5);
    Serial1.readBytes(temp, 5);

    if (temp[2] == 17)
    {
      Serial.println("\n IMU Initilised Successfully");
      delay(1000);
      break;
    }
    i = i + 1;
    Serial.print(". ");
  }
  if (i == 11)
  {
    Serial.println("IMU Initilisation Failed 10 Attempts");
    //
    while(1);  // Code stop here.
  }
}

//▓▒▒▒╠═══ IMU Read ═════╣▒▒▒▓
void read_IMU(void)
{

  Serial1.write(Request_IMU_Packet, 5);
  N = 0;
  while (Serial1.available() > 0 && N < Len) IMUPacket[N++] = Serial1.read();
  while (Serial1.available() > 0)Serial1.read(); //flush
  //Serial.println(IMUPacket[8]);
}

//▓▒▒▒╠═══ EtherCAT Data Update ╣▒▒▒▓
void EtherCAT_Frame_Update()
{
  for (byte a = 0; a <= 7; a++)
  {
    EASYCAT.BufferIn.Byte[a] = M2ADC[a];  // two bytes for each channel ADC
  }

  for (byte b = 0; b <= 11; b++)
  {
    EASYCAT.BufferIn.Byte[b + 8] = IMUPacket[b + 7]; // four bytes for each roll, pitch, yaw
  }

  for (byte c = 0; c <= 11; c++)
  {
    EASYCAT.BufferIn.Byte[c + 20] = IMUPacket[c + 22]; // four bytes for each ax, ay, az
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
