#include <PS2X_lib.h>

PS2X ps2x; // create PS2 Controller Class object

#define PS2_DAT 12 // MISO  19
#define PS2_CMD 13 // MOSI  23
#define PS2_SEL 15 // SS     5
#define PS2_CLK 14 // SLK   18

#define SERVO 3
#define SERVO_GOC_BAN 3
#define SERVO_LOC_BONG 5

#define TOP_SPEED 4090
#define NORM_SPEED 2048
#define TURNING_FACTOR 0.7

int speed = NORM_SPEED;

void setupPS2controller()
{
  int err = -1;
  while (err != 0)
  {
    err = ps2x.config_gamepad(PS2_CLK, PS2_CMD, PS2_SEL, PS2_DAT, true, true);
  }

  SemaphoreHandle_t mutex = xSemaphoreCreateMutex();
  assert(mutex);

  bool needSpool = false;
  bool hasSpooled = false;
}

bool PS2control()
{
  //TODO: test neu bam l2 2 lan thi co crash ko
  if (ps2x.Button(PSB_L2)) {
    xTaskCreatePinnedToCore(
                    waitBanBong,   /* Task function. */
                    "waitBanBong",     /* name of task. */
                    8192,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &Task2,      /* Task handle to keep track of created task */
                    1);          /* pin task to core 1 */
    xSemaphoreTake(mutex, 1);
    needSpool = true;
    xSemaphoreGive(mutex);
    if (ps2x.Button(PSB_L1)) {
      xSemaphoreTake(mutex, 1);
      bool prv_hasSpooled = hasSpooled;
      xSemaphoreGive(mutex);
      if (prv_hasSpooled) {
        pwm.setPWM(SERVO, 0, 400);
        delay(500);
        pwm.setPWM(SERVO, 0, 210);
      }
    }
  }
  if(ps2x.ButtonPressed(PSB_GREEN)){
    turnServo90(SERVO_GOC_BAN);
  } else if (ps2x.ButtonPressed(PSB_RED)) {
    turnServoM90(SERVO_GOC_BAN);
  }
  if (ps2x.ButtonPressed(PSB_PAD_RIGHT)) {
    // Black ball
    // Position servo1 at -90 degrees
    turnServoM90(SERVO_LOC_BONG);
  } else if (ps2x.ButtonPressed(PSB_PAD_LEFT)) {
    // White ball
    // Position servo1 at 90 degrees
    turnServo90(SERVO_LOC_BONG);
  } else if (ps2x.ButtonPressed(PSB_PAD_UP)){
    int irvalue = digitalRead(irvalue);
    if (irvalue == 0) {
      // Black ball
      // Position servo1 at -90 degrees
      turnServoM90(SERVO_LOC_BONG);
    } else if (irvalue== 1){ 
      // White ball
      // Position servo1 at 90 degrees
      turnServo90(SERVO_LOC_BONG);
    }
  }

  int nJoyX = 128 - ps2x.Analog(PSS_RY); // read x-joystick
  int nJoyY = 128 - ps2x.Analog(PSS_LX); // read y-joystick
  int nMotMixL;                          // Motor (left) mixed output
  int nMotMixR;                          // Motor (right) mixed output


  bool temp = (nJoyY * nJoyX > 0);
  if (nJoyY > 1 or nJoyY < -1) // Turning
  {
    nMotMixL = -nJoyY * !temp;
    nMotMixR = -nJoyY * !temp;
    nMotMixL *= TURNING_FACTOR;
    nMotMixR *= TURNING_FACTOR;
  }
  else // Forward or Reverse
  {
    nMotMixL = -nJoyX;
    nMotMixR = nJoyX;
  }

  int c1 = 0, c2 = 0, c3 = 0, c4 = 0;

  if (nMotMixR > 0)
  {
    c3 = nMotMixR;
    c3 = map(c3, 0, 128, 0, speed);
  }

  else if (nMotMixR < 0)
  {
    c4 = abs(nMotMixR);
    c4 = map(c4, 0, 128, 0, speed);
  }

  if (nMotMixL > 0)
  {
    c1 = nMotMixL;
    c1 = map(c1, 0, 128, 0, speed);
  }
  else if (nMotMixL < 0)
  {
    c2 = abs(nMotMixL);
    c2 = map(c2, 0, 128, 0, speed);
  }
  setPWMMotors(c1, c2, c3, c4);
  } else {
    if (ps2x.Button(PSB_L2)) {
      Serial.println("ban bong");
      pwm.setPWM(14, 0, TOP_SPEED);
      delay(500);
      while (ps2x.Button(PSB_L2)) {
        pwm.setPWM(SERVO, 0, 400);
        delay(500);
        pwm.setPWM(SERVO, 0, 210);
        ps2x.read_gamepad(0, 0);
        delay(250);
      }
      pwm.setPWM(14, 0, 0);
      pwm.setPWM(SERVO, 0, 400); 
    }
  }
  return 1;
}

void waitBanBong() {
  xSemaphoreTake(mutex, 1);
  bool prv_needSpool = needSpool;
  bool prv_hasSpooled = hasSpooled;
  xSemaphoreGive(mutex);
  if (prv_needSpool) {
    if (!prv_hasSpooled) {
      // bat motor chay da
      delay(500);
      xSemaphoreTake(mutex, 1);
      hasSpooled = true;
      xSemaphoreGive(mutex);
    }
  } else {
    //tat motor chay da
    //tu huy task
  }
}