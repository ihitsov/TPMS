#include <EEPROM.h>
#include <SPI.h>
#include "mcp_can.h"

float Cal[3];
boolean CalMode=0; // Calibration mode boolean for the TPMS

unsigned char SIDBuf0[6][8]={
{69, 150, 129, 67, 104, 101, 99, 107},//Check
{4, 150, 129, 32, 116, 105, 114, 101},//_tire
{3, 150, 129, 0, 0, 0, 0, 0}, //__
{2, 150, 130, 70, 72, 65, 73, 73}, //press
{1, 150, 130, 75, 72, 65, 73, 21}, //ures!
{0, 150, 130, 0, 0, 0, 0, 0} //__
};

unsigned char SIDBuf1[6][8]={
{69, 150, 129, 84, 80, 77, 83, 32}, //TPMS_
{4, 150, 129, 99, 97, 108, 105, 98}, //calib
{3, 150, 129, 45, 0, 0, 0, 0}, //-
{2, 150, 130, 114, 97, 116, 105, 111},//ratio
{1, 150, 130, 110, 32, 109, 111, 100},//n mod
{0, 150, 130, 101, 0, 0, 0, 0} //e_
};
        

unsigned char SIDBuf2[6][8]={
{69, 150, 129, 84, 80, 77, 83, 32}, //TPMS_
{4, 150, 129, 114, 101, 97, 100, 121}, //ready
{3, 150, 129, 33, 0, 0, 0, 0},//!
{2, 150, 130, 0, 0, 0, 0, 0},
{1, 150, 130, 0, 0, 0, 0, 0},
{0, 150, 130, 0, 0, 0, 0, 0}
};

unsigned char SIDBuf[6][8]={};

unsigned char SIDMystBuf[3][8]={
  {0, 50, 0, 0, 0, 0, 0, 0},
  {1, 255, 0, 0, 0, 0, 0, 0},
  {2, 255, 0, 0, 0, 0, 0, 0}
};
int SIDMystID=872;

//int SIDBufID=824;
int SIDBufID=831;

//unsigned char SIDDispOn[8]={31, 1, 5, 18,  0, 0, 0, 0};
//unsigned char SIDDispOff[8]={31, 0, 5, 8,  0, 0, 0, 0};
unsigned char SIDDispOn[8]={33, 0, 3, 50,  0, 0, 0, 0};
unsigned char SIDDispOff[8]={33, 0, 255, 50,  0, 0, 0, 0};

int SIDDispID=856;

//unsigned char SIDDispReq[8]={17, 0, 3, 25,  0, 0, 0, 0};
unsigned char SIDDispReq[8]={33, 0, 255, 50,  0, 0, 0, 0
};

int SIDDispReqID=856;

unsigned char SIDBeep[8]={128, 4, 0, 0, 0, 0, 0, 0};
int SIDBeepID=1072;

boolean Send2SID[2]={0,0};

MCP_CAN CAN(D8);                                    // D8 for esp8266 setup plus cheap MCP board, 10 for the elecfreaks on mega

void setup() {
  Serial.begin(230400);

  // Read TPMS calibration data
   union c_tag {
   byte b[4]; 
   float fval;
   } c;
  byte eepromsize=12;
  EEPROM.begin(eepromsize);
  delay(10);
  int j=0;
  int k=0;
  for (int i=0;i<eepromsize;i++){
    c.b[j]=EEPROM.read(i);
    j++;
    if (j==4)
    {
      Serial.println("");
      Serial.print("c.fval:");//Debugging if the eeprom value of the Cals are OK
      Serial.println(c.fval,6);
      if (fabs(c.fval-1)>0.05){
      c.fval=1.0;
      Serial.println("Ignoring the TPMS calibration data");// if eeprom is empty or full of random data, ignore the calibration
      } 
      j=0;
      Cal[k]=c.fval;
      k++;
    }
  }
  
    while (CAN_OK != CAN.begin(CAN_500KBPS, MCP_16MHz))              // init can bus : baudrate = 500k // Go to C:\Program Files (x86)\Arduino\libraries\CAN_BUS_Shield-master\mcp_can.h and change line:   byte begin(byte speedset, const byte clockset = MCP_16MHz);   to      byte begin(byte speedset, const byte clockset);     // init can

  {
    Serial.println("CAN BUS no es bueno!"); // Ref to AvE
    delay(1000);
  }

  Serial.println("CAN BUS Shield init ok!");
   
}

union c_tag {
byte b[4]; 
float fval;
} c;
   
void SIDMessages(){ // Function to re-send preiodical messages to SID
  static long ElapsedSIDPacketTime;
  static long ElapsedMysteryPacketTime;  
  static int i;
  static int j;
  if ((millis() - ElapsedSIDPacketTime) > 100 && Send2SID[1]!=0) {
      if (i==0){
      CAN.sendMsgBuf(SIDDispID, 0, 8, SIDDispOn);  // Send the messages to SID
      }
      if (i>0){
      CAN.sendMsgBuf(SIDBufID, 0, 8, SIDBuf[i-1]);  // Send the messages to SID
      }
      i++;
      if (i==7){
        i=0;
        ElapsedSIDPacketTime = millis(); // if its done, reset the timer
      }
  }
//    if ((millis() - ElapsedMysteryPacketTime) > 0 && Send2SID[1]!=0) { // Seems like this package is sent whenewer open sid is active, doing this in an attempt to alleviate the bug where the SID mesasges are displayed but are fighting with the radio messages
//          CAN.sendMsgBuf(SIDMystID, 0, 8, SIDMystBuf[j]);  // Send the messages to SID
//          j++;
//          if (j==3){
//            ElapsedMysteryPacketTime = millis(); // if its done, reset the timer
//            j=0;
//          }
//    }
    
//          CAN.sendMsgBuf(SIDDispID, 0, 8, SIDDispOn);  // Send command to SID to display the above messages
}

void FillSIDBuf(unsigned char TempBuf[6][8]){
  for (int i=0;i<6;i++){
    for (int j=0; j<8;j++){
      SIDBuf[i][j]=TempBuf[i][j];
    }
  }
}
void TPMS(int FL_UF,int FR_UF,int RL_UF,float RR) // get ther unfiltered values, RR is reference, so no need to cal it
{
  if (CalMode==0){
  float FL = (float)FL_UF/(Cal[0]*Cal[2]);
  float FR = (float)FR_UF/Cal[2];
  float RL = (float)RL_UF/Cal[1];
  float TPMSTol=0.011; // 0.01 should be about 0.5 bar difference on 17in wh., set a bit higher to allow for measurement errors and to prevent false alarms
  float TPMSTestRange=0.03;
  static long VTScoreOp;
  static long VTScoreOpSlow;
  static int TPMSCount;
  static int TPMSCountSlow;
  static float TPMSScore0; 
  static float TPMSScore1; 
  static float TPMSScore2; 
  static float TPMSScore3; 
  static float TPMSScore4; 
  static float TPMSScore5; 
  static float PrevRR;
  static boolean TPMSTripped;
  if (FL>300){ // Around 20 km/h
    if (fabs(RR-PrevRR)<10 &&(fabs(1-(float)FL/RL)<TPMSTestRange && fabs(1-(float)RL/RR)<TPMSTestRange)){ // if the front or the rear axle is going straight and the speed gradient is no larger than (5km/h)/sec
       Serial.print("FL:");
       Serial.print(FL);
       Serial.print(", FR:");
       Serial.print(FR);
       Serial.print(", RL:");
       Serial.print(RL);
       Serial.print(", RR:");
       Serial.println(RR);
       TPMSScore0+=1-(float)(FL+FR)/(RR+RL); // Ratio front to rear tires, fast eval, should be OK even in a slight turn
       TPMSScore1+=(float)FL/FR-(float)RL/RR;// Difference in turning between the front and rear tires, fast eval, should be OK even in a slight turn
       TPMSScore2+=1-(float)FL/RL; // Ratio of left tires, fast eval, should be OK even in a slight turn
       TPMSScore3+=1-(float)FR/RR; // Ratio of right tires, fast eval, should be OK even in a slight turn
       TPMSScore4+=1-(float)FL/FR;// average turning of the front tires, slow eval because its sensitive to turning(catches both ties on one side to be underinflated)
       TPMSScore5+=1-(float)RL/RR;// average turning of the rear tires, slow eval  because its sensitive to turning (catches both ties on one side to be underinflated)      
       VTScoreOp+=RR;
       VTScoreOpSlow+=RR; 
       TPMSCount++;
       TPMSCountSlow++;
    }
    PrevRR=RR;  
  }
  if (VTScoreOpSlow>9e6){
    if ((fabs((float)TPMSScore4/TPMSCountSlow))>TPMSTol || fabs((float)(TPMSScore5/TPMSCountSlow))>TPMSTol){
      TPMSTripped=1;
    }
        // Clean up for the next measurement window
        Serial.print("; TPMSScore4:");
        Serial.print((float)TPMSScore4/TPMSCountSlow,6);
        Serial.print("; TPMSScore5:");
        Serial.println((float)TPMSScore5/TPMSCountSlow,6);
        TPMSScore4=0;
        TPMSScore5=0;
        VTScoreOpSlow=0;
        TPMSCountSlow=0;
  }
  if (VTScoreOp>1.5e6){// Define measurement window of about 0.5km on 17in wheels
        if ((fabs((float)TPMSScore0/TPMSCount))>TPMSTol || fabs((float)(TPMSScore1/TPMSCount))>TPMSTol || fabs((float)(TPMSScore2/TPMSCount)>TPMSTol) || fabs((float)(TPMSScore3/TPMSCount))>TPMSTol){
           TPMSTripped=1;
        } 
        Serial.print("TPMSScore0:");
        Serial.print((float)TPMSScore0/TPMSCount,6);
        Serial.print("; TPMSScore1:");
        Serial.print((float)TPMSScore1/TPMSCount,6);
        Serial.print("; TPMSScore2:");
        Serial.print((float)TPMSScore2/TPMSCount,6);
        Serial.print("; TPMSScore3:");
        Serial.println((float)TPMSScore3/TPMSCount,6);


        // Clean up for the next measurement window
        VTScoreOp=0;
        TPMSScore0=0;
        TPMSScore1=0;
        TPMSScore2=0;
        TPMSScore3=0;
        TPMSCount=0;
    }
      if (TPMSTripped==1){
        TPMSTripped=0;
        // Send a beep and SID Text
        CAN.sendMsgBuf(SIDBeepID, 0, 8, SIDBeep);  // Send a chime to SID
        Serial.println("Underinflated Tire!!!");
        FillSIDBuf(SIDBuf0);

        Send2SID[0]=0; // Send messages from 0 to 5
        Send2SID[1]=5; // Send messages from 0 to 5
    }
  }
 

  if (CalMode==1){
    float CalTol=0.05;
    static long VTScore;// VTScore is the time*velocity of the wheel, in this way we can define the calibration window as path travelled, instead of simple sample number taken
    static long CalSum[4];
    if (RR>500 && (fabs(1-FL_UF/FR_UF)<CalTol && fabs(1-RL_UF/RR)<CalTol)){ // if the front and the rear axles are going straight-ish average the wheelspeeds
      static long CalScoreLimit=15e6;
      if (VTScore<CalScoreLimit){ //3e6 is 5km with 17in wheels // 2000 wheel RPS is obtained at 120km/h, 50 FPS, hence 2000*50*3600/120
      VTScore+=RR;
      CalSum[0]+=FL_UF;
      CalSum[1]+=FR_UF;
      CalSum[2]+=RL_UF;
      CalSum[3]+=RR;
      }
      if (VTScore>CalScoreLimit){ 
        Cal[0]=(float)CalSum[0]/CalSum[1];
        Cal[1]=(float)CalSum[2]/CalSum[3];
        Cal[2]=(float)CalSum[1]/CalSum[3];

          for (int i=0; i<3; i++){
          for (int j=0; j<4; j++){
          c.fval=Cal[i];
          EEPROM.write(i*4+j,c.b[j]);
          Serial.println(i*4+j);
          Serial.println(Cal[i]);
          }
        }
       EEPROM.commit();
        //delete temp calibration in case another one is done
        for (byte i=0; i<3;i++){CalSum[i]=0;}
        VTScore=0;
        CalMode=0;
        // Send a beep and SID Text
        CAN.sendMsgBuf(SIDBeepID, 0, 8, SIDBeep);  // Send a chime to SID
        Serial.println("Sending chime to SID for Cal complete"); // debugging
        FillSIDBuf(SIDBuf2); 

        Send2SID[0]=0; // Send messages from 0 to 5
        Send2SID[1]=5; // Send messages from 0 to 5
        }
    }
  }
}

void ReceiveCheck(){
  if (CAN_MSGAVAIL == CAN.checkReceive())           // check if data is coming
    {
      static long unsigned canID;
      static unsigned char len;
      static unsigned char buf[8];
      CAN.readMsgBufID(&canID, &len, buf);   // read data,  len: data length, buf: data buf
      static long ElapsedSIDSendTime;
     if ((millis() - ElapsedSIDSendTime) > 40 && Send2SID[1]!=0) {
          SIDMessages();
          ElapsedSIDSendTime = millis(); // if its time to send the SID messages, do it and reset the timer
        }
      if (canID == 768) {
        TPMS(buf[0] << 8 | buf[1] << 0,buf[2] << 8 | buf[3] << 0,buf[4] << 8 | buf[5] << 0,buf[6] << 8 | buf[7] << 0);
      }

      if (canID == 656){
        if (buf[5]==128){ // clear button pressed
          Send2SID[1]=0;
          CAN.sendMsgBuf(SIDDispID, 0, 8, SIDDispOff);  // Initialize the T7 data communication
          Serial.println("Clear button pressed");
        }
        if (buf[5]==160){ // SID clear+down pressed, start calibration
          CalMode=1;
          Serial.println("TPMS Calibration started!"); 
          FillSIDBuf(SIDBuf1);
        
        Send2SID[0]=0; // Send messages from 0 to 5
        Send2SID[1]=5; // Send messages from 0 to 5
        }
      }
      
    }
}

void loop() {
  ReceiveCheck();
}
