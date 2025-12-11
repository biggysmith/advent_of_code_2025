#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>

using outputs_t = std::vector<std::string>;
using connections_t = std::unordered_map<std::string, std::vector<std::string>>;

connections_t load_input(const std::string& file){
    connections_t ret;
    std::ifstream fs(file);
    std::string line;
    while(std::getline(fs, line)) {
        std::istringstream iss(line);
        std::string key, value;
        std::getline(iss, key, ':');
        while(iss >> value) {
            ret[key].push_back(value);
        }
    }
    return ret;
}

struct path_state_t {
    std::string node;
    bool seen_fft = false;
    bool seen_dac = false;
};
bool operator==(const path_state_t& a, const path_state_t& b) { return a.node == b.node && a.seen_fft == b.seen_fft && a.seen_dac == b.seen_dac; }

struct path_state_hash_t {
    size_t operator()(const path_state_t& k) const {
        return std::hash<std::string>{}(k.node) ^ (((unsigned)k.seen_fft | ((unsigned)k.seen_dac << 1)) * 0x9e3779b97f4a7c15ULL);
    }
};

using memo_cache_t = std::unordered_map<path_state_t, int64_t, path_state_hash_t>;

int64_t find_paths_to_out(const std::string& device, bool seen_fft, bool seen_dac, const connections_t& connections, bool is_part1, memo_cache_t& memo){
    if(device == "fft") seen_fft = true;
    if(device == "dac") seen_dac = true;

    if(device == "out"){
        return ((is_part1 || seen_fft && seen_dac) ? 1 : 0);
    }

    path_state_t path_state { device, seen_fft, seen_dac };
    if(memo.count(path_state)){
        return memo[path_state];
    }

    int64_t sum = 0;
    if(connections.count(device)){
        for(auto& output : connections.at(device)){
            sum += find_paths_to_out(output, seen_fft, seen_dac, connections, is_part1, memo);
        }
    }

    memo[path_state] = sum;
    return sum;
}

size_t part1(const connections_t& connections)
{
    return find_paths_to_out("you", false, false, connections, true, memo_cache_t());
}

size_t part2(const connections_t& connections)
{
    return find_paths_to_out("svr", false, false, connections, false, memo_cache_t());
}

void main() 
{
    auto test_values1 = load_input("../src/day11/test_input.txt");
    auto test_values2 = load_input("../src/day11/test_input2.txt");
    auto actual_values = load_input("../src/day11/input.txt");

    std::cout << "part1: " << part1(test_values1) << std::endl;
    std::cout << "part1: " << part1(actual_values) << std::endl;

    std::cout << "part2: " << part2(test_values2) << std::endl;
    std::cout << "part2: " << part2(actual_values) << std::endl;
}