#define pinServoMakanan               4
#define waktuBukaServo                1000//milidetik
#define servoBuka                     20//derajat
#define servoTutup                    60//derajat

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Sodaq_DS3231.h>
#include <Servo.h>
#include <EEPROM.h>

// FIX 1: waktuMakan1-6 sekarang variable DateTime beneran (bukan #define),
// karena nilainya di-overwrite dari EEPROM saat setup().
DateTime waktuMakan1 = DateTime(0, 1, 1, 10, 52, 0, 0); // jam pagi SET WAKTU MAKAN 1
DateTime waktuMakan2 = DateTime(0, 1, 1, 10, 53, 0, 0); // jam sore SET WAKTU MAKAN 2
DateTime waktuMakan3 = DateTime(0, 1, 1, 9, 9, 0, 0);   // SET WAKTU MAKAN 3
DateTime waktuMakan4 = DateTime(0, 1, 1, 9, 9, 0, 0);   // SET WAKTU MAKAN 4
DateTime waktuMakan5 = DateTime(0, 1, 1, 9, 9, 0, 0);   // SET WAKTU MAKAN 5
DateTime waktuMakan6 = DateTime(0, 1, 1, 9, 9, 0, 0);   // SET WAKTU MAKAN 6

// FIX 2: alamat EEPROM sekarang unik per waktu makan (kelipatan sizeof(DateTime)),
// jadi waktu makan 2-6 gak lagi saling timpa data satu sama lain.
#define EEPROM_ADDR_WAKTU_MAKAN1 (0 * sizeof(DateTime))
#define EEPROM_ADDR_WAKTU_MAKAN2 (1 * sizeof(DateTime))
#define EEPROM_ADDR_WAKTU_MAKAN3 (2 * sizeof(DateTime))
#define EEPROM_ADDR_WAKTU_MAKAN4 (3 * sizeof(DateTime))
#define EEPROM_ADDR_WAKTU_MAKAN5 (4 * sizeof(DateTime))
#define EEPROM_ADDR_WAKTU_MAKAN6 (5 * sizeof(DateTime))

char weekDay[][6 ] = {"Selasa", "Rabu", "Kamis", "Jumat", "Sabtu", "Minggu", "Senin"};

LiquidCrystal_I2C lcd(0x27, 16, 2); // kalau LCD blank/tidak nyala, coba ganti ke 0x3F
Servo servoMakanIkan;

byte detikSebelumnya;
char buf[80]; // dilebarin dari 17 -> 80 supaya sprintf string panjang di setup() gak overflow

void saveTimeToEEPROM(DateTime time, int address) {
  EEPROM.put(address, time);
}

DateTime loadTimeFromEEPROM(int address) {
  DateTime time;
  EEPROM.get(address, time);
  return time;
}

void setup() {
  Serial.begin(9600);
  Serial.println("Pemberi Pakan Ikan Otomatis");
  Serial.println("Project Elektro");

  servoMakanIkan.attach(pinServoMakanan);
  servoMakanIkan.write(servoTutup);

  Wire.begin();
  rtc.begin();
  //DateTime dt(2020, 10, 17, 20, 03, 0, 7); // set tanggal dan waktu (format): tahun, bulan,tanggal, jam, menit, detik, hari (1=minggu, 7=sabtu)
  //rtc.setDateTime(dt);

  Wire.beginTransmission(0x27);
  if (Wire.endTransmission())
  {
    lcd = LiquidCrystal_I2C(0x27, 16, 2);
  }
  lcd.init();

  lcd.backlight();
  lcd.setCursor(3, 0);
  lcd.print("Pakan Ikan");
  lcd.setCursor(4, 1);
  lcd.print("Otomatis");
  delay(3000);
  lcd.clear();

  Serial.println("Sistem Mulai");
  waktuMakan1 = loadTimeFromEEPROM(EEPROM_ADDR_WAKTU_MAKAN1);
  waktuMakan2 = loadTimeFromEEPROM(EEPROM_ADDR_WAKTU_MAKAN2);
  waktuMakan3 = loadTimeFromEEPROM(EEPROM_ADDR_WAKTU_MAKAN3);
  waktuMakan4 = loadTimeFromEEPROM(EEPROM_ADDR_WAKTU_MAKAN4);
  waktuMakan5 = loadTimeFromEEPROM(EEPROM_ADDR_WAKTU_MAKAN5);
  waktuMakan6 = loadTimeFromEEPROM(EEPROM_ADDR_WAKTU_MAKAN6);

  // FIX 3: tanda kutip sprintf dibetulin + %02d:%02d ditambahin supaya
  // jam/menit yang tercetak beneran ambil dari variabel, bukan teks statis.
  sprintf(buf, "Set waktu 1 = %02d:%02d (Senin, Selasa, Rabu, Kamis, Jum'at, Sabtu, Minggu)", waktuMakan1.hour(), waktuMakan1.minute());
  Serial.println(buf);
  sprintf(buf, "Set waktu 2 = %02d:%02d (Senin, Selasa, Rabu, Kamis, Jum'at, Sabtu, Minggu)", waktuMakan2.hour(), waktuMakan2.minute());
  Serial.println(buf);
  sprintf(buf, "Set waktu 3 = %02d:%02d (Senin, Selasa, Rabu, Kamis, Jum'at, Sabtu, Minggu)", waktuMakan3.hour(), waktuMakan3.minute());
  Serial.println(buf);
  sprintf(buf, "Set waktu 4 = %02d:%02d (Senin, Selasa, Rabu, Kamis, Jum'at, Sabtu, Minggu)", waktuMakan4.hour(), waktuMakan4.minute());
  Serial.println(buf);
  sprintf(buf, "Set waktu 5 = %02d:%02d (Senin, Selasa, Rabu, Kamis, Jum'at, Sabtu, Minggu)", waktuMakan5.hour(), waktuMakan5.minute());
  Serial.println(buf);
  sprintf(buf, "Set waktu 6 = %02d:%02d (Senin, Selasa, Rabu, Kamis, Jum'at, Sabtu, Minggu)", waktuMakan6.hour(), waktuMakan6.minute());
  Serial.println(buf);
}

void loop() {

  DateTime now = rtc.now();
  rtc.convertTemperature();
  lcd.setCursor(12, 1);
  lcd.print("T:");
  lcd.setCursor(14, 1);
  lcd.print(rtc.getTemperature()); //tampilan temperature
  lcd.print("C");
  lcd.setCursor(10, 0);
  lcd.print(weekDay[now.dayOfWeek()]); //tampilan hari
  lcd.setCursor(0, 1);
  lcd.print(now.date(), DEC); // tanggal
  lcd.print("/");
  lcd.print(now.month(), DEC); //bulan
  lcd.print("/");
  lcd.print(now.year(), DEC); //tahun

  if (detikSebelumnya != now.second())
  {
    char jamBuf[9];
    sprintf(jamBuf, "%02d:%02d:%02d", now.hour(), now.minute(), now.second());

    lcd.setCursor(0, 0);
    lcd.print(jamBuf);
    Serial.print(jamBuf);

    detikSebelumnya = now.second();

    uint32_t epoch = now.get() % 86400; //hanya jam menit detik

    if ((epoch == waktuMakan1.get()) ||
        (epoch == waktuMakan2.get()) ||
        (epoch == waktuMakan3.get()) ||
        (epoch == waktuMakan4.get()) ||
        (epoch == waktuMakan5.get()) ||
        (epoch == waktuMakan6.get()))
    {
      char pesanBuf[17];
      sprintf(pesanBuf, "WAKTU = %02d:%02d", now.hour(), now.minute());
      lcd.clear();
      lcd.setCursor(1, 0);
      lcd.print(pesanBuf);
      lcd.setCursor(0, 1);
      lcd.print("Waktunya Makan!!");
      Serial.println(pesanBuf);
      // 4 KALI BUKA PAKAN , BISA DI KURANGIN/TAMBAH
      servoMakanIkan.write(servoBuka);
      delay(waktuBukaServo);
      servoMakanIkan.write(servoTutup);
      delay(1000);

      lcd.clear();
    }
  }
}
