#include <iarduino_RTC.h>
#include <Servo.h>
#include <Wire.h>
#include <Keypad.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>

// LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Keypad
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {2,3,4,5};
byte colPins[COLS] = {9,10,11,12};
Keypad kpd = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// RTC DS1302: подключение к пинам Mega
// CE(RST)=6, IO=7, SCLK=8 (можно изменить при необходимости)
iarduino_RTC clock(RTC_DS1302, 8, 6, 7);

Servo servo_test;

boolean feed = true;
char key;
int r[4]; // HHMM

// --- Сохранение времени ---
void saveFeedingTime() {
  Serial.println("Saving feeding time to EEPROM:");
  for (int k=0; k<4; k++) {
    EEPROM.update(k, r[k]);
    Serial.print("r["); Serial.print(k); Serial.print("] = ");
    Serial.println(r[k]);
    delay(10);
  }
}

// --- Загрузка времени ---
void loadFeedingTime() {
  Serial.println("Loading feeding time from EEPROM:");
  for (int k=0; k<4; k++) {
    r[k] = EEPROM.read(k);
    Serial.print("r["); Serial.print(k); Serial.print("] = ");
    Serial.println(r[k]);
  }
}

void setup() {
  lcd.init();
  lcd.backlight();
  servo_test.attach(13);

  clock.begin();
  //clock.settime(30, 48, 14, 22, 9, 25, 6);
  // Установить время при первой загрузке (раскомментируйте при первой прошивке)
  // rtc.setDOW(SATURDAY);  // День недели
  // rtc.setTime(12, 0, 0); // Часы, минуты, секунды
  // rtc.setDate(9, 19, 2025); // День, месяц, год

  Serial.begin(9600);
  pinMode(A3, INPUT);
  loadFeedingTime();
  servo_test.write(55);
}

void loop() {
  int buttonPress = digitalRead(A3);
  if (buttonPress == HIGH) {
    setFeedingTime();
  }

  String s = clock.gettime("H:i");
  char t[6];
  s.toCharArray(t, sizeof(t)); // копируем содержимое String в массив char

  // Отображаем время
  lcd.setCursor(0,0);
  lcd.print("                ");
  lcd.setCursor(0,0);
  lcd.print("Time: ");
  lcd.print(s);

  // Получаем отдельные цифры для сравнения
  char h1 = t[0];
  char h2 = t[1];
  char m1 = t[3];
  char m2 = t[4];

  // Отображаем дату
  lcd.setCursor(0,1);
  lcd.print("                ");
  lcd.setCursor(0,1);
  lcd.print("Date: ");
  lcd.print(clock.gettime("d-m-Y")); // Вернёт строку "DD.MM.YYYY"

  // Проверяем время кормления
  if (h1==r[0] && h2==r[1] && m1==r[2] && m2==r[3] && feed==true) {
    Serial.println("Feeding time! Activating servo.");
    servo_test.write(150);
    delay(700);
    servo_test.write(55);
    feed = false;
  }

  delay(500);
}

void setFeedingTime() {
  feed = true;
  int i = 0;
  int j = 0;

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Set feeding Time");
  delay(1000);

  lcd.clear();
  lcd.print("HH:MM");
  lcd.setCursor(0,1);

  while (1) {
    key = kpd.getKey();
    if (key != NO_KEY) {
      if (key == 'D') { // завершение ввода
        key = 0;
        break;
      }
      if (i < 4 && key >= '0' && key <= '9') {
        r[i] = key;
        lcd.setCursor(j,1);
        lcd.print(key);
        i++;
        j++;
        if (j == 2) {
          lcd.setCursor(j,1);
          lcd.print(':');
          j++;
        }
      }
      delay(300);
    }
  }
  saveFeedingTime();
}
