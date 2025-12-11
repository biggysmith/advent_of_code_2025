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
std::vector<Z> compute_button_upper_bounds(const matrix<rational<Z>>& A, const std::vector<rational<Z>>& b)
{
    size_t rows = A.rows;
    size_t cols = A.cols;
    std::vector<Z> xmax(cols, Z(0));

    for(int j=0; j<cols; ++j) {
        bool seen = false;
        Z best = Z(0);

        for(int i=0; i<rows; ++i) {
            auto& a = A(i, j);
            if(a.num == 0) {
                continue;
            }

            const auto& bi = b[i];

            if(a.den != 1 || bi.den != 1) {
                continue;
            }

            Z bound = bi.num / a.num;
            if(!seen || bound < best) {
                best = bound;
                seen = true;
            }
        }

        if(seen){
            xmax[j] = best;
        }else{
            xmax[j] = Z(0); 
        }
    }

    return xmax;
}

template<typename Z>
bool build_full_x_from_free(const matrix<rational<Z>>& Ab, const linear_solution<Z>& sol, const std::vector<Z>& x_free, std::vector<rational<Z>>& x_out)
{
    size_t n = sol.n_vars;
    size_t rows = Ab.rows;
    size_t k = sol.free_cols.size();

    x_out.assign(n, rational<Z>(0));

    for(int j=0; j<k; ++j) {
        int col = sol.free_cols[j];
        x_out[col] = rational<Z>(x_free[j]);
    }

    for(int r=0; r<rows; ++r) {
        int pc = sol.pivot_col_by_row[r];
        if (pc < 0) {
            continue;
        }

        rational<Z> val = Ab(r, n);

        for(int j=0; j<k; ++j) {
            int col = sol.free_cols[j];
            auto& a = Ab(r, col);
            if (!a.isZero()){
                val -= a * x_out[col];
            }
        }

        x_out[pc] = val;
    }

    return true;
}

template<typename Z>
void dfs(const matrix<rational<Z>>& Ab, const linear_solution<Z>& sol, const std::vector<Z>& xmax, size_t idx, std::vector<Z>& x_free,  min_press_result<Z>& result)
{
    size_t k = sol.free_cols.size();
    size_t n = sol.n_vars;

    if(idx == k) {
        std::vector<rational<Z>> x_rat;
        build_full_x_from_free(Ab, sol, x_free, x_rat);

        Z cost = 0;
        std::vector<Z> x_int(n);

        for(int j=0; j<n; ++j) {
            const auto& v = x_rat[j];

            if (v.den != 1) return; 
            if (v.num < 0) return;
            if (v.num > xmax[j]) return;

            x_int[j] = v.num;
            cost += v.num;

            if(cost >= result.best_cost){
                return; 
            }
        }

        result.found = true;
        result.best_cost = cost;
        result.best_solution = x_int;
        return;
    }

    int col = sol.free_cols[idx];
    Z hi = xmax[col];

    for(Z v=0; v<=hi; ++v) {
        x_free[idx] = v;
        dfs(Ab, sol, xmax, idx + 1, x_free, result);
    }
}

template<typename Z>
min_press_result<Z> solve(const matrix<rational<Z>>& Ab, const linear_solution<Z>& sol, const std::vector<Z>& xmax)
{
    min_press_result<Z> result;

    if(sol.inconsistent){
        return result;
    }

    size_t n = sol.n_vars;
    size_t k = sol.free_cols.size();

    if(k == 0) {
        std::vector<rational<Z>> x_rat(n, rational<Z>(0));

        for(int r=0; r<Ab.rows; ++r) {
            int pc = sol.pivot_col_by_row[r];
            if(pc >= 0){
                x_rat[pc] = Ab(r, n);
            }
        }

        Z cost = 0;
        std::vector<Z> x_int(n);

        for(int j=0; j<n; ++j) {
            auto& v = x_rat[j];
            if(v.den != 1 || v.num < 0 || v.num > xmax[j]){
                return result;
            }
            x_int[j] = v.num;
            cost += v.num;
        }

        result.found = true;
        result.best_cost = cost;
        result.best_solution = x_int;
        return result;
    }

    std::vector<Z> x_free(k, 0);
    dfs(Ab, sol, xmax, 0, x_free, result);
    return result;
}

int64_t part2(const machines_t& machines)
{
    std::vector<int64_t> sums(machines.size(), 0);

    #pragma omp parallel for
    for(int i=0; i<machines.size(); ++i) {
        const auto& machine = machines[i];

        size_t num_buttons = machine.buttons.size();
        size_t num_outputs = machine.joltages.size();

        matrix<rational<int64_t>> A(num_outputs, num_buttons, rational<int64_t>(0));

        for(int btn=0; btn<num_buttons; ++btn){
            for(int out : machine.buttons[btn]){
                A(out, btn) = 1;
            }
        }

        std::vector<rational<int64_t>> b(num_outputs);
        for(int out=0; out<num_outputs; ++out){
            b[out] = machine.joltages[out];
        }

        auto Ab = augment(A, b);
        rref(Ab);

        auto sol = extract_solution<int64_t>(Ab);
        auto xmax = compute_button_upper_bounds(A, b);
        auto res = solve(Ab, sol, xmax);

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