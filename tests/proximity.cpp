#include <cassert>
#include "Proximity.h"

int main() {
  assert(estimatedFeet(-42) < 1.0f);
  assert(estimatedFeet(-64) > 3.2f && estimatedFeet(-64) < 3.3f);
  assert(estimatedFeet(-70) > 6.0f && estimatedFeet(-70) < 6.3f);
  assert(estimatedFeet(-80) > estimatedFeet(-70));

  Proximity proximity;
  assert(proximity.update(0) == Mood::Sad);
  assert(proximity.update(1800) == Mood::Sleep);

  for (uint32_t time = 1900; time <= 2400; time += 100) {
    proximity.observe(-68, time);
    proximity.update(time);
  }
  assert(proximity.mood() == Mood::Smile);

  // An isolated spike may move the filter, but cannot jump to the dance.
  proximity.observe(-18, 2500);
  assert(proximity.update(2500) != Mood::Dance);

  for (uint32_t time = 2600; time <= 4600; time += 100) {
    proximity.observe(-34, time);
    proximity.update(time);
  }
  assert(proximity.mood() == Mood::Dance);
  assert(proximity.update(5700) == Mood::Sad);
  assert(proximity.update(7500) == Mood::Sleep);

  // The first packet after a long gap seeds a fresh filter.
  proximity.observe(-89, 7600);
  assert(proximity.filtered() == -89);
  assert(proximity.update(7600) == Mood::Sleep);

  // A fading signal can put both toys to sleep even if packets still arrive.
  for (uint32_t time = 7700; time <= 9900; time += 100) {
    proximity.observe(-90, time);
    proximity.update(time);
  }
  assert(proximity.mood() == Mood::Sleep);

  Proximity wrapping;
  wrapping.observe(-35, UINT32_MAX - 100);
  wrapping.update(UINT32_MAX - 100);
  wrapping.observe(-35, 50);
  wrapping.update(50);
  wrapping.observe(-35, 200);
  assert(wrapping.update(200) == Mood::Dance);
}
