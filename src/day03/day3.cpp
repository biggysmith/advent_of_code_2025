#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>

using banks_t = std::vector<std::string>;

banks_t load_input(const std::string& file){
    banks_t ret;
    std::ifstream fs(file);
    std::string line;
    while (std::getline(fs, line)) {
        ret.push_back(line);
    }
    return ret;
}

size_t find_largest_joltage(const std::string& bank, int target) {
    int to_remove = (int)bank.size() - target;

    std::vector<char> digits;
    digits.reserve(bank.size());

    for(char b : bank) {
        while(to_remove > 0 && !digits.empty() && digits.back() < b) {
            digits.pop_back();
            --to_remove;
        }
        digits.push_back(b);
    }

    while(to_remove > 0) {
        digits.pop_back();
        --to_remove;
    }

    return std::stoull(std::string(digits.begin(), digits.begin() + target));
}

size_t part1(const banks_t& banks)
{
    size_t sum = 0;
    for(auto& bank : banks){
        sum += find_largest_joltage(bank, 2);
    }
    return sum;
}

size_t part2(const banks_t& banks)
{
    size_t sum = 0;
    for(auto& bank : banks){
        sum += find_largest_joltage(bank, 12);
    }
    return sum;
}

void main()
{
    auto test_values = load_input("../src/day03/test_input.txt");
    auto actual_values = load_input("../src/day03/input.txt");

    std::cout << "part1: " << part1(test_values) << std::endl;
    std::cout << "part1: " << part1(actual_values) << std::endl;

    std::cout << "part2: " << part2(test_values) << std::endl;
    std::cout << "part2: " << part2(actual_values) << std::endl;
}