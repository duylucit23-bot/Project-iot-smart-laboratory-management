#include <SPI.h>
#include <MFRC522.h>
#include <Keypad.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ESP32Servo.h>

// --- 1. CẤU HÌNH RFID RC522 ---
#define SS_PIN 5
MFRC522 mfrc522(SS_PIN, UINT8_MAX); 

// KHI KHAI BÁO MẢNG THẺ HỢP LỆ (Mày muốn thêm thẻ thì cứ phẩy rồi ghi thêm vào đây)
String danhSachThe[] = {"9E5D1305", "45D78512", "11223344"}; 
int soLuongThe = sizeof(danhSachThe) / sizeof(danhSachThe[0]); // Tự động đếm số lượng thẻ

// --- 2. CẤU HÌNH KEYPAD 4x4 --- 
const byte ROWS = 4; 
const byte COLS = 4; 
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {13, 14, 25, 26}; 
byte colPins[COLS] = {27, 16, 17, 4}; 
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// Cài đặt Mật khẩu mở cửa
String passDung = "1234"; 
String passNhap = "";     

// --- 3. CẤU HÌNH LCD & SERVO ---
LiquidCrystal_I2C lcd(0x27, 16, 2); 
Servo myservo;
#define SERVO_PIN 12

void setup() {
  Serial.begin(115200);
  
  SPI.begin();
  mfrc522.PCD_Init();
  
  lcd.init();
  lcd.backlight();
  
  ESP32PWM::allocateTimer(0);
  myservo.setPeriodHertz(50);
  myservo.attach(SERVO_PIN);
  myservo.write(0); 
  
  manHinhCho();
}

void loop() {
  // ---------------------------------------------------------
  // TÍNH NĂNG 1: KIỂM TRA QUÉT THẺ RFID
  // ---------------------------------------------------------
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    String cardID = "";
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      cardID += String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : "");
      cardID += String(mfrc522.uid.uidByte[i], HEX);
    }
    cardID.toUpperCase(); 
    
    Serial.println("Ma the vua quet: " + cardID); 
    
    // Thuật toán dò tìm thẻ trong mảng
    bool theXin = false; // Biến cờ hiệu, ban đầu cho là thẻ dỏm
    for (int j = 0; j < soLuongThe; j++) {
      if (cardID == danhSachThe[j]) {
        theXin = true; // Tìm thấy thẻ trong danh sách thì lật cờ
        break; // Dừng vòng lặp luôn cho đỡ tốn thời gian
      }
    }

    if (theXin == true) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("The Hop Le!");
      moCua();
    } else {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("The Khong Hop Le");
      delay(2000);
      manHinhCho();
    }
    mfrc522.PICC_HaltA(); 
  }

  // ---------------------------------------------------------
  // TÍNH NĂNG 2: KIỂM TRA BẤM PHÍM MẬT KHẨU
  // ---------------------------------------------------------
  char key = keypad.getKey();
  if (key) { 
    if (key == '#') { 
      lcd.clear();
      if (passNhap == passDung) {
        lcd.setCursor(0, 0);
        lcd.print("Pass Hop Le!");
        moCua();
      } else {
        lcd.setCursor(0, 0);
        lcd.print("Sai Pass!!!");
        lcd.setCursor(0, 1);
        lcd.print("Vui long thu lai");
        delay(2000); 
        manHinhCho();
      }
      passNhap = ""; 
      
    } else if (key == '*') { 
      passNhap = "";
      manHinhCho();
      
    } else { 
      if (passNhap.length() == 0) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Nhap Pass:");
      }
      passNhap += key;
      lcd.setCursor(0, 1);
      for (int i = 0; i < passNhap.length(); i++) {
        lcd.print("*"); 
      }
    }
  }
}

// ---------------------------------------------------------
// CÁC HÀM XỬ LÝ PHỤ
// ---------------------------------------------------------
void moCua() {
  lcd.setCursor(0, 1);
  lcd.print("Cua dang mo...");
  
  myservo.write(90); 
  delay(3000);       
  myservo.write(0);  
  
  manHinhCho();      
}

void manHinhCho() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("He thong An ninh");
  lcd.setCursor(0, 1);
  lcd.print("Quet the hoac Phim");
}
