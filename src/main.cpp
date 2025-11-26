#include <Wire.h>
#include <RTClib.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);
RTC_DS1307 rtc;

#define BUZZER_PIN 8
#define SET_BUTTON 9
#define UP_BUTTON 10
#define DOWN_BUTTON 6
#define ALARM_BUTTON 7

int setMode = 0;
int settingIndex = 0;
int alarmHour = 7;
int alarmMinute = 0;
bool alarmEnabled = false;
bool alarmTriggered = false;

int setHour = 0;
int setMinute = 0;

bool setPressed = false;
bool upPressed = false;
bool downPressed = false;
bool alarmPressed = false;

void displayTime(DateTime now)
{
    lcd.setCursor(0, 0);
    lcd.print("Date: ");
    if (now.day() < 10)
        lcd.print("0");
    lcd.print(now.day());
    lcd.print("/");
    if (now.month() < 10)
        lcd.print("0");
    lcd.print(now.month());
    lcd.print("/");
    lcd.print(now.year());
    lcd.print("  ");

    lcd.setCursor(0, 1);
    lcd.print("Time: ");
    if (now.hour() < 10)
        lcd.print("0");
    lcd.print(now.hour());
    lcd.print(":");
    if (now.minute() < 10)
        lcd.print("0");
    lcd.print(now.minute());
    lcd.print(":");
    if (now.second() < 10)
        lcd.print("0");
    lcd.print(now.second());

    lcd.print(" ");
    if (alarmEnabled)
    {
        lcd.print("A");
    }
    else
    {
        lcd.print(" ");
    }
}

void displaySetTime()
{
    lcd.setCursor(0, 0);
    lcd.print("Set Time:       ");
    lcd.setCursor(0, 1);

    if (settingIndex == 0)
    {
        lcd.print("Hour> ");
    }
    else
    {
        lcd.print("Min > ");
    }

    if (setHour < 10)
        lcd.print("0");
    lcd.print(setHour);
    lcd.print(":");
    if (setMinute < 10)
        lcd.print("0");
    lcd.print(setMinute);
    lcd.print("    ");
}

void displaySetAlarm()
{
    lcd.setCursor(0, 0);
    lcd.print("Set Alarm:      ");
    lcd.setCursor(0, 1);

    if (settingIndex == 0)
    {
        lcd.print("Hour> ");
    }
    else
    {
        lcd.print("Min > ");
    }

    if (alarmHour < 10)
        lcd.print("0");
    lcd.print(alarmHour);
    lcd.print(":");
    if (alarmMinute < 10)
        lcd.print("0");
    lcd.print(alarmMinute);

    lcd.print(" ");
    if (alarmEnabled)
    {
        lcd.print("ON ");
    }
    else
    {
        lcd.print("OFF");
    }
}

void handleButtons(DateTime now)
{
    bool setNow = (digitalRead(SET_BUTTON) == LOW);
    bool upNow = (digitalRead(UP_BUTTON) == LOW);
    bool downNow = (digitalRead(DOWN_BUTTON) == LOW);
    bool alarmNow = (digitalRead(ALARM_BUTTON) == LOW);

    if (setNow && !setPressed)
    {
        setPressed = true;

        if (setMode == 0)
        {
            setMode = 1;
            settingIndex = 0;
            setHour = now.hour();
            setMinute = now.minute();
            Serial.println("Mode: Set Time");
        }
        else if (setMode == 1)
        {
            rtc.adjust(DateTime(now.year(), now.month(), now.day(), setHour, setMinute, 0));
            setMode = 2;
            settingIndex = 0;
            Serial.println("Mode: Set Alarm");
        }
        else if (setMode == 2)
        {
            setMode = 0;
            settingIndex = 0;
            Serial.println("Mode: Normal");
        }

        lcd.clear();
        delay(300);
    }
    if (!setNow)
        setPressed = false;

    if (upNow && !upPressed)
    {
        upPressed = true;

        if (setMode == 1)
        {
            if (settingIndex == 0)
            {
                setHour++;
                if (setHour > 23)
                    setHour = 0;
            }
            else
            {
                setMinute++;
                if (setMinute > 59)
                    setMinute = 0;
            }
        }
        else if (setMode == 2)
        {
            if (settingIndex == 0)
            {
                alarmHour++;
                if (alarmHour > 23)
                    alarmHour = 0;
            }
            else
            {
                alarmMinute++;
                if (alarmMinute > 59)
                    alarmMinute = 0;
            }
        }

        delay(200);
    }
    if (!upNow)
        upPressed = false;

    if (downNow && !downPressed)
    {
        downPressed = true;

        if (setMode == 1)
        {
            if (settingIndex == 0)
            {
                setHour--;
                if (setHour < 0)
                    setHour = 23;
            }
            else
            {
                setMinute--;
                if (setMinute < 0)
                    setMinute = 59;
            }
        }
        else if (setMode == 2)
        {
            if (settingIndex == 0)
            {
                alarmHour--;
                if (alarmHour < 0)
                    alarmHour = 23;
            }
            else
            {
                alarmMinute--;
                if (alarmMinute < 0)
                    alarmMinute = 59;
            }
        }

        delay(200);
    }
    if (!downNow)
        downPressed = false;

    if (alarmNow && !alarmPressed)
    {
        alarmPressed = true;

        if (setMode == 0)
        {
            if (alarmTriggered)
            {
                alarmTriggered = false;
                noTone(BUZZER_PIN);
                Serial.println("Alarm stopped");
            }
            else
            {
                alarmEnabled = !alarmEnabled;
                Serial.print("Alarm: ");
                Serial.println(alarmEnabled ? "ON" : "OFF");
            }
        }
        else
        {
            settingIndex = 1 - settingIndex;
        }

        delay(300);
    }
    if (!alarmNow)
        alarmPressed = false;
}

void checkAlarm(DateTime now)
{
    if (alarmEnabled && !alarmTriggered)
    {
        if (now.hour() == alarmHour && now.minute() == alarmMinute && now.second() == 0)
        {
            alarmTriggered = true;
            Serial.println("🔔 ALARM TRIGGERED!");
        }
    }

    if (alarmTriggered)
    {
        // Beeping pattern with tone
        static unsigned long lastBeep = 0;
        static bool beepState = false;

        if (millis() - lastBeep > 500)
        {
            beepState = !beepState;

            if (beepState)
            {
                tone(BUZZER_PIN, 1000); // 1000Hz beep
                Serial.println("BEEP!");
            }
            else
            {
                noTone(BUZZER_PIN);
            }

            lastBeep = millis();
        }
    }
    else
    {
        noTone(BUZZER_PIN);
    }
}

void setup()
{
    Serial.begin(9600);

    lcd.init();
    lcd.backlight();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("RTC Clock Alarm");
    delay(2000);
    lcd.clear();

    if (!rtc.begin())
    {
        lcd.print("RTC Error!");
        while (1)
            ;
    }

    if (!rtc.isrunning())
    {
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }

    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(SET_BUTTON, INPUT_PULLUP);
    pinMode(UP_BUTTON, INPUT_PULLUP);
    pinMode(DOWN_BUTTON, INPUT_PULLUP);
    pinMode(ALARM_BUTTON, INPUT_PULLUP);

    digitalWrite(BUZZER_PIN, LOW);

    Serial.println("System Ready!");

    // TEST BUZZER
    Serial.println("Testing buzzer...");
    tone(BUZZER_PIN, 1000, 300);
    delay(400);
    tone(BUZZER_PIN, 1500, 300);
    delay(400);
    Serial.println("Buzzer test done!");
}

void loop()
{
    DateTime now = rtc.now();
    handleButtons(now);

    if (setMode == 0)
    {
        displayTime(now);
        checkAlarm(now);
    }
    else if (setMode == 1)
    {
        displaySetTime();
    }
    else if (setMode == 2)
    {
        displaySetAlarm();
    }

    delay(50);
}
