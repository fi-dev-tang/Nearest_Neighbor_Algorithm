# Nearest_Neighbor_Algorithm
最近邻算法求解 TSP 问题，采取常规的优化思路，对比提升的性能

TSP 的数据集采集自:
https://www.math.uwaterloo.ca/tsp/world/countries.html

比较直接的增量式开发

#### 1. 增加从 .tsp 格式的文件中读取(x,y) 坐标到 std::vector<City> 中
