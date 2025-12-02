#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

struct id_t {
    size_t first;
    size_t second;
};

using ids_t = std::vector<id_t>;

ids_t load_input(const std::string& file){
    ids_t ret;
    std::ifstream fs(file);
    std::string line;
    while (std::getline(fs, line)) {
        std::string item;
        std::stringstream ss(line);
        while (std::getline(ss, item, ',')) {
            auto pos = item.find_first_of("-");
            ret.push_back({ std::stoull(item.substr(0, pos)), std::stoull(item.substr(pos+1))});
        }
    }
    return ret;
}

size_t part1(const ids_t& ids)
{
    size_t sum = 0;
    for(auto& [first, second] : ids) {
        for (size_t i=first; i<=second; ++i) {
            std::string str = std::to_string(i);

            if (str.size() % 2 != 0){
                continue;
            }

            size_t half = str.size() / 2;
            if(str.substr(0, half) == str.substr(half)){
                sum += i;
            }
        }
    }
    return sum;
}

size_t part2(const ids_t& ids)
{
    size_t sum = 0;
    for (auto& [first, second] : ids) {
        for (size_t i = first; i <= second; ++i) {
            std::string str = std::to_string(i);
            size_t len = str.size();

            for (size_t p=1; p<=len/2; ++p)
            {
                if (len % p != 0) {
                    continue;
                }

                size_t repeats = len / p;
                if (repeats < 2) {
                    continue;
                }

                std::string sub = str.substr(0, p);

                bool ok = true;
                for (size_t k=1; k<repeats; ++k) {
                    if (str.compare(k * p, p, sub) != 0) {
                        ok = false;
                        break;
                    }
                }

                if (ok) {
                    sum += i;
                    break;
                }
            }
        }
    }
    return sum;
}

void main()
{
    auto test_values = load_input("../src/day02/test_input.txt");
    auto actual_values = load_input("../src/day02/input.txt");

    std::cout << "part1: " << part1(test_values) << std::endl;
    std::cout << "part1: " << part1(actual_values) << std::endl;

    std::cout << "part2: " << part2(test_values) << std::endl;
    std::cout << "part2: " << part2(actual_values) << std::endl;
}