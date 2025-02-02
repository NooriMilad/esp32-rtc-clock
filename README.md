Plan:
Structure README with proper Markdown
Include all project components
Format code blocks correctly
Add clear installation steps


# ESP32 RTC-Uhr

## 🔧 Hardware
* ESP32 NodeMCU
* DS3231 RTC Modul
* Verbindungskabel

## 📌 Pinbelegung
```cpp
#define SDA_PIN 22    // I2C Datenleitung
#define SCL_PIN 21    // I2C Taktleitung
```

⚙️ Funktionen
Echtzeituhr-Anzeige
Datums- und Wochennummeranzeige
Alarmfunktion
Automatische Modusumschaltung


🔄 Zustandsmaschine
```cpp
enum ClockState {
    TIME_MODE,      // Zeitanzeige
    DATE_MODE,      // Datumsanzeige
    ALARM_MODE,     // Alarmeinstellung
    ALARM_RINGING   // Alarm aktiv
};
```

📥 Installation
Voraussetzungen
Visual Studio Code
PlatformIO Extension
Git
Projekt klonen

```cpp
git clone https://github.com/[username]/esp32-rtc-clock.git
cd esp32-rtc-clock
```

Bibliotheken

- RTClib
- Wire (Arduino Standard)

Build & Upload
- VS Code öffnen
- PlatformIO: Build ausführen
- PlatformIO: Upload starten

🎯 Verwendung

- Zeitanzeige: HH:MM:SS Format
- Datumsanzeige: Wochentag + KW
- Alarm: Einstellbare Weckzeit
- Auto-Return: Nach 3 Sekunden

🔌 Verbindungen
| ESP32	      | RTC DS3231  |
| ----------- | ----------- |
| RTC         |  DS3231     |
| Paragraph   | Text        |
| GPIO22      | SDA         |
| GPIO21      | SCL         |
|3.3V	      | VCC         |
|GND	      | GND         |
	

	





