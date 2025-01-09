/*
base 版本的数据处理:
version 1. define readCityFromDataSet()
把 .tsp 格式的文件读入 std::vector<City> 中，按格式存储

数据集采集自官网:
https://www.math.uwaterloo.ca/tsp/world/countries.html

其中格式为 tsp 类型，定义了问题的节点数，EDGE_WEIGHT_TYPE 表示边的权重是基于欧几里得距离计算的，适用于二维平面的点
NODE_COORD_SECTION
跟着各个节点的具体坐标，表示节点数目，x 坐标和 y 坐标

目前该版本的实现，将数据集中的城市距离 x 和 y 记录进数据结构中进行存储
*/
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <filesystem>
#include <cmath>

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

//=================================== 1. 数据集读取部分 =======================================
/*
对数据集读取部分作一个修改，正确性上无影响，
实际上可以针对待读取数据 .tsp 文件的特定格式，前面是文字部分，后面是正式的数字部分，设置的 while 循环中可以一上来先判断是否是文字部分
好处是: 省略了数字部分还要多余增加 std::istringstream iss(line) 和 std::string keyword; iss >> keyword; 的表述
*/
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

        // 不属于这个 if 条件的部分，则属于数字部分或者最后的 eof 读取过程
        if(line == "EOF" || file.eof()){
            break;
        }

        // 接下来是数字部分的读取过程
        std::istringstream dataStream(line);
        City city;

        if(dataStream >> city.id >> city.x >> city.y){
            cities.emplace_back(city);
        }else{
            std::cerr << "Error: Can not parse line ---" << line << std::endl;
        }
    }
    return cities;
}

std::vector<City> insert_tsp_information(){
    try{
        // 这里使用相对路径来读取 world.tsp 等格式
        // 首先显示一个当前的工作路径，读取的数据集在子文件夹 dataSet 中
        std::cout << "Current working directory: " << fs::current_path() << std::endl;
        std::ifstream file("dataSet/dj38.tsp");  // 传递已经打开的文件流

        if(!file.is_open()){
            throw std::runtime_error("Could not open file: dataSet/dj38.tsp");
        }

        auto cities = readCityFromDataSet(file);

        file.close();

        // for(const auto&city: cities){
        //     std::cout << city << std::endl;
        // }
        std::cout << "Read " << cities.size() << " cities from the TSP file." << std::endl;
        return cities;
    }catch(const std::exception& e){
        std::cerr << "Exception occurred: " << e.what() << std::endl;
        return {};
    }
}
//===========================================2. 创建距离矩阵 ===================================================
/*
base 版本的算法部分: 
首先构建距离矩阵(base 版本采取最直观的思路,后续在优化的时候加入新的加速计算方法)
version 2 对 version 1 提出了一个修改，希望传递这个 cities 数组, 对抛出的异常进行处理
version 2. 构建当前 City 数组对应的距离矩阵
传入的参数是 City 数组，返回一个距离矩阵 distanceMatrix, 其中 distanceMatrix[i][j] 表示 City[i] 和 City[j] 之间的距离
观察到执行过程中，对于 ch71009.tsp 数据集，base 测试时执行打印距离矩阵，结果直接报 killed
*/
std::vector<std::vector<double>> calculate_distanceMatrix(std::vector<City> cities){
    // 1. 初始化要返回的距离矩阵
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
//==================================================================================================
/*
version 3: 正式的最近邻算法
算法的流程是:
1. 构建距离矩阵
2. 选择起始城市，初始化路径长度
3. 迭代
    3.1 从当前城市出发，找到距离最近的未访问城市。
    3.2 将最近的未访问城市添加到路径中，并将路径长度增加上从当前城市到最近城市的距离。
    3.3 将最近的未访问城市标记为已访问
4. 回到起始城市: 在所有城市都被访问后，路径的最后一个城市设置为起始城市，形成一条回路
5. 计算路径长度
*/
//===========================================3. 算法的主要部分===========================================
double nearest_neighbour(std::vector<std::vector<double>>& distanceMatrix){
    // 1. 设置城市是否被访问的情况
    int n = distanceMatrix.size();
    std::vector<bool> visited(n, false);
    std::vector<int> visited_cities_id;
    visited[0] = true;  // 从第一个城市开始
    
    int startCity = 0;
    int currentCity = startCity;
    double totalDistance = 0.0;

    while(true){
        double minimalDistance = std::numeric_limits<double>::max();
        // 设置第一个 for 循环找到最小距离
        for(int col = 0; col < n; col++){
            if(visited[col] == false && distanceMatrix[currentCity][col] < minimalDistance){
                minimalDistance = distanceMatrix[currentCity][col];
            }
        }
        // 设置第二个 for 循环找到最小距离对应的 Cities(但实际上如果距离没有重复相等的情况下，实际不需要两个 for 循环)
        for(int col = 0; col < n; col++){
            if(visited[col] == false && distanceMatrix[currentCity][col] == minimalDistance){
                visited[col] = true;
                visited_cities_id.push_back(col);
                totalDistance += minimalDistance;
                currentCity = col;
                break;
            }
        }

        // 所有城市都被访问后，跳出循环
        if(visited_cities_id.size() == n - 1){
            // 设置一个 debug 的打印功能
            break;
        }
    }
    // 设置一个 debug 的打印功能
    std::cout << "Visited cities: ";
    for(const auto&city_id: visited_cities_id){
        std::cout << city_id << " ";
    } 
    std::cout << std::endl;
    return totalDistance;
}

int main(){
    auto cities = insert_tsp_information();
    auto distanceMatrix = calculate_distanceMatrix(cities);
    // print_distanceMatrix(distanceMatrix);                // 辅助测试输出的距离矩阵，实际上会是一张很大的表，针对 ch71009.tsp, 直接报 killed
    std::cout << "total distance: " << nearest_neighbour(distanceMatrix) << std::endl;
    return 0;
}