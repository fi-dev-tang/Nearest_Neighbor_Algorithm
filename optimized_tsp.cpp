#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <filesystem>
#include <cmath>
#include <omp.h>

namespace fs = std::filesystem;

struct City{
    int id;
    double x, y;

    // 直接使用 for(const auto&city : cities) 输出信息，发现格式错误
    // 修正输出错误，重载 << Operator
    friend std::ostream& operator<<(std::ostream& os, const City&city){
        std::ostringstream oss;
        oss << "City " << city.id << ": ("
            << std::fixed << std::setprecision(2) << city.x << ", "
            << std::fixed << std::setprecision(2) << city.y << ")";
        return os << oss.str();
    }
};


std::vector<City> readCityFromDataSet(std::ifstream& file){
    std::string line;
    std::vector<City> cities;
    bool alreadyUsefulDataSection = false; // 是否采集到真正有效的坐标数据

    while(std::getline(file, line)){
        if(!alreadyUsefulDataSection){
            std::istringstream iss(line);
            std::string keyword;
            iss >> keyword;

            if(keyword == "NODE_COORD_SECTION"){
                alreadyUsefulDataSection = true;
            }
            continue;
        }

        // 接下来是数字部分的读取过程
        std::istringstream dataStream(line);
        City city;

        if(dataStream >> city.id >> city.x >> city.y){
            cities.emplace_back(city);
        }
        else if(line == "EOF" || file.eof()){       // 不属于这个 if 条件的部分，则属于数字部分或者最后的 eof 读取过程
            break;
        }else {
            std::cerr << "Error: Can not parse line ---" << line << std::endl;
        }
    }
    return cities;
}

std::vector<City> insert_tsp_information(const std::string& filename){
    try{
        // 构建完整的文件路径
        std::string filepath = "dataSet/" + filename;

        std::cout << "Current working directory: " << fs::current_path() << std::endl;
        std::cout << "Reading file: " << filepath << std::endl;

        std::ifstream file(filepath);
        if(!file.is_open()){
            throw std::runtime_error("Could not open file: " + filepath);
        } 
        auto cities = readCityFromDataSet(file);
        file.close();

        std::cout << "Reading from tsp file: " << cities.size() << " cities." << std::endl;
        return cities;
    }catch(const std::exception& e){
        std::cerr << "Exception occurred: " << e.what() << std::endl;
        throw;
    }
}

/*
优化版本 version 1:
第一步想优化的是内存访问带宽，每秒钟可以传输的数据量。
影响内存访问带宽的主要有下面的三个因素:
1. 是否可以更好地利用访问的时间局部性和空间局部性
2. 是否可以降低 Cache miss
3. 是否可以更好地利用 Prefecting, 预取机制
需要更好的利用内存访问模式，这里的一个问题在于，每一次内层循环中都需要读取 cities[i] 和 cities[j]
而 cities[j] 是顺序、连续的访问, cities[i] 是不连续的访问，可以把 cities[i] 的读取提前到外层，由变量代替。

cities[i].x 和 cities[i].y 存储在局部变量中，由于局部变量通常位于CPU寄存器或靠近 CPU 的缓存中，访问速度更快。
在整个内循环期间，cities[i].x 和 cities[i].y 的值不变，更好地利用CPU缓存，减少 cache miss。

优化版本 version 2:
开启并行执行 openMP 和向量化(SIMD)

做法: 1. 添加 #pragma omp parallel for schedule(dynamic) 指令 
        外层循环可以并行化，动态调度策略，更好地平衡工作负载
      2. 添加 #pragma omp simd 指令
        内层循环启用向量化，编译器生成 SIMD 指令加速计算
*/
std::vector<std::vector<double>> calculate_distanceMatrix(std::vector<City> cities){
    // 1. 初始化要返回的距离矩阵
    std::vector<std::vector<double>> distanceMatrix(cities.size(), std::vector<double>(cities.size(), 0.0));
    
    #pragma omp parallel for schedule(dynamic)  // 外层 for 循环可以并行
    for(int i = 0; i < cities.size(); i++){
        double x_i = cities[i].x;
        double y_i = cities[i].y;

        #pragma omp simd                        // 内层使用单指令多数据
        for(int j = i + 1; j < cities.size(); j++){
            double x_diff = x_i - cities[j].x;
            double y_diff = y_i - cities[j].y;
            double distance = std::sqrt(x_diff * x_diff + y_diff * y_diff);
            distanceMatrix[i][j] = distance;
            distanceMatrix[j][i] = distance;
        }
    }
    return distanceMatrix;
}

void print_distanceMatrix(std::vector<std::vector<double>> &distanceMatrix){
    int n = distanceMatrix.size();
    int width = 8;                      // 设置小数的输出格式，两位小数点，以及正负号的空间

    // 打印列标题
    std::cout << "      ";
    for(int col = 0; col < n; col++){
        std::cout << std::setw(width) << col << "";
    }
    std::cout << std::endl;

    // 打印矩阵内容
    for(int row = 0; row < n; row++){
        std::cout << std::setw(3) << row << " "; // 打印行标记
        for(int col = 0; col < n; col++){
            std::cout << std::setw(width) << distanceMatrix[row][col] << " ";
        }
        std::cout << std::endl;
    }
}

double nearest_neighbour(std::vector<std::vector<double>>& distanceMatrix){
    // 1. 初始化
    int n = distanceMatrix.size();
    std::vector<bool> visited(n, false);
    std::vector<int> visited_cities_id;

    // 2. 设置起始点
    int startCity = 0;
    int currentCity = startCity;
    visited[startCity] = true;
    visited_cities_id.push_back(startCity + 1);             // city_id 实际在 .tsp 文件中，从 1 开始编号
    double totalDistance = 0.0;

    // 3. 主循环: 访问所有城市
    while(visited_cities_id.size() < n){
        double minimalDistance = std::numeric_limits<double>::max();
        int nextCity = -1;

        // 3.1 寻找最近的未访问城市
        for(int j = 0; j < n; j++){
            if(!visited[j] && distanceMatrix[currentCity][j] < minimalDistance){
                minimalDistance = distanceMatrix[currentCity][j];
                nextCity = j;
            }
        }

        // 3.2 更新路径和距离
        visited[nextCity] = true;
        visited_cities_id.push_back(nextCity + 1);
        totalDistance += minimalDistance;
        currentCity = nextCity;
    }

    // 4. 返回起点，完成回路
    totalDistance += distanceMatrix[currentCity][startCity];

    std::cout << "total distance: " ;
    return totalDistance;
}

int main(int argc, char* argv[]){
    if(argc != 2){
        std::cerr << "Usage: " << argv[0] << " <tsp filename>" << std::endl;
        std::cerr << "Example: " << argv[0] << " world.tsp" << std::endl;
        return 1;
    }

    try{
        auto cities = insert_tsp_information(argv[1]);
        auto distanceMatrix = calculate_distanceMatrix(cities);
        // print_distanceMatrix(distanceMatrix);                // 辅助测试输出的距离矩阵，实际上会是一张很大的表，针对 ch71009.tsp, 直接报 killed
        std::cout << nearest_neighbour(distanceMatrix) << std::endl;
        return 0;
    }catch(const std::exception& e){
        return 1;
    }
}