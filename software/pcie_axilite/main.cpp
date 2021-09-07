#include "pcie_fun.h"
#include <sys/stat.h>

unsigned int m_ddrpass=0;
#define FPGA_AXI_START_ADDR (0)




int main()
{
    
    if(pcie_init()<0)
    {
        printf("pcie_init error\n");
    }
    uint32_t val;
    unsigned int m_bar_failed=0;
    unsigned int m_bar_pass=0;
    unsigned int i=0;

    write_control(0,1000);
    val = read_control(i);
    printf("read_reg:%d\n",val);
    return 0;
 


}
