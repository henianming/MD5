#include "MD5.h"

int main(int argc, char *argv[]) {
	char resultBuf[32] = {0};
	char source[] = "jklmn";
	GetMD5(resultBuf, source, strlen(source));

	while (1);
	return 0;
}
