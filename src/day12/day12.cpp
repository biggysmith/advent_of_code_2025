#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_set>
#include <algorithm>

struct grid_t {
    std::string data;
    int width = 0;
    int height = 0;

    char& operator()(int x, int y){ return data[y*width + x]; }
    char& operator()(int idx){ return data[idx]; }
};
bool operator==(const grid_t& a, const grid_t& b) { return a.data == b.data; }

struct grid_hash_t {
    size_t operator()(const grid_t& grid) const {
        return std::hash<std::string>{}(grid.data);
    }
};
using grid_set = std::unordered_set<grid_t, grid_hash_t>;

struct region_t {
    int width, height;
    std::vector<int> quantity;
};

struct situation_t {
    std::vector<grid_t> shapes;
    std::vector<region_t> regions;
};

situation_t load_input(const std::string& file){
    situation_t ret;
    std::ifstream fs(file);
    std::string line;
    bool parse_shapes = true;
    while(std::getline(fs, line)) {
        if(line.empty()) continue;
        if(line.find('x') != std::string::npos){
            parse_shapes = false;
        }
        if(parse_shapes){
            if(line.back() == ':') {
                ret.shapes.push_back(grid_t());
                continue;
            }
            std::copy(line.begin(), line.end(), std::back_inserter(ret.shapes.back().data));
            ret.shapes.back().width = (int)line.size();
            ret.shapes.back().height++;
        }else{
            ret.regions.push_back(region_t());
            std::string dims = line.substr(0, line.find(':'));
            std::string data = line.substr(line.find(':')+2);

            ret.regions.back().width = std::stoi(dims.substr(0, line.find('x')));
            ret.regions.back().height = std::stoi(dims.substr(line.find('x')+1));

            int v;
            std::istringstream values(data);
            while(values >> v){
                ret.regions.back().quantity.push_back(v);
            }
        }
    }
    return ret;
}

bool place(std::vector<std::vector<grid_t>>& shapes_list, grid_t& grid, int x_steps, int y_steps, int depth, int max_depth){
    std::vector<int> modified;
    modified.reserve(9);

    auto& shapes = shapes_list[depth];

    for(int y=0; y<y_steps; ++y){
        for(int x=0; x<x_steps; ++x){

            for(int z=0; z<shapes.size(); ++z)
            {
                for(int idx : modified){ 
                    grid(idx) = '.'; 
                } 
                modified.clear();

                bool valid = true;
                for(int py=0; py<3 && valid; ++py) {
                    for(int px=0; px<3; ++px) {
                        if(shapes[z](px, py) == '#') {
                            int gx = x + px; int gy = y + py;
                            int idx = gy * grid.width + gx;
                            if(grid(idx) == '#'){
                                valid = false;
                                break;
                            }
                            modified.push_back(idx);
                            grid(idx) = '#';
                        }
                    }
                }

                if(!valid){
                    continue;
                }

                if(depth+1 == max_depth){
                    return true;
                }

                if(place(shapes_list, grid, x_steps, y_steps, depth+1, max_depth)){
                    return true;
                }
            }

        }
    }
    return false;
};

grid_t transform(grid_t& in, const int map[9]) {
    grid_t out = in;
    for(int i=0; i<9; ++i){
        out.data[i] = in.data[map[i]];
    }
    return out;
}

size_t part1(situation_t& situation)
{
    int rot[9]  = { 6, 3, 0, 7, 4, 1, 8, 5, 2 };
    int flip[9] = { 2, 1, 0, 5, 4, 3, 8, 7, 6 };

    size_t sum = 0;
    for(auto& region : situation.regions)
    {
        std::vector<std::vector<grid_t>> shape_variants;
        for(int i=0; i<region.quantity.size(); ++i){
            for(int j=0; j<region.quantity[i]; ++j){
                grid_t shape = situation.shapes[i];
                grid_set shape_set;
                for(int r=0; r<4; ++r){
                    shape_set.insert(shape);
                    shape_set.insert(transform(shape, flip));
                    shape = transform(shape, rot);
                }
                shape_variants.push_back(std::vector<grid_t>(shape_set.begin(), shape_set.end()));
            }
        }

        grid_t new_grid { std::string(region.width*region.height, '.'), region.width, region.height };
        sum += place(shape_variants, new_grid, region.width-2, region.height-2, 0, (int)shape_variants.size());
    }

    return sum;
}

size_t part2(const situation_t& situation)
{
    std::vector<int> shape_areas;
    for(auto& shape : situation.shapes){
        int area = (int)std::count(shape.data.begin(), shape.data.end(), '#');
        shape_areas.push_back(area);
    }

    size_t sum = 0;
    for(auto& region : situation.regions){
        int region_spaces = region.width * region.height;

        int spaces_needed = 0;
        for(int i=0; i<region.quantity.size(); ++i){
            spaces_needed += shape_areas[i] * region.quantity[i];
        }

        if(spaces_needed <= region_spaces){
            sum++;
        }
    }
    return sum;
}

void main() 
{
    auto test_values = load_input("../src/day12/test_input.txt");
    auto actual_values = load_input("../src/day12/input.txt");

    std::cout << "part1: " << part1(test_values) << std::endl;
    std::cout << "part1: " << part2(actual_values) << std::endl;
}