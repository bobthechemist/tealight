/* 
* Create a blowing candle effect where the flicker is 
* influenced by change in a photoresistor reading
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* 
 * My implementation of the Box-Muller transform, the 
 * equations for which can be found at:
 * https://en.wikipedia.org/wiki/Box-Muller_transform
 * I use the first random number to determine which side
 * of the distribution (pos or neg of mu) is returned.
 * This implementation is "wasteful" in the sense that it requires
 * two (uniform) random numbers to form one normally distributed
 * random number.  For this particular problem, I don't see an issue
 * with the inefficiency.
*/

float normalDistribution(float mu, float sigma)
{
  const float two_pi = 2.0*3.14159265358979323846;
  float u1, u2, z;
  u1 = rand()*1.0/RAND_MAX;
  u2 = rand()*1.0/RAND_MAX;

  if(u1 < 0.5)
  {
    z = sqrt(-2.0*log(u1))*cos(two_pi*u2);
  }
  else
  {
    z = sqrt(-2.0*log(u1))*sin(two_pi*u2);
  }

  return z*sigma+mu;
}

/* 
 * Analog pins 2 and 3 should not be connected to anything.
 * Use their readings to generate a random number seed.
*/
long genSeed()
{
  return analogRead(2)+ analogRead(3)*1000;
}

/*
 * There are times when we want to constrain numbers within a 
 * certain range.  This function helps us do that easily
*/
float rescale(float val,float oldmin, float oldmax, float newmin, float newmax)
{
  float i;
  
  i = (newmin*oldmax-newmax*oldmin+newmax*val-newmin*val)/(oldmax-oldmin); 
  return i;
}
   
/* 
 * Global variables
*/

// normally distributed random number;
float z;
// previous and current light reading
int plr, clr;
// default values for LED intensity and flicker
// Change to suit your desire.  Max is 255.
float mu = 25, sigma = 4;
// variables containing change when wind active
// These can be chaged in the loop routine below.
float dmu, dsigma;
// photoresistor min and max
// I played around with this routine on an ATMega328p which allows
// for serial output.  Under my conditions, these were the limits of
// the photoresistor values.  Set to 0 and 1023 to get the largest 
// range of values or smaller to get a more dramatic flickering effect.
int pmin = 300, pmax = 800;
// led min and max
// Even though the maximum for PWM is 255, the sequin LED doesn't
// seem to have much additional intensity variation above this value. 
// Feel free to change if you wish.
int lmin = 0, lmax = 125;
// Pins (photoresistor at pin 2[analog] and LED at pin 9[PWM]
const int PPIN = 1; // Physical pin 7 on ATtiny85
const int LPIN = 0; // Physical pin 5 on ATtiny85
// We need to know if the windy LED values are active for the current loop
boolean wind = false;
// The windy LED values will only stick around for a short time
unsigned long windtimer = 0;
// This variable determined if a light change is triggered.
// A higher value indicates a greater change is required.
// Right now a light intensity change of ~ 1% will do the trick.
const int windlimit = 30;

void setup(){
  // Get the initial photoresistor reading
  plr = analogRead(PPIN);
}

void loop (){
  // Get the current photoresistor reading
  clr = analogRead(PPIN);
  // If a change in the light level is detected and we are not already
  // in a wind pattern, set the windy light values.
  if(abs(clr-plr)>windlimit && !wind)
  {
    wind = true;
    windtimer = millis();
    dmu = -10; dsigma = 10;
  }
  // If the windy LED settings have be active for 1000 ms, turn them off.
  // Feel free to change the 1000 to something longer or shorter.  
  if (wind && (millis()-windtimer>1000))
  {
    wind = false;
    dmu = 0; dsigma = 0;
  }
  // Change the LED intensity
  analogWrite(LPIN,constrain(
    (int)normalDistribution(mu+dmu,sigma+dsigma),
    0,255
  ));
  // Resent the previous light level.
  plr = clr;  
  // Slow down a bit.  This value can be modified to suit your desires.
  delay(25);
}
