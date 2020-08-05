#include <esp_now.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <EEPROM.h>



#define uS_TO_S_FACTOR 1000000ULL 
#define TIME_TO_SLEEP  11
// Khai báo địa chỉ Mac của các ESP
char broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; // gửi địa chỉ đến tất cacr xá 
char slave1[]={0x30,0xAE,0xA4,0x99,0x74,0x98};// địa chỉ mac của slave 1
char slave2[]={0x80,0x7D,0x3A,0x89,0x41,0x1C};// địa chỉ mac của slave 2
char slave3[]={0x30,0xAE,0xA4,0x97,0x4C,0x08};// địa chỉ mac của slave 3
char slave4[]={0x80,0x7D,0x3A,0x81,0x6B,0x10};// địa chỉ mac của slave 4

char slaveC[6][4]; // mảng 2 chiều để lưu địa chỉ mac của slave gửi dữ liệu tới 

uint8_t start[]={111},Stop[]={123}, checking[]={127}, request_data={222};// đây là các lệnh yêu cầu dữ liệu 

char macStr[18];
uint8_t count[4]={0,0,0,0};
char slaveSleep[]="sleep";
int ngu=0;
unsigned long time_oldM=0;
typedef struct struct_message {
  char a[32];
  int b;
  float c;
  String d;
  bool e;
} struct_message;

struct_message myData;
char test[150];
uint8_t countSlave;
uint8_t ran;

bool switch_DiemDanh = 1; // biến cho phép Master gửi dữ liệu
bool switch_request=1;
uint8_t number_of_sent ;
//--------------------------- khai báo các chương trình con--------------------------------------------- 
void print_wakeup_reason();
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) ;
void OnDataRecv(const uint8_t * mac_addr, const uint8_t *incomingData, int len);
void write_nvs(bool diemdanh);
bool read_nvs();






//----------------------------Đây là phần code Setup()-----------------------------------------------------
void setup() {
    EEPROM.begin(10);
    Serial.begin(115200);
    pinMode(2,OUTPUT);
    WiFi.mode(WIFI_STA);
    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
         return;
    }// end if
    esp_now_register_send_cb(OnDataSent);
    esp_now_peer_info_t peerInfo;
    memcpy(peerInfo.peer_addr, broadcastAddress, 6);
    peerInfo.channel = 0;  
    peerInfo.encrypt = false;

   if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
   }// end if 

   //----------------------Đoạn code Master gửi lệnh điểm danh đến các slave-------------------------
  
       
 
  
  }// end setup


//-----------------------------Đây là chương trình vòng loop ----------------------------------------------------
void loop() {
   digitalWrite(2,HIGH);
   esp_err_t result = esp_now_send((uint8_t*)broadcastAddress,(uint8_t*) start, sizeof(start));  
   digitalWrite(2,LOW);
        if (result == ESP_OK) { 
              Serial.println("Sent start");
             } // end if 
        else {
             Serial.println("Error sending start");
             }// end if
  delay(2000);           
}// end loop

//-----------------------------Đây là phần code chương trình con ------------------------------------------------

void print_wakeup_reason(){
  esp_sleep_wakeup_cause_t wakeup_reason;
  wakeup_reason = esp_sleep_get_wakeup_cause();
  switch(wakeup_reason){
    case ESP_SLEEP_WAKEUP_EXT0 :     Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1 :     Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER :    Serial.println("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP :      Serial.println("Wakeup caused by ULP program"); break;
    default :                        Serial.printf("Wakeup was not caused by deep sleep: %d\n",wakeup_reason); break;
  }// end switch
}// print_wakeup_reason()


//------------------------------Đây là phần code OnDataSent()-----------------------------------------
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
 }// end onsent

//------------------------------Đây là phần code OnDataRecv()------------------------------------------

void OnDataRecv(const uint8_t * mac_addr, const uint8_t *incomingData, int len) {
   countSlave++; // đếm số lượng slave 
   Serial.println("Number of slave is "+(String)(countSlave)); // in ra số lượng slave gửi tin nhắn tới 
   memcpy(test, incomingData, sizeof(test));
   //Đoạn code dùng để sao chép và in ra màn hình địa chỉ slave gửi dữ liệu tới 
   for( int i=0;i<6;i++){
    slaveC[i][countSlave-1]=mac_addr[i];
   }// end for

  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",slaveC[0][countSlave-1],slaveC[1][countSlave-1],
           slaveC[2][countSlave-1],slaveC[3][countSlave-1],slaveC[4][countSlave-1],slaveC[5][countSlave-1]);//mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]
  Serial.print("Last Packet Recv from: "); Serial.println(macStr); 

  //Đoạn code dùng để in dữ liệu nhận được ra màn hình
  Serial.print("Bytes received: ");
  Serial.println(len);
  Serial.print("Char: ");
  Serial.println(test);
  Serial.println();

  ran=random(countSlave); // hàm random để chọn slave ngẫu nhiên để
  // Đoạn code gửi yêu cầu gửi dữ liệu đến slave được chọn 
    esp_err_t result_request_data = esp_now_send((uint8_t*)slaveC[6][ran], (uint8_t *) request_data, sizeof(request_data));
  if (result_request_data == ESP_OK) {
    Serial.println("Sent result_request_data with success");
  }// end if
  else {
    Serial.println("Error sending the result_request_data");
  }// end else-if
  
  switch_DiemDanh=0;
}// end ondatarev 

void write_EEPROM(bool diemdanh){
   EEPROM.write(0,diemdanh);
   EEPROM.commit();
}
