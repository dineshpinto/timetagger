#include "TimeTagger.h"

int main() {
  TimeTagger* t = createTimeTagger();
  t->autoCalibration();
  return 0;
}
