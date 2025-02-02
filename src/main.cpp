#include <Wire.h>
#include <RTClib.h>
#include<SPI.h>

#define SDA_PIN 22
#define SCL_PIN 21
#define LED_PIN 2


enum State {
    CLOCK_DISPLAY,
    ALARM_SET,
    ALARM_TRIGGERED,
    ALARM_RINGING,
    ALARM_MODE,
    TIME_MATCH
};

enum DisplayMode {
    TIME_MODE,
    DATE_MODE
};

RTC_DS3231 rtc;
State currentState = CLOCK_DISPLAY;
DateTime alarmTime;

// Global variables
DisplayMode displayMode = TIME_MODE;
const char* daysOfWeek[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
unsigned long lastModeSwitch = 0;
const unsigned long MODE_SWITCH_INTERVAL = 3000; // Switch every 3 seconds

// Global variables
unsigned long lastStateChange = 0;
const unsigned long AUTO_RETURN_TIMEOUT = 3000; // 3 seconds
bool alarmEnabled = false;
uint8_t alarmHour = 0;
uint8_t alarmMinute = 0;

// State handler prototypes
void handleTimeMode(DateTime now);
void handleDateMode(DateTime now);
void handleAlarmMode(DateTime now);
void handleAlarmRinging();

void displayTime(DateTime time) {
    char timeStr[20];
    sprintf(timeStr, "%02d:%02d:%02d", time.hour(), time.minute(), time.second());
    Serial.println(timeStr);
}
// dt class object the class is defined in the library RTClib.h 
void displayDateTime(DateTime dt) {
    char dateStr[32];
    
    if (displayMode == TIME_MODE) {
        sprintf(dateStr, "%02d:%02d:%02d", dt.hour(), dt.minute(), dt.second());
    } else {
        uint16_t dayOfYear = dt.day();
        for (uint8_t i = 1; i < dt.month(); ++i) {
            if (i == 2) {
                dayOfYear += 28 + (dt.year() % 4 == 0 && (dt.year() % 100 != 0 || dt.year() % 400 == 0));
            } else if (i == 4 || i == 6 || i == 9 || i == 11) {
                dayOfYear += 30;
            } else {
                dayOfYear += 31;
            }
        }
        uint8_t weekNumber = (dayOfYear + 6) / 7;  // Calculate week number
        sprintf(dateStr, "%s W%d", daysOfWeek[dt.dayOfTheWeek()], weekNumber);
    }
    Serial.println(dateStr);
    
    // Auto switch mode
    if (millis() - lastModeSwitch > MODE_SWITCH_INTERVAL) {
        displayMode = (displayMode == DisplayMode::TIME_MODE) ? DisplayMode::DATE_MODE : DisplayMode::TIME_MODE;
        lastModeSwitch = millis();
    }
}

void setup() {
    Wire.begin(SDA_PIN, SCL_PIN);
    Serial.begin(115200);
    pinMode(LED_PIN, OUTPUT);
    
    if (!rtc.begin()) {
        Serial.println("Couldn't find RTC");
        while (1);
    }
    
    if (rtc.lostPower()) {
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }
}



void loop() {
    DateTime now = rtc.now();
    
    // Auto return to TIME_MODE after timeout
    if (currentState != TIME_MODE && 
        (millis() - lastStateChange > AUTO_RETURN_TIMEOUT)) {
        currentState = CLOCK_DISPLAY;
    }
    
    // State machine
    switch (currentState) {
        case TIME_MODE:
            handleTimeMode(now);
            break;
        case DATE_MODE:
            handleDateMode(now);
            break;
        case ALARM_MODE:
            handleAlarmMode(now);
            break;
        // case ALARM_RINGING:
        //     handleAlarmRinging();
        //     break;
    }
    
    // Check for alarm trigger
    if (alarmEnabled && currentState != ALARM_RINGING &&
        now.hour() == alarmHour && now.minute() == alarmMinute) {
        currentState = ALARM_RINGING;
    }
    
    delay(100);
}

void handleTimeMode(DateTime now) {
    char timeStr[20];
    sprintf(timeStr, "%02d:%02d:%02d", now.hour(), now.minute(), now.second());
    Serial.println(timeStr);
}

void handleDateMode(DateTime now) {
    char dateStr[32];
    sprintf(dateStr, "%s W%d", daysOfWeek[now.dayOfTheWeek()], 
            (calculateDayOfYear(now) + 6) / 7);
    Serial.println(dateStr);
}

int calculateDayOfYear(DateTime dt) {
    int dayOfYear = dt.day();
    for (int i = 1; i < dt.month(); ++i) {
        if (i == 2) {
            dayOfYear += 28 + (dt.year() % 4 == 0 && (dt.year() % 100 != 0 || dt.year() % 400 == 0));
        } else if (i == 4 || i == 6 || i == 9 || i == 11) {
            dayOfYear += 30;
        } else {
            dayOfYear += 31;
        }
    }
    return dayOfYear;
}

void setAlarm(int hour, int minute) {
    DateTime now = rtc.now();
    alarmTime = DateTime(now.year(), now.month(), now.day(), hour, minute, 0);
    currentState = ALARM_SET;
}