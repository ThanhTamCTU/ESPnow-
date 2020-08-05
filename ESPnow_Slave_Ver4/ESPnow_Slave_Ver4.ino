
#include <esp_now.h>
#include <WiFi.h>
#define uS_TO_S_FACTOR 1000000ULL 
#define TIME_TO_SLEEP  10 




uint8_t broadcastAddress[] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};

char    master[]={0x30,0xAE,0xA4,0x97,0xD3,0xD4};

char ID[]="esp1";
char test[]="999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999";//9999999999
char answer_diemdanh[]="ESP1", answer_request_data[]="toi gui du lieu";
char master_data[150];
char t[10];
uint8_t datarcv;
RTC_DATA_ATTR bool check ;

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len);
void print_wakeup_reason();



void setup() {
  Serial.begin(9600);
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_register_send_cb(OnDataSent);
  esp_now_peer_info_t peerInfo;
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
   }
   esp_now_register_recv_cb(OnDataRecv);

}// end void setup()

void loop() {
 
}// end void loop()

//------------------------------------------------------------------------------------
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "sent Success" : "sent Fail");
}// end OnDataSent()

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
//  memcpy(&myData, incomingData, sizeof(myData));
  memcpy(master_data,incomingData, sizeof(master_data));
  Serial.print("Bytes received: ");
  Serial.println(len);
  
//  tring=t.tostring;
  datarcv=*incomingData;
  Serial.println("Dữ liệu nhận: "+(String)datarcv);
//----------------------------------------------------------------------------------  
  if(datarcv==111){
//    check=true;
    Serial.println("data recieve:"+(String)datarcv);
    esp_err_t result = esp_now_send((uint8_t*)broadcastAddress, (uint8_t *) answer_diemdanh, sizeof(answer_diemdanh));
  if (result == ESP_OK) {
    Serial.println("Sent answer diemdanh with success");
  }
  else {
    Serial.println("Error sending answer diemdanh");
  }
//  }// end for
    datarcv=0;
  }// end if(datarcv==111)
//----------------------------------------------------------- 
 if(datarcv==222){
//    check=true;
    Serial.println("data receive:"+(String)datarcv);
    esp_err_t result = esp_now_send((uint8_t*)broadcastAddress, (uint8_t *) answer_request_data, sizeof(answer_request_data));
  if (result == ESP_OK) {
    Serial.println("Sent answer request data with success");
  }
  else {
    Serial.println("Error sending answer request data");
  }
//  }// end for
    datarcv=0;
  }// end if(datarcv==111)
//----------------------------------------------------------------------------------  
  if(strcmp(master_data,"sleep")==0){
    Serial.println("bắt đầu sleep");
    esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
    print_wakeup_reason();
    esp_deep_sleep_start();
  }  
  
}// end OnDataRecv

//---------------------------------------------
void print_wakeup_reason(){
  esp_sleep_wakeup_cause_t wakeup_reason;
  wakeup_reason = esp_sleep_get_wakeup_cause();
  switch(wakeup_reason){
    case ESP_SLEEP_WAKEUP_EXT0 : Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER : Serial.println("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP : Serial.println("Wakeup caused by ULP program"); break;
    default : Serial.printf("Wakeup was not caused by deep sleep: %d\n",wakeup_reason); break;
  }// end switch
}// print_wakeup_reason()
