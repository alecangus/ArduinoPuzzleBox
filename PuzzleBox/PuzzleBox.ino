#include <LiquidCrystal.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_LSM303_U.h>
#include <Adafruit_BMP085_U.h>
#include <Adafruit_L3GD20_U.h>
#include <Adafruit_10DOF.h>

/*

  TODO:
    Calibrate the sensors?
    Get the ceasar cipher working
    Confirm team number

*/

// rs, rw, e, d0, d1, d2, d3, d4, d5, d6, d7
// LiquidCrystal lcd(4, 2, 3, 13, 12, 11, 10, 9, 8, 7, 6);
LiquidCrystal lcd(4, 2, 3, 9, 8, 7, 6);

Adafruit_LSM303_Accel_Unified accel = Adafruit_LSM303_Accel_Unified(30301);
Adafruit_BMP085_Unified       bmp   = Adafruit_BMP085_Unified(18001);
Adafruit_LSM303_Mag_Unified   mag   = Adafruit_LSM303_Mag_Unified(30302);

const int levelOneLED = 10;
const int levelTwoLED = 11;
const int levelThreeLED = 12;
const int stageZeroTimeout = 2000; // Number of milliseconds to wait for knocking to end
const int PIEZO = A0;

const float heightThreshold = 0.7; // Height the box has to travel upwards to complete level
const int knockThreshold = 100;     // threshold value to decide when a knock value is detected
const int clapThreshold = 50;       // Threshold value to decide when applause is detected
const float seaLevelPressure = SENSORS_PRESSURE_SEALEVELHPA;
const int smoothingCount = 50;

int state; // 0 = Team number, 1 = Liftmeup, 2 = Roll me over, 3 = spin me round, 4 = applause, 5 = eliza
unsigned long lastKnock = false;
int knockVal; // variable to store the value read from the piezo
float startHeight;
float startHeading;

// Smoothing
float heightSmoothing[smoothingCount];
int heightSmoothingIndex = 0;
float heightSmoothingTotal = 0.0;
float heightSmoothingAverage = 0.0;

float headingSmoothing[smoothingCount];
int headingSmoothingIndex = 0;
float headingSmoothingTotal = 0.0;
float headingSmoothingAverage = 0.0;

int teamNumber = 0;

void setup() {
  Serial.begin(9600);
  Serial.println("Starting setup");
  lcd.begin(16, 2);
  accel.begin();
  bmp.begin();
  mag.begin();

  // *************** Get smoothed start height ***************
  float temperature;
  bmp.getTemperature(&temperature);
  sensors_event_t event;
  for (int i = 0; i < smoothingCount; i++) {
    bmp.getTemperature(&temperature);
    bmp.getEvent(&event);
    heightSmoothing[i] = bmp.pressureToAltitude(seaLevelPressure,
                                          event.pressure,
                                          temperature);
    startHeight += heightSmoothing[i];
    heightSmoothingTotal += heightSmoothing[i];
  }
  startHeight = startHeight / smoothingCount;
  heightSmoothingAverage = heightSmoothingTotal / smoothingCount;
  // *************** End smoothed start height ***************
  
  pinMode(levelOneLED, OUTPUT);
  pinMode(levelTwoLED, OUTPUT);
  pinMode(levelThreeLED, OUTPUT);

  // Set up for State 0
  state = 0;
  Serial.println("Finished setup");
  randomSeed(analogRead(0));
  display("Knock on wood...", "                ");
}

void loop() {
  /* State zero, "Knock on wood"
   *  Lights are cycling or flashing randomly
   */
  if (state == 0) {
    // Light everything up
    //setLights(random(0,2), random(0,2), random(0,2));
    setLights(0, 0, 0);
    
    // ******************* Count the knocks *************************
    
    knockVal = analogRead(PIEZO); // read the sensor
    if (lastKnock && (lastKnock + stageZeroTimeout) < millis()) { // Knocking has begun and stopped
      Serial.print("Finished listening for knocks, Team Number = ");
      Serial.println(teamNumber);
      // Move to stage 1
      state = 1;
      display(encrypt("Hold me up high"), "                ");
    }
    else if (knockVal >= knockThreshold) { // If knock detected, handle
      lastKnock = millis();
      teamNumber++;
    setLights(1, 0, 0);
    delay(10);
    setLights(0, 1, 0);
    delay(10);
    setLights(0, 0, 1);
    delay(10);
    }
    //delay(30);
    teamNumber = random(1,10);
    // *********** End count the knocks **************
  }

  /* Lift Me Up */
  if (state == 1) {
    setLights(random(0,2), random(0,2), random(0,2));

    // ******************* Lift Me Up *************************
    heightSmoothingTotal -= heightSmoothing[heightSmoothingIndex];
    sensors_event_t event;
    bmp.getEvent(&event);
    float temperature;
    bmp.getTemperature(&temperature);
    heightSmoothing[heightSmoothingIndex] = bmp.pressureToAltitude(seaLevelPressure,
                                        event.pressure,
                                        temperature);
    heightSmoothingTotal += heightSmoothing[heightSmoothingIndex];
    heightSmoothingIndex++;
    if (heightSmoothingIndex >= smoothingCount) {
      heightSmoothingIndex = 0;
    }
    heightSmoothingAverage = heightSmoothingTotal / smoothingCount;
    if (heightSmoothingAverage - startHeight > heightThreshold) {
      Serial.println("Moving to stage 2");
      state = 2;
      display(encrypt("Roll me over"), ""); // 3 = Uroo#ph#ryhu
    } else {
      Serial.print("Stage 2: startHeight: ");
      Serial.print(startHeight);
      Serial.print(", heightSmoothingAverage: ");
      Serial.print(heightSmoothingAverage);
      Serial.print(". Total: ");
      Serial.println(heightSmoothingAverage - startHeight);
    }
    // ***************** End Lift Me Up ***********************
  }

  /* Roll me over */
  if (state == 2) {
    setLights(1, 0, 0);

    // ******************* Roll Me Over *************************
    sensors_event_t event;
    accel.getEvent(&event);
    
    if (event.acceleration.z > 10) { // Upside down


      delay(2000);


      
      accel.getEvent(&event);
      while (!(event.acceleration.z < -10)) {
        // Do nothing waiting for the box to return to the right way up
        accel.getEvent(&event);
      }



      
      startHeading = (atan2(event.magnetic.y,event.magnetic.x) * 180) / PI;
      if (startHeading < 0) {
        startHeading += 360;
      }
      Serial.println("Moving to stage 3");
      state = 3;
      display(encrypt("Turn Me Round"), ""); // 3 = Uroo#ph#ryhu
    }
    // ***************** End Roll Me Over ***********************
  }

  /* Spin me round */
  if (state == 3) {
    setLights(1, 1, 0);

    // ******************* Turn Me Round *************************
    sensors_event_t event;
    mag.getEvent(&event);
    accel.getEvent(&event);
    float currentHeading = (atan2(event.magnetic.y,event.magnetic.x) * 180) / PI;
    if (currentHeading < 0) {
      currentHeading += 360;
    }
    if (abs(startHeading - currentHeading) > 170 && (event.acceleration.z < -10)) { // The box is 180 degrees turned in a flat spin and the right way up
      Serial.println("Moving to state 4");
      state = 4;
      display(encrypt("Applaud"), "");
    }
    // ***************** End Turn Me Round ***********************
  }

  /* Applaude */
  if (state == 4) {
    setLights(1, 1, 1);
    
    // ******************* Applause *************************
    
    knockVal = analogRead(PIEZO); // read the sensor
    if (knockVal >= clapThreshold) { // I hear applause
      // Move to stage 5
      state = 5;
    }
    else { // Not loud enough
      if (knockVal > 0) {
        Serial.print("Clapping: [");
        Serial.print(knockVal);
        Serial.println("]");
      }
    }

    // *********** End Applause *******************************
  }

  if (state == 5) {
    scrollLongText("Congratulations! Passcode: We shall wear our glitter with pride ");
  }
}

void display (String line1, String line2) {
  lcd.clear();
  Serial.println("Displaying on 1: [" + line1 + "]");
  Serial.println("Displaying on 2: [" + line2 + "]");
  int padding = (16-line1.length())/2;
  lcd.setCursor(padding,0);
  lcd.print(line1);
  lcd.setCursor(0, 1);
  lcd.print(line2);

}

void setLights (bool one, bool two, bool three) {
  digitalWrite(levelOneLED, one);
  digitalWrite(levelTwoLED, two);
  digitalWrite(levelThreeLED, three);
}

String encrypt (String s) {
  int len = s.length();
  String encrypted = s;
  for (int i=0;i<len;i++) {
    if ((s[i] + teamNumber) <= 122) {
      encrypted[i] = s[i] + teamNumber;
    } else {
      encrypted[i] = (s[i] + teamNumber) - 90; // Wrap around 
    }
  }
  return encrypted;
}

void scrollLongText (String text) {
int stringStart, stringStop = 0;
int scrollCursor = 16;
  for (int i = 0; i < text.length(); i++) {
  lcd.setCursor(scrollCursor, 0);
  lcd.clear();
  lcd.print(text.substring(stringStart,stringStop));
  lcd.setCursor(0, 1);
  lcd.print("");
  delay(300);
  if(stringStart == 0 && scrollCursor > 0){
    scrollCursor--;
    stringStop++;
  } else if (stringStart == stringStop){
    stringStart = stringStop = 0;
    scrollCursor = 16;
  } else if (stringStop == text.length() && scrollCursor == 0) {
    stringStart++;
  } else {
    stringStart++;
    stringStop++;
  }
  }
  delay(5000);
}

