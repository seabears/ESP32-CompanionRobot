#include <DHT.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

#include <Adafruit_GFX.h>
#include <Adafruit_IS31FL3731.h>
Adafruit_IS31FL3731 matrix = Adafruit_IS31FL3731();
static const uint8_t PROGMEM
normal_bmp[] = {
  B00000000,
  B00000000,
  B11100111,
  B00000000,
  B00100100,
  B00100100,
  B00000000,
  B00000000
};
static const uint8_t PROGMEM
victory_bmp[] = {
  B00000000,
  B00000000,
  B11100111,
  B00000000,
  B01000010,
  B10100101,
  B00000000,
  B00011000
};
static const uint8_t PROGMEM
bad_bmp[] = {
  B00000000,
  B00000000,
  B10000001,
  B01111110,
  B00100100,
  B00100100,
  B00000000,
  B00000000
};
static const uint8_t PROGMEM
defeat1_bmp[] = {
  B00000000,
  B00000000,
  B01100110,
  B10000001,
  B00100100,
  B00100100,
  B00000000,
  B00100100,
  B00000000
};
static const uint8_t PROGMEM
defeat2_bmp[] = {
  B00000000,
  B00000000,
  B01100110,
  B10000001,
  B00100100,
  B00100100,
  B00000000,
  B00000000,
  B00100100
};
static const uint8_t PROGMEM
defeat3_bmp[] = {
  B00000000,
  B00000000,
  B01100110,
  B10000001,
  B00100100,
  B00100100,
  B00000000,
  B00000000,
  B00000000
};
static const uint8_t PROGMEM
sleep_bmp[] = {
  B00000000,
  B00000000,
  B01111000,
  B00010000,
  B00100000,
  B01111000,
  B00000000,
  B00000000
};


const char tryagain[8] = {'T', 'R', 'Y', 'A', 'G', 'A', 'I', 'N'};  //TRYAGAIN
const char draw[4] = {'D', 'R', 'A', 'W'};  //DRAW

int last_control = 0;
unsigned long last_time = 0;
//dht sensor
#define DHTPIN 16
#define DHTTYPE DHT11

//buzzer
#define buzzer_gpio 2
#define buzzer_dac 25

//I/O
//Button
#define but 17
bool isServerOn = true;
bool lastButtonState = HIGH; 

//register  //potential meter
#define reg_gpio 34 //A2
int reg = 0;
int lastregState = 0;

//light
#define light_s 35  //A3

//BLE
#define SERVICE_UUID "89e15284-9989-4052-bfd2-f0384adc92d4"
#define CHARACTERISTIC_UUID "e640560d-9c92-4b69-a704-c0327b42b2ca"

//pwm
int freq = 2000;
int channel = 0;
int resolution = 8;

//bt object
BLEServer* pServer;
BLEService* pService;
//BLECharacteristic* pCharacteristic;
BLECharacteristic* gCharacteristic;
BLEAdvertising* pAdvertising;

DHT dht(DHTPIN, DHTTYPE); //dht object

class MyCallbacks: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* pCharacteristic) {
    int temp = random(900) % 3; // 0 : scissors   1 : rock   2 : paper
    std::string value = pCharacteristic->getValue();
    //Serial.println("callback");//
    if (value.length() > 0) {
      //Serial.println(last_control);
      last_control = 0;
      //Serial.println(last_control);

      if(value == "rock" || value == "paper" || value == "scissors"){
        if(who_win(value, temp)){
          Serial.println("draw");//
          matrix.clear();
          matrix.setTextSize(1);
          matrix.setTextColor(100);
          for(int i = 0; i < 4; i++){
            matrix.clear();
            matrix.setCursor(1,1);
            matrix.printf("%c", draw[i]);
            delay(400);  
          }
        matrix.clear();
        }
      }
      else{
        Serial.println("tryagain");//

        matrix.clear();
        matrix.setTextSize(1);
        matrix.setTextColor(100);
        for(int i = 0; i < 8; i++){
          matrix.clear();
          matrix.setCursor(1,1);
          matrix.printf("%c", tryagain[i]);
          delay(400);       
        }
        matrix.clear();
      }
    }
  }
  private :
    int who_win(const std::string& value, int temp){
      int human; //가위 바위 보
      int len = static_cast<int>(value.length());
      switch(len){
        case 4: //바위
          human = 1;
          break;
        case 5://보
          human = 2;
          break;
        case 8://가위
          human =0;
          break;
        }

      //Serial.println(reg);
      if(human == temp){  //비김
        return 1;
      }
      else if(human == (temp+1)%3){  //인간이 이김  가위 -> 바위 -> 보
       Serial.println("Human win");
       buzzer(4);
       return 0;
      }
    else{ //기계가 이김
      Serial.println("bot win");
      buzzer(2);
      return 0;
   }
  }
  private : 
  void buzzer(int mod){        //buzzer 함수
    digitalWrite(buzzer_gpio, HIGH);  //buzzer on
switch(mod){
  case 2:       //bot win  
    //win expression
    matrix.clear();
    matrix.drawBitmap(0, 0, victory_bmp, 8, 8, 40);

    //win buzzer    
    reg = analogRead(reg_gpio);
    ledcWriteTone(channel, reg+100);
    //Serial.println("bot win : ");
    //Serial.println(reg);
    delay(200);
    reg = analogRead(reg_gpio);
    ledcWriteTone(channel, reg);
    //Serial.println(reg);
    delay(100);
    reg = analogRead(reg_gpio);
    ledcWriteTone(channel, reg+100);
    //Serial.println(reg);
    delay(500);
  break;
  case 4:       //bot defeat(human win)
    matrix.clear();
    matrix.drawBitmap(0, 0, defeat1_bmp, 8, 9, 40);
    reg = analogRead(reg_gpio);
    ledcWriteTone(channel, reg);
    //Serial.println("bot defeat : ");
    //Serial.println(reg);
    delay(150);

    matrix.clear();
    matrix.drawBitmap(0, 0, defeat2_bmp, 8, 9, 40);
    reg = analogRead(reg_gpio);
    //Serial.println(reg);
    ledcWriteTone(channel, reg-100);
    delay(150);
    reg = analogRead(reg_gpio);
    //Serial.println(reg);
    ledcWriteTone(channel, reg-200);
    delay(150);

    matrix.clear();
    matrix.drawBitmap(0, 0, defeat3_bmp, 8, 9, 40);
    reg = analogRead(reg_gpio);
    //Serial.println(reg);
    ledcWriteTone(channel, reg-300);
    delay(500);
  break;
}
digitalWrite(buzzer_gpio, LOW);   //buzzer off
}
};


void setup() {
  randomSeed(analogRead(0));
  Serial.begin(115200);   // 시리얼 통신 시작
  dht.begin();            //dht 시작
  matrix.begin();         //matrix start

  //I/O module
  pinMode(but, INPUT);
  pinMode(reg_gpio, INPUT);
  pinMode(light_s, INPUT);

  //부저 ON
  ledcSetup(channel, freq, resolution);
  ledcAttachPin(buzzer_dac, channel);
  pinMode(buzzer_gpio, OUTPUT);
  pinMode(buzzer_dac, OUTPUT);  

  //블루투스 ON
  startBLEServer();
}

void loop() {

  int buttonState = digitalRead(but);    //read button I/O
  //push button
  if(buttonState == LOW && lastButtonState == HIGH){  //low active  //only one work when pushing button
    if(isServerOn){   //first : in start, already Server on
      stopBLEServer();
      Serial.println("if");//
      matrix_fun(5);
    }
    else{
      Serial.println("else");//
      //startBLEServer();
      isServerOn = true;
      last_control = 0;   //init working time
      matrix_fun(1);
    }
  }
  lastButtonState = buttonState;

  int regState = analogRead(reg_gpio);  
  if(regState > lastregState + 80 || regState < lastregState - 80){   //range +-80
    last_control = 0;
    /*
    Serial.print(regState);
    Serial.print(" , ");
    Serial.println(lastregState);
    Serial.println("change");
    */
  }
  lastregState = regState;

  

  /*
  Serial.print("linght sensor : ");
  Serial.println(analogRead(light_s));     //light sensor value check
  //light : 900~    dark : 2000~3000
  */

  if(isServerOn){ //켜져있을 때만 동작
    int light_level = analogRead(light_s);  //read light sensor
    last_control = last_control + millis() - last_time;

    if(last_control > 30000 || light_level > 2000){ //sleep : working time > 3minutes or light : dark level
      matrix_fun(5);    //sleep matrix
      
      if(last_control > 30000)  stopBLEServer();  //bt end
    }

    else if(read_dht()){  //이게 1이면 안좋은 상태
      matrix_fun(3);    //bad matrix
    }

    else{ //ledmatrix는 defalut로 평상상태를 유지하도록 해야 함.
      matrix_fun(1);    //normal matrix
    }

    last_time = millis();   //check current working time
  }
  delay(3000);
}

//normal, victory, bad, defeat, sleep expression
void matrix_fun(int num){
  matrix.setRotation(0);
  matrix.clear();

  switch(num){
    case 1:
      matrix.drawBitmap(0, 0, normal_bmp, 8, 8, 40);
      break;
    case 2:
      matrix.drawBitmap(0, 0, victory_bmp, 8, 8, 40);
      break;
    case 3:
      matrix.drawBitmap(0, 0, bad_bmp, 8, 8, 40);
      break;
    case 4:
      for(int i = 0; i < 3; i++){
        matrix.clear();
        if(i == 0) matrix.drawBitmap(0, 0, defeat1_bmp, 8, 9, 40);
        else if(i == 1) matrix.drawBitmap(0, 0, defeat2_bmp, 8, 9, 40);
        else if(i == 2) matrix.drawBitmap(0, 0, defeat3_bmp, 8, 9, 40);
        delay(500);
      }
      break;
    case 5:
      for(int sleep_cnt = 0; sleep_cnt < 7; sleep_cnt++){
        matrix.clear();
        matrix.drawBitmap(sleep_cnt, 0, sleep_bmp, 8, 8, 40);
        delay(500);
      }
      break;
  }
  delay(500);
}



void startBLEServer(){
  isServerOn = true;

  BLEDevice::init("MyESP32_6_2");
  pServer = BLEDevice::createServer();
  pService = pServer->createService(SERVICE_UUID);
  gCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID,BLECharacteristic::PROPERTY_READ |BLECharacteristic::PROPERTY_WRITE);
  gCharacteristic->setCallbacks(new MyCallbacks());  

  Serial.println("start1");
  gCharacteristic->setValue("rock scissors paper");
  pService->start();
  pAdvertising = pServer->getAdvertising();
  pAdvertising->start();

  Serial.println("start2");
}

void stopBLEServer(){
  isServerOn = false;

  Serial.println("BLE Server stopped");
  //pServer->removeService(pService);
  //BLEDevice::deinit(false); // BLE 종료
  Serial.println("stop right?");//
  
}




int read_dht(){
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  // 데이터 출력
  /*
  Serial.print("습도: ");
  Serial.print(humidity);
  Serial.print("%\t");
  Serial.print("온도: ");
  Serial.print(temperature);
  Serial.println("°C");
  */

  if(humidity > 80 || temperature > 30)  //안좋으면 1
    return 1;
  else
    return 0;
}

