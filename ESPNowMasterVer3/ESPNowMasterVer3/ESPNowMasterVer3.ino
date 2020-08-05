#include <esp_now.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#define uS_TO_S_FACTOR 1000000ULL 
#define TIME_TO_SLEEP  11 // thời gian master ngủ 11s

char broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; // địa chỉ mac gửi cho tất cả các peer
char slave1[]={0x30,0xAE,0xA4,0x99,0x74,0x98};
char slave2[]={0x80,0x7D,0x3A,0x89,0x41,0x1C};
char slave3[]={0x30,0xAE,0xA4,0x97,0x4C,0x08};
char slave4[]={0x80,0x7D,0x3A,0x81,0x6B,0x10};
char slaveC[4][6];// mảng lưu địa chỉ mac, 4 hàng do lúc làm thí nghiệm có 4 peer 

uint8_t start[]={111}; 
char macStr[18];
char slaveSleep[]="sleep";// mảng dữ liệu gửi đến các peer yêu cầu ngủ
unsigned long time_oldM=0;
char test[150]; // mảng lưu giá trị nhận được 
uint8_t countSlave;


void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) ;// chương trình con callback kết quả gửi dữ liệu qua espnow
void OnDataRecv(const uint8_t * mac_addr, const uint8_t *incomingData, int len);// chương trình ngắt khi nhận dữ liệu qua espnow
void print_wakeup_reason();

void setup() {
   Serial.begin(115200);
   WiFi.mode(WIFI_STA);
   if (esp_now_init() != ESP_OK) {// khởi động ESPnow, khởi động sau khi khởi động wifi
    Serial.println("Error initializing ESP-NOW");
    return;
    }
  esp_now_register_send_cb(OnDataSent);// đăng kí chức năng gửi thông báo kết quả gửi dữ liệu
  // khối lệnh đăng ký thông tin peer ( các espnow thành phần được gọi là peer)
  esp_now_peer_info_t peerInfo;
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;

   if (esp_now_add_peer(&peerInfo) != ESP_OK){// gọi hàm này kết nối các esp qua địa chỉ mac
    Serial.println("Failed to add peer");
    return;
  }// end if 

  
//--------------Start-----------------------------
  esp_err_t result = esp_now_send((uint8_t*)broadcastAddress,(uint8_t*) start, sizeof(start)); // gửi yêu cầu start đến các peer
  if (result == ESP_OK) { Serial.println("Sent start");
  } else {Serial.println("Error sending start");
  }
   time_oldM=millis();
   esp_now_register_recv_cb(OnDataRecv);// đăng ký hàm callback khi có dữ liệu nhận 
   while(!((millis()-time_oldM)>=6000));// chờ 6s sau khi gửi tín hiệu start để nhận phản hồi từ các peer, sau đó gửi yêu cầu các peer sleep và sleep
   esp_err_t rs = esp_now_send((uint8_t*)broadcastAddress, (uint8_t *) slaveSleep, sizeof(slaveSleep));// gửi yêu cầu sleep
   if( rs== ESP_OK){
    Serial.println(" Đã gửi sleep");esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
   }else{
    Serial.println(" chua gui tin keu ngu");
   }
    print_wakeup_reason();
    esp_deep_sleep_start();// sleep
  
  
}// end setup

void loop() {
  

}// end loop
//=======================================================================
void print_wakeup_reason(){
  esp_sleep_wakeup_cause_t wakeup_reason;
  wakeup_reason = esp_sleep_get_wakeup_cause();
  switch(wakeup_reason){
    case ESP_SLEEP_WAKEUP_EXT0 :     Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1 :     Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER :    Serial.println("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP : Serial.println("Wakeup caused by ULP program"); break;
    default : Serial.printf("Wakeup was not caused by deep sleep: %d\n",wakeup_reason); break;
  }// end switch
}// print_wakeup_reason()
//-----------------------------------------------------------------------
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}// end onsent
//------------------------------------------------------------------------

void OnDataRecv(const uint8_t * mac_addr, const uint8_t *incomingData, int len) {
  
   countSlave++;// đến các peer đã gửi tín hiệu 
   Serial.println("Number of slave is "+(String)(countSlave));
   memcpy(test, incomingData, sizeof(test));
   // lưu địa chỉ mac của peer vừa nhận dữ liệu 
   for( int i=0;i<6;i++){
    slaveC[i][countSlave-1]=mac_addr[i];
   }// end for

   // in địa chỉ mac của peer 
   snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",slaveC[0][countSlave-1],slaveC[1][countSlave-1],
           slaveC[2][countSlave-1],slaveC[3][countSlave-1],slaveC[4][countSlave-1],slaveC[5][countSlave-1]);//mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]
  Serial.print("Last Packet Recv from: "); Serial.println(macStr); 
  Serial.print("Bytes received: ");
  Serial.println(len);
  Serial.print("Char: ");
  Serial.println(test);
  Serial.println();
}// end ondatarev 
