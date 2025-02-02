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

## 2. 比较直接地对外层循环加 omp 并行化，对内层循环加 SIMD(Singe Instruction Multiple Data)
使用lscpu 查看当前可用的 cpu 核心数是 40, 还是使用 33708 城市数据集
```bash
# 编译使用 openmp 进行并行化的程序
g++ -fopenmp -O3 -std=c++17 optimized_tsp.cpp -o optimized_tsp
# 开启 40 个核的运行命令
likwid-perfctr -C 0-39 -g MEM ./optimized_tsp bm33708.tsp
```
实验的结果如下:
```bash
tangyufei@dell-PowerEdge-R730:~/fi_github_repo/Nearest_Neighbor_Algorithm$ likwid-perfctr -C 0-39 -g MEM ./optimized_tsp bm33708.tsp
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
+-----------------------+---------+-------------+------------+------------+------------+------------+------------+------------+------------+------------+------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+
|         Event         | Counter |  HWThread 0 | HWThread 1 | HWThread 2 | HWThread 3 | HWThread 4 | HWThread 5 | HWThread 6 | HWThread 7 | HWThread 8 | HWThread 9 | HWThread 10 | HWThread 11 | HWThread 12 | HWThread 13 | HWThread 14 | HWThread 15 | HWThread 16 | HWThread 17 | HWThread 18 | HWThread 19 | HWThread 20 | HWThread 21 | HWThread 22 | HWThread 23 | HWThread 24 | HWThread 25 | HWThread 26 | HWThread 27 | HWThread 28 | HWThread 29 | HWThread 30 | HWThread 31 | HWThread 32 | HWThread 33 | HWThread 34 | HWThread 35 | HWThread 36 | HWThread 37 | HWThread 38 | HWThread 39 |
+-----------------------+---------+-------------+------------+------------+------------+------------+------------+------------+------------+------------+------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+
|   INSTR_RETIRED_ANY   |  FIXC0  | 20679149634 | 2175012031 | 2960545124 | 4548185178 |  620015327 | 1409598598 | 2526309567 |  374536124 |  414608507 |  465194446 |   487763570 |  2750578574 |   564356290 | 16657430284 |  6231524474 |  4228434678 |   487733246 |   612770729 |   606760429 |   478180640 |   396948312 |   509354742 |   596123999 |  1435311080 |   391168197 |   700730869 |  1438948101 |   653673951 |   419094451 |   457406761 |   413089716 |  2959264646 |   421919117 |  6863566139 |   467784899 |  4511414757 |   573382907 |   694056954 |  2659992536 |  1319242235 |
| CPU_CLK_UNHALTED_CORE |  FIXC1  | 16373592699 | 4065557999 | 5178571276 | 5744613964 | 3387438350 | 3773037834 | 4843346211 | 3274087019 | 3229577110 | 3237885972 |  6939591634 |  4291415571 |  3327349334 | 12879266374 |  4627389191 |  5702679617 |  3217045148 |  3333923447 |  3481294708 |  3319845435 |  3381574142 |  3291440003 |  3446393336 |  3300592736 |  3193664456 |  3369200931 |  3705567968 |  3446558179 |  3229735409 |  3130996870 |  3367565039 |  4529886353 |  3181636956 |  9053333563 |  3246672832 |  7469226594 |  3280988607 |  3414822086 |  4760782760 |  3754498802 |
|  CPU_CLK_UNHALTED_REF |  FIXC2  | 13563087072 | 4307308080 | 5372965824 | 5346890112 | 3628563432 | 3922672248 | 4921475448 | 3409922088 | 3428417640 | 3439024632 |  6203085144 |  4462606368 |  3601427616 | 10537559208 |  4614568584 |  6181373784 |  3442450536 |  3503819040 |  3566161488 |  3459200616 |  3381979680 |  3455121384 |  3573526272 |  3365193168 |  3373449360 |  3506061816 |  3819294312 |  3578388672 |  3409512840 |  3285157296 |  3385206696 |  4931298624 |  3424176864 |  7933755312 |  3438176832 |  7309187472 |  3579974592 |  3671469984 |  5211257688 |  3935468880 |
|      CAS_COUNT_RD     | MBOX0C0 |    74057437 |   56723819 |          0 |          0 |          0 |          0 |          0 |          0 |          0 |          0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |
|      CAS_COUNT_WR     | MBOX0C1 |    78039769 |   58555760 |          0 |          0 |          0 |          0 |          0 |          0 |          0 |          0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |
|      CAS_COUNT_RD     | MBOX1C0 |    73906153 |   56690997 |          0 |          0 |          0 |          0 |          0 |          0 |          0 |          0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |
|      CAS_COUNT_WR     | MBOX1C1 |    77946725 |   58540655 |          0 |          0 |          0 |          0 |          0 |          0 |          0 |          0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |
|      CAS_COUNT_RD     | MBOX2C0 |    72445654 |   58149089 |          0 |          0 |          0 |          0 |          0 |          0 |          0 |          0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |
|      CAS_COUNT_WR     | MBOX2C1 |    76506209 |   59994007 |          0 |          0 |          0 |          0 |          0 |          0 |          0 |          0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |
|      CAS_COUNT_RD     | MBOX3C0 |    72516984 |   56858387 |          0 |          0 |          0 |          0 |          0 |          0 |          0 |          0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |
|      CAS_COUNT_WR     | MBOX3C1 |    76559447 |   58661600 |          0 |          0 |          0 |          0 |          0 |          0 |          0 |          0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |
|      CAS_COUNT_RD     | MBOX4C0 |      -      |      -     |      -     |      -     |      -     |      -     |      -     |      -     |      -     |      -     |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |
|      CAS_COUNT_WR     | MBOX4C1 |      -      |      -     |      -     |      -     |      -     |      -     |      -     |      -     |      -     |      -     |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |
|      CAS_COUNT_RD     | MBOX5C0 |      -      |      -     |      -     |      -     |      -     |      -     |      -     |      -     |      -     |      -     |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |
|      CAS_COUNT_WR     | MBOX5C1 |      -      |      -     |      -     |      -     |      -     |      -     |      -     |      -     |      -     |      -     |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |
|      CAS_COUNT_RD     | MBOX6C0 |           0 |          0 |          0 |          0 |          0 |          0 |          0 |          0 |          0 |          0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |
|      CAS_COUNT_WR     | MBOX6C1 |           0 |          0 |          0 |          0 |          0 |          0 |          0 |          0 |          0 |          0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |
|      CAS_COUNT_RD     | MBOX7C0 |      -      |      -     |      -     |      -     |      -     |      -     |      -     |      -     |      -     |      -     |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |
|      CAS_COUNT_WR     | MBOX7C1 |      -      |      -     |      -     |      -     |      -     |      -     |      -     |      -     |      -     |      -     |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |      -      |
+-----------------------+---------+-------------+------------+------------+------------+------------+------------+------------+------------+------------+------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+

+----------------------------+---------+--------------+------------+-------------+--------------+
|            Event           | Counter |      Sum     |     Min    |     Max     |      Avg     |
+----------------------------+---------+--------------+------------+-------------+--------------+
|   INSTR_RETIRED_ANY STAT   |  FIXC0  |  97161161819 |  374536124 | 20679149634 | 2.429029e+09 |
| CPU_CLK_UNHALTED_CORE STAT |  FIXC1  | 183782646515 | 3130996870 | 16373592699 | 4.594566e+09 |
|  CPU_CLK_UNHALTED_REF STAT |  FIXC2  | 182480236704 | 3285157296 | 13563087072 | 4.562006e+09 |
|      CAS_COUNT_RD STAT     | MBOX0C0 |    130781256 |          0 |    74057437 | 3.269531e+06 |
|      CAS_COUNT_WR STAT     | MBOX0C1 |    136595529 |          0 |    78039769 | 3.414888e+06 |
|      CAS_COUNT_RD STAT     | MBOX1C0 |    130597150 |          0 |    73906153 | 3.264929e+06 |
|      CAS_COUNT_WR STAT     | MBOX1C1 |    136487380 |          0 |    77946725 | 3.412184e+06 |
|      CAS_COUNT_RD STAT     | MBOX2C0 |    130594743 |          0 |    72445654 | 3.264869e+06 |
|      CAS_COUNT_WR STAT     | MBOX2C1 |    136500216 |          0 |    76506209 | 3.412505e+06 |
|      CAS_COUNT_RD STAT     | MBOX3C0 |    129375371 |          0 |    72516984 | 3.234384e+06 |
|      CAS_COUNT_WR STAT     | MBOX3C1 |    135221047 |          0 |    76559447 | 3.380526e+06 |
|      CAS_COUNT_RD STAT     | MBOX4C0 |            0 |     inf    |           0 |            0 |
|      CAS_COUNT_WR STAT     | MBOX4C1 |            0 |     inf    |           0 |            0 |
|      CAS_COUNT_RD STAT     | MBOX5C0 |            0 |     inf    |           0 |            0 |
|      CAS_COUNT_WR STAT     | MBOX5C1 |            0 |     inf    |           0 |            0 |
|      CAS_COUNT_RD STAT     | MBOX6C0 |            0 |          0 |           0 |            0 |
|      CAS_COUNT_WR STAT     | MBOX6C1 |            0 |          0 |           0 |            0 |
|      CAS_COUNT_RD STAT     | MBOX7C0 |            0 |     inf    |           0 |            0 |
|      CAS_COUNT_WR STAT     | MBOX7C1 |            0 |     inf    |           0 |            0 |
+----------------------------+---------+--------------+------------+-------------+--------------+

+-----------------------------------+------------+------------+------------+------------+------------+------------+------------+------------+------------+------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+
|               Metric              | HWThread 0 | HWThread 1 | HWThread 2 | HWThread 3 | HWThread 4 | HWThread 5 | HWThread 6 | HWThread 7 | HWThread 8 | HWThread 9 | HWThread 10 | HWThread 11 | HWThread 12 | HWThread 13 | HWThread 14 | HWThread 15 | HWThread 16 | HWThread 17 | HWThread 18 | HWThread 19 | HWThread 20 | HWThread 21 | HWThread 22 | HWThread 23 | HWThread 24 | HWThread 25 | HWThread 26 | HWThread 27 | HWThread 28 | HWThread 29 | HWThread 30 | HWThread 31 | HWThread 32 | HWThread 33 | HWThread 34 | HWThread 35 | HWThread 36 | HWThread 37 | HWThread 38 | HWThread 39 |
+-----------------------------------+------------+------------+------------+------------+------------+------------+------------+------------+------------+------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+
|        Runtime (RDTSC) [s]        |    10.8829 |    10.8829 |    10.8829 |    10.8829 |    10.8829 |    10.8829 |    10.8829 |    10.8829 |    10.8829 |    10.8829 |     10.8829 |     10.8829 |     10.8829 |     10.8829 |     10.8829 |     10.8829 |     10.8829 |     10.8829 |     10.8829 |     10.8829 |     10.8829 |     10.8829 |     10.8829 |     10.8829 |     10.8829 |     10.8829 |     10.8829 |     10.8829 |     10.8829 |     10.8829 |     10.8829 |     10.8829 |     10.8829 |     10.8829 |     10.8829 |     10.8829 |     10.8829 |     10.8829 |     10.8829 |     10.8829 |
|        Runtime unhalted [s]       |     6.8225 |     1.6940 |     2.1578 |     2.3936 |     1.4115 |     1.5721 |     2.0181 |     1.3642 |     1.3457 |     1.3491 |      2.8916 |      1.7881 |      1.3864 |      5.3665 |      1.9281 |      2.3762 |      1.3405 |      1.3892 |      1.4506 |      1.3833 |      1.4090 |      1.3715 |      1.4360 |      1.3753 |      1.3307 |      1.4039 |      1.5440 |      1.4361 |      1.3458 |      1.3046 |      1.4032 |      1.8875 |      1.3257 |      3.7723 |      1.3528 |      3.1122 |      1.3671 |      1.4229 |      1.9837 |      1.5644 |
|            Clock [MHz]            |  2897.2556 |  2265.2472 |  2313.1151 |  2578.4633 |  2240.4644 |  2308.3970 |  2361.8459 |  2304.3430 |  2260.7540 |  2259.5795 |   2684.8965 |   2307.8807 |   2217.3032 |   2933.2729 |   2406.6132 |   2214.0903 |   2242.8014 |   2283.5752 |   2342.8320 |   2303.2628 |   2399.6577 |   2286.2515 |   2314.5642 |   2353.8745 |   2272.0426 |   2306.2624 |   2328.4828 |   2311.5297 |   2273.4007 |   2287.3248 |   2387.4384 |   2204.5877 |   2229.9535 |   2738.6157 |   2266.2702 |   2452.4937 |   2199.5111 |   2232.1813 |   2192.4878 |   2289.5855 |
|                CPI                |     0.7918 |     1.8692 |     1.7492 |     1.2631 |     5.4635 |     2.6767 |     1.9172 |     8.7417 |     7.7895 |     6.9603 |     14.2274 |      1.5602 |      5.8958 |      0.7732 |      0.7426 |      1.3487 |      6.5959 |      5.4407 |      5.7375 |      6.9427 |      8.5189 |      6.4620 |      5.7813 |      2.2996 |      8.1644 |      4.8081 |      2.5752 |      5.2726 |      7.7065 |      6.8451 |      8.1521 |      1.5307 |      7.5409 |      1.3190 |      6.9405 |      1.6556 |      5.7222 |      4.9201 |      1.7898 |      2.8460 |
|  Memory read bandwidth [MBytes/s] |  1722.6354 |  1343.3018 |          0 |          0 |          0 |          0 |          0 |          0 |          0 |          0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |
|  Memory read data volume [GBytes] |    18.7473 |    14.6190 |          0 |          0 |          0 |          0 |          0 |          0 |          0 |          0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |
| Memory write bandwidth [MBytes/s] |  1817.4685 |  1386.4064 |          0 |          0 |          0 |          0 |          0 |          0 |          0 |          0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |
| Memory write data volume [GBytes] |    19.7793 |    15.0881 |          0 |          0 |          0 |          0 |          0 |          0 |          0 |          0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |
|    Memory bandwidth [MBytes/s]    |  3540.1039 |  2729.7082 |          0 |          0 |          0 |          0 |          0 |          0 |          0 |          0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |
|    Memory data volume [GBytes]    |    38.5266 |    29.7072 |          0 |          0 |          0 |          0 |          0 |          0 |          0 |          0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |           0 |
+-----------------------------------+------------+------------+------------+------------+------------+------------+------------+------------+------------+------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+

+----------------------------------------+------------+-----------+-----------+-----------+
|                 Metric                 |     Sum    |    Min    |    Max    |    Avg    |
+----------------------------------------+------------+-----------+-----------+-----------+
|        Runtime (RDTSC) [s] STAT        |   435.3160 |   10.8829 |   10.8829 |   10.8829 |
|        Runtime unhalted [s] STAT       |    76.5778 |    1.3046 |    6.8225 |    1.9144 |
|            Clock [MHz] STAT            | 94052.5090 | 2192.4878 | 2933.2729 | 2351.3127 |
|                CPI STAT                |   189.3375 |    0.7426 |   14.2274 |    4.7334 |
|  Memory read bandwidth [MBytes/s] STAT |  3065.9372 |         0 | 1722.6354 |   76.6484 |
|  Memory read data volume [GBytes] STAT |    33.3663 |         0 |   18.7473 |    0.8342 |
| Memory write bandwidth [MBytes/s] STAT |  3203.8749 |         0 | 1817.4685 |   80.0969 |
| Memory write data volume [GBytes] STAT |    34.8674 |         0 |   19.7793 |    0.8717 |
|    Memory bandwidth [MBytes/s] STAT    |  6269.8121 |         0 | 3540.1039 |  156.7453 |
|    Memory data volume [GBytes] STAT    |    68.2338 |         0 |   38.5266 |    1.7058 |
+----------------------------------------+------------+-----------+-----------+-----------+
```

### omp 和 simd 的优化效果
![optimized_2_omp](/picture/opimized_way2/omp_2_0.png)

可以看到，在 40 核的优化下，运行时间提升了 10.4717 倍，可以认为是 10 倍的加速比。

