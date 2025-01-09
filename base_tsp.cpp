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
std::vector<City> readCityFromDataSet(std::ifstream& file){
    std::string line;
    std::vector<City> cities;

    bool alreadyUsefulDataSection = false; // 是否采集到真正有效的坐标数据

    while(std::getline(file, line)){
        std::istringstream iss(line);
        std::string keyword;
        iss >> keyword;         // 问题出在这个 keyword, 即使已经判断了处于 NODE_COORD_SECTION 部分，之后的每行输入也会吃掉第一个直到空格前的内容

        if(keyword == "NODE_COORD_SECTION"){
            alreadyUsefulDataSection = true;
            continue;
        }else if(keyword == "EOF" || file.eof()){
            break;
        }

        if(alreadyUsefulDataSection){
            // 查出一个 bug: 直接使用 std::istringstream iss(line) 进行参数的读入 
            // iss >> city.id >> city.x >> city.y 会发生错位，没有正确对应
            // 正确情况: city.id = 71009 city.x = 53500.0000 city.y = 123150.0000 但采用上述的输入方式，得到的对应是 city.id = 53500, city.x = 0. city.y = 123150 [❌]
            // 修 bug 的解释: 再次重建一个流，因为之前的流 iss 读过之后，指针往后走，不会再向后，所以造成错位，需要再新建一个 dataStream 的流
            std::istringstream dataStream(line);
            City city;

            if(dataStream >> city.id >> city.x >> city.y){
                cities.emplace_back(city);
            }
            else{
                std::cerr << "Error: Can not parse line --- " << line << std::endl;
            } 
        }
    }

    return cities;
}

std::vector<City> insert_tsp_information(){
    try{
        // 这里使用相对路径来读取 world.tsp 等格式
        // 首先显示一个当前的工作路径，读取的数据集在子文件夹 dataSet 中
        std::cout << "Current working directory: " << fs::current_path() << std::endl;
        std::ifstream file("dataSet/ch71009.tsp");  // 传递已经打开的文件流

        if(!file.is_open()){
            throw std::runtime_error("Could not open file: dataSet/ch71009.tsp");
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

int main(){
    auto cities = insert_tsp_information();
    auto distanceMatrix = calculate_distanceMatrix(cities);
    // print_distanceMatrix(distanceMatrix);                // 辅助测试输出的距离矩阵，实际上会是一张很大的表，针对 ch71009.tsp, 直接报 killed

    return 0;
}