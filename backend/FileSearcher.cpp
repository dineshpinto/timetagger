#include "FileSearcher.h"

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <iostream>
#include <string>

#ifndef _MSC_VER
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#pragma warning( disable : 4996)

#endif

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#ifdef WIN32

/**
* @param location The location of the registry key. For example "Software\\Bethesda Softworks\\Morrowind"
* @param name the name of the registry key, for example "Installed Path"
* @return the value of the key or an empty string if an error occured.
*/
std::string getRegKey(const std::string& location, const std::string& name){
	HKEY key;
	char value[1024];
	DWORD bufLen = 1024*sizeof(TCHAR);
	long ret;
	ret = RegOpenKeyExA(HKEY_LOCAL_MACHINE, location.c_str(), 0, KEY_QUERY_VALUE, &key);
	if( ret != ERROR_SUCCESS ){
		std::cerr << "OPEN KEY FAILED" << std::endl;
		return std::string();
	}
	ret = RegQueryValueExA(key, name.c_str(), 0, 0, (LPBYTE) value, &bufLen);
	RegCloseKey(key);
	if ( (ret != ERROR_SUCCESS) || (bufLen > 1024*sizeof(TCHAR)) ){
		std::cerr << "QUERY VALUE FAILED" << std::endl;
		return std::string();
	}
	std::string stringValue = std::string(value, (size_t)bufLen - 1);
	size_t i = stringValue.length();
	while( i > 0 && stringValue[i-1] == '\0' ) {
		--i;
	}
	return stringValue.substr(0,i);
}


std::string search_file(const std::string& filename, const std::string& path) {
	std::string appdir = getRegKey("SOFTWARE\\SwabianInstruments\\TimeTagger","InstDir");
	if (appdir=="") {
		//fatal("problem with installation, aborting");
	}
	appdir += "\\" + path + "\\"; appdir += filename;
	FILE *fp = fopen(appdir.c_str(),"r");
	if (fp) {
		fclose(fp);
		return appdir;
	}
	return "";
}
#else

std::string search_file(const std::string& filename, const std::string& path) {
	struct stat st_stat;
	if (stat(filename.c_str(), &st_stat)==0) return filename;
	std::string home = getenv("HOME");
	home += "/.timetagger/" + path + "/"; home+=filename;
	if (stat(home.c_str(), &st_stat)==0) return home;
	std::string libdir="/usr/lib/timetagger/";
	libdir+=path + "/" + filename;
	if (stat(libdir.c_str(), &st_stat)==0) return libdir;
	return "";
}

#endif
