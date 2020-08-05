
#include <esp_now.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <EEPROM.h>

#define uS_TO_S_FACTOR 1000000ULL 
#define TIME_TO_SLEEP  11

#define adrr_diemdanh     0
#define adrr_request      1
#define adrr_number_sent  2
#define adrr_slave_mac_0  3
#define adrr_slave_mac_1  4
#define adrr_slave_mac_2  5
#define adrr_slave_mac_3  6
#define adrr_slave_mac_4  7
#define adrr_slave_mac_5  8

// Khai báo địa chỉ Mac của các ESP
char broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; // gửi địa chỉ đến tất cacr xá 
char slave1[]={0x30,0xAE,0xA4,0x99,0x74,0x98};// địa chỉ mac của slave 1
char slave2[]={0x80,0x7D,0x3A,0x89,0x41,0x1C};// địa chỉ mac của slave 2
char slave3[]={0x30,0xAE,0xA4,0x97,0x4C,0x08};// địa chỉ mac của slave 3
char slave4[]={0x80,0x7D,0x3A,0x81,0x6B,0x10};// địa chỉ mac của slave 4

uint8_t start[]={111},Stop[]={123}, checking[]={127}, request_data={222};// đây là các lệnh yêu cầu dữ liệu 
char slaveSleep[]="sleep";

char slaveC[6][4]; // mảng 2 chiều để lưu địa chỉ mac của slave gửi dữ liệu tới 
char slave_mac_adrr[6];
char macStr[18];
char slave_data[150];
uint8_t count[4]={0,0,0,0};
int ngu=0;
unsigned long time_oldM=0;
uint8_t countSlave,ran;

bool switch_DiemDanh,switch_request;
uint8_t number_sent;


//--------------------------- khai báo các chương trình con--------------------------------------------- 
void print_wakeup_reason();
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) ;
void OnDataRecv(const uint8_t * mac_addr, const uint8_t *incomingData, int len);
void write_nvs(bool diemdanh);
bool read_nvs();


void setup() {
           EEPROM.begin(10);
           Serial.begin(115200);
           WiFi.mode(WIFI_STA);
           if (esp_now_init() != ESP_OK) {
                Serial.println("Error initializing ESP-NOW");
                return;
              }// end if


           switch_DiemDanh    = EEPROM.read(adrr_diemdanh);
           switch_request     = EEPROM.read(adrr_request);
           number_sent        = EEPROM.read(adrr_number_sent);
           slave_mac_adrr[0]  = EEPROM.read(adrr_slave_mac_0);
           slave_mac_adrr[1]  = EEPROM.read(adrr_slave_mac_1);
           slave_mac_adrr[2]  = EEPROM.read(adrr_slave_mac_2); 
           slave_mac_adrr[3]  = EEPROM.read(adrr_slave_mac_3);
           slave_mac_adrr[4]  = EEPROM.read(adrr_slave_mac_4);
           slave_mac_adrr[5]  = EEPROM.read(adrr_slave_mac_5);
           
           esp_now_register_send_cb(OnDataSent);
           esp_now_peer_info_t peerInfo;
           memcpy(peerInfo.peer_addr, broadcastAddress, 6);
           peerInfo.channel = 0;  
           peerInfo.encrypt = false;

           if (esp_now_add_peer(&peerInfo) != ESP_OK){
                Serial.println("Failed to add peer");
                 return;
               }// end if 


//---------------------- Kiểm tra số lần gửi dữ liệu ----------------------------------------------

            if( number_sent >50){
                switch_DiemDanh = 0;
                switch_request = 0;
                number_sent = 0;
            }

 //----------------------Đoạn code Master gửi lệnh điểm danh đến các slave-------------------------
 //code này chỉ chạy khi switch_DiemDanh =1 
           if(switch_DiemDanh==0){
                 esp_err_t result = esp_now_send((uint8_t*)broadcastAddress,(uint8_t*) start, sizeof(start));  
                 if (result == ESP_OK) { 
                    Serial.println("Sent start");
                  } // end if 
                 else {
                    Serial.println("Error sending start");
                  }// end if       
             }else{
                esp_err_t result_request = esp_now_send((uint8_t*)broadcastAddress,(uint8_t*) request_data, sizeof(start));  
                 if (result_request == ESP_OK) { 
                    Serial.println("Sent request data");
                  } // end if 
                 else {
                    Serial.println("Error sending request data");
                  }// end if 
             }// end else-if

           time_oldM=millis();
           esp_now_register_recv_cb(OnDataRecv);
           while(!((millis()-time_oldM)>=6000));// chờ 6s sau đó Master gửi lệnh yêu cầu các slave sleep

//-----------Đoạn code Master gửi yêu cầu sleep đến các slave -----------------------------------------
           esp_err_t rs = esp_now_send((uint8_t*)broadcastAddress, (uint8_t *) slaveSleep, sizeof(slaveSleep));
           if( rs== ESP_OK){
                Serial.println(" Đã gửi sleep");esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
           }else{
                Serial.println(" chua gui tin keu ngu");
            }

            
          print_wakeup_reason();
          esp_deep_sleep_start();

}

void loop() {
  

}


//-----------------------------Đây là phần code chương trình con ------------------------------------------------
//---------------------------- chương trình con Sleep-------------------------
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


//------------------------------Đây là phần code OnDataRecv()------------------------------------------

void OnDataRecv(const uint8_t * mac_addr, const uint8_t *incomingData, int len) {
   
 if( switch_DiemDanh ==0){
   countSlave++; // đếm số lượng slave 
   Serial.println("Number of slave is "+(String)(countSlave)); // in ra số lượng slave gửi tin nhắn tới 
   memcpy(slave_data, incomingData, sizeof(slave_data));
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
  Serial.println(slave_data);
  Serial.println();
  ran=random(countSlave); // hàm random để chọn slave ngẫu nhiên để
  
  slave_mac_adrr[0]=slaveC[ran][0];
  slave_mac_adrr[1]=slaveC[ran][1];
  slave_mac_adrr[2]=slaveC[ran][2];
  slave_mac_adrr[3]=slaveC[ran][3];
  slave_mac_adrr[4]=slaveC[ran][4];
  slave_mac_adrr[5]=slaveC[ran][5];

  EEPROM.write(adrr_slave_mac_0, slave_mac_adrr[0]);
  EEPROM.write(adrr_slave_mac_1, slave_mac_adrr[1]);
  EEPROM.write(adrr_slave_mac_2, slave_mac_adrr[2]);
  EEPROM.write(adrr_slave_mac_3, slave_mac_adrr[3]);
  EEPROM.write(adrr_slave_mac_4, slave_mac_adrr[4]);
  EEPROM.write(adrr_slave_mac_5, slave_mac_adrr[5]);
  EEPROM.commit();
  
 }else{// end if ( switch_DiemDanh==0)
  switch_request=1;
 }// end else-if( switch_DiemDanh==0)


 if( switch_request==0){
   // Đoạn code gửi yêu cầu gửi dữ liệu đến slave được chọn 
      esp_err_t result_request_data = esp_now_send((uint8_t*)slave_mac_adrr, (uint8_t *) request_data, sizeof(request_data));
      if (result_request_data == ESP_OK) {
          Serial.println("Sent result_request_data with success");
        }// end if
      else {
          Serial.println("Error sending the result_request_data");
      }// end else-if

       switch_DiemDanh=1;
       EEPROM.write(switch_DiemDanh,switch_DiemDanh);
       EEPROM.commit();
 }// end if( switch_request ==0)
 else{
  number_sent++;
  EEPROM.write(adrr_number_sent,number_sent);
  EEPROM.commit();
  memcpy(slave_data, incomingData, sizeof(slave_data));
  Serial.println("Lenght data: "+ (String)(len)); 
  Serial.println("Data receive: " + (String)(slave_data));
 }// end else-if( switch_request ==0)   
}// end ondatarev 

//------------------------------Đây là phần code OnDataSent()-----------------------------------------
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
 }// end onsent

void write_EEPROM(bool variable){
   EEPROM.write(0,variable);
   EEPROM.commit();
}
