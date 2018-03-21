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
  WorkMode_ON,WorkMode_OFF,WorkMode_HEAT_DETECT
};
enum SenserType{
  SenserType_DIGITAL,SenserType_ANALOG,SenserType_UNUSE
};

//******* data variable ***************
WorkMode workMode;
SenserType type_modeUsingSeneser;
int pin_modeUsingSeneser;
int levelOutput_whenDigitalHigh;
int thresholdValue_analogPin;
int levelOutput_whenBiggerThanthreshold;


//************ function prototype ***********
void wifiRestart();
bool checkConnect();
bool handleInput();
void handleModeUpdate();
bool isSenserMode();
bool setOutputRelay(int);

//************ pin and state ***********
const int PIN_OUTPUT_RELAY=13;//插座输出继电器控制端 //when debug,set to 13
int ele_level_outputRelay;

const int PIN_SENSER_USB_ANALOG_YELLOW=22;


//*********** command ************
const String CMD_SET_MODE="MODE ";

//********* mode ***********
const String MODE_HEAT_DETECT="heat";
const String MODE_ON="on";
const String MODE_OFF="off";



void setup() {
  //******** pin config init **********
  pinMode(PIN_OUTPUT_RELAY,OUTPUT);
  pinMode(PIN_SENSER_USB_ANALOG_YELLOW,INPUT);

  //********** data init ***********
  workMode=WorkMode_OFF;
  ele_level_outputRelay=LOW;
  digitalWrite(PIN_OUTPUT_RELAY,ele_level_outputRelay);
  
  type_modeUsingSeneser=SenserType_UNUSE;

  //unnecessary
  pin_modeUsingSeneser=-1;
  levelOutput_whenDigitalHigh=-1;
  thresholdValue_analogPin=-1;
  levelOutput_whenBiggerThanthreshold=-1;
 
  
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

void wifiRestart(){
  wifi.setTransportToTCP();       // TCP模式（默认设置）
  wifi.endSendWithNewline(false); // 发送数据自动以\r\n结尾（默认设置）
  
  wifi_started = wifi.begin();    // WIFI启动
  if (wifi_started) { // 如果WIFI初始化成功
    //wifi.connectToAP(MY_SSID, PASSWORD); // 指定用户名密码连接WIFI
    //wifi.startLocalServer(SERVER_PORT);
    wifi.startLocalAPAndServer("MY_CONFIG_AP", "password", "5", "2121");
    Serial.println("ESP8266 is working!");
    Serial.println(wifi.getIP());
  } else {
    // ESP8266 isn't working..
    Serial.println("ESP8266 isn't working..");
  }
}

void loop() {
    
    if(checkConnect()){   
      bool isModeChanged=handleInput();
      if(isModeChanged){
        handleModeUpdate();
      }
    }

    if(isSenserMode()){
      if(type_modeUsingSeneser==SenserType_ANALOG){
        int input=analogRead(pin_modeUsingSeneser);
        if(input>thresholdValue_analogPin xor levelOutput_whenBiggerThanthreshold==LOW)
          setOutputRelay(HIGH);
        else
          setOutputRelay(LOW);
      }
      
    }
    
}

bool setOutputRelay(int level){
  if(ele_level_outputRelay==level){
    Serial.println("Relay keep "+ele_level_outputRelay);
    return false;//not change
  }
   else{
    //change output
    ele_level_outputRelay=level;
    digitalWrite(PIN_OUTPUT_RELAY,ele_level_outputRelay);
    Serial.println("Relay change to "+ele_level_outputRelay);
    return true;
   }
}

bool isSenserMode(){
  if(workMode==WorkMode_HEAT_DETECT)
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
  WifiMessage in = wifi.listenForIncomingMessage(6000);
  if (in.hasData) {
    String inMassage=in.message;
    String echoText="";
    if(inMassage.startsWith(CMD_SET_MODE)){
      echoText+="command=set_mode ";
      String argument=inMassage.substring(CMD_SET_MODE.length());
      if(argument.equalsIgnoreCase(MODE_HEAT_DETECT)){
        echoText+="mode=heat_detect ";
        workMode=WorkMode_HEAT_DETECT;
        type_modeUsingSeneser=SenserType_ANALOG;

        
      }
      else if(argument.equalsIgnoreCase(MODE_ON)){
        echoText+="mode=always_on ";
        workMode=WorkMode_ON;
      }
      else if(argument.equalsIgnoreCase(MODE_OFF)){
        echoText+="mode=always_off ";
        workMode=WorkMode_OFF;
      }
      else{
        echoText+="mode erro ";
        isModeChanged=false;
      }
    }
    else{
        echoText+="command erro ";
        isModeChanged=false;
    }
    
    //Echo back;
    wifi.send(in.channel, "Echo:",false);
    wifi.send(in.channel, echoText);
    //nextPing = millis() + 10000;

    return isModeChanged;
  }
  
}

void handleModeUpdate(){

  if(workMode==WorkMode_HEAT_DETECT){
    type_modeUsingSeneser=SenserType_ANALOG;
    pin_modeUsingSeneser=PIN_SENSER_USB_ANALOG_YELLOW;
    levelOutput_whenDigitalHigh=-1;
    thresholdValue_analogPin=150;
    levelOutput_whenBiggerThanthreshold=HIGH; //bigger<->have people<->turn on electry
  }
  else if(workMode==WorkMode_ON){
    setOutputRelay(HIGH);
    
    type_modeUsingSeneser=SenserType_UNUSE;
    pin_modeUsingSeneser=-1;
    levelOutput_whenDigitalHigh=-1;
    thresholdValue_analogPin=-1;
    levelOutput_whenBiggerThanthreshold=-1;
  }
  else if(workMode==WorkMode_OFF){
    setOutputRelay(LOW);
    
    type_modeUsingSeneser=SenserType_UNUSE;
    pin_modeUsingSeneser=-1;
    levelOutput_whenDigitalHigh=-1;
    thresholdValue_analogPin=-1;
    levelOutput_whenBiggerThanthreshold=-1;
  }
}


