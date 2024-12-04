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

namespace fs = std::filesystem;

struct City{
    int id;
    double x, y;
};

std::vector<City> readCityFromDataSet(std::ifstream& file){
    std::string line;
    std::vector<City> cities;

    bool alreadyUsefulDataSection = false; // 是否采集到真正有效的坐标数据

    while(std::getline(file, line)){
        std::istringstream iss(line);
        std::string keyword;
        iss >> keyword;

        if(keyword == "NODE_COORD_SECTION"){
            alreadyUsefulDataSection = true;
            continue;
        }else if(keyword == "EOF" || file.eof()){
            break;
        }

        if(alreadyUsefulDataSection){
            City city;
            if(iss >> city.id >> city.x >> city.y){
                cities.push_back(city);
            }else{
                std::cerr << "Error reading city data from line:" << line << std::endl;
            }
        }
    }

    return cities;
}


int main(){
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
        std::cout << "Read " << cities.size() << " cities from the TSP file." << std::endl;
    }catch(const std::exception& e){
        std::cerr << "Exception occurred: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}