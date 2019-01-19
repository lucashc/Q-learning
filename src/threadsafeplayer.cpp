#ifndef PLAYER
#define PLAYER
#include "gamestate.cpp"
#include <vector>
#include <string>
#include <sstream>
#include <tuple>
#include "matrix.cpp"
#include "neural-network.cpp"
#include <iostream>
#include <random>
#include <mutex>


class ThreadSafePlayer : public NeuralNetwork<double>{

	private:

		using VectorMatrix = std::vector<Matrix<double>>;
		using VectorGameState = std::vector<GameState>;

		std::mutex *scoreMutex;
		double score = 0;

		static std::default_random_engine generator;

	public:

		ThreadSafePlayer(std::vector<size_t> sizes, const Function<double>* av, const Function<double>* f) : NeuralNetwork(sizes, av, f) {
			scoreMutex = new std::mutex();
		}

		void addScore(double change) {
			scoreMutex->lock();
			score += change;
			scoreMutex->unlock();
		}

		void setScore(double nScore) {
			scoreMutex->lock();
			score = nScore;
			scoreMutex->unlock();
		}

		double getScore() {
			return score;
		}


		static double eval(ThreadSafePlayer* p1, ThreadSafePlayer* p2) {

			GameState board;

			for (int k = 0; k < BOARD_SIZE/2 - 2; k++) {

				auto [x1, y1] = p1->predictMove(board);
				board.placePiece(x1, y1, 1);

				auto [x2, y2] = p2->predictMove(board);
				board.placePiece(x2, y2, -1);

			}
			return board.getScore();

		}

		std::tuple<int, int> predictMove(const GameState& s) {

			int c = s.getColour();
			auto moves = s.validMoves(c);
			auto [p, q] = moves[0];
			double m = evaluate(c*s.potentialBoard(p, q, c).input())[0]; 
			int r = 0;

			for (unsigned int i = 1; i < moves.size(); i++) {

				auto [x, y] = moves[i];
				double p = evaluate(c*s.potentialBoard(x, y, c).input())[0];

				if (p > m) {
					m = p;
					r = i;
				}
			}

			return moves[r];
		}

		std::tuple<int, int, int> randomBenchmarkerSingle() {

			GameState white;
			GameState black;

			for (int k= 0; k < BOARD_SIZE/2-2; k++) {

				// White AI
				auto [x1, y1] = predictMove(white);
				white.placePiece(x1, y1, 1);

				// White Random
				std::vector<std::tuple<int, int>> moves1 = white.validMoves(-1);
				std::uniform_int_distribution<int> distribution1(0, moves1.size()-1);
				auto [i1, j1] = moves1[distribution1(generator)];
				white.placePiece(i1, j1, -1);

				// Black Random
				std::vector<std::tuple<int, int>> moves2 = black.validMoves(1);
				std::uniform_int_distribution<int> distribution2(0, moves2.size()-1);
				auto [i2, j2] = moves2[distribution2(generator)];
				black.placePiece(i2, j2, 1);

				// Black AI
				auto [x2, y2] = predictMove(black);
				black.placePiece(x2, y2, -1);
			}

			int whiteScore = white.getScore();
			int blackScore = black.getScore() * -1;
			int wins = (whiteScore > 0 ? 1 : 0) + (blackScore > 0 ? 1 : 0);
			int loses = (whiteScore < 0 ? 1 : 0) + (blackScore < 0 ? 1 : 0);

			return {(whiteScore + blackScore), wins, loses};
		}

		std::tuple<double, double, double> randomBenchmarker(int n = 1000) {
			int result = 0;
			int wins = 0;
			int loses = 0;
			for (int i = 0; i < n; i++) {
				auto [score, win, lose] = randomBenchmarkerSingle();
				result += score;
				wins += win;
				loses += lose;
			}
			return {(double) result / ((double)n * 2), (double)wins/((double)n * 2) * 100, (double) loses / ((double) n*2) * 100};
		}
		~ThreadSafePlayer() {
			delete scoreMutex;
		}

};

std::default_random_engine ThreadSafePlayer::generator = std::default_random_engine(time(0));

#endif
