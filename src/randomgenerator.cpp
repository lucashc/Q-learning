#ifndef RANDOMGEN
#define RANDOMGEN

#include <random>

class RandomGenerator {

	public:
		static std::default_random_engine generator;

		static double randomDouble(double min, double max) {
			std::uniform_real_distribution<double> dist(min, max);
			return dist(generator);
		}
};

std::default_random_engine RandomGenerator::generator = std::default_random_engine(time(0));

#endif
