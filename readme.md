# PCIE_repo：

document：PCIE通路的相关教程与手册

driver：PCIE通路的linux驱动

software：针对不同模式下的PCIE上位机，目前包括：AXI4-ST AXI4-MM AXI4-LITE

## 文档：document

米联客官方PCIE教程：linux版与windows版

xilinx官方xdma—ip手册：中文版与英文版



## 驱动与测试：driver

首先工程烧入板卡，重新启动ubuntu系统
进入驱动目录 dma_ip_drivers-master/XDMA/linux-kernel/
驱动初始化
1.进入 xdma    

```
make clean
```

2.进入 xdma    

```
make
```

3.进入 tools   

```
make
```

进入root权限**（xdma的使用必须在root权限下操作）**
加载驱动 进入tests  

```
./load_driver.sh
```

测试通路 进入tests  

```
./run_test.sh
```



## 上位机：software

将data.zip压缩包解压后放在不同目录/build目录内

分别在build目录下执行 

```
cmake .. 
make
```

生成可执行文件并执行



