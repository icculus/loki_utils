#include <stdio.h>

#include "loki_utils.h"

char *game_version = "Loki Utils Test Program";

int main(int argc, char **argv)
{
  const char *opt;

  loki_registeroption("optarg", 'o', "\tOptional argument", LOKI_OPTSTRING);
  loki_registeroption("string", 'm', "\tRequired argument", LOKI_STRING);
  loki_initialize(argc, argv, "");

  if(loki_getconfig_optstr("optarg", &opt)) {
	printf("optarg = %s\n", opt);
  } else {
	printf("optarg not specified\n");
  }
  printf("string = %s\n", loki_getconfig_str("string"));
  return 0;
}
