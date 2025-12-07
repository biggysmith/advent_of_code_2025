#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>

struct pos_t {
    int x, y;
};
bool operator==(const pos_t& a, const pos_t& b){ return a.x == b.x && a.y == b.y; }

struct grid_t {
    std::vector<char> data;
    int width = 0;
    int height = 0;
    char operator()(const pos_t& p) const { return data[p.y*width + p.x]; }
};

grid_t load_input(const std::string& file){
    grid_t ret;
    std::ifstream fs(file);
    std::string line;
    while (std::getline(fs, line)) {
        std::copy(line.begin(), line.end(), std::back_inserter(ret.data));  
        ret.width = (int)line.size();
        ret.height++;
    }
    return ret;
}

struct pos_hash{
    std::size_t operator()(const pos_t& pos) const {
        return (pos.x + pos.y + 1ull) * (pos.x + pos.y) / 2ull + pos.y; // cantor
    }
};
using cache_t = std::unordered_map<pos_t, size_t, pos_hash>;

size_t travel(const pos_t& beam_pos, const grid_t& grid, cache_t& visited)
{
    pos_t pos = { beam_pos.x, beam_pos.y + 1 };
    if(pos.y == grid.height){ return 0; } // end of beam

    if(grid(pos) == '^'){
        size_t splits = 0;
        bool can_split = false;

        for(int dx : { -1, 1 }) {
            pos_t new_pos { pos.x + dx, pos.y };
            if(!visited.count(new_pos)) {
                visited[new_pos] = 1;
                splits += travel(new_pos, grid, visited);
                can_split = true;
            }
        }

        return splits + can_split;
    }
    visited[pos] = 1;
    return travel(pos, grid, visited);
}

size_t part1(const grid_t& grid)
{
    pos_t start_pos { (int)std::distance(grid.data.begin(), std::find(grid.data.begin(), grid.data.end(), 'S')), 0 };
    return travel(start_pos, grid, cache_t());
}

size_t timeline_travel(const pos_t& beam_pos, const grid_t& grid, cache_t& cache)
{
    pos_t pos = { beam_pos.x, beam_pos.y+1 };
    if(pos.y == grid.height){ return 1; } // end of beam

    if(grid(pos) == '^'){
        if(cache.find(pos) != cache.end()){
            return cache[pos];
        }
        cache[pos] = timeline_travel({ pos.x-1, pos.y }, grid, cache) + timeline_travel({ pos.x+1, pos.y }, grid, cache);
        return cache[pos];
    }
    return timeline_travel(pos, grid, cache);
}

size_t part2(const grid_t& grid)
{
    pos_t start_pos { (int)std::distance(grid.data.begin(), std::find(grid.data.begin(), grid.data.end(), 'S')), 0 };
    return timeline_travel(start_pos, grid, cache_t());
}

void main()
{
    auto test_values = load_input("../src/day07/test_input.txt");
    auto actual_values = load_input("../src/day07/input.txt");

    std::cout << "part1: " << part1(test_values) << std::endl;
    std::cout << "part1: " << part1(actual_values) << std::endl;

    std::cout << "part2: " << part2(test_values) << std::endl;
    std::cout << "part2: " << part2(actual_values) << std::endl;
}