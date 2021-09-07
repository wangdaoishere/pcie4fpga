/*
 * xdma_h2c&c2h_api  allinone  20210317 16:12
 * read_c2h dma_from_device 20210316 23:22 start
 * write_h2c dma_to_device  20210317 afternoon start
 * read_c2h dma_from_device 20210316 23:22 start 
 * This source code is licensed under BSD-style license (found in the
 * LICENSE file in the root directory of this source tree)
 */

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

#include "dma_utils.h"

//#define DEVICE_NAME_DEFAULT "/dev/xdma0_h2c_0"
//#define DEVICE_NAME_DEFAULT "/dev/xdma0_c2h_0"
#define SIZE_DEFAULT (32)
#define COUNT_DEFAULT (1)

/*
static struct option const long_opts[] = {
	{"device", required_argument, NULL, 'd'},
	{"address", required_argument, NULL, 'a'},
	{"aperture", required_argument, NULL, 'k'},
	{"size", required_argument, NULL, 's'},
	{"offset", required_argument, NULL, 'o'},
	{"count", required_argument, NULL, 'c'},
	{"data infile", required_argument, NULL, 'f'},
	{"data outfile", required_argument, NULL, 'w'},
	{"help", no_argument, NULL, 'h'},
	{"verbose", no_argument, NULL, 'v'},
	{0, 0, 0, 0}
};
*/

/////////////////////



static int eop_flush = 0;



static int test_dma_c2h(char *devname, uint64_t addr, uint64_t aperture,
			uint64_t size, uint64_t offset, uint64_t count,
			char *ofname)
{

	/*
	int cmd_opt;
	
	uint64_t address = 0;
	uint64_t aperture = 0;
	uint64_t size = SIZE_DEFAULT;
	uint64_t offset = 0;
	uint64_t count = COUNT_DEFAULT;
	char *ofname = NULL;
	*/

	if (1)
	fprintf(stdout,
		"test_dma_c2h_init:\n,dev %s, addr 0x%lx, aperture 0x%lx, size 0x%lx, offset 0x%lx, "
		"count %lu\n",
		devname, addr, aperture, size, offset, count);


	ssize_t rc = 0;
	size_t out_offset = 0;
	size_t bytes_done = 0;
	uint64_t i;
	uint64_t apt_loop = aperture ? (size + aperture - 1) / aperture : 0;
	char *buffer = NULL;
	char *allocated = NULL;
	struct timespec ts_start, ts_end;
	int out_fd = -1;
	int fpga_fd;
	long total_time = 0;
	float result;
	float avg_time = 0;
	int underflow = 0;

	/*
	 * use O_TRUNC to indicate to the driver to flush the data up based on
	 * EOP (end-of-packet), streaming mode only
	 */
	if (eop_flush)
		fpga_fd = open(devname, O_RDWR | O_TRUNC);
	else
		fpga_fd = open(devname, O_RDWR);

	if (fpga_fd < 0) {
                fprintf(stderr, "unable to open device %s, %d.\n",
                        devname, fpga_fd);
		perror("open device");
                return -EINVAL;
        }

	/* create file to write data to */
	if (ofname) {
		out_fd = open(ofname, O_RDWR | O_CREAT | O_TRUNC | O_SYNC,
				0666);
		if (out_fd < 0) {
                        fprintf(stderr, "unable to open output file %s, %d.\n",
                                ofname, out_fd);
			perror("open output file");
                        rc = -EINVAL;
                        goto out;
                }
	}

	posix_memalign((void **)&allocated, 4096 /*alignment */ , size + 4096);
	if (!allocated) {
		fprintf(stderr, "OOM %lu.\n", size + 4096);
		rc = -ENOMEM;
		goto out;
	}

	buffer = allocated + offset;
	if (verbose)
	fprintf(stdout, "host buffer 0x%lx, %p.\n", size + 4096, buffer);

	for (i = 0; i < count; i++) {
		rc = clock_gettime(CLOCK_MONOTONIC, &ts_start);
		if (apt_loop) {
			uint64_t j;
			uint64_t len = size;
			char *buf = buffer;

			bytes_done = 0;
			for (j = 0; j < apt_loop; j++, len -= aperture, buf += aperture) {
				uint64_t bytes = (len > aperture) ? aperture : len,
				rc = read_to_buffer(devname, fpga_fd, buf,
						bytes, addr);
				if (rc < 0)
					goto out;

				if (!underflow && rc < bytes)
					underflow = 1;
				bytes_done += rc;
			}
		} else {
			rc = read_to_buffer(devname, fpga_fd, buffer, size, addr);
			if (rc < 0)
				goto out;
			bytes_done = rc;

			if (!underflow && bytes_done < size)
				underflow = 1;
		}
		clock_gettime(CLOCK_MONOTONIC, &ts_end);


		/* subtract the start time from the end time */
		timespec_sub(&ts_end, &ts_start);
		total_time += ts_end.tv_nsec;
		/* a bit less accurate but side-effects are accounted for */
		if (verbose)
		fprintf(stdout,
			"#%lu: CLOCK_MONOTONIC %ld.%09ld sec. read %ld/%ld bytes\n",
			i, ts_end.tv_sec, ts_end.tv_nsec, bytes_done, size);

		/* file argument given? */
		if (out_fd >= 0) {

			fprintf(stdout, "c2h_write_from_buffer'para:devname:%s ,fpga_fd:%d ,buffer:%p ,size:%lx , addr:0x%lx\n",
							devname ,fpga_fd ,buffer,size,addr);
			rc = write_from_buffer(ofname, out_fd, buffer,
					 bytes_done, out_offset);
			if (rc < 0 || rc < bytes_done)
				goto out;
			out_offset += bytes_done;
		}
	}

	if (!underflow) {
		avg_time = (float)total_time/(float)count;
		result = ((float)size)*1000/avg_time;
		if (verbose)
			printf("** Avg time device %s, total time %ld nsec, avg_time = %f, size = %lu, BW = %f \n",
				devname, total_time, avg_time, size, result);
		printf("%s ** Average BW = %lu, %f\n", devname, size, result);
		printf("** Avg time device %s, total time %ld nsec, avg_time = %f, size = %lu, BW = %f \n",
				devname, total_time, avg_time, size, result);
		
		rc = 0;
	} else if (eop_flush) {
		/* allow underflow with -e option */
		rc = 0;
	} else 
		rc = -EIO;

out:
	close(fpga_fd);
	if (out_fd >= 0)
		close(out_fd);
	free(allocated);

	return rc;
}


static int test_dma_h2c(char *devname, uint64_t addr, uint64_t aperture,
		    uint64_t size, uint64_t offset, uint64_t count,
		    char *infname, char *ofname)
{
	/*
	int cmd_opt;
	char *device = DEVICE_NAME_DEFAULT;
	uint64_t address = 0;
	uint64_t aperture = 0;
	uint64_t size = SIZE_DEFAULT;
	uint64_t offset = 0;
	uint64_t count = COUNT_DEFAULT;
	char *infname = NULL;
	char *ofname = NULL;
	*/
	fprintf(stdout, 
		"test_dma_h2c_init ,dev %s, addr 0x%lx, aperture 0x%lx, size 0x%lx, offset 0x%lx, "
	        "count %lu\n",
		devname, addr, aperture, size, offset, count);
	
	uint64_t i;
	ssize_t rc;
	size_t bytes_done = 0;
	size_t out_offset = 0;
	uint64_t apt_loop = aperture ? (size + aperture - 1) / aperture : 0;
	char *buffer = NULL;
	char *allocated = NULL;
	struct timespec ts_start, ts_end;
	int infile_fd = -1;
	int outfile_fd = -1;
	int fpga_fd = open(devname, O_RDWR);
	long total_time = 0;
	float result;
	float avg_time = 0;
	int underflow = 0;

	if (fpga_fd < 0) {
		fprintf(stderr, "unable to open device %s, %d.\n",
			devname, fpga_fd);
		perror("open device");
		return -EINVAL;
	}

	if (infname) {
		infile_fd = open(infname, O_RDONLY);
		if (infile_fd < 0) {
			fprintf(stderr, "unable to open input file %s, %d.\n",
				infname, infile_fd);
			perror("open input file");
			rc = -EINVAL;
			goto out;
		}
	}

	if (ofname) {
		outfile_fd =
		    open(ofname, O_RDWR | O_CREAT | O_TRUNC | O_SYNC,
			 0666);
		if (outfile_fd < 0) {
			fprintf(stderr, "unable to open output file %s, %d.\n",
				ofname, outfile_fd);
			perror("open output file");
			rc = -EINVAL;
			goto out;
		}
	}

	posix_memalign((void **)&allocated, 4096 /*alignment */ , size + 4096);
	if (!allocated) {
		fprintf(stderr, "OOM %lu.\n", size + 4096);
		rc = -ENOMEM;
		goto out;
	}
	buffer = allocated + offset;
	if (verbose)
		fprintf(stdout, "host buffer 0x%lx = %p\n",
			size + 4096, buffer); 

	if (infile_fd >= 0) {
		rc = read_to_buffer(infname, infile_fd, buffer, size, 0);
		if (rc < 0 || rc < size)
			goto out;
	}



	for (i = 0; i < count; i++) {
		// write buffer to AXI MM address using SGDMA 
		rc = clock_gettime(CLOCK_MONOTONIC, &ts_start);

		if (apt_loop) {
			uint64_t j;
			uint64_t len = size;
		       		char *buf = buffer;

			bytes_done = 0;
			for (j = 0; j < apt_loop; j++, len -= aperture,
					buf += aperture) {
				uint64_t bytes = (len > aperture) ? aperture : len,
				rc = write_from_buffer(devname, fpga_fd, buf,
							bytes, addr);
				if (rc < 0)
					goto out;

				bytes_done += rc;
				if (!underflow && rc < bytes)
					underflow = 1;
			}
		} else {
			fprintf(stdout, "h2c_write_from_buffer'para:devname:%s ,fpga_fd:%d ,buffer:%p ,size:%lx , addr:0x%lx\n",
							devname ,fpga_fd ,buffer,size,addr);
			rc = write_from_buffer(devname, fpga_fd, buffer, size, addr);/////////////问题根源
			if (rc < 0)
				goto out;

			bytes_done = rc;
			if (!underflow && bytes_done < size)
				underflow = 1;
		}

		rc = clock_gettime(CLOCK_MONOTONIC, &ts_end);
		//subtract the start time from the end time 
		timespec_sub(&ts_end, &ts_start);
		total_time += ts_end.tv_nsec;
		// a bit less accurate but side-effects are accounted for 
		if (verbose)
		fprintf(stdout,
			"#%lu: CLOCK_MONOTONIC %ld.%09ld sec. write %ld bytes\n",
			i, ts_end.tv_sec, ts_end.tv_nsec, size); 
			
		if (outfile_fd >= 0) {
			rc = write_from_buffer(ofname, outfile_fd, buffer,
						 bytes_done, out_offset);
			if (rc < 0 || rc < bytes_done)
				goto out;
			out_offset += bytes_done;
		}
	}

	

	if (!underflow) {
		avg_time = (float)total_time/(float)count;
		result = ((float)size)*1000/avg_time;
		if (verbose)
			printf("** Avg time device %s, total time %ld nsec, avg_time = %f, size = %lu, BW = %f \n",
			devname, total_time, avg_time, size, result);
		printf("%s ** Average BW = %lu, %f\n", devname, size, result);
	}

out:
	close(fpga_fd);
	if (infile_fd >= 0)
		close(infile_fd);
	if (outfile_fd >= 0)
		close(outfile_fd);
	free(allocated);

	if (rc < 0)
		return rc;
	/* treat underflow as error */
	return underflow ? -EIO : 0;
}
