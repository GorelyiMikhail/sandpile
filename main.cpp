#include <iostream>
#include <queue>
#include <random>
#include <fstream>
#include <thread>
#include <cassert>

int di[4] = { 0, 0, 1, -1 };
int dj[4] = { 1, -1, 0, 0 };

size_t N = 2;

size_t add(int x, int y, size_t L, std::uniform_int_distribution<> rand0P, std::uniform_int_distribution<> rand0L, std::vector<std::vector<size_t>>& field, double& sand_amount, std::mt19937 rnd) {
    std::uniform_int_distribution<> random_step(0, 1);

    bool sand_dropped = false;

    ++field[x][y];
    ++sand_amount;

    std::queue<std::pair<int, int>> q;
    q.emplace(x, y);
    size_t avalanche = 0;

    while (!q.empty()) {
        auto [current_x, current_y] = q.front();
        q.pop();

         if (field[current_x][current_y] >= 4) {
            ++avalanche;

            field[current_x][current_y] -= 4;

            for (int t = 0; t < 4; ++t) {
                int new_x = current_x + di[t], new_y = current_y + dj[t];

                if (new_x >= 0 && new_y >= 0 && new_x < L && new_y < L) {
                    ++field[new_x][new_y];

                    if (field[new_x][new_y] >= 4) {
                        q.push({new_x, new_y});
                    }
                } else {
                    --sand_amount;
                }
            }

            if (!sand_dropped && rand0P(rnd) == 1) {
                sand_dropped = true;

                for (size_t sand_drop_i = 0; sand_drop_i < N; ++sand_drop_i) {
                    int add_x, add_y;
                    int step = sqrt(avalanche);

                    bool x_above_boundary = x - step >= 0;
                    bool x_below_boundary = x + step < L;
                    bool y_above_boundary = y - step >= 0;
                    bool y_below_boundary = y + step < L;

                    if (x_above_boundary && x_below_boundary) {
                        if (random_step(rnd) == 0) {
                            add_x = x - step;
                        } else {
                            add_x = x + step;
                        }
                    } else if (x_above_boundary || x_below_boundary) {
                        if (x_above_boundary) {
                            add_x = x - step;
                        } else {
                            add_x = x + step;
                        }
                    } else {
                        add_x = rand0L(rnd);
                    }

                    if (y_above_boundary && y_below_boundary) {
                        if (random_step(rnd) == 0) {
                            add_y = y - step;
                        } else {
                            add_y = y + step;
                        }
                    } else if (y_above_boundary || y_below_boundary) {
                        if (y_above_boundary) {
                            add_y = y - step;
                        } else {
                            add_y = y + step;
                        }
                    } else {
                        add_y = rand0L(rnd);
                    }

                    if (0 < field[add_x][add_y] && field[add_x][add_y] < 4) {
                        std::vector<size_t> possible_moves;

                        for (size_t i = 0; i < 4; ++i) {
                            if (add_x + di[i] >= 0 && add_x + di[i] < L && add_y + dj[i] >= 0 && add_y + dj[i] < L) {
                                possible_moves.push_back(i);
                            }
                        }

                        std::uniform_int_distribution<> random_add(0, possible_moves.size() - 1);
                        int random_index = random_add(rnd);

                        int new_x = add_x + di[possible_moves[random_index]], new_y = add_y + dj[possible_moves[random_index]];

                        --field[add_x][add_y];
                        ++field[new_x][new_y];

                        if (field[new_x][new_y] >= 4) {
                            q.push({new_x, new_y});
                        }
                    }
                }
            }
        }
    }
    return avalanche;
}

std::vector<size_t> sizes{2048, 1024, 512, 256, 128, 64};

std::vector<size_t> probability{333};

// std::vector<std::ofstream> sequences(probability.size() * sizes.size());
std::vector<std::ofstream> distributions(probability.size() * sizes.size());
// std::vector<std::ofstream> sand_amounts(probability.size() * sizes.size());

void concurrent_calculate_sandpile(size_t iteration, size_t L) {
    std::mt19937 rnd(time(0));

    size_t SKIP = 3 * L * L;

    size_t MAX_ITER = SKIP + 1e6;

    size_t MAX_AVALANCHE_SIZE = 1e7;

    std::vector<std::vector<size_t>> field(L, std::vector<size_t>(L));

    std::vector<unsigned> cnt_avalanche(MAX_AVALANCHE_SIZE);

    std::uniform_int_distribution<> rand0L(0, L - 1);

    std::uniform_int_distribution<> rand0P(1, probability[iteration % probability.size()]);

    size_t iter = 0;

    double sand_amount = 0;

    while (iter < MAX_ITER) {
        size_t x = rand0L(rnd), y = rand0L(rnd);

        size_t avalanche_size = add(x, y, L, rand0P, rand0L, field, sand_amount, rnd);

        if (iter > SKIP && avalanche_size < MAX_AVALANCHE_SIZE) {
            // sequences[iteration] << avalanche_size << ' ';
            // sand_amounts[iteration] << sand_amount / (L * L) << ' ';
            ++cnt_avalanche[avalanche_size];
        }

        ++iter;

        if (iter % 100000 == 0) {
            std::cout << L << " : " << iter << " / " << MAX_ITER << '\n';
        }
    }

    for (int i = 0; i < MAX_AVALANCHE_SIZE; ++i) {
        distributions[iteration] << cnt_avalanche[i] << ' ';
    }
}

int main() {
    for (size_t i = 0; i < probability.size() * sizes.size(); ++i) {
        // sequences[i].open("sequences/sequence" + std::to_string(sizes[i / probability.size()]) + "_" + std::to_string(probability[i % probability.size()]) + ".out");
        distributions[i].open("distributions/distribution" + std::to_string(sizes[i / probability.size()]) + "_" + std::to_string(probability[i % probability.size()]) + ".out");
        // sand_amounts[i].open("sand_amounts/sand_amount" + std::to_string(sizes[i / probability.size()]) + "_" + std::to_string(probability[i % probability.size()]) + ".out");
    }

    std::vector<std::thread> threads;

    for (size_t cur_size = 0; cur_size < sizes.size(); ++cur_size) {
        for (size_t iteration = 0; iteration < probability.size(); ++iteration) {
            threads.emplace_back(concurrent_calculate_sandpile, iteration + cur_size * probability.size(), sizes[cur_size]);
        }
    }

    for (size_t iteration = 0; iteration < probability.size() * sizes.size(); ++iteration) {
        threads[iteration].join();
    }
}
