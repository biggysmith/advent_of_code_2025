#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

struct rotation_t{
    char direction;
    int distance;
};

using rotations_t = std::vector<rotation_t>;

rotations_t load_input(const std::string& file){
    rotations_t ret;
    std::ifstream fs(file);
    std::string line;
    while (std::getline(fs, line)) {
        ret.push_back({ line[0], std::stoi(line.substr(1)) });
    }
    return ret;
}

int mod(int a, int b) {
    return ((a % b) + b) % b;
}

int part1(const rotations_t& rotations)
{
    int password = 0;
    int pos = 50;

    for(auto& rotation : rotations){
        pos += rotation.direction == 'L' ? -rotation.distance : rotation.distance;
        password += mod(pos, 100) == 0;
    }

    return password;
}

int part2(const rotations_t& rotations)
{
    int password = 0;
    int pos = 50;

    for(auto& rotation : rotations){
        int dist = rotation.direction == 'L' ? -rotation.distance :  rotation.distance;
        int abs_dist = std::abs(dist);
        int target = (dist > 0) ? mod(-pos, 100) : mod(pos, 100);
        int first = (target == 0) ? 100 : target;

        if (first <= abs_dist){
            password += 1 + (abs_dist - first) / 100;
        }

        pos += dist;
    }

    return password;
}

void main()
{
    auto test_values = load_input("../src/day01/test_input.txt");
    auto actual_values = load_input("../src/day01/input.txt");

    std::cout << "part1: " << part1(test_values) << std::endl;
    std::cout << "part1: " << part1(actual_values) << std::endl;

    std::cout << "part2: " << part2(test_values) << std::endl;
    std::cout << "part2: " << part2(actual_values) << std::endl;
}