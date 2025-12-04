#include <iostream>
#include <fstream>
#include <string>
#include <vector>

struct grid_t {
    std::vector<char> data;
    int width = 0;
    int height = 0;

    char& operator()(int x, int y){ return data[y*width + x]; }
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

bool removable(grid_t& grid, int px, int py) {
    if(grid(px, py) != '@'){
        return false;
    }

    int adjecent_papers = 0;
    for(int y=std::max(py-1, 0); y<std::min(py+2, grid.height); ++y){
        for(int x=std::max(px-1, 0); x<std::min(px+2, grid.width); ++x){
            if(!(py == y && px == x)){
                adjecent_papers += grid(x, y) == '@';
                if(adjecent_papers == 4){
                    return false;
                }
            }
        }
    }

    return true;
}

int part1(grid_t& grid)
{
    int sum = 0;
    for(int y=0; y<grid.height; ++y){
        for(int x=0; x<grid.width; ++x){
            sum += removable(grid, x, y);
        }
    }
    return sum;
}

size_t part2(grid_t& grid)
{
    int sum = 0;
    grid_t grid2 = grid;
    bool all_removed = false;

    while(!all_removed){
        int removed = 0;
        for(int y=0; y<grid.height; ++y){
            for(int x=0; x<grid.width; ++x){
                if(removable(grid, x, y)){
                    removed++;
                    grid2(x, y) = '.';
                };
            }
        }
        sum += removed;
        all_removed |= removed == 0;
        grid = grid2;
    }

    return sum;
}

void main()
{
    auto test_values = load_input("../src/day04/test_input.txt");
    auto actual_values = load_input("../src/day04/input.txt");

    std::cout << "part1: " << part1(test_values) << std::endl;
    std::cout << "part1: " << part1(actual_values) << std::endl;

    std::cout << "part2: " << part2(test_values) << std::endl;
    std::cout << "part2: " << part2(actual_values) << std::endl;
}