#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);

#define S0 4
#define S1 5
#define S2 6
#define S3 7
#define sensorOut 8

#define NUM_SAMPLES 5

String hasilDeteksi = "Belum Terdeteksi";
bool terkunci = false;
unsigned long totalUang = 0;
unsigned long nominalTerdeteksi = 0;
bool lcdSiap = false;

void setup() {
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
  pinMode(sensorOut, INPUT);

  digitalWrite(S0, HIGH); 
  digitalWrite(S1, LOW);

  Serial.begin(9600);

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("    Celengan    ");
  lcd.setCursor(0, 1);
  lcd.print("    Otomatis    ");
  delay(3000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Silakan Masukkan");
  lcd.setCursor(0, 1);
  lcd.print("Uang Kertas");
  lcdSiap = true;
}

void loop() {
  int red = bacaRata2Warna("R", NUM_SAMPLES);
  int green = bacaRata2Warna("G", NUM_SAMPLES);
  int blue = bacaRata2Warna("B", NUM_SAMPLES);

  Serial.print("R: "); Serial.print(red);
  Serial.print(" | G: "); Serial.print(green);
  Serial.print(" | B: "); Serial.println(blue);

  if (!terkunci && red > 150 && green > 150 && blue > 150) {
    Serial.println("Kemungkinan uang masuk. Menunggu warna stabil...");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Tunggu Sebentar");
    delay(1000); // Delay agar warna stabil

    red = bacaRata2Warna("R", NUM_SAMPLES);
    green = bacaRata2Warna("G", NUM_SAMPLES);
    blue = bacaRata2Warna("B", NUM_SAMPLES);

    Serial.print("Setelah Delay - R: "); Serial.print(red);
    Serial.print(" | G: "); Serial.print(green);
    Serial.print(" | B: "); Serial.println(blue);
  }

  if (!terkunci) {
    String hasil = deteksiUang(red, green, blue);
    if (hasil != "Tidak Dikenal") {
      hasilDeteksi = hasil;
      terkunci = true;

      if (hasilDeteksi == "Rp 1.000") nominalTerdeteksi = 1000;
      else if (hasilDeteksi == "Rp 2.000") nominalTerdeteksi = 2000;
      else if (hasilDeteksi == "Rp 5.000") nominalTerdeteksi = 5000;
      else if (hasilDeteksi == "Rp 10.000") nominalTerdeteksi = 10000;
      else if (hasilDeteksi == "Rp 20.000") nominalTerdeteksi = 20000;
      else if (hasilDeteksi == "Rp 50.000") nominalTerdeteksi = 50000;
      else if (hasilDeteksi == "Rp 100.000") nominalTerdeteksi = 100000;

      Serial.print(" => Nominal Terdeteksi: ");
      Serial.println(hasilDeteksi);

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Terdeteksi:");
      lcd.setCursor(0, 1);
      lcd.print(hasilDeteksi);

      bool uangMasihAda = true;
      unsigned long start = millis();
      while (millis() - start < 3000) {
        int r = bacaRata2Warna("R", NUM_SAMPLES);
        int g = bacaRata2Warna("G", NUM_SAMPLES);
        int b = bacaRata2Warna("B", NUM_SAMPLES);
        if (r < 100 && g < 100 && b < 100) {
          uangMasihAda = false;
          Serial.println("Uang dicabut saat delay. Dibatalkan.");
          break;
        }
        delay(100);
      }

      if (uangMasihAda) {
        totalUang += nominalTerdeteksi;

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(hasilDeteksi);
        lcd.setCursor(0, 1);
        lcd.print("Total: Rp");
        lcd.print(totalUang);

        Serial.print("Total Uang Masuk: Rp ");
        Serial.println(totalUang);
      } else {
        nominalTerdeteksi = 0;
        hasilDeteksi = "Belum Terdeteksi";
        terkunci = false;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Uang Dicabut");
        lcd.setCursor(0, 1);
        lcd.print("Batal Tambah");
        delay(2000);
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Silakan Masukkan");
        lcd.setCursor(0, 1);
        lcd.print("Uang Kertas");
      }
    } else {
      Serial.println(" => Uang Tidak Dikenal");
      if (lcdSiap) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Uang Tidak");
        lcd.setCursor(0, 1);
        lcd.print("Dikenal");
        lcdSiap = false;
      }
    }
  } else {
    Serial.print(" => Nominal Terkunci: ");
    Serial.println(hasilDeteksi);

    if (red < 100 && green < 100 && blue < 100) {
      Serial.println("Uang dijauhkan. Reset...");
      hasilDeteksi = "Belum Terdeteksi";
      nominalTerdeteksi = 0;
      terkunci = false;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Silakan Masukkan");
      lcd.setCursor(0, 1);
      lcd.print("Uang Kertas");
      lcdSiap = true;
    }
  }

  delay(500);
}

// ------- Fungsi Pembantu -------
int bacaRata2Warna(String warna, int jumlah) {
  long total = 0;
  for (int i = 0; i < jumlah; i++) {
    if (warna == "R") total += getRedPW();
    else if (warna == "G") total += getGreenPW();
    else if (warna == "B") total += getBluePW();
    delay(30);
  }
  int raw = total / jumlah;

  if (warna == "R") return map(raw, 21, 159, 255, 0);
  if (warna == "G") return map(raw, 20, 194, 255, 0);
  if (warna == "B") return map(raw, 17, 161, 255, 0);
  return 0;
}

String deteksiUang(int r, int g, int b) {
  if (inRange(r, 251, 256) && inRange(g, 248, 254) && inRange(b, 238, 244)) return "Rp 1.000";
  else if (inRange(r, 243, 246) && inRange(g, 247, 250) && inRange(b, 247, 250)) return "Rp 2.000";
  else if (inRange(r, 252, 259) && inRange(g, 243, 250) && inRange(b, 236, 243)) return "Rp 5.000";
  else if (inRange(r, 248, 254) && inRange(g, 248, 254) && inRange(b, 251, 257)) return "Rp 10.000";
  else if (inRange(r, 246, 251) && inRange(g, 250, 255) && inRange(b, 246, 252)) return "Rp 20.000";
  else if (inRange(r, 239, 245) && inRange(g, 245, 251) && inRange(b, 246, 252)) return "Rp 50.000";
  else if (inRange(r, 255, 261) && inRange(g, 249, 255) && inRange(b, 250, 257)) return "Rp 100.000";
  else return "Tidak Dikenal";
}

bool inRange(int val, int minVal, int maxVal) {
  return (val >= minVal && val <= maxVal);
}

int getRedPW() {
  digitalWrite(S2, LOW); digitalWrite(S3, LOW);
  delay(10);
  return pulseIn(sensorOut, LOW);
}

int getGreenPW() {
  digitalWrite(S2, HIGH); digitalWrite(S3, HIGH);
  delay(10);
  return pulseIn(sensorOut, LOW);
}

int getBluePW() {
  digitalWrite(S2, LOW); digitalWrite(S3, HIGH);
  delay(10);
  return pulseIn(sensorOut, LOW);
}
