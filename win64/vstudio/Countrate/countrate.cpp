
#include <iostream>
#include "Thread.h"
#include "TimeTagger.h"
#include "Counter.cpp"

int main() {
	TimeTagger *tagger=new TimeTagger();

	Countrate *ch0=new Countrate(tagger, 0);

	for (;;) {
		std::cout << "Countrate on channel #0:" << ch0->getData() << std::endl;
		current_sleep(1000);
	}
}