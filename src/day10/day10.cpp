#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <regex>
#include <unordered_set>
#include <numeric>
#include <queue>
#include <functional>
#include <matrix.hpp>
#include <omp.h>

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

template<typename Z>
struct min_press_result {
    bool found = false;
    Z best_cost = std::numeric_limits<Z>::max();
    std::vector<Z> best_solution;
};

template<typename Z>
void estimate_t_upper_bounds(const linear_solution<Z>& sol, std::vector<Z>& tmax)
{
    size_t k = sol.free_cols.size();
    size_t n = sol.n_vars;

    tmax.assign(k, std::numeric_limits<Z>::max() / 4);

    for (size_t j = 0; j < k; ++j) {
        for (size_t i = 0; i < n; ++i) {
            const auto& p = sol.particular[i];
            const auto& d = sol.dirs[j][i];

            if (d.num == 0) continue;

            Z num = -p.num * d.den;
            Z den =  p.den * d.num;

            if (den < 0) {
                Z hi = num / den;
                tmax[j] = std::min(tmax[j], hi);
            }
        }

        tmax[j] += 60; // found by hand :(
        if (tmax[j] < 0)
            tmax[j] = 0;
    }
}

template<typename Z>
void reorder_free_variables(linear_solution<Z>& sol, std::vector<Z>& tmax)
{
    size_t k = sol.free_cols.size();
    std::vector<size_t> order(k);
    for (size_t i = 0; i < k; ++i)
        order[i] = i;

    std::sort(order.begin(), order.end(), [&](size_t a, size_t b) {
        return tmax[a] < tmax[b];
    });

    auto remap = [&](auto& v) {
        auto old = v;
        for (size_t i = 0; i < k; ++i)
            v[i] = old[order[i]];
    };

    remap(tmax);
    remap(sol.free_cols);
    remap(sol.dirs);
}

template<typename Z>
min_press_result<Z> find_min_press_solution(linear_solution<Z> sol) {
    min_press_result<Z> result;

    if (sol.inconsistent)
        return result;

    const size_t n = sol.n_vars;
    const size_t k = sol.free_cols.size();

    if (k == 0) {
        Z cost = 0;
        std::vector<Z> xi(n);

        for (size_t i = 0; i < n; ++i) {
            const auto& v = sol.particular[i];
            if (v.den != 1 || v.num < 0)
                return result;
            xi[i] = v.num;
            cost += v.num;
        }

        result.found = true;
        result.best_cost = cost;
        result.best_solution = xi;
        return result;
    }

    std::vector<Z> tmax;
    estimate_t_upper_bounds(sol, tmax);
    reorder_free_variables(sol, tmax);

    std::vector<Z> t(k, 0);
    std::vector<rational<Z>> x = sol.particular;

    std::function<void(size_t)> dfs = [&](size_t idx)
    {
        if (idx == k) {
            Z cost = 0;
            std::vector<Z> xi(n);

            for (size_t i = 0; i < n; ++i) {
                const auto& v = x[i];
                if (v.den != 1 || v.num < 0)
                    return;
                xi[i] = v.num;
                cost += v.num;
                if (cost >= result.best_cost)
                    return;
            }

            result.found = true;
            result.best_cost = cost;
            result.best_solution = xi;
            return;
        };

        for (Z v = 0; v <= tmax[idx]; ++v) {
            t[idx] = v;

            auto old_x = x;
            for (size_t i = 0; i < n; ++i)
                x[i] += rational<Z>(v) * sol.dirs[idx][i];

            dfs(idx + 1);
            x = old_x;
        }
    };

    dfs(0);
    return result;
}

int64_t part2(const machines_t& machines)
{
    std::vector<int64_t> sums(machines.size(), 0);

    for (int i = 0; i < machines.size(); ++i) {
        const auto& machine = machines[i];

        size_t num_buttons = machine.buttons.size();
        size_t num_outputs = machine.joltages.size();

        matrix<rational<int64_t>> A(num_outputs, num_buttons);

        for (size_t btn = 0; btn < num_buttons; ++btn)
            for (int out : machine.buttons[btn])
                A(out, btn) += 1;

        std::vector<rational<int64_t>> b(num_outputs);
        for (size_t out = 0; out < num_outputs; ++out)
            b[out] = machine.joltages[out];

        auto Ab = augment(A, b);
        rref(Ab);

        auto sol = extract_solution<int64_t>(Ab);
        auto res = find_min_press_solution(sol);
        sums[i] = res.best_cost;
    }

    return std::accumulate(sums.begin(), sums.end(), 0LL);
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