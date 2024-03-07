#include <stdio.h>
#include <string.h>
#include "utility.h"

int upload(const char* file) {
	FILE* file = fopen(file, "r");
	if (file == NULL) {
		write_error("invalid file");
	}

}
int main() {
	return 0;
}