#ifndef FLIPPO2
#define FLIPPO2
#include "flippo.cpp"
#include <vector>
#include <string>
#include <sstream>
#include <tuple>
#include "matrix.cpp"
#include "neural-network.cpp"
#include <iostream>
#include <random>


class FlippoGen {

	private:

		using VectorMatrix = std::vector<Matrix<double>>;
		using VectorGameState = std::vector<GameState>;

		NeuralNetwork<double>* nn;
		std::default_random_engine generator;

	public:

		Flippo(NeuralNetwork<double>* p) : nn(p) {
			generator = std::default_random_engine(time(0));
		}

		VectorGameState createGameWinner(double epsilon) {

			VectorGameState p1, p2;
			GameState current;
			double c = 1.0;
			std::uniform_real_distribution<double> randDouble(0.0, 1.0);
		
			for (int k = 0; k < BOARD_SIZE - 4; k++) {
				std::tuple<int, int> move;
				if (epsilon > randDouble(generator)) {
					std::vector<std::tuple<int, int>> moves = current.validMoves(c);
					std::uniform_int_distribution<int> distribution(0, moves.size()-1);
					move = moves[distribution(generator)];
				} else {
					move = predictMove(current);
				}
				auto [i, j] = move;
				if (c == 1.0) {
					p1.push_back(current);
				}else {
					p2.push_back(current);
				}
				current = current.potentialBoard(i, j, c);
				c *= -1.0;
			}
			if (p1.back().getScore() > p2.back().getScore()) {
				return p1;
			}else {
				return p2;
			}
			
		}

		VectorGameState createGame(double epsilon) {

			VectorGameState v;
			v.push_back(GameState());
			double c = 1.0;
			std::uniform_real_distribution<double> randDouble(0.0, 1.0);
			
			for (int k = 0; k < BOARD_SIZE - 4; k++) {
			
				if (epsilon > randDouble(generator)) {
					std::vector<std::tuple<int, int>> moves = v.back().validMoves(c);
					std::uniform_int_distribution<int> distribution(0, moves.size()-1);
					auto [i, j] = moves[distribution(generator)];
					v.push_back(v.back().potentialBoard(i, j, c));
				} else {
					auto [i, j] = predictMove(v.back());
					v.push_back(v.back().potentialBoard(i, j, c));
				}

				c *= -1.0;

			}

			return v;
		}

		std::tuple<int, int> predictMove(const GameState& s) {

			int c = s.getColour();
			auto moves = s.validMoves(c);
			auto [p, q] = moves[0];
			double m = nn->evaluate(c*s.potentialBoard(p, q, c).input())[0]; 
			int r = 0;

			for (unsigned int i = 1; i < moves.size(); i++) {

				auto [x, y] = moves[i];
				double p = nn->evaluate(c*s.potentialBoard(x, y, c).input())[0];

				if (p > m) {
					m = p;
					r = i;
				}
			}

			return moves[r];
		}

		Matrix<double> getTarget(GameState& s) {
			if (s.isFinal())
				return Matrix<double>(1, 1, {-s.getScore()})/MAX_SCORE;

			int c = s.getColour();
			auto moves = s.validMoves(c);
			auto [p, q] = moves[0];
			double m = nn->evaluate(c*s.potentialBoard(p, q, c).input())[0]; 

			for (auto [i, j] : moves) {
				double p = nn->evaluate(c*s.potentialBoard(i, j, c).input())[0];
				if (p > m)
					m = p;
			}

			return Matrix<double>(1, 1, std::vector<double>{m});
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

		std::tuple<double, double, double> randomBenchmarker(int n = 10) {
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

};

#endif
