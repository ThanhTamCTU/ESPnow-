#include <esp_now.h>
#include <WiFi.h>
#define uS_TO_S_FACTOR 1000000ULL 
#define TIME_TO_SLEEP  10 

uint8_t broadcastAddress[] = {0x30,0xAE,0xA4,0x97,0xD3,0xD4}; // địa chỉ mac của ESP cần gửi
uint8_t datarcv; // biến nhận dữ liệu
int i=0;
char t[6];//  biến nhận dữ liệu
String tring;
unsigned long time_old;// biến điếm thời gian
char ID[]="esp1";// gói tin gửi đến master khi có yêu cầu start từ master


void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status); // chương trình con gửi dữ liệu qua espnow
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len); // chương trình ngắt khi nhận dữ liệu qua espnow
void print_wakeup_reason();

void setup() {
   Serial.begin(115200);
   WiFi.mode(WIFI_STA);
   if (esp_now_init() != ESP_OK) {// khởi động ESPnow, khởi động sau khi khởi động wifi
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_register_send_cb(OnDataSent);// đăng kí chức năng gửi thông báo kết quả gửi dữ liệu
  esp_now_peer_info_t peerInfo;

  // khối lệnh đăng ký thông tin peer ( các espnow thành phần được gọi là peer)
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;

 if (esp_now_add_peer(&peerInfo) != ESP_OK){ // gọi hàm này kết nối các esp qua địa chỉ mac
    Serial.println("Failed to add peer");
    return;
  }
  esp_now_register_recv_cb(OnDataRecv);// đăng ký hàm gọi lại khi có nhận dữ liệu
}// end setup

void loop() {
  
}// end loop
//=======================================================================
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}// end OnDataSent()

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {

 memcpy(t,incomingData, sizeof(t));
 // in ra số byte nhận được
  Serial.print("Bytes received: ");
  Serial.println(len);
 // in ra dữ liệu nhận được 
  Serial.print("data: ");
  Serial.println(t);
  datarcv=*incomingData; // lưu dữ liệu nhận được vào biến datarcv
  Serial.println(datarcv);

  if(datarcv==111){ // 111 là giá trị start bên master gửi qua, so sánh nếu gói tin nhận được == 111 thì bắt đầu gửi dữ liệu 
    esp_err_t result = esp_now_send((uint8_t*)broadcastAddress, (uint8_t *) ID, sizeof(ID)); // lệnh gửi dữ liệu đến master
   if (result == ESP_OK) {
     Serial.println("Sent with success");
   }
   else {
    Serial.println("Error sending the data");
  }
    datarcv=0;
  }// end if(datarcv==111)
  
  if(strcmp(t,"sleep")==0){ // so sánh có phải master yêu cầu ngủ
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
 
