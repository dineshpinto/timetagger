/*
 * main.cpp
 *
 *  Created on: 27.04.2013
 *      Author: mag
 */

#include "ServerController.h"


int main() {
	  extern void init_memwatch();
	  if (0) init_memwatch();


	ServerControl ctl("timetaggerd.json");

	for (;;)
		sleep(1);

}

