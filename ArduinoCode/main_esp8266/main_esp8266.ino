#include <SerialESP8266wifi.h>

#define esp8266_reset_pin 5
#define SERVER_PORT "2121"
#define MY_SSID "Dim"
#define PASSWORD "20152015"

//******* wifi part ***********
SerialESP8266wifi wifi(Serial1, Serial1, esp8266_reset_pin, Serial);// 串口1和ESP8266交互，串口0做调试输出
WifiConnection *connections;
bool wifi_started = false;

enum WorkMode{
  WorkMode_ON,WorkMode_OFF,WorkMode_ANALOG_USB,WorkMode_TIMING_ON,WorkMode_TIMING_OFF
};
enum SenserType{
  SenserType_DIGITAL,SenserType_ANALOG,SenserType_UNUSE
};

//******* data variable ***************
//common part
WorkMode workMode;
SenserType type_modeUsingSeneser;
int pin_modeUsingSeneser;
//digital part
int levelOutput_whenDigitalHigh;
//analog part
int thresholdValue_analogPin;
int levelOutput_whenBiggerThanThreshold;
int targetLevelLast_analogIgnore;   //prevent  jitter,record trying ele-level
int numAnalogIgnore;   //prevent  jitter,record trying times
const int MAX_ANALOG_IGNORE=2; //after count this num times ,accept the analog 
//timing part
WorkMode mode_afterTimeout;
unsigned long time_timingTarget;
const int TIMING_MAX_MINUTE=1440;
//************ function prototype ***********
void wifiRestart();
bool checkConnect();
bool handleInput();
void handleModeUpdate();
bool isSenserMode();
bool setOutputRelay(int);
void resetSenserArgument();
void resetDigitalArgument();
void resetAnalogArgument();
void resetTimingArgument();
//************ pin and state ***********
const int PIN_OUTPUT_RELAY=13;//插座输出继电器控制端 //when debug,set to 13
int ele_level_outputRelay;

const int PIN_SENSER_USB_ANALOG_YELLOW=A0;


//*********** command ************
const String CMD_SET_MODE="MODE ";
const String CMD_SET_TIMING="TIMING ";

//********* mode ***********
const String MODE_ANALOG_USB="analog";
const String MODE_ON="on";
const String MODE_OFF="off";
const String MODE_TIMING_ON="timing_on";
const String MODE_TIMING_OFF="timing_off";


//******* for debug *******
const int PIN_DEBUG_SETMODE=50;//按下按钮，进入某个mode，方便改mode的debug
const WorkMode DEBUG_MODE=WorkMode_ANALOG_USB;


void setup() {
  //******** pin config init **********
  pinMode(PIN_OUTPUT_RELAY,OUTPUT);
  pinMode(PIN_SENSER_USB_ANALOG_YELLOW,INPUT);
  pinMode(PIN_DEBUG_SETMODE,INPUT_PULLUP);

  //********** data init ***********
  workMode=WorkMode_OFF;
  ele_level_outputRelay=LOW;
  digitalWrite(PIN_OUTPUT_RELAY,ele_level_outputRelay);
  
  type_modeUsingSeneser=SenserType_UNUSE;

  //unnecessary

  
  //******** wifi config init **********
  const long BAUD_RATE=115200;
  // 调试串口启动，波特率可随意指定
  Serial.begin(BAUD_RATE);

  // ESP8266默认波特率为115200（可用AT指令修改）
  Serial1.begin(BAUD_RATE);
  while (!Serial); // 等待串口初始化成功
    
  Serial1.println("Starting wifi"); // 命令
  Serial.println("Starting wifi");  // 调试log输出
  
  wifiRestart();
  
}


void loop() {
    if(digitalRead(PIN_DEBUG_SETMODE)==LOW){
      Serial.println("set to debug mode.");
      workMode=DEBUG_MODE;
      handleModeUpdate();
    }
      
    
    if(checkConnect()){   
      bool isModeChanged=handleInput();
      if(isModeChanged){
        handleModeUpdate();
      }
    }

    if(isSenserMode()){
      if(type_modeUsingSeneser==SenserType_ANALOG){
        int input=analogRead(pin_modeUsingSeneser);
        //int asDigital=digitalRead(pin_modeUsingSeneser);
        Serial.print("analogValue=");
        Serial.print(input);
        //Serial.print(" asDigitalogValue=");
        //Serial.print(asDigital);
        //Serial.println("\n");

        int targetLevelCurrent;
        if(input>thresholdValue_analogPin xor levelOutput_whenBiggerThanThreshold==LOW) // True(set HIGH) only: (in>thd && whenBig==High),(in<thd && whenBig==LOW),thus operateion "xor"
          targetLevelCurrent=HIGH;
        else
          targetLevelCurrent=LOW;

        if(targetLevelCurrent==targetLevelLast_analogIgnore){
          numAnalogIgnore--;
            if(numAnalogIgnore==0){
              //commit this analog caused relay change
              numAnalogIgnore=MAX_ANALOG_IGNORE;
              setOutputRelay(targetLevelCurrent);
            }
        }
        else{
          //not same target ele-level with before,reset counter and renew targetLevelLast_analogIgnore
          numAnalogIgnore=MAX_ANALOG_IGNORE;
          targetLevelLast_analogIgnore=targetLevelCurrent;
        }
      }
      
    }
    else if(workMode==WorkMode_TIMING_ON||workMode==WorkMode_TIMING_OFF){
      if(time_timingTarget==-1){
        Serial.println("Timing not set time.");
        delay(1000);
      }
      else{
         if(millis()>=time_timingTarget){
          Serial.println("Timing done.");
          workMode=mode_afterTimeout;
          handleModeUpdate();
        }
        else{
          Serial.print("Timing:");
          Serial.println((time_timingTarget-millis())/1000);
          delay(1000);
        }
      }
      
    }
    
}

bool setOutputRelay(int level){
  if(ele_level_outputRelay==level){
    Serial.println("Relay keep "+getEleLevelString(ele_level_outputRelay));
    return false;//not change
  }
   else{
    //change output
    ele_level_outputRelay=level;
    digitalWrite(PIN_OUTPUT_RELAY,ele_level_outputRelay);
    Serial.println("Relay change to "+getEleLevelString(ele_level_outputRelay));
    return true;
   }
}

String getEleLevelString(int level){
  if(level==HIGH)
    return "HIGH";
  else if(level==LOW)
    return "LOW";
  else
    return "NOT_DEFINE_LEVEL";  
}

bool isSenserMode(){
  if(workMode==WorkMode_ANALOG_USB)
    return true;
  else
    return false;
}

bool checkConnect(){
  // check connections if the ESP8266 is there
  if (wifi_started){
    wifi.checkConnections(&connections); // 检查/获取当前所有连接的状况
    return true;
  }
  else{
    wifiRestart();
    return false;
  }
 
}

bool handleInput(){
  
  bool isModeChanged=true;
  WifiMessage in = wifi.listenForIncomingMessage(1000);
  if (in.hasData) {
    String inMassage=in.message;
    String echoText="";
    if(inMassage.startsWith(CMD_SET_MODE)){
      echoText+="command=set_mode ";
      String argument=inMassage.substring(CMD_SET_MODE.length());
      if(argument.equalsIgnoreCase(MODE_ANALOG_USB)){
        echoText+="mode=heat_detect ";
        workMode=WorkMode_ANALOG_USB;
      }
      else if(argument.equalsIgnoreCase(MODE_ON)){
        echoText+="mode=always_on ";
        workMode=WorkMode_ON;
      }
      else if(argument.equalsIgnoreCase(MODE_OFF)){
        echoText+="mode=always_off ";
        workMode=WorkMode_OFF;
      }
      else if(argument.equalsIgnoreCase(MODE_TIMING_ON)){
        echoText+="mode=timing_on ";
        workMode=WorkMode_TIMING_ON;
      }
      else if(argument.equalsIgnoreCase(MODE_TIMING_OFF)){
        echoText+="mode=timing_off ";
        workMode=WorkMode_TIMING_OFF;
      }
      else{
        echoText+="mode erro ";
        isModeChanged=false;
      }
    }
    else if(inMassage.startsWith(CMD_SET_TIMING)){
      echoText+="command=set_timing ";
      long argument=inMassage.substring(CMD_SET_TIMING.length()).toInt();
      if(argument>0&&argument<=TIMING_MAX_MINUTE){
        echoText+="timing="+String(argument*1000);//when debug,set argument as second
        time_timingTarget=millis()+argument*1000;//when debug,set argument as second
      }
      else{
        echoText+="time erro,time should between 1 and 1440";
      }
      isModeChanged=false;
    }
    else{
        echoText+="command erro ";
        isModeChanged=false;
    }
    
    //Echo back;
    wifi.send(in.channel, "Echo:",false);
    wifi.send(in.channel, echoText);
    //nextPing = millis() + 10000;

  }
  else
    isModeChanged=false;
    
  return isModeChanged;
}

void handleModeUpdate(){
  if(workMode==WorkMode_ANALOG_USB){
    type_modeUsingSeneser=SenserType_ANALOG;
    pin_modeUsingSeneser=PIN_SENSER_USB_ANALOG_YELLOW;
    
    thresholdValue_analogPin=150;
    levelOutput_whenBiggerThanThreshold=HIGH; //bigger<->have people<->turn on electry
    targetLevelLast_analogIgnore=-1;
    numAnalogIgnore=MAX_ANALOG_IGNORE;
    
    resetDigitalArgument();
    resetTimingArgument(); 
  }
  else if(workMode==WorkMode_TIMING_ON){
    type_modeUsingSeneser=SenserType_UNUSE;
    pin_modeUsingSeneser=-1;
    
    mode_afterTimeout=WorkMode_ON;
    time_timingTarget=-1;
    
    resetDigitalArgument();
    resetAnalogArgument(); 
  }
  else if(workMode==WorkMode_TIMING_OFF){
    type_modeUsingSeneser=SenserType_UNUSE;
    pin_modeUsingSeneser=-1;
    
    mode_afterTimeout=WorkMode_OFF;
    time_timingTarget=-1;
    
    resetDigitalArgument();
    resetAnalogArgument(); 
  }
  else if(workMode==WorkMode_ON){
    setOutputRelay(HIGH);
    
    resetSenserArgument();
    resetDigitalArgument();
    resetAnalogArgument();
    resetTimingArgument();
  }
  else if(workMode==WorkMode_OFF){
    setOutputRelay(LOW);
    
    resetSenserArgument();
    resetDigitalArgument();
    resetAnalogArgument();
    resetTimingArgument();
  }
}

void resetSenserArgument(){
    type_modeUsingSeneser=SenserType_UNUSE;
    pin_modeUsingSeneser=-1;
}
void resetDigitalArgument(){
  levelOutput_whenDigitalHigh=-1;
}
void resetAnalogArgument(){
    thresholdValue_analogPin=-1;
    levelOutput_whenBiggerThanThreshold=-1;
    targetLevelLast_analogIgnore=-1;
    numAnalogIgnore=-1;
}
void resetTimingArgument(){
  mode_afterTimeout=WorkMode_OFF;
  time_timingTarget=-1;
}


void wifiRestart(){
  wifi.setTransportToTCP();       // TCP模式（默认设置）
  wifi.endSendWithNewline(true); // 发送数据自动以\r\n结尾（默认设置）

  //for(int retry=0;retry<3;retry++){
    wifi_started = wifi.begin();    // WIFI启动
    if (wifi_started) { // 如果WIFI初始化成功
      //wifi.connectToAP(MY_SSID, PASSWORD); // 指定用户名密码连接WIFI
      //wifi.startLocalServer(SERVER_PORT);
      wifi.startLocalAPAndServer("MY_CONFIG_AP", "password", "5", "2121");
      Serial.println("ESP8266 is working!");
      Serial.println(wifi.getIP());
      //break;
    } else {
      // ESP8266 isn't working..
      Serial.println("ESP8266 isn't working..");
    }
  //}
}


