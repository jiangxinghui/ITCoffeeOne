#include "utils.h"
#include "math.h"

float percentageWithTransition(float pct, TransitionCurve transition);


float mapRange(float refNumber, float refStart, float refEnd, float targetStart, float targetEnd, int decimalPrecision, TransitionCurve transition) {
  volatile float deltaRef = refEnd - refStart;
  volatile float deltaTarget = targetEnd - targetStart;

  if (deltaRef == 0) {
    return targetEnd;
  }

  volatile float pct = fmax(0.0f, fmin(1.0f, abs((refNumber - refStart) / deltaRef)));

  volatile float finalNumber = targetStart + deltaTarget * percentageWithTransition(pct, transition);

 volatile int calcScale = (int)pow(10, decimalPrecision >= 0 ? decimalPrecision : 1);
  return (float)round(finalNumber * calcScale) / calcScale;
}

float percentageWithTransition(float pct, TransitionCurve transition) {
  if (transition == TransitionCurve::LINEAR) {
    return pct;
  }
//   else if (transition == TransitionCurve::EASE_IN) {
//     return easeIn(pct);
//   }
//   else if (transition == TransitionCurve::EASE_OUT) {
//     return easeOut(pct);
//   }
//   else if (transition == TransitionCurve::INSTANT) {
//     return 1.f;
//   }
//   else {
//     return easeInOut(pct);
//   }
}