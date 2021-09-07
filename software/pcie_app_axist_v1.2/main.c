#define _BSD_SOURCE
#define _XOPEN_SOURCE 500
#include <assert.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>


#include "xdma_axist_utils.h"

#define DEVICE_NAME_DEFAULT_C2H "/dev/xdma0_c2h_0"
#define DEVICE_NAME_DEFAULT_H2C "/dev/xdma0_h2c_0"


int main(int argc, char *argv[])
{	


	char *device_c2h = DEVICE_NAME_DEFAULT_C2H;
	char *device_h2c = DEVICE_NAME_DEFAULT_H2C;
	uint64_t address = 0;
	uint64_t aperture = 0;
	uint64_t size = 65536 *4;
	uint64_t offset = 0;
	uint64_t count = 1;
	char *infname_h2c = "data/datafile_32M.bin";
	char *ofname_h2c = NULL;
	char *ofname_c2h = "data/data_out32M_c2h_0824.bin";

	printf("this is h2c\n");

	test_dma_h2c(device_h2c, address, aperture, size, offset, count, infname_h2c, ofname_h2c);


	printf("this is c2h\n");

	test_dma_c2h(device_c2h, address, aperture, size, offset, count, ofname_c2h);
	

	//sleep(1);


 






}
