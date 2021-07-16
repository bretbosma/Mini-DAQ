//►►►►►►►►►►►►►►►►►►►►►►►►  ◄◄◄◄◄◄◄◄◄◄◄◄◄◄◄◄◄◄◄◄◄◄◄◄
//►►►►►►►►►►►►►►►►►►►►► Start of Code ◄◄◄◄◄◄◄◄◄◄◄◄◄◄◄◄◄◄◄


//♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦Header Files♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦
//                                    ▼▼▼
#define CUSTOM                   // To use custom configured EasyCAT headerfile
#include "MiniDAQ_EasyCAT.h"     // Header File generated by EasyCAT Configurator 
#include <dmtimer.h>             // Sample Time setting Library
#include "EasyCAT.h"             // EasyCAT SPI Library   
#include <Wire.h>
#include "ADS1115.h"

//♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦Instances Definition♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦
//                                    ▼▼▼
EasyCAT EASYCAT(9);      // Define EasyCAT Instance and specify EasyCAT SPI ChipSelect Pin in paranthesis
DMTimer myTimer(10000);   // Define myTimer Instance and specify Sample Time in Micro Seconds, 1 msec
ADS1115 ads;              // Define 4-20mA transducer instance

//♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦Global Variables♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦
//                                    ▼▼▼

const int Debug_LED = 48;   // LED used for debuging purposes

//--->>>Current sensor variables<<<<
int16_t adc0;           // current sensor variable

//--->>>ADC Variables<<<<
const int CS_ADC = 53;   // ADC SPI ChipSelect 
word M1ADC[4];          // four 16bit differential ADC variables

unsigned long t;        // variable to store time in MicroSecs, for loop execution time monitoring

//--->>>IMU Commands<<<<

byte Put2_Config_Mode[5] = {250,  255,  48,   0,  209};  // Command to put IMU in configuration mode
//                         0xFA  0xFF  0x30  0x00  0xD1  // Hex form
byte SetOutputConfiguration[17] = {250,  255,  192,   12,  32,  48,   0, 100,  64,  48,   0, 100,   8,  16, 255, 255, 151};  // 
//                                0xFA  0xFF  0xC0  0x0C 0x20 0x30 0x00 0x64 0x40 0x30 0x00 0x64 0x08 0x10 0xFF 0xFF 0xA7 // Hex form
//                                                       [ Euler ]           [ Accel ]           [ Temp  ] 
byte Put2_Measure_Mode[5]= {250,  255,  16,   0,  241};  // Command to put IMU in Measurement mode
//                         0xFA  0xFF  0x10  0x00 0xF1   // Hex form
byte Request_IMU_Packet[5]={250,  255,  52,   0,  205};  // Command to Request Data from IMU in Measument mode
//                         0xFA  0xFF  0x34  0x00 0xCD   // Hex form

const int Len=41;             //IMU Packet Length  4*3_Euler + 4*3_Acc + 4_Temp
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
        {Serial.println ("EasyCAT Initialized");}                                                                                 
  else 
        {Serial.print ("EasyCAT Initialization Failed");
         while(1); } // Stop Here if EtherCAT Init fails
//▬▬▬▲

//▼▬▬IMU Initialization
  Serial.print ("\n IMU Initialization In Process");           // Print the banner
  Initialize_IMU();    // Stop Here if IMU Init fails
//▬▬▬▲

//▼▬▬4-20mA transducer Initialization
//  Serial.print ("\n 4-20mA transducer Initialization In Process");           // Print the banner
//  Initialize_currentSens();
//▬▬▬▲
}


//♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦Main Loop♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦
//                                     ▼▼▼ 
void loop()                                           
{               
  t=micros();               // Monitor Controller internal time at loop start
  heartbeat();              // Blink LED to prove it is alive!
  EASYCAT.MainTask();       // UDF for EasyCAT Trigger
  read_ADC();               // UDF for ADC reading
  //adc0 = ads.Measure_SingleEnded(0);  // current sensor read value
  read_IMU();               // UDF for IMU reading
  EtherCAT_Frame_Update();  // UDF for EtherCAT data Frame Updating
  IMU_Packet_Deframe();     // UDF for IMU Data frame decoding

  //Serial.print(N);

  
  //Serial.print("\t");
  
  //Serial.println((micros()-t)/1000.); // Loop execution time

  while(!myTimer.isTimeReached());   // Wait here Till 10 msec sample time Tick
}

//♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦User Defined Functions♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦♦
//                                        ▼▼▼
//▓▒▒▒╠═══ Bin To Float ═══╣▒▒▒▓
float binToFloat(byte D3, byte D2, byte D1, byte D0)
{
  byte bin[4];
  bin[3] = D3;  bin[2] = D2;  bin[1] = D1;  bin[0] = D0;
  return *( (float*) bin ); 
}

//▓▒▒▒╠═══ ADC Read ══════╣▒▒▒▓
void read_ADC()
{
 for(word add=0;add<=3;add++)
 {
 digitalWrite(CS_ADC, LOW);
 word x= add<<12 | 0x4400;
 M1ADC[add]=SPI.transfer16( x ); 
 digitalWrite(CS_ADC, HIGH);
 digitalWrite(CS_ADC, LOW);
 M1ADC[add]=SPI.transfer16( x ); 
 digitalWrite(CS_ADC, HIGH);
 }
}
//▓▒▒▒╠═══ Initialize current sensor ══╣▒▒▒▓
void Initialize_currentSens(void)
{
  ads.getAddr_ADS1115(ADS1115_DEFAULT_ADDRESS);   // 0x48, 1001 000 (ADDR = GND)
  ads.setGain(GAIN_ONE);       // 1x gain   +/- 4.096V  1 bit = 0.125mV
  ads.setMode(MODE_CONTIN);       // Continuous conversion mode
  ads.setRate(RATE_128);          // 128SPS (default)
  ads.setOSMode(OSMODE_SINGLE);   // Set to start a single-conversion
  ads.begin();
  
}
//▓▒▒▒╠═══ Initialize IMU ══╣▒▒▒▓
void Initialize_IMU(void)
{
  byte temp[5]={0,0,0,0,0};
  int i=0;
  while(i<=10) // Ten attempts to put IMU in Measure Mode
  {  
    delay(500);     //Needed to let the IMU wake up before serial communication
    Serial1.write(Put2_Config_Mode,5);
    //delay(5);
    while(Serial1.available()>0) Serial1.read();
    delay(5);
    
    Serial1.write(SetOutputConfiguration,17);
    delay(100);                     // This delay is needed to prevent fail
    while(Serial1.available()>0) Serial1.read();

    Serial1.write(Put2_Measure_Mode,5);
    Serial1.readBytes(temp, 5);
    
    if(temp[2]==17)
    {
      Serial.println("\n IMU Initilised Successfully");
      delay(1000);
      break;
    }
    i=i+1;
    Serial.print(". ");
  }
  if(i==11)
  {
  Serial.println("IMU Initilisation Failed 10 Attempts");
  //
  //while(1);  // Code stop here.
    }
}

//▓▒▒▒╠═══ IMU Read ═════╣▒▒▒▓
void read_IMU(void)
{ 

  Serial1.write(Request_IMU_Packet,5);
  N=0;
  while(Serial1.available()>0 && N<Len) IMUPacket[N++]=Serial1.read();
  while(Serial1.available()>0)Serial1.read(); //flush
  Serial.println(IMUPacket[8]);
}

//▓▒▒▒╠═══ IMU Data Deframe ╣▒▒▒▓
void IMU_Packet_Deframe(void)
{
  if(N==Len)
  {
    r = binToFloat(IMUPacket[7],IMUPacket[8],IMUPacket[9],IMUPacket[10]);      //deg
    p = binToFloat(IMUPacket[11],IMUPacket[12],IMUPacket[13],IMUPacket[14]);
    y = binToFloat(IMUPacket[15],IMUPacket[16],IMUPacket[17],IMUPacket[18]);
    
    ax=binToFloat(IMUPacket[22],IMUPacket[23],IMUPacket[24],IMUPacket[25]);    //m per second^2
    ay=binToFloat(IMUPacket[26],IMUPacket[27],IMUPacket[28],IMUPacket[29]);
    az=binToFloat(IMUPacket[30],IMUPacket[31],IMUPacket[32],IMUPacket[33]);

    T=binToFloat(IMUPacket[37],IMUPacket[38],IMUPacket[39],IMUPacket[40]);   // deg C
  }
}

//▓▒▒▒╠═══ EtherCAT Data Update ╣▒▒▒▓
void EtherCAT_Frame_Update()                                        
{
    EASYCAT.BufferIn.Cust.ADC0 = M1ADC[0];  // analog to digital channel 0
    EASYCAT.BufferIn.Cust.ADC1 = M1ADC[1];
    EASYCAT.BufferIn.Cust.ADC2 = M1ADC[2];
    EASYCAT.BufferIn.Cust.ADC3 = M1ADC[3]; 

    EASYCAT.BufferIn.Cust.roll  = r;        //roll
    EASYCAT.BufferIn.Cust.pitch = p;        //pitch
    EASYCAT.BufferIn.Cust.yaw   = y;        //yaw

    EASYCAT.BufferIn.Cust.acc_x = ax;       //acceleration in x
    EASYCAT.BufferIn.Cust.acc_y = ay;       //acceleration in y
    EASYCAT.BufferIn.Cust.acc_z = az;       //acceleration in z

    EASYCAT.BufferIn.Cust.current0 = adc0;

    EASYCAT.BufferIn.Cust.temperature = T; 
}

//▓▒▒▒╠═══ Toggle LED to prove alive ╣▒▒▒▓
void heartbeat()
{
    count++; 
  if (count == 1){
    digitalWrite(Debug_LED, HIGH); //Turn LED on                        
  }
  else if (count == 1000){
    digitalWrite(Debug_LED, LOW); //Turn LED off                      
  }
  else if (count == 2000){
    count = 0;                      
  }
}

//►►►►►►►►►►►►►►►►►►►►►►►► END of CODE ◄◄◄◄◄◄◄◄◄◄◄◄◄◄◄◄◄◄◄◄◄◄◄◄
//►►►►►►►►►►►►►►►►►►►►►►►►►►►► ◄◄◄◄◄◄◄◄◄◄◄◄◄◄◄◄◄◄◄◄◄◄◄◄◄◄◄◄
