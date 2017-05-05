/*
 * test.cpp
 *
 *  Created on: 24.04.2013
 *      Author: mag
 */

#include <iostream>
#include <string>

#include "httpd/json.h"

int ostream_test() {

	std::cout << JSON( "test" << "bla" << "test2" << "blubb") ;
	std::cout << std::endl;

	std::cout << JSON( "test" << 1 << "test2" << true << "test3" << 1e4) ;
	std::cout << std::endl;

	/* testbed: macro expanded.. */
	std::cout
		<< ObjectWriter()
			<< "a" << 1
			<< "b" << ObjectWriter()
				<< "a" << 3
			<< ObjectWriter::endObject()
			<< "c" << 3
		<< ObjectWriter::endObject()
	;
	std::cout << std::endl;

	std::cout << JSON( "test1" << "bla"
					<< "test2" << JSON(
							"inner1" << "test"
						)
					<< "test3" << 6
					) ;

	std::cout << std::endl;

	std::cout << JSARRAY( 1 << 2 << 3 );
	std::cout << std::endl;
}


#include "../backend/okFrontPanelDLL.h"

using namespace std;

const char *ok_dev_model[]={
		"Unknown",
		"XEM3001v1",
		"XEM3001v2",
		"XEM3010",
		"XEM3005",
		"XEM3001CL",
		"XEM3020",
		"XEM3050",
		"XEM9002",
		"XEM3001RB",
		"XEM5010"
};

int oktest() {

	okCUsbFrontPanel ok;

	int cnt=ok.GetDeviceCount();
	for (int n=0; n<cnt; ++n) {
		string serial=ok.GetDeviceListSerial(n);
		okCUsbFrontPanel::BoardModel model = ok.GetDeviceListModel(n);

		cout << "S:" << serial << "  - " << ok_dev_model[model] << endl;
	}

}




