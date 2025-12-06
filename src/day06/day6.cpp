#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>

struct problem_t {
    std::vector<char> digits;
    int width = 0;
    int height = 0;
    char& operator()(int x, int y){ return digits[y*width + x]; }
};

problem_t load_input(const std::string& file){
    problem_t ret;
    std::ifstream fs(file);
    std::string line;
    while (std::getline(fs, line)) {
        std::copy(line.begin(), line.end(), std::back_inserter(ret.digits));
        ret.width = std::max(ret.width, (int)line.size());
        ret.height++;
    }
    return ret;
}

size_t part1(problem_t& problem)
{
    std::vector<std::vector<int>> matrix(problem.height-1);
    std::vector<char> ops;

    for(int y=0; y<problem.height; ++y){
        std::string num;
        for(int x=0; x<problem.width; ++x){
            num += problem(x, y);
        }

        if(num.front() == '+' || num.front() == '*'){
            char op;
            std::istringstream iss(num);
            while(iss >> op){
                ops.push_back(op);
            }
        }else{
            int inum;
            std::istringstream iss(num);
            while(iss >> inum){
                matrix[y].push_back(inum);
            }
        }
    }

    size_t sum = 0;
    for(int x=0; x<matrix[0].size(); ++x){
        size_t col_sum = matrix[0][x];
        for(int y=1; y<matrix.size(); ++y){
            col_sum = (ops[x] == '+') ? col_sum + matrix[y][x] : col_sum * matrix[y][x];
        }
        sum += col_sum;
    }
    return sum;
}

size_t part2(problem_t& problem)
{
    size_t sum = 0;
    char op = '+';
    size_t col_sum = 0;
    for(int x=0; x<problem.width; ++x){
        std::string num;
        for(int y=0; y<problem.height; ++y){
            num += problem(x, y);
        }

        if(num.back() == '+' || num.back() == '*'){
            op = num.back();
            sum += col_sum;
            col_sum = (op == '+') ? 0 : 1;
        }

        if(num.find_first_not_of(' ') != std::string::npos){ // not only spaces
            int inum = std::stoi(num);
            col_sum = (op == '+') ? col_sum + inum : col_sum * inum;
        }
    }
    sum += col_sum;
    return sum;
}

void main()
{
    auto test_values = load_input("../src/day06/test_input.txt");
    auto actual_values = load_input("../src/day06/input.txt");

    std::cout << "part1: " << part1(test_values) << std::endl;
    std::cout << "part1: " << part1(actual_values) << std::endl;

    std::cout << "part2: " << part2(test_values) << std::endl;
    std::cout << "part2: " << part2(actual_values) << std::endl;
}