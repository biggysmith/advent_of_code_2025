#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <regex>
#include <set>
#include <unordered_set>
#include <unordered_map>
#include <queue>
#include <numeric>
#include <matrix.hpp>
#include <functional>
#include <omp.h>
#include <timer.hpp>

using light_t = std::string;
using joltage_t = std::vector<int>;

struct machine_t {
    light_t light;
    std::vector<std::vector<int>> buttons;
    joltage_t joltages;
};

using machines_t = std::vector<machine_t>;

void parse_ints(const std::string& s, std::vector<int>& out) {
    static const std::regex num_re(R"(\d+)");
    for(auto it = std::sregex_iterator(s.begin(), s.end(), num_re); it != std::sregex_iterator(); ++it) {
        out.push_back(std::stoi((*it).str()));
    }
}

machines_t load_input(const std::string& file){
    machines_t ret;
    std::ifstream fs(file);
    std::string line;
    while(std::getline(fs, line)) {
        ret.push_back(machine_t());

        std::smatch m;
        if(std::regex_search(line, m, std::regex(R"(\[([^\]]+)\])"))) {
            const std::string& pattern = m[1];
            ret.back().light.insert(ret.back().light.end(), pattern.begin(), pattern.end());
        }

        static const std::regex paren(R"(\(([^)]+)\))");
        for(auto it=std::sregex_iterator(line.begin(), line.end(), paren); it != std::sregex_iterator(); ++it) {
            ret.back().buttons.emplace_back();
            parse_ints((*it)[1], ret.back().buttons.back());
        }

        if(std::regex_search(line, m, std::regex(R"(\{([^}]+)\})"))) {
            parse_ints(m[1], ret.back().joltages);
        }
    }
    return ret;
}

struct state_t {
    light_t light;
    std::vector<int> presses;
    int total_presses = 0;
};
bool operator==(const state_t& a, const state_t& b) { return a.light == b.light && a.presses == b.presses && a.total_presses == b.total_presses; }
bool operator<(const state_t& a, const state_t& b) { return a.total_presses > b.total_presses; }

struct hash_t{
    size_t operator()(const state_t& s) const {
        size_t h = std::hash<std::string>{}(s.light);
        for(int v : s.presses) {
            h ^= std::hash<int>{}(v) + 0x9e3779b97f4a7c15ULL + (h << 6) + (h >> 2);
        }
        return h;
    }
};

auto bfs(const machine_t& machine)
{
    light_t dst = machine.light;

    std::priority_queue<state_t> q;
    std::unordered_set<state_t, hash_t> visited;
    q.push({ light_t(dst.size(), '.'), std::vector<int>(machine.buttons.size(), 0) });

    while (!q.empty()) 
    {
        auto curr = q.top();
        q.pop();

        if(curr.light == dst){
            return curr.total_presses;
        }

        if(visited.count(curr)){
            continue;
        }

        visited.insert(curr);

        for(int b=0; b<machine.buttons.size(); ++b){
            state_t new_state = curr;
            for(int i : machine.buttons[b]){
                new_state.light[i] = new_state.light[i] == '#' ? '.' : '#';
            }
            new_state.presses[b]++;
            new_state.total_presses++;
            q.push(new_state);
        }
    }

    return -1;
}

size_t part1(const machines_t& machines)
{
    size_t sum = 0;
    for(auto& machine : machines){
        sum += bfs(machine);
    }
    return sum;
}

struct searcher_t 
{
    int N = 0, answer = INT_MAX;
    std::vector<int> need, contain, masks;
    std::vector<std::vector<int>> bits_of;

    void build_bits() 
    {
        int M = (int)need.size();
        bits_of.assign(N, {});
        for(int i=0; i<N; ++i){
            for(int j=0; j<M; ++j){
                if(masks[i] & (1 << j)){
                    bits_of[i].push_back(j);
                }
            }
        }
    }

    int lower_bound() const 
    {
        int lb = 0;
        for (int j = 0; j < (int)need.size(); ++j) {
            if(!need[j]) continue;
            if(!contain[j]) return INT_MAX;
            lb = std::max(lb, (need[j] + contain[j] - 1) / contain[j]);
        }
        return lb;
    }

    int pick_best(uint32_t mask) const 
    {
        int best = -1;
        int best_cnt = INT_MAX;
        for (int i = 0; i < N; ++i) {
            if (mask & (1u << i)) {
                continue;
            }
            int cnt = INT_MAX;
            for (int j : bits_of[i]) {
                cnt = std::min(cnt, contain[j]);
            }
            if (cnt < best_cnt) {
                best_cnt = cnt, best = i;
            }
        }
        return best;
    }

    void dfs(uint32_t mask, int cur) 
    {
        if(cur >= answer) {
            return;
        }

        if(mask == (1u << N) - 1) {
            answer = std::min(answer, cur);
            return;
        }

        int lb = lower_bound();
        if (lb == INT_MAX || cur + lb >= answer) {
            return;
        }

        int id = pick_best(mask);
        const auto& covered = bits_of[id];

        int best_cnt = INT_MAX;
        for (int j : covered) {
            best_cnt = std::min(best_cnt, contain[j]);
        }

        if (best_cnt == 1) {
            int must = -1;
            for(int j : covered){
                if(contain[j] == 1){
                    if(must == -1) {
                        must = need[j];
                    }else if(must != need[j]) {
                        return;
                    }
                }
            }

            for(int j : covered){
                if (must > need[j]) {
                    return;
                }
            }

            for(int j : covered) {
                need[j] -= must, contain[j]--;
            }

            dfs(mask | (1u << id), cur + must);

            for(int j : covered) {
                need[j] += must;
                contain[j]++;
            }
            return;
        }

        int mx = INT_MAX;
        for(int j : covered) {
            mx = std::min(mx, need[j]);
            contain[j]--;
        }
        for(int j : covered) {
            need[j] -= mx;
        }

        dfs(mask | (1u << id), cur + mx);

        for(int take = 1; take <= mx; ++take) {
            for (int j : covered) {
                need[j]++;
            }
            dfs(mask | (1u << id), cur + mx - take);
        }

        for(int j : covered) {
            contain[j]++;
        }
    }

    int process(const machine_t& machine) 
    {
        for(auto& button : machine.buttons) {
            int mask = 0;
            for (int bit : button) {
                mask |= (1 << bit);
            }
            masks.push_back(mask);
        }

        N = (int)masks.size();
        need = machine.joltages;

        int M = (int)machine.joltages.size();
        contain.assign(M, 0);

        for(int p : masks){
            for(int i = 0; i < M; ++i){
                if(p & (1 << i)){
                    contain[i]++;
                }
            }
        }

        answer = INT_MAX;
        build_bits();
        dfs(0u, 0);
        return answer;
    }
};

int64_t part2(const machines_t& machines)
{
    std::vector<int> sums(machines.size(), 0);

    #pragma omp parallel for
    for(int i=0; i<machines.size(); ++i){
        auto& machine = machines[i];
        searcher_t searcher;
        int sub_sum = searcher.process(machine);
        sums[i] = sub_sum;
    }

    return std::accumulate(sums.begin(), sums.end(), 0);
}

void main() 
{
    auto test_values = load_input("../src/day10/test_input.txt");
    auto actual_values = load_input("../src/day10/input.txt");

    std::cout << "part1: " << part1(test_values) << std::endl;
    std::cout << "part1: " << part1(actual_values) << std::endl;

    std::cout << "part2: " << part2(test_values) << std::endl;
    std::cout << "part2: " << part2(actual_values) << std::endl;
}