#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <SD.h>
#include <SPI.h>
#include <DFRobotDFPlayerMini.h>

DFRobotDFPlayerMini dfPlayer;

// LCD configuration
LiquidCrystal_I2C lcd(0x27, 20, 4);

// Keypad configuration
const byte ROWS = 3;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'Y', 'N', '0', '1'},
  {'2', '3', '4', '5'},
  {'6', '7', '8', '9'}
};
byte rowPins[ROWS] = {9, 8, 7};
byte colPins[COLS] = {6, 5, 4, 3};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// Array of track names
const char* trackNames[] = {
  "001.wav", "002.wav", "003.wav", "004.wav", "005.wav",
  // ... add remaining track names here
  "286.wav"
};

// Pin configuration
const int ledPin = 2;
const int buzzerPin = 51;

// Variables
int skor = 5;
int nomorFile;
unsigned long waktuMulaiBuzzer = 0;
const unsigned long durasiCuitan = 500;
bool audioSedangDiputar = false;

// Function declarations
void tampilkanMenuUtama();
void modeMendengarkan();
void modeMenebak();
void putarFileAudio(int nomorFile);
void mintaKonfirmasi(const char* pesan, void (*callbackYa)(), void (*callbackTidak)());
void dapatkanInputKeypad(char* bufferInput, size_t ukuranBuffer);
void prosesInputKeypad(char key);
void sistemBerjalan();
void jawabanSalah();
void saatYaMendengarkan();
void saatTidakMendengarkan();
void saatYaMenebak();
void saatTidakMenebak();
void saatPutarFile();
void saatTebakanJawaban();
void pilihCaraMendengarkan();
void mendengarkanManual();
void mendengarkanRandom();

// Setup function
void setup() {
  lcd.init();
  lcd.backlight();
  Serial.begin(9600);
  Serial1.begin(9600);
  pinMode(ledPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);

  if (!dfPlayer.begin(Serial1)) {
    Serial.println("DFPlayer Mini tidak terdeteksi.");
    while (true);
  }

  sistemBerjalan();
  tampilkanMenuUtama();
}

// Main loop
void loop() {
  // Handle buzzer timing
  if (waktuMulaiBuzzer > 0 && millis() - waktuMulaiBuzzer >= durasiCuitan) {
    digitalWrite(buzzerPin, LOW);
    waktuMulaiBuzzer = 0;
  }

  // Check audio playback status
  if (audioSedangDiputar && dfPlayer.available() && dfPlayer.readType() == DFPlayerPlayFinished) {
    audioSedangDiputar = false;
    tampilkanMenuUtama();
  }

  // Check keypad input
  char key = keypad.getKey();
  if (key) {
    prosesInputKeypad(key);
  }
}

// Process keypad input
void prosesInputKeypad(char key) {
  if (!audioSedangDiputar) {
    if (key == '1') {
      mintaKonfirmasi("Anda yakin?", saatYaMendengarkan, saatTidakMendengarkan);
    } else if (key == '2') {
      mintaKonfirmasi("Anda yakin?", saatYaMenebak, saatTidakMenebak);
    }
  }
}

// System running indication
void sistemBerjalan() {
  digitalWrite(ledPin, HIGH);
}

// Handle incorrect answer
void jawabanSalah() {
  digitalWrite(buzzerPin, HIGH);
  waktuMulaiBuzzer = millis();
}

// Display main menu
void tampilkanMenuUtama() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Selamat Datang!");
  lcd.setCursor(0, 1);
  lcd.print("Pilih Permainan:");
  lcd.setCursor(0, 2);
  lcd.print("1: Dengar Ayat");
  lcd.setCursor(0, 3);
  lcd.print("2: Tebak Ayat");
}

// Listening mode
void modeMendengarkan() {
  lcd.clear();
  lcd.print("Pilih metode:");
  lcd.setCursor(0, 1);
  lcd.print("1: Manual");
  lcd.setCursor(0, 3);
  lcd.print("2: Random");

  while (true) {
    char key = keypad.getKey();
    if (key == '1') {
      mendengarkanManual();
      break;
    }
    if (key == '2') {
      mendengarkanRandom();
      break;
    }
  }
}

// Manual listening
void mendengarkanManual() {
  lcd.clear();
  lcd.print("Masukkan nomor file:");
  char input[4] = "";
  dapatkanInputKeypad(input, sizeof(input));
  nomorFile = atoi(input);

  mintaKonfirmasi("Putar file?", saatPutarFile, tampilkanMenuUtama);
}

// Random listening
void mendengarkanRandom() {
  nomorFile = random(1, 287);
  saatPutarFile();
}

// Guessing mode
void modeMenebak() {
  nomorFile = random(1, 287);
  mintaKonfirmasi("Konfirmasi tebakan?", saatTebakanJawaban, tampilkanMenuUtama);
}

// Request confirmation
void mintaKonfirmasi(const char* pesan, void (*callbackYa)(), void (*callbackTidak)()) {
  lcd.clear();
  lcd.print(pesan);
  lcd.setCursor(0, 1);
  lcd.print("Y: Ya  N: Tidak");

  while (true) {
    char key = keypad.getKey();
    if (key == 'Y') {
      callbackYa();
      break;
    }
    if (key == 'N') {
      callbackTidak();
      break;
    }
  }
}

// Get keypad input
void dapatkanInputKeypad(char* bufferInput, size_t ukuranBuffer) {
  size_t indeks = 0;
  while (indeks < ukuranBuffer - 1) {
    char key = keypad.getKey();
    if (key) {
      bufferInput[indeks++] = key;
      lcd.setCursor(indeks - 1, 1);
      lcd.print(key);
    }
  }
  bufferInput[indeks] = '\0';
}

// Callback functions
void saatYaMendengarkan() {
  modeMendengarkan();
}

void saatTidakMendengarkan() {
  tampilkanMenuUtama();
}

void saatYaMenebak() {
  modeMenebak();
}

void saatTidakMenebak() {
  tampilkanMenuUtama();
}

void saatPutarFile() {
  dfPlayer.play(nomorFile);
  audioSedangDiputar = true;
  lcd.clear();
  lcd.print("Memutar file ");
  lcd.print(nomorFile);
}

void saatTebakanJawaban() {
  char input[4] = "";
  dapatkanInputKeypad(input, sizeof(input));
  int tebakan = atoi(input);

  if (tebakan == nomorFile) {
    skor++;
    lcd.clear();
    lcd.print("Benar! Skor: ");
    lcd.print(skor);
  } else {
    skor--;
    lcd.clear();
    lcd.print("Salah! Skor: ");
    lcd.print(skor);
    jawabanSalah();
  }

  if (skor == 0) {
    lcd.clear();
    lcd.print("Permainan Selesai");
    while (true);
  } else if (skor == 10) {
    lcd.clear();
    lcd.print("Anda Menang!");
    while (true);
  } else {
    mintaKonfirmasi("Lanjutkan?", modeMenebak, tampilkanMenuUtama);
  }
}