#ifndef NEURALNETWORK
#define NEURALNETWORK
#include "matrix.cpp"
#include <vector>
#include <algorithm>
#include <cmath>
#include "activators.cpp"
#include <functional>
#include <utility>
#include <numeric>
#include <fstream>
#include <string>


template<typename T>
class NeuralNetwork {
	using VectorMatrix = typename std::vector<Matrix<T>>;
	private:

		size_t inputSize;
		size_t outputSize;

		const Function<T>* activationFunction;
		const Function<T>* finalFunction;
		constexpr static T threshold = 100;

		std::vector<std::function<Matrix<T>(const Matrix<T>&)>> activations;
		std::vector<std::function<Matrix<T>(const Matrix<T>&)>> derivatives;

	public:

		VectorMatrix weights;
		VectorMatrix biases;

		NeuralNetwork() {}

		NeuralNetwork(const std::string &filename) {
			std::cout << "Initializing from filename" << std::endl;
			readNetwork(filename);
		}

		NeuralNetwork(std::vector<size_t> sizes, const Function<T>* av, const Function<T>* f) {

			activationFunction = av;
			finalFunction = f;

			for (unsigned int i = 0; i < sizes.size() - 2; i++) {
				activations.push_back(Matrix<T>::wrap([av] (const T x) { return av->function(x);}));
				derivatives.push_back(Matrix<T>::wrap([av] (const T x) { return av->derivative(x);}));
			}

			activations.push_back(Matrix<T>::wrap([f] (const T x) { return f->function(x);}));
			derivatives.push_back(Matrix<T>::wrap([f] (const T x) { return f->derivative(x);}));

			inputSize = sizes[0];
			outputSize = *sizes.rbegin();

			for (auto p = sizes.begin(), c = sizes.begin() + 1; c < sizes.end(); c++, p++) {
				biases.push_back(Matrix<T>::initializeRandom(*c, 1));
				weights.push_back(Matrix<T>::initializeRandom(*c, *p));
			}
		}


		Matrix<T> evaluate(Matrix<T> m){

			for (unsigned int i = 0; i < weights.size(); i++)
				m = activations[i]((weights[i] ^ m) + biases[i]);

			return m;
		}

	void saveNetwork(const std::string &filename) const {
		/*
		* Save file layout:
		* Magic: SSVN
		* inputSize, outputSize
		* numLayers
		* weights, biases
		* avID, fID
		* File extension: .ssvn
		*/
		std::ofstream file;
		try {
			file.open(filename);
		} catch (const std::ofstream::failure &e) {
			std::cerr << "Error reading file" << std::endl;
			throw "Error writing file";
		}

		file << "SSVN ";
		file << inputSize << " " << outputSize << " ";

		file << weights.size() << " ";
		for (auto w : weights) {
			w.writeToFile(file);
		}

		for (auto b : biases) {
			b.writeToFile(file);
		}

		file << activationFunction->getID() << " " << finalFunction->getID();
		file.close();
	}

	void readNetwork(const std::string &filename) {
		std::ifstream file;
		try {
			file.open(filename);
		} catch (const std::ifstream::failure &e) {
			std::cerr << "Error reading file" << std::endl;
			throw "Error reading file";
		}

		// Ignore magic SSVN
		file.ignore(4, ' ');

		// Initialize weights and biases
		int numLayers;
		file >> inputSize >> outputSize >> numLayers;
		for (int i = 0; i < numLayers; i++) {
			Matrix<T> temp;
			temp = Matrix<T>::readFromFile(file);
			weights.push_back(temp);
		}

		// Get sizes from biases
		std::vector<size_t> sizes;
		sizes.push_back(inputSize);
		for (int j = 0; j < numLayers; j++) {
			Matrix<T> temp;
			temp = Matrix<T>::readFromFile(file);
			biases.push_back(temp);
			sizes.push_back(temp.rows);
		}

		// Initialize activations and derivatives
		int aID, fID;
		file >> aID >> fID;
		Function<T>* av = getFunctionFromID(aID);
		Function<T>* f  = getFunctionFromID(fID);
		activationFunction = av;
		finalFunction = f;

		for (unsigned int i = 0; i < sizes.size() - 2; i++) {
			activations.push_back(Matrix<T>::wrap([av] (const T x) { return av->function(x);}));
			derivatives.push_back(Matrix<T>::wrap([av] (const T x) { return av->derivative(x);}));
		}
		activations.push_back(Matrix<T>::wrap([f] (const T x) { return f->function(x);}));
		derivatives.push_back(Matrix<T>::wrap([f] (const T x) { return f->derivative(x);}));

		file.close();
	}

	static Function<T>* getFunctionFromID(const int ID, const T extra = 0) {
		// This function has to be put here, as the templates can now be matched
		switch (ID) {
			case 1: return new Linear<T>();
			case 2: return new Sigmoid<T>();
			case 3: return new TanH<T>();
			case 4: return new RELU<T>();
			case 5: return new LeakyRELU<T>();
			case 6: return new ELU<T>(extra);
			default: return new Linear<T>();
		}
	}

	void printNetwork() {
		for (unsigned int i = 0; i < weights.size(); i++) {
			std::cout << weights[i] << std::endl << biases[i] << std::endl;
		}

	}

};

#endif
