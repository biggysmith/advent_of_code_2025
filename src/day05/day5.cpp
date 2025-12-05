#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>

struct range_t {
    size_t low;
    size_t high;
};

struct database_t {
    std::vector<range_t> id_ranges;
    std::vector<size_t> ids;
};

database_t load_input(const std::string& file){
    database_t ret;
    std::ifstream fs(file);
    bool load_stage = 0;
    std::string line;
    while (std::getline(fs, line)) {
        if(line == ""){
            load_stage = 1;
            continue;
        }else if(load_stage == 0){
            auto dash_pos = line.find_first_of('-'); 
            ret.id_ranges.push_back({ std::stoull(line.substr(0, dash_pos)), std::stoull(line.substr(dash_pos+1)) });
        }else{
            ret.ids.push_back( std::stoull(line) );
        }
    }
    return ret;
}

struct interval_set_t
{
    void insert(const range_t& range) {
        auto after = ranges.upper_bound(range.low);
        auto insert_range = after;

        if(after == ranges.begin() || std::prev(after)->second < range.low) {
            insert_range = ranges.insert(after, { range.low, range.high+1 }); // +1 since inclusive
        }else{
            insert_range = std::prev(after);
            if(insert_range->second >= range.high+1) {
                return;
            }else{
                insert_range->second = range.high+1;
            }   
        }   

        while(after != ranges.end() && range.high+1 >= after->first) {
            insert_range->second = std::max(after->second, insert_range->second);
            after = ranges.erase(after);
        }   
    }

    std::map<size_t, size_t> ranges;
};

size_t part1(const database_t& database)
{
    size_t sum = 0;
    for(auto& id : database.ids){
        for(auto& range : database.id_ranges){
            if(id >= range.low && id <= range.high){
                sum++;
                break;
            }
        }
    }
    return sum;
}

size_t part2(const database_t& database)
{
    interval_set_t intervals;
    for(auto& range : database.id_ranges){
        intervals.insert({ range.low, range.high });
    }

    size_t sum = 0;
    for(auto& range : intervals.ranges){
        sum += (range.second - range.first);
    }
    return sum;
}

void main()
{
    auto test_values = load_input("../src/day05/test_input.txt");
    auto actual_values = load_input("../src/day05/input.txt");

    std::cout << "part1: " << part1(test_values) << std::endl;
    std::cout << "part1: " << part1(actual_values) << std::endl;

    std::cout << "part2: " << part2(test_values) << std::endl;
    std::cout << "part2: " << part2(actual_values) << std::endl;
}