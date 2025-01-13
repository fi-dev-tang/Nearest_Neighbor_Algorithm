# 优化步骤的实现对比

## 1. 使用局部变量优化内存访问，尽可能利用内存的空间局部性和 cache prefect 预取

测试集使用 bm33708.tsp, 一共有 33708 个城市

### 实现命令:
```Bash
# 先使用 -O0 的编译选项，【psNote】建议使用 gcc 11 及以上的版本进行编译，物理机自带的 gcc 9 版本略有点老
g++ -g -O0 optimized_tsp.cpp -o optimized_tsp
g++ -g -O0 base_tsp.cpp -o base_tsp 

# 使用 likwid 的命令，这里第一步尚未实现多进程或多线程
# 1. likwid-perfctr: 调用 likwid 的性能计数器模块
# 2. -C 0: 指定使用 CPU 核心 0 来执行下列命令，特定核的性能
# 3. -g MEM: 指定 MEM 性能组，感兴趣的是内存相关的硬件时间
likwid-perfctr -C 0 -g MEM ./base_tsp bm33708.tsp
likwid-perfctr -C 0 -g MEM ./optimized_tsp bm33708.tsp
```
#### 实验结果
```bash
tangyufei@dell-PowerEdge-R730:~/fi_github_repo/Nearest_Neighbor_Algorithm$ likwid-perfctr -C 0 -g MEM ./base_tsp bm33708.tsp
--------------------------------------------------------------------------------
CPU name:       Intel(R) Xeon(R) CPU E5-2640 v4 @ 2.40GHz
CPU type:       Intel Xeon Broadwell EN/EP/EX processor
CPU clock:      2.40 GHz
--------------------------------------------------------------------------------
Current working directory: "/home/tangyufei/fi_github_repo/Nearest_Neighbor_Algorithm"
Reading file: dataSet/bm33708.tsp
Error: Can not parse line ---EOF
Reading from tsp file: 33708 cities.
total distance: 1.20341e+06
--------------------------------------------------------------------------------
Group 1: MEM
+-----------------------+---------+--------------+
|         Event         | Counter |  HWThread 0  |
+-----------------------+---------+--------------+
|   INSTR_RETIRED_ANY   |  FIXC0  | 454421715271 |
| CPU_CLK_UNHALTED_CORE |  FIXC1  | 303936872339 |
|  CPU_CLK_UNHALTED_REF |  FIXC2  | 227824724520 |
|      CAS_COUNT_RD     | MBOX0C0 |    138706163 |
|      CAS_COUNT_WR     | MBOX0C1 |     96217372 |
|      CAS_COUNT_RD     | MBOX1C0 |    143371367 |
|      CAS_COUNT_WR     | MBOX1C1 |    100910946 |
|      CAS_COUNT_RD     | MBOX2C0 |    153623122 |
|      CAS_COUNT_WR     | MBOX2C1 |    111046111 |
|      CAS_COUNT_RD     | MBOX3C0 |    139019707 |
|      CAS_COUNT_WR     | MBOX3C1 |     96542974 |
|      CAS_COUNT_RD     | MBOX4C0 |       -      |
|      CAS_COUNT_WR     | MBOX4C1 |       -      |
|      CAS_COUNT_RD     | MBOX5C0 |       -      |
|      CAS_COUNT_WR     | MBOX5C1 |       -      |
|      CAS_COUNT_RD     | MBOX6C0 |            0 |
|      CAS_COUNT_WR     | MBOX6C1 |            0 |
|      CAS_COUNT_RD     | MBOX7C0 |       -      |
|      CAS_COUNT_WR     | MBOX7C1 |       -      |
+-----------------------+---------+--------------+

+-----------------------------------+------------+
|               Metric              | HWThread 0 |
+-----------------------------------+------------+
|        Runtime (RDTSC) [s]        |   103.9265 |
|        Runtime unhalted [s]       |   126.6427 |
|            Clock [MHz]            |  3201.7367 |
|                CPI                |     0.6688 |
|  Memory read bandwidth [MBytes/s] |   353.9243 |
|  Memory read data volume [GBytes] |    36.7821 |
| Memory write bandwidth [MBytes/s] |   249.2331 |
| Memory write data volume [GBytes] |    25.9019 |
|    Memory bandwidth [MBytes/s]    |   603.1574 |
|    Memory data volume [GBytes]    |    62.6840 |
+-----------------------------------+------------+

tangyufei@dell-PowerEdge-R730:~/fi_github_repo/Nearest_Neighbor_Algorithm$ likwid-perfctr -C 0 -g MEM ./optimized_tsp bm33708.tsp
--------------------------------------------------------------------------------
CPU name:       Intel(R) Xeon(R) CPU E5-2640 v4 @ 2.40GHz
CPU type:       Intel Xeon Broadwell EN/EP/EX processor
CPU clock:      2.40 GHz
--------------------------------------------------------------------------------
Current working directory: "/home/tangyufei/fi_github_repo/Nearest_Neighbor_Algorithm"
Reading file: dataSet/bm33708.tsp
Error: Can not parse line ---EOF
Reading from tsp file: 33708 cities.
total distance: 1.20341e+06
--------------------------------------------------------------------------------
Group 1: MEM
+-----------------------+---------+--------------+
|         Event         | Counter |  HWThread 0  |
+-----------------------+---------+--------------+
|   INSTR_RETIRED_ANY   |  FIXC0  | 428289635495 |
| CPU_CLK_UNHALTED_CORE |  FIXC1  | 274185760330 |
|  CPU_CLK_UNHALTED_REF |  FIXC2  | 199596389688 |
|      CAS_COUNT_RD     | MBOX0C0 |    136150224 |
|      CAS_COUNT_WR     | MBOX0C1 |    104026648 |
|      CAS_COUNT_RD     | MBOX1C0 |    140000074 |
|      CAS_COUNT_WR     | MBOX1C1 |    107921102 |
|      CAS_COUNT_RD     | MBOX2C0 |    147801241 |
|      CAS_COUNT_WR     | MBOX2C1 |    115780315 |
|      CAS_COUNT_RD     | MBOX3C0 |    136277993 |
|      CAS_COUNT_WR     | MBOX3C1 |    104196921 |
|      CAS_COUNT_RD     | MBOX4C0 |       -      |
|      CAS_COUNT_WR     | MBOX4C1 |       -      |
|      CAS_COUNT_RD     | MBOX5C0 |       -      |
|      CAS_COUNT_WR     | MBOX5C1 |       -      |
|      CAS_COUNT_RD     | MBOX6C0 |            0 |
|      CAS_COUNT_WR     | MBOX6C1 |            0 |
|      CAS_COUNT_RD     | MBOX7C0 |       -      |
|      CAS_COUNT_WR     | MBOX7C1 |       -      |
+-----------------------+---------+--------------+

+-----------------------------------+------------+
|               Metric              | HWThread 0 |
+-----------------------------------+------------+
|        Runtime (RDTSC) [s]        |    91.0037 |
|        Runtime unhalted [s]       |   114.2460 |
|            Clock [MHz]            |  3296.8263 |
|                CPI                |     0.6402 |
|  Memory read bandwidth [MBytes/s] |   393.9914 |
|  Memory read data volume [GBytes] |    35.8547 |
| Memory write bandwidth [MBytes/s] |   303.7590 |
| Memory write data volume [GBytes] |    27.6432 |
|    Memory bandwidth [MBytes/s]    |   697.7504 |
|    Memory data volume [GBytes]    |    63.4979 |
+-----------------------------------+------------+

```
对于未优化的性能和优化后的性能比较:
##### 未优化
![optimized_1_base](/picture/optimized_way1/optimized_1_base.png)

##### 优化
![optimized_1_optimized](/picture/optimized_way1/optimized_1_optimized.png)

从下面的几个项中，可以看到性能提升:

1. 程序执行时间 Runtime(RDTSC)
读取时间戳计数器(RDTSC)，程序执行时间 103.9265s -> 91.0037s

2. CPU 核心在未停止状态下执行指令的时间 Runtime unhalted
优化版本 126.6427s -> 114.2460s

3. CLock 时钟频率
优化版本的时钟频率 3201.7367 MHz -> 3296.8263 MHz 
更高效的代码允许CPU更多地处于高性能状态

4. CPI(Cycles Per Instruction):
每条指令需要的时钟周期数，CPI较低意味着更高的指令执行效率。
0.6688 -> 0.6402

5. Memory bandwidth
总内存带宽和总数居交换量
603.1574 MBytes/s -> 697.7504 MBytes/s

