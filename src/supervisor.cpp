#ifndef SUPERVISOR
#define SUPERVISOR
#include <thread>
#include <vector>
#include "threadsafeplayer.cpp"
#include "activators.cpp"
#include <iostream>


class Supervisor {
private:

	int size;
    
public:

    std::vector<ThreadSafePlayer*> players;
    Supervisor(int n, std::vector<size_t> sizes, const Function<double>* av, const Function<double>* f) {
		size = n;
        for (int i = 0; i < n; i++) {
            players.push_back(new ThreadSafePlayer(sizes, av, f));
        }
    }

	void evolve(int i = -1, double mutationChance = 0.08, double crossoverChance = 0.5, bool verbose = true, bool test = true, int ffrequency = 100, int testSize = 1000) {

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
			newPlayers.push_back(new ThreadSafePlayer(&players[distance(arr, std::lower_bound(arr, arr + size, RandomGenerator::randomDouble()))]));
		}

		for (int i = 0; i < size; i++) {
			delete players[i];
			players[i] = newPlayers[i];
		}
		
	}
	
	void mutate(double mutationChance) {

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

    void sortPlayers() {
        std::sort(players.begin(), players.end(), [] (ThreadSafePlayer *a, ThreadSafePlayer *b) {
            return a->getScore() > b->getScore();
        });
    }

    void benchmarkBestRandom(int n = 1000) {
        sortPlayers();
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
