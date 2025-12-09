#include <iostream>
#include <fstream>
#include <string>
#include <vector>

struct tile_t {
    int64_t x, y;
};

using tiles_t = std::vector<tile_t>;

tiles_t load_input(const std::string& file){
    tiles_t ret;
    std::ifstream fs(file);
    std::string line;
    while (std::getline(fs, line)) {
        int x, y;
        sscanf_s(line.c_str(), "%d,%d\n", &x, &y); 
        ret.push_back({ x, y });
    }
    return ret;
}

int64_t area(const tile_t& a, const tile_t& b){
    return (std::abs(b.x-a.x)+1) * (std::abs(b.y-a.y)+1);
}

int64_t part1(const tiles_t& tiles)
{
    int64_t largest_area = 0;
    for(int i=0; i<tiles.size(); ++i){
        for(int j=i+1; j<tiles.size(); ++j){
            largest_area = std::max(largest_area, area(tiles[i], tiles[j]));
        }
    }
    return largest_area;
}

int64_t orient(const tile_t& a, const tile_t& b, const tile_t& c) {
    return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
}

bool point_in_poly(const tiles_t& tiles, int64_t px, int64_t py)
{
    bool inside = false;

    for(size_t i=0, j=tiles.size()-1; i<tiles.size(); j=i++)
    {
        tile_t a = tiles[j];
        tile_t b = tiles[i];

        if(!orient(a, b, {px, py}) && px >= std::min(a.x, b.x) && px <= std::max(a.x, b.x) && py >= std::min(a.y, b.y) && py <= std::max(a.y, b.y)){
            return true; // point on segment
        }

        if(!((a.y > py) != (b.y > py))) {
            continue;
        }

        int64_t dx = b.x - a.x;
        int64_t dy = b.y - a.y;

        int64_t lhs = 1LL * (px - a.x) * dy;
        int64_t rhs = 1LL * dx * (py - a.y);

        if((dy > 0 && lhs < rhs) || (dy < 0 && lhs > rhs)){
            inside = !inside;
        }
    }

    return inside;
}

bool seg_intersect(const tile_t& a, const tile_t& b, const tile_t& c, const tile_t& d)
{
    int64_t o1 = orient(a, b, c);
    int64_t o2 = orient(a, b, d);
    int64_t o3 = orient(c, d, a);
    int64_t o4 = orient(c, d, b);

    if((o1 > 0 && o2 < 0 || o1 < 0 && o2 > 0) && (o3 > 0 && o4 < 0 || o3 < 0 && o4 > 0)){
        return true;
    }

    return false;
}

bool inside_poly(const tiles_t& tiles, const tile_t& a, const tile_t& b)
{
    int64_t x0 = std::min(a.x, b.x);
    int64_t y0 = std::min(a.y, b.y);
    int64_t x1 = std::max(a.x, b.x);
    int64_t y1 = std::max(a.y, b.y);

    if(!point_in_poly(tiles, x0, y0)) return false;
    if(!point_in_poly(tiles, x1, y0)) return false;
    if(!point_in_poly(tiles, x0, y1)) return false;
    if(!point_in_poly(tiles, x1, y1)) return false;

    tile_t r0 { x0, y0 };
    tile_t r1 { x1, y0 };
    tile_t r2 { x1, y1 };
    tile_t r3 { x0, y1 };

    for(int i=0; i<tiles.size(); ++i)
    {
        tile_t p0 = tiles[i];
        tile_t p1 = tiles[(i+1) % tiles.size()];

        if(seg_intersect(r0, r1, p0, p1)) return false;
        if(seg_intersect(r1, r2, p0, p1)) return false;
        if(seg_intersect(r2, r3, p0, p1)) return false;
        if(seg_intersect(r3, r0, p0, p1)) return false;
    }

    return true;
}

int64_t part2(const tiles_t& tiles)
{
    int64_t largest = 0;
    for(int i=0; i<tiles.size(); ++i) {
        for(int j=i+1; j<tiles.size(); ++j) {
            if(inside_poly(tiles, tiles[i], tiles[j])){
                largest = std::max(largest, area(tiles[i], tiles[j]));
            }
        }
    }

    return largest;
}

void main()
{
    auto test_values = load_input("../src/day09/test_input.txt");
    auto actual_values = load_input("../src/day09/input.txt");

    std::cout << "part1: " << part1(test_values) << std::endl;
    std::cout << "part1: " << part1(actual_values) << std::endl;

    std::cout << "part2: " << part2(test_values) << std::endl;
    std::cout << "part2: " << part2(actual_values) << std::endl;
}
