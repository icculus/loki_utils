#include <stdio.h>

#include "loki_inifile.h"

int main(int argc, char **argv)
{
	ini_file_t *ini;
	ini_iterator_t *it;
	char buf[256], command = '\0', key[100], section[100];
	const char *ptr;

	if ( argc < 2 ) {
		fprintf(stderr,"Usage: %s file.ini\n", argv[0]);
		return 1;
	}

	ini = loki_openinifile(argv[1]);
	if ( ini ) {
		while(*buf != 'q') {
			printf("INI I/O test :\n"
				   "g - Get a value from the file\n"
				   "a - Add a new keyed value\n"
				   "p - Print all lines of a given section\n"
				   "q - Quit and write the file to test.ini\n\n"
				   "Your choice ? "
				   );
			fflush(stdin);
			scanf("%c", buf);
			command = *buf;
			switch(command) {
			case 'a':
				printf("Section ? ");
				scanf("%s", section);
				printf("Key ? ");
				scanf("%s", key);
				printf("Value ? ");
				scanf("%s", buf);
				if( loki_putinistring(ini, section, key, buf) )
					printf("Value successfully added.\n");
				else
					printf("Error while adding value.\n");
				break;
			case 'g':
				printf("Section ? ");
				scanf("%s", section);
				printf("Key ? ");
				scanf("%s", key);
				ptr = loki_getinistring(ini, section, key);
				if(ptr)
					printf("Value = %s\n", ptr);
				else
					printf("Unknown key/section!\n");
				break;
			case 'p':
				printf("Section ? ");
				scanf("%s", section);
				it = loki_begininisection(ini, section);
				if( it ) {
					do {
						const char *k, *v;
						if ( loki_getiniline(it, &k, &v)) {
							printf("%s = %s\n", k, v);
						}
					} while( loki_nextiniline(it));
					loki_freeiniiterator(it);
				} else 
					printf("Section not found in file!\n");
				break;
			case 'q':
				break;
			default:
				printf("Error!!\n");
			}
		}

		if(loki_writeinifile(ini, "test.ini"))
			printf("File test.ini has been written.\n");

		loki_closeinifile(ini);
	} else {
		fprintf(stderr,"Parse error reading %s\n", argv[1]);
	}

	return 0;
}
