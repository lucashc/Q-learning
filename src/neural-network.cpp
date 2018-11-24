#ifndef NEURALNETWORK
#define NEURALNETWORK
#include "matrix.cpp"
#include <vector>
#include <algorithm>
#include <cmath>
#include "approx.cpp"
#include "activators.cpp"
#include <functional>

template<typename T>
class NeuralNetwork : public Approximator<T> {

	private:

		std::vector<Matrix<T>> weights;
		std::vector<Matrix<T>> biases;
		size_t inputSize;
		size_t outputSize;
		std::function<Matrix<T>(const Matrix<T>&)> activation;
		std::function<Matrix<T>(const Matrix<T>&)> derivative;

	public:

		NeuralNetwork(std::vector<size_t> sizes, const Function<T>& av) {
			activation = Matrix<T>::wrap([&av] (const T x) { return av.function(x);});
			derivative = Matrix<T>::wrap([&av] (const T x) { return av.derivative(x);});
			inputSize = sizes[0];
			outputSize = *sizes.rbegin();
			for (auto p = sizes.begin(), c = sizes.begin() + 1; c < sizes.end(); c++, p++) {
				biases.push_back(Matrix<T>::initializeRandom(*c, 1));
				weights.push_back(Matrix<T>::initializeRandom(*c, *p));
			}
		}

		Matrix<T> evaluate(Matrix<T> m){
			for (auto w = weights.begin(), b = biases.begin(); w < weights.end(); w++, b++)
				m = activation((*w ^ m) + *b);
			return m;
		}

		std::vector<Matrix<T>> playback(Matrix<T> m){
			std::vector<Matrix<T>> backwards;
			backwards.push_back(m);
			for (auto w = weights.begin(), b = biases.begin(); w < weights.end(); w++, b++){
				m = activation((*w ^ m) + *b);
				backwards.push_back(m);
			}
			return backwards;
		}

		void update(Matrix<T>& data, Matrix<T>& target, const T &learning_rate) {
			std::vector<Matrix<T>> x = playback(data);
			Matrix<T> error_derivative = x.back() - target;
			std::cout << x.back().rows << "x" << x.back().columns << std::endl;
			std::cout << weights.size() << std::endl;
			Matrix<T> last_delta = error_derivative * Activator<T>::activationDerivative(weights.back() ^ x.rbegin()[1], activationType);
			std::cout << last_delta.rows << "x" << last_delta.columns << std::endl;
			std::vector<Matrix<T>> deltas;
			deltas.push_back(last_delta);
			std::cout << "Going into loop" << std::endl;
			for (int i = weights.size() - 2; i >= 0; i--) {
				std::cout << i << std::endl;
				std::cout << weights[i].rows << "x" << weights[i].columns << std::endl;
				std::cout << x[i].rows << "x" << x[i].columns << std::endl;
				Matrix<T> delta = weights[i+1].transpose() ^ deltas.back() * Activator<T>::activationDerivative(weights[i] ^ x[i], activationType);;
				std::cout << delta.rows << "x" << delta.columns << std::endl;
				deltas.push_back(delta);
			};
			std::reverse(deltas.begin(), deltas.end());
			for (unsigned int i = 0; i < deltas.size(); i++) {
				std::cout << "Updating " << weights[i].rows << "x" << weights[i].columns << std::endl;
				std::cout << "with: " << (deltas[i] ^ x[i].transpose()).rows << "x" << (deltas[i] ^ x[i].transpose()).columns << std::endl;
				weights[i] -= (deltas[i] ^ x[i].transpose()) * learning_rate;
			};
		}

		void train(Matrix<T> &data, Matrix<T> &target, const T &learning_rate){
			for (int i = 0; i < 1000; i++) {
				update(data, target, learning_rate);
			};
		}
};

#endif
