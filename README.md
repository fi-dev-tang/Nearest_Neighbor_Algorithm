# Nearest_Neighbor_Algorithm
最近邻算法求解 TSP 问题，采取常规的优化思路，对比提升的性能

TSP 的数据集采集自:
https://www.math.uwaterloo.ca/tsp/world/countries.html

比较直接的增量式开发

#### 1. 增加从 .tsp 格式的文件中读取(x,y) 坐标到 std::vector<City> 中

#### 2. 将对应的(x,y) 坐标读入 std::vector<City> 之后，创建二维的距离矩阵，类型是 std::vector<std::vector<double>>
这个距离矩阵的计算很直接，计算上半矩阵的距离值，然后对称映射到下半矩阵。
距离计算过程的步骤形如:
```cpp
std::vector<std::vector<double>> distanceMatrix(cities.size(), std::vector<double>(cities.size(), 0.0));
for(int i = 0; i < cities.size(); i++){
    for(int j = i + 1; j < cities.size(); j++){
        double x_diff = cities[i].x - cities[j].x;
        double y_diff = cities[i].y - cities[j].y;
        double distance = std::sqrt(x_diff * x_diff + y_diff * y_diff);
        distanceMatrix[i][j] = distance;
        distanceMatrix[j][i] = distance;
    }
}
```
这个步骤对于 ch71009.tsp 的测试集来说，运行结果直接报 killed

![71009.jsp错误](base_error.png)

### 分析 v0.0: 关于 ch71009.tsp 运行报 killed 的原因
> 内存不足
对于每个城市对之间的距离进行计算，存储 double 类型的值。存储一个完整的距离矩阵，对于 71009 个城市来说，
所需的空间是 71009 * 71009 * 8(double 值占用的空间)。
所需要的总内存大约是 (71009 * 71009 * 8) / （1024 * 1024 * 1024）GB = 37.5679 GB
大约需要 38 GB 的内存仅仅用于存储这个矩阵。
观察对应系统的 RAM, 使用
```bash
free -h
```
使用该命令显示系统内存的使用情况，包括物理内存(RAM)和交换空间(Swap)。
这里显示我的总物理内存大小为 31Gi, 问题出在内存空间不足。