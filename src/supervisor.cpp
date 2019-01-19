#ifndef SUPERVISOR
#define SUPERVISOR
#include <thread>
#include <vector>
#include "threadsafeplayer.cpp"
#include "activators.cpp"
#include <iostream>


class Supervisor {
private:
    
public:
    std::vector<ThreadSafePlayer*> players;
    Supervisor(int n, std::vector<size_t> sizes, const Function<double>* av, const Function<double>* f) {
        for (int i = 0; i < n; i++) {
            players.push_back(new ThreadSafePlayer(sizes, av, f));
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
            p1->addScore(score);
            p2->addScore(-score);
        }
    }

    void PlayCompetition() {
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
    void SortPlayers() {
        std::sort(players.begin(), players.end(), [] (ThreadSafePlayer *a, ThreadSafePlayer *b) {
            return a->getScore() > b->getScore();
        });
    }
    void BenchmarkBestRandom(int n = 1000) {
        SortPlayers();
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
