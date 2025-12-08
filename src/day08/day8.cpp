#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>

struct box_t {
    int64_t x, y, z;
};
box_t operator-(const box_t& a, const box_t& b){ return { a.x-b.x, a.y-b.y, a.z-b.z }; }

using boxes_t = std::vector<box_t>;

boxes_t load_input(const std::string& file){
    boxes_t ret;
    std::ifstream fs(file);
    std::string line;
    while (std::getline(fs, line)) {
        int x, y, z;
        sscanf_s(line.c_str(), "%d,%d,%d\n", &x, &y, &z); 
        ret.push_back({ x, y, z });
    }
    return ret;
}

int64_t sq_dist(const box_t& a, const box_t& b){
    box_t c = b - a;
    return c.x*c.x + c.y*c.y + c.z*c.z;
}

struct edge_t {
    int a, b;
    int64_t dist;
    bool operator<(const edge_t& right) const { return dist < right.dist; }
};

struct dsu_t {
    std::vector<int> parent, size;

    dsu_t(int n) : parent(n), size(n, 1) {
        for(int i=0; i<parent.size(); ++i){ parent[i] = i; }
    }

    int find(int x) {
        if(parent[x] != x){
            parent[x] = find(parent[x]);
        }
        return parent[x];
    }

    bool uunion(int a, int b) {
        a = find(a);
        b = find(b);
        if (a == b) return false;
        if (size[a] < size[b]) std::swap(a, b);
        parent[b] = a;
        size[a] += size[b];
        return true;
    }
};

size_t part1(const boxes_t& boxes, int pair_count)
{
    std::vector<edge_t> edges;

    for(int i=0; i<boxes.size(); ++i) {
        for(int j=i+1; j<boxes.size(); ++j) {
            edges.push_back({ i, j, sq_dist(boxes[i], boxes[j]) });
        }
    }

    std::partial_sort(edges.begin(), edges.begin()+pair_count, edges.end());

    dsu_t dsu((int)boxes.size());
    for(int i=0; i<pair_count; ++i) {
        dsu.uunion(edges[i].a, edges[i].b);
    }

    std::partial_sort(dsu.size.begin(), dsu.size.begin()+3, dsu.size.end(), std::greater<int>());
    return dsu.size[0] * dsu.size[1] * dsu.size[2];
}

size_t part2(const boxes_t& boxes)
{
    std::vector<edge_t> edges;

    for(int i=0; i<boxes.size(); ++i) {
        for(int j=i+1; j<boxes.size(); ++j) {
            edges.push_back({ i, j, sq_dist(boxes[i], boxes[j]) });
        }
    }

    std::sort(edges.begin(), edges.end());

    dsu_t dsu((int)boxes.size());
    int idx = -1;

    for(int i=0; i<edges.size(); ++i) {
        if(dsu.uunion(edges[i].a, edges[i].b)) {
            idx = i;
        }
    }

    edge_t edge = edges[idx];
    return boxes[edge.a].x * boxes[edge.b].x;
}

void main()
{
    auto test_values = load_input("../src/day08/test_input.txt");
    auto actual_values = load_input("../src/day08/input.txt");

    std::cout << "part1: " << part1(test_values, 10) << std::endl;
    std::cout << "part1: " << part1(actual_values, 1000) << std::endl;

    std::cout << "part2: " << part2(test_values) << std::endl;
    std::cout << "part2: " << part2(actual_values) << std::endl;
}