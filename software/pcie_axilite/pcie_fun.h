#ifdef __cplusplus
 extern "C" {
 #endif
#include <assert.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdarg.h>
#include <syslog.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/sysinfo.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
/* ltoh: little to host */
/* htol: little to host */
#if __BYTE_ORDER == __LITTLE_ENDIAN
#  define ltohl(x)       (x)
#  define ltohs(x)       (x)
#  define htoll(x)       (x)
#  define htols(x)       (x)
#elif __BYTE_ORDER == __BIG_ENDIAN
#  define ltohl(x)     __bswap_32(x)
#  define ltohs(x)     __bswap_16(x)
#  define htoll(x)     __bswap_32(x)
#  define htols(x)     __bswap_16(x)
#endif

#define MAP_SIZE (4*32UL)
#define MAP_MASK (MAP_SIZE - 1)

#define FPGA_AXI_START_ADDR (0)

void *control_base;
int control_fd;
int c2h_dma_fd;
int h2c_dma_fd;

static unsigned int h2c_fpga_ddr_addr;
static unsigned int c2h_fpga_ddr_addr;


static int open_control(char *filename)
{
    int fd;
    fd = open(filename, O_RDWR | O_SYNC);
    if(fd == -1)
    {
        printf("open control error\n");
        return -1;
    }
    return fd;
}
static void *mmap_control(int fd,long mapsize)
{
    void *vir_addr;
    vir_addr = mmap(0, mapsize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    return vir_addr;
}
void write_control(int offset,uint32_t val)
{
    uint32_t writeval = htoll(val);
    *((uint32_t *)(control_base+offset)) = writeval;
}
uint32_t read_control(int offset)
{
    uint32_t read_result = *((uint32_t *)(control_base+offset));
    read_result = ltohl(read_result);
    return read_result;
}


void put_data_to_fpga_ddr(unsigned int fpga_ddr_addr,unsigned char *buffer,unsigned int len)
{
    lseek(h2c_dma_fd,fpga_ddr_addr,SEEK_SET);
    write(h2c_dma_fd,buffer,len);
}
void get_data_from_fpga_ddr(unsigned int fpga_ddr_addr,unsigned char *buffer,unsigned int len)
{
    lseek(c2h_dma_fd,fpga_ddr_addr,SEEK_SET);
    read(c2h_dma_fd,buffer,len);
}

int pcie_init()
{
    c2h_dma_fd = open("/dev/xdma0_c2h_0",O_RDWR | O_NONBLOCK);
    if(c2h_dma_fd < 0)
    {    
        printf("c2h");
        return -1;
    }
    h2c_dma_fd = open("/dev/xdma0_h2c_0",O_RDWR );
    if(h2c_dma_fd < 0)
    {    
        printf("h2c");
        return -2;
    }
      control_fd = open_control("/dev/xdma0_user");
    if(control_fd < 0)
    {
        printf("control");
        return -5;
    }    
    control_base = mmap_control(control_fd,MAP_SIZE);
    printf("base");
    return 1;
}
void pcie_deinit()
{
    close(c2h_dma_fd);
    close(h2c_dma_fd);
    close(control_fd);
}


#ifdef __cplusplus
}
#endif
