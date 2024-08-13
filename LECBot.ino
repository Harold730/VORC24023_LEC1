#include "motors.h"
#include "PS2_controller.h"
#include <cmath>

void setup()
{
  
  Serial.begin(115200);
  initMotors();
  setupPS2controller();
  Serial.println("Done setup!");
}
int deg = 0;

void loop()
{
  ps2x.read_gamepad(0, 0);
  PS2control();
  delay(50);
}



