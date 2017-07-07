/*
 * collect-files.cpp
 *
 *  Created on: 18.05.2013
 *      Author: mag
 */

#include <string>
using namespace std;

#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>

void collect_files(std::string path, std::string prefix) {

	string dirpath=path;

	DIR *dir=opendir(dirpath.c_str());

	if (dir) {
		for (;;) {
			struct dirent *entry=readdir(dir);
			if (!entry) break;

			if (entry->d_type==DT_DIR) {
				if (entry->d_name[0]!='.') {
					string new_path=path+"/"+entry->d_name;
					string new_prefix=prefix+entry->d_name+"/";
					collect_files(new_path, new_prefix);
				}
			} else {
				string filepath=path+"/"+entry->d_name;
				struct stat filestat;
				stat(filepath.c_str(), &filestat);

				printf("\t{ \"%s%s\", 0x%06x,\n", prefix.c_str(), entry->d_name, (int) filestat.st_size );

				FILE *fp=fopen(filepath.c_str(), "r");
				int off=0;
				for (;;) {

					unsigned char buffer[1024];
					int sz=fread(buffer, 1, 1024, fp);
					if (sz==0) break;

					for (int n=0; n<sz; ++n) {
						if ((off%16) == 0) {
							printf("\t  \"");
						}
						printf("\\x%02x", buffer[n]);
						if ((off%16) == 15) {
								printf("\"\n");
						}
						++off;
					}
				}
				if ((off%16) != 0) {
						printf("\"\n");
				}
				printf("\t},\n");

			}
		}
	} else {
		fprintf(stderr, "FATAL: cannot stat %s: ", path.c_str());
		perror("");
		abort();
	}
}


int main(int argc, char *argv[]) {

	printf(
	"#include \"httpd/Httpd.h\"\n\n"
	"struct internal_http_files internal_files[] =\n"
	"  {\n");

	collect_files(argv[1], "/");

	printf("\t0 };\n");
	return 0;
}
