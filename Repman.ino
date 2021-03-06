/* 
  This sketch reads the acceleration from the Bean's on-board accelerometer. 
  
  The acceleration readings are sent over serial and can be accessed in Arduino's Serial Monitor.
  
  To use the Serial Monitor, set Arduino's serial port to "/tmp/tty.LightBlue-Bean"
  and the Bean as "Virtual Serial" in the OS X Bean Loader.
    
  This example code is in the public domain.
*/

#define INT_MAX 32767

const int numReadings = 5;

int readingsX[numReadings];      // the readings from the analog input
int readingsY[numReadings];
int readingsZ[numReadings];
int data[10];
int dataIdx = 0;
int readIndex = 0;              // the index of the current reading
int totalX = 0;                  // the running total
int totalY = 0;
int totalZ = 0;
int average = 0;                // the average

int reps = 0;
int maxReps = 10;
int lastRepIdx = 0;
int currIdx = 0;
bool seenRep = 0;

int distThresh = 13;
int peakThresh = INT_MAX;


void setup() {
  // Bean Serial is at a fixed baud rate. Changing the value in Serial.begin() has no effect.
  Serial.begin(); 
  pinMode(0, OUTPUT);  
  for (int thisReading = 0; thisReading < numReadings; thisReading++) {
    readingsX[thisReading] = 0;
    readingsY[thisReading] = 0;
    readingsZ[thisReading] = 0;
  }

  // zero init data
  for(int i = 0; i < 10; i++)
    data[i] = 0;
  // Optional: Use Bean.setAccelerationRange() to set the sensitivity to something other than the default of ±2g.
}

void modifyData() {
  if (dataIdx < 10) {
    data[dataIdx] = totalY/numReadings;
    dataIdx ++;
  } else {
    for(int i = 0; i < 9; i++) {
      data[i] = data[i+1];
    }
    data[9] = totalY/numReadings;
  }
}

int avgMiddle() {
  int tot = 0;
  for(int i = 3; i < 8; i++) {
    tot += data[i];
  }
  
  return tot/5;
}

void buzz() {
  digitalWrite(0, HIGH);
  delay(500);
  digitalWrite(0, LOW);
}

void checkForRep() {
  if (dataIdx < 10) return;
  int avg = avgMiddle();

  // check that the tail of the list is less than head of list
  bool isPeak = data[9] < data[0] && avg > data[9] + 10 && 
                avg > data[0] + 10;
                
  // check that middle is greater than peak threshold
  bool validPeak = peakThresh == INT_MAX || avg > peakThresh;
//  Serial.println(avg);
//  Serial.println(peakThresh);

  // check that current peak is not too close to last peak
  bool peakSig = (currIdx - lastRepIdx) >= distThresh;

  //Serial.print(seenRep); Serial.print(isPeak); Serial.print(validPeak); Serial.println(peakSig);

  if(!seenRep && isPeak && validPeak && peakSig) {
    reps ++;
    Serial.println(reps);
    lastRepIdx = currIdx;
    seenRep = 1;
    if (reps == maxReps) buzz();
    if (peakThresh == INT_MAX) peakThresh = data[9];
    else peakThresh = (peakThresh + data[9]) / 2;
  }

  if(data[0] > avg && avg > data[9])
    seenRep = 0;
}

int count = 0;
void loop() {
  // subtract the last reading:
  totalX = totalX - readingsX[readIndex];
  totalY = totalY - readingsY[readIndex];
  totalZ = totalZ - readingsZ[readIndex];
  // read from the sensor:
  // Get the current acceleration with range of ±2g, and a conversion of 3.91×10-3 g/unit or 0.03834(m/s^2)/units. 
  AccelerationReading acceleration = Bean.getAcceleration();
  readingsX[readIndex] = acceleration.xAxis;
  readingsY[readIndex] = acceleration.yAxis;
  readingsZ[readIndex] = acceleration.zAxis;
  // add the reading to the total:
  totalX = totalX + readingsX[readIndex];
  totalY = totalY + readingsY[readIndex];
  totalZ = totalZ + readingsZ[readIndex];
  // advance to the next position in the array:
  readIndex = readIndex + 1;
  
  // Format the serial output like this:    "X: 249  Y: -27   Z: -253"
  String stringToPrint = String();

  // if we're at the end of the array...
  if (readIndex >= numReadings) {
    // ...wrap around to the beginning:
    readIndex = 0;
  }

  if(currIdx > 5) {
    modifyData();
    checkForRep();
  }

  currIdx ++;
  
  //stringToPrint = stringToPrint + "X: " + totalX/numReadings + "\tY: " + totalY/numReadings + "\tZ: " + totalZ/numReadings;
  //Serial.println();
  // Bean.sleep(1000);
}
