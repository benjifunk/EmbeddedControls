#include "tach.h"
#include "Pins.h"
#include "Configuration.h"

const int timeoutValue = 10;
volatile int timeoutCounter;

long rpm, rpm_last;
int activation_rpm,
shift_rpm;

//array for rpm averaging, filtering comparison
const int numReadings = 5;
int rpmarray[numReadings];
int index = 0;                   // the index of the current reading
long total = 0;                  // the running total
long average = 0;                // the average

