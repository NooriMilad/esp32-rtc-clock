#include <Wire.h>
#include <RTClib.h>

#define SDA_PIN 22
#define SCL_PIN 21
#define LED_PIN 2
#define BUZZER_PIN 15
#define MODE_BUTTON_PIN 13
#define ALARM_BUTTON_PIN 12
//
RTC_DS3231 rtc;

enum State {
    CLOCK_DISPLAY,
    DATE_DISPLAY,
    ALARM_SETTING,
    ALARM_ACTIVE
};

// ISO 8601 Wochennummernberechnung
int calculateISOWeek(const DateTime& dt) {
    int year = dt.year();
    int month = dt.month();
    int day = dt.day();
    
    int ordinal = day;
    for (int m = 1; m < month; m++) {
        if (m == 2) {
            ordinal += 28 + (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0));
        } else {
            ordinal += (m == 4 || m == 6 || m == 9 || m == 11) ? 30 : 31;
        }
    }
    
    int weekNumber = (ordinal - dt.dayOfTheWeek() + 10) / 7;
    if (weekNumber < 1) return calculateISOWeek(DateTime(year-1, 12, 31));
    if (weekNumber > 52 && (31 - day + 1) > 3) return 1;
    return weekNumber;
}

struct Alarm {
    DateTime time;
    bool enabled = false;
    bool triggered = false;
    
    void set(uint8_t hour, uint8_t minute) {
        DateTime now = rtc.now();
        time = DateTime(now.year(), now.month(), now.day(), hour, minute, 0);
        enabled = true;
    }
    
    void check(const DateTime& now) {
        if(enabled && !triggered && 
           now.hour() == time.hour() && 
           now.minute() == time.minute() &&
           now.second() <= 5) {
            triggered = true;
        }
    }
};

State currentState = CLOCK_DISPLAY;
Alarm alarm;
unsigned long lastAction = 0;
const unsigned long DISPLAY_TIMEOUT = 3000;
const char* daysOfWeek[] = {"So", "Mo", "Di", "Mi", "Do", "Fr", "Sa"};

void displayTime(const DateTime& dt) {
    char buffer[20];
    sprintf(buffer, "%02d:%02d:%02d", dt.hour(), dt.minute(), dt.second());
    Serial.println(buffer);
}

void displayDate(const DateTime& dt) {
    char buffer[30];
    sprintf(buffer, "%s KW%02d %02d.%02d.%04d", 
           daysOfWeek[dt.dayOfTheWeek()],
           calculateISOWeek(dt),
           dt.day(),
           dt.month(),
           dt.year());
    Serial.println(buffer);
}

void handleAlarm() {
    static unsigned long lastBlink = 0;
    if(millis() - lastBlink > 500) {
        digitalWrite(LED_PIN, !digitalRead(LED_PIN));
        lastBlink = millis();
    }
    tone(BUZZER_PIN, 1000, 500);
}

void checkButtons() {
    static unsigned long lastDebounce = 0;
    if(millis() - lastDebounce < 200) return;
    
    if(digitalRead(MODE_BUTTON_PIN)) {
        currentState = (currentState == CLOCK_DISPLAY) ? DATE_DISPLAY : CLOCK_DISPLAY;
        lastAction = millis();
    }
    
    if(digitalRead(ALARM_BUTTON_PIN)) {
        if(currentState == ALARM_ACTIVE) {
            alarm.triggered = false;
            currentState = CLOCK_DISPLAY;
        } else {
            currentState = ALARM_SETTING;
        }
        lastAction = millis();
    }
}

void setup() {
    Serial.begin(115200);
    Wire.begin(SDA_PIN, SCL_PIN);
    pinMode(LED_PIN, OUTPUT);
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(MODE_BUTTON_PIN, INPUT_PULLUP);
    pinMode(ALARM_BUTTON_PIN, INPUT_PULLUP);

    if(!rtc.begin()) {
        Serial.println("RTC nicht gefunden!");
        while(1);
    }
    
    if(rtc.lostPower()) {
        Serial.println("RTC Zeit gesetzt!");
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }
    
    // Beispielalarm um 7:30
    alarm.set(7, 30);
}

void loop() {
    DateTime now = rtc.now();
    alarm.check(now);
    
    checkButtons();
    
    if(millis() - lastAction > DISPLAY_TIMEOUT && currentState != ALARM_ACTIVE) {
        currentState = CLOCK_DISPLAY;
    }

    switch(currentState) {
        case CLOCK_DISPLAY:
            displayTime(now);
            break;
            
        case DATE_DISPLAY:
            displayDate(now);
            break;
            
        case ALARM_ACTIVE:
            handleAlarm();
            break;
            
        case ALARM_SETTING:
            // Alarmeinstellungslogik hier erweitern
            Serial.println("Alarm Mode");
            break;
    }
    
    if(alarm.triggered && currentState != ALARM_ACTIVE) {
        currentState = ALARM_ACTIVE;
        lastAction = millis();
    }
    
    delay(100);
}
