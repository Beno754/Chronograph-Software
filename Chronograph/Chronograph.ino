/*
Revision 9 - LCD screen

Using 2 laser gates to calculate projectile speed and display results on an LCD screen

//Serial has been commented out to reduce unnecesary code, uncomment for debugging

*/

#include <Wire.h>               // Library for I2C communication
#include <LiquidCrystal_I2C.h>  // Library for LCD

// Wiring: SDA pin is connected to A4 and SCL pin to A5.
// Connect to LCD via I2C, default address 0x27 (A0-A2 not jumpered)
LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x27, 20, 4);  // Change to (0x27,20,4) for 20x4 LCD.


// VELOCITY VARS
bool startRead = false;           // A projectile broken the first gate
long readTimeOut = 200L * 1000L;  // time for the second gate to be made by. (ms->us)
long startReadTime = 0;           // at which BB entered first gate.
//projectile TIME -   S = D / T


// ROUNDS PER SECOND VARS
byte rounds = 0;                             // counter for rounds passed
long firstRoundTime = 0;                     // time when RPS started
long lastRoundTime = 0;                      // time when last round entered
const long RpsTimeout = 3L * 1000L * 1000L;  // time after last round to stop RPS calc. (seconds->ms->us)
bool RpsStarted = false;                     // check if start time should be logged.



void setup() {

  // LCD
  lcd.init();
  lcd.backlight();


  // initialize Serial communications at 9600 bps:
  //Serial.begin(115200);


  pinMode(3, INPUT);  // first sensor
  pinMode(4, INPUT);  // second sensor

  pinMode(5, OUTPUT);  //left confirm led
  pinMode(6, OUTPUT);  //right confirm led

  pinMode(12, INPUT_PULLUP);  // push button as pullup to remove resistor


  // Show LED works initially
  digitalWrite(5, HIGH);
  digitalWrite(6, HIGH);
  delay(2000);
  digitalWrite(5, LOW);
  digitalWrite(6, LOW);
  delay(1000);


  // Calibrate while button held down
  while (!digitalRead(12)) {
    sensitivityTraining();
  }

  DrawScreen();

  //Serial.println("Ready");  //after adjusting
}




void DrawScreen() {

  // Print text to LCD Screen:
  lcd.setCursor(0, 0);
  lcd.print("FPS:");  // Print the string

  lcd.setCursor(0, 1);
  lcd.print("#");

  lcd.setCursor(0, 2);
  lcd.print("RPS:");

  lcd.setCursor(0, 3);
  lcd.print("#");
}



// While calibrating, adjust the intensity of the laser so that the receiver is on threshold of detection.
void sensitivityTraining() {


  // Using internal registers to reduce overhead time
  if ((PIND & 1 << 3)) {  // PIN3 is HIGH
    digitalWrite(5, HIGH);
  } else {
    digitalWrite(5, LOW);
  }


  if ((PIND & 1 << 4)) {  // PIN4 is HIGH
    digitalWrite(6, HIGH);
  } else {
    digitalWrite(6, LOW);
  }
}








// Main entry point
void loop() {

  loopDigitalRead();
}









// Main function to run
void loopDigitalRead() {


  // Display laser sensitivity through LEDs
  sensitivityTraining();


  // Check if first gate has been passed
  if ((PIND & 1 << 3)) {  // PIN3 is high

    // VEL
    startRead = true;          // set to wait for second gate
    startReadTime = micros();  // time start VELOCITY

    //RPS
    if (!RpsStarted) {
      firstRoundTime = startReadTime;  // take time when RPS measure started
      RpsStarted = true;               // mem to say we have started.
    }

    rounds++;                       // increase count of rounds;
    lastRoundTime = startReadTime;  // take time of last round
  }



  //check if timeout for RPS
  if (RpsStarted) {
    if (micros() > lastRoundTime + RpsTimeout) {
      calcRPS();  //timed out, calc rps
    }
  }



  // Loop until second gate has been made
  while (startRead) {

    // Display laser sensitivity through LEDs
    sensitivityTraining();

    // check for second gate
    if ((PIND & 1 << 4)) {
      ////Serial.print("sensor2Low  ");
      ////Serial.println(sensor2Value);


      calcSpeed();        // calculate time between gates
      startRead = false;  //reset and wait for first gate
      delayMicroseconds(10);
    }

    //if elapsed time - timeout
    if (startReadTime + readTimeOut < micros()) {
      //printDebug1();
      //Serial.println("Time Out");
      startRead = false;
    }
  }
}



void calcRPS() {

  long delta = lastRoundTime - firstRoundTime;  //clauclate delta time
  //Serial.print("delta us RPS: ");
  //Serial.println(delta);

  //Serial.print("Rounds: ");
  //Serial.println(rounds);


  float UsToS = float(delta / 1000) / 1000;  // us to S
  //Serial.print("UsToS: ");
  //Serial.println(UsToS);

  float rps = rounds / UsToS;  // calculate Rounds per Second
  //Serial.print("Rounds per second: ");
  //Serial.println(rps);

  // Write result to screen
  lcd.setCursor(0, 3);
  lcd.print(rps);


  // Reset globals
  rounds = 0;          // counter for rounds passed
  firstRoundTime = 0;  // time when RPS started
  lastRoundTime = 0;   // time when last round entered
  RpsStarted = false;  // check if start time should be logged.
}


// Calculate from mm/us to fps
void calcSpeed() {

  long dif = micros() - startReadTime;
  //Serial.print("us taken: ");
  //Serial.println(dif);  // TIME (us)


  long speed = 100000000L / dif;
  //Serial.print("mm/s : ");
  //Serial.println(speed);
  //Serial.print("fps : ");
  float fps = (float)speed / 304.8; // Conversion factor
  ////Serial.println(fps);

  // Write result to screen
  lcd.setCursor(0, 1);
  lcd.print(fps);

}
