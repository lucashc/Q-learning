#ifndef GENFLIPPO
#define GENFLIPPO
#include "flippo.cpp"
#include "neural-network.cpp"
#include <mutex>

class sNeuralNetwork : public NeuralNetwork<double> {
	
	private:

		std::mutex scoreMutex;
		double score = 0;

	public:

		static double eval(FlippoGen* p1, FlippoGen* p2) {

			GameState board;

			for (int k = 0; k < BOARD_SIZE/2 - 2; k++) {

				auto [x1, y1] = p1->predictMove(board);
				board.placePiece(x1, y1, 1);

				auto [x2, y2] = p2->predictMove(board);
				board.placePiece(x2, y2, -1);

			}

			return board.getScore();

		}

		void addScore(double change) {
			scoreMutex.lock();
			score += change;
			scoreMutex.unlock();
		}

		void setScore(double nScore) {
			scoreMutex.lock();
			score = nScore;
			scoreMutex.unlock();
		}

		double getScore() {
			return score;
		}
};
