#include <SPI.h>
#include <SD.h>

// ====== SD card config ======
const int chipSelect = 5;  // CS pin for SPI microSD (SPI: SCK=18, MISO=19, MOSI=23)

// ====== Button config ======
// Your wiring: physical Buttons 1..5 should be on GPIOs 27, 26, 25, 33, 32 respectively.
// (We swapped the first two from last time so presses read 1,2,3,4,5 in order.)
const int buttonPins[5] = {27, 26, 25, 33, 32};  // index 0=Btn1 ... index 4=Btn5

// Debounce time (ms)
const uint32_t DEBOUNCE_MS = 25;

// Per-button state
bool     btnStableState[5];
bool     btnLastRead[5];
uint32_t btnLastChangeMs[5];

void printDirectory(File dir, int numTabs);
bool isAppleHidden(const char *name);

// ====== Setup ======
void setup() {
  Serial.begin(115200);
  delay(50);
  Serial.println();
  Serial.println("=== ESP32 MP3 Board: SD + Button Self-Test ===");

  // Configure buttons (all with internal pull-ups; wiring = GPIO --(button)--> GND)
  for (int i = 0; i < 5; i++) {
    pinMode(buttonPins[i], INPUT_PULLUP);
  }

  // Initialize button state trackers
  uint32_t now = millis();
  for (int i = 0; i < 5; ++i) {
    bool level = digitalRead(buttonPins[i]);
    btnStableState[i]   = level;   // assume current read is stable at boot
    btnLastRead[i]      = level;
    btnLastChangeMs[i]  = now;
  }

  // --- SD card test ---
  Serial.println("\nStarting SD card test...");

  if (!SD.begin(chipSelect)) {
    Serial.println("SD card initialization failed!");
    Serial.println("Check:");
    Serial.println("- SD card is inserted");
    Serial.println("- SD card is formatted as FAT32");
    Serial.println("- Wiring connections (CS=5, SCK=18, MISO=19, MOSI=23)");
    // Continue anyway so you can still test buttons
  } else {
    Serial.println("SD card initialized successfully!");

    Serial.print("Card type: ");
    auto ctype = SD.cardType();
    if (ctype == CARD_NONE)      Serial.println("No SD card attached");
    else if (ctype == CARD_MMC)  Serial.println("MMC");
    else if (ctype == CARD_SD)   Serial.println("SDSC");
    else if (ctype == CARD_SDHC) Serial.println("SDHC/SDXC");
    else                         Serial.println("UNKNOWN");

    uint64_t cardSizeMB = SD.cardSize() / (1024ULL * 1024ULL);
    Serial.printf("SD Card Size: %llu MB\n", cardSizeMB);

    // List files in root (filtering macOS junk)
    Serial.println("\nFiles in root directory (hidden macOS files filtered):");
    File root = SD.open("/");
    if (root) {
      printDirectory(root, 0);
      root.close();
    } else {
      Serial.println("(Could not open root directory)");
    }

    // Ensure /music exists
    if (!SD.exists("/music")) {
      if (SD.mkdir("/music")) Serial.println("Created /music directory");
      else                    Serial.println("Failed to create /music directory");
    } else {
      Serial.println("/music directory already exists");
    }
  }

  Serial.println("\nButton pins ready (active-LOW):");
  for (int i = 0; i < 5; ++i) {
    Serial.printf("  Button %d on GPIO%d  (released=HIGH, pressed=LOW)\n", i + 1, buttonPins[i]);
  }
  Serial.println("-------------------------------------------------\n");
}

// ====== Loop ======
void loop() {
  uint32_t now = millis();

  for (int i = 0; i < 5; ++i) {
    bool level = digitalRead(buttonPins[i]);   // HIGH=released, LOW=pressed

    if (level != btnLastRead[i]) {
      btnLastChangeMs[i] = now;   // start/refresh debounce timer
      btnLastRead[i] = level;
    }

    if ((now - btnLastChangeMs[i]) >= DEBOUNCE_MS && level != btnStableState[i]) {
      btnStableState[i] = level;

      if (level == LOW) {
        Serial.printf("[BUTTON] Pressed  -> Button %d (GPIO%d)\n", i + 1, buttonPins[i]);
      } else {
        Serial.printf("[BUTTON] Released -> Button %d (GPIO%d)\n", i + 1, buttonPins[i]);
      }
    }
  }

  delay(1);
}

// ====== Helpers ======
bool isAppleHidden(const char *name) {
  // Skip typical macOS metadata junk
  if (!name || !name[0]) return false;
  if (name[0] == '.') return true;     // ".Spotlight-V100", ".Trashes", ".DS_Store", "._*"
  if (name[0] == '_' && name[1] == 0) return true; // extremely unlikely, but guard
  if (name[0] == '.' && name[1] == '_') return true; // "._ResourceFork"
  // Some SD libs report AppleDouble as starting with "._"; covered above.
  return false;
}

void printDirectory(File dir, int numTabs) {
  while (true) {
    File entry = dir.openNextFile();
    if (!entry) break;

    const char *nm = entry.name();
    if (isAppleHidden(nm)) { entry.close(); continue; }

    for (int i = 0; i < numTabs; i++) Serial.print('\t');
    Serial.print(nm);

    if (entry.isDirectory()) {
      Serial.println("/");
      printDirectory(entry, numTabs + 1);
    } else {
      Serial.print("\t\t");
      Serial.println(entry.size(), DEC);
    }
    entry.close();
  }
}
