#ifndef SUPERVISOR
#define SUPERVISOR
#include <thread>
#include <vector>
#include "threadsafeplayer.cpp"
#include "activators.cpp"
#include <iostream>
#include <random>


class Supervisor {
private:

	int size;
	const double exchangeChance = 0.5;

public:

    std::vector<ThreadSafePlayer*> players;
    Supervisor(int n, std::vector<size_t> sizes, const Function<double>* av, const Function<double>* f) {
		size = n;
        for (int i = 0; i < n; i++) {
            players.push_back(new ThreadSafePlayer(sizes, av, f));
        }
    }

	void evolve(int i = -1, double mutationChance = 0.08, double crossoverChance = 0.5, bool verbose = true, bool test = true, int frequency = 5, int testSize = 1000) {

		int start = i;

		while (i >= 0) {

			playCompetition();

			if (test && !(i % frequency)) {
				benchmarkBestRandom(testSize);
			}

			select();
			crossover(crossoverChance);
			mutate(mutationChance);
			i--;

			if (verbose) {
				std::cout << "Episode " << start - i << std::endl;
			}
		}
	}


	void crossover(double alpha) {
		std::random_shuffle(players.begin(), players.end());
		int half = size/2;
		for (int i = 0; i < half; i++)
			if (RandomGenerator::randomDouble(0, 1) > alpha)
				randomSelect(*players[i], *players[half + i]);
	}

	void cut(ThreadSafePlayer& p1, ThreadSafePlayer& p2) {
		//todo implement single cut and switch
	}

	void randomSelect(ThreadSafePlayer& p1, ThreadSafePlayer& p2) {
		for (unsigned int i = 0; i < p1.weights.size(); i++) {
			for (unsigned int j = 0; j < p1.weights[i].rows*p1.weights[i].columns; j++) {
				if (RandomGenerator::randomDouble(0, 1) > exchangeChance) {
					double temp = p1.weights[i][j];
					p1.weights[i][j] = p2.weights[i][j];
					p2.weights[i][j] = temp;
				}
			}

			for (unsigned int j = 0; j < p1.biases[i].rows*p1.biases[i].columns; j++) {
				if (RandomGenerator::randomDouble(0, 1) > exchangeChance) {
					double temp = p1.biases[i][j];
					p1.biases[i][j] = p2.biases[i][j];
					p2.biases[i][j] = temp;
				}
			}
		}
	}

	void biologicalCut(ThreadSafePlayer& p1, ThreadSafePlayer& p2) {
		//todo implement biological cut and switch
	}

	double getTotalScore() {
		double total = 0;
		for (unsigned int i = 0; i < players.size(); i++) {
			total += players[i]->getScore();
		}
		return total;
	}

	void select() {
		double totalScore = getTotalScore();
		double arr[size];
		arr[0] = 0;
		for (int i = 1; i < size; i++) {
			arr[i] = arr[i - 1] + players[i]->getScore()/totalScore;
		}
		std::vector<ThreadSafePlayer*> newPlayers;
		for (int i = 0; i < size; i++) {
			auto index = std::lower_bound(arr, arr + size, RandomGenerator::randomDouble(0, 1)) - arr - 1;
			newPlayers.push_back(new ThreadSafePlayer(*players[index]));
		}

		for (int i = 0; i < size; i++) {
			delete players[i];
			players[i] = newPlayers[i];
		}

	}

	void mutate(double mutationChance) {
		for (int i = 0; i < size; i++) {
			if (RandomGenerator::randomDouble(0,1) > 0.5) {
				defaultMutate(i);
			}
		}
	}

	void defaultMutate(int p) {
		double mutation_rate = 0.02;
		double mutation_amount = 0.2;
		for (unsigned int i = 0; i < players[p]->weights.size(); i++) {
			for (unsigned int j = 0; j < players[p]->biases[i].size(); j++) {
				if (RandomGenerator::randomDouble(0,1) < mutation_rate) {
					players[p]->biases[i][j] *= RandomGenerator::randomDouble(1-mutation_amount, 1 + mutation_amount);
				}
			}
			for (unsigned int j = 0; j < players[p]->weights[i].size(); j++) {
				if (RandomGenerator::randomDouble(0,1) < mutation_rate) {
					players[p]->weights[i][j] *= RandomGenerator::randomDouble(1 - mutation_amount, 1 + mutation_amount);
				}
			}
		}
	}

	static void _worker(std::vector<std::tuple<ThreadSafePlayer*, ThreadSafePlayer*>> *pairs, std::mutex *index_mutex, unsigned int *index) {
        while (true) {
            index_mutex->lock();
            unsigned int next_index = *index;
            if (next_index >= pairs->size()) {
                index_mutex->unlock();
                break;
            }
            *index += 1;
            index_mutex->unlock();
            auto [p1, p2] = (*pairs)[next_index];
            double score = ThreadSafePlayer::eval(p1, p2);
            p1->addScore(32 + score/2);
            p2->addScore(32 - score/2);
        }
    }

    void playCompetition() {
        // Pair players
        std::vector<std::tuple<ThreadSafePlayer*, ThreadSafePlayer*>> pairs;
        for (auto i : players) {
            for (auto j : players) {
                if (i != j) {
                    pairs.push_back(std::make_tuple(i, j));
                }
            }
        }
        // Play competition
        unsigned int current_index = 0;
        auto m = new std::mutex();
        std::vector<std::thread*> thread_vector;
        int n = std::thread::hardware_concurrency();
        for (int i = 0; i < n; i++) {
            thread_vector.push_back(new std::thread(_worker, &pairs, m, &current_index));
        }
        for (auto i : thread_vector) {
            i->join();
            delete i;
        }
        delete m;
    }

    void sortPlayersByScore() {
        std::sort(players.begin(), players.end(), [] (ThreadSafePlayer *a, ThreadSafePlayer *b) {
            return a->getScore() > b->getScore();
        });
    }

    void benchmarkBestRandom(int n = 1000) {
        sortPlayersByScore();
        auto [score, wins, loses] = players[0]->randomBenchmarker(n);
        std::cout << "Score: " << score << " Wins: " << wins << " Loses: " << loses << " Draws: " << (100 - loses - wins) << std::endl;
    }

    ~Supervisor() {
        for (unsigned int i = 0; i < players.size(); i++) {
            delete players[i];
        }
    }
};

#endif
