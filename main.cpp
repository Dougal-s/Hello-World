#include <algorithm>
#include <array>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>

namespace {
	constexpr int populationSize = 1000;

	std::mt19937 rng;
}

class String {
public:
	static size_t maxStrLen;

	String() {
		std::uniform_int_distribution<size_t> dist(0, maxStrLen);
		outString.resize(dist(rng));

		std::uniform_int_distribution<char> charDist(32, 126);
		for (char& character : outString)
			character = charDist(rng);
	}

	const std::string& getString() const {return outString;}

	void calculateError(const std::string& expectedString) {
		// difference in size * max character value
		error = std::abs(static_cast<int>(expectedString.size()-outString.size()))*128;
		for (size_t i = 0; i < expectedString.size(); ++i)
			error += std::abs(expectedString[i]-outString[i]);
		error *= error;
	}

	int getError() const {return error;}

	void mutate() {
		std::uniform_real_distribution<float> probabilityDist(0.f, 1.f);

		if (probabilityDist(rng) < resizeProb) {
			std::uniform_int_distribution<size_t> sizeDist(0, maxStrLen);
			outString.resize(sizeDist(rng));
		}

		std::uniform_int_distribution<char> charDist(-4, 4);
		for (char& character : outString) {
			if (probabilityDist(rng) < charChangeProb)
				character = std::clamp(character + charDist(rng), 32, 126);
		}
	}

private:
	static constexpr float resizeProb = 0.2f;
	static constexpr float charChangeProb = 0.4f;

	int error;

	std::string outString;
};

size_t String::maxStrLen = 0;

template <int size>
class Population {
public:
	Population(const std::string& inTargetString) : targetString(inTargetString) {}

	int gen() {return generation;}

	const String& getBestString() const {return strings.front();}

	void update() {
		calculateError();
		selectNextGeneration();
		mutate();
		++generation;
	}

private:

	int generation = 0;

	const std::string targetString;

	std::array<String, size> strings;

	void calculateError() {
		for (auto& string : strings)
			string.calculateError(targetString);
	}


	// Sets every string to the string with the smallest error
	void selectNextGeneration() {
		const String* bestStr = &strings.front();
		for (const auto& string : strings) {
			if (string.getError() < bestStr->getError())
				bestStr = &string;
		}

		strings.fill(*bestStr);
	}

	void mutate() {
		for (size_t i = 1; i < strings.size(); ++i)
			strings[i].mutate();
	}
};

int main(int argc, char *argv[]) {
	rng.seed(std::chrono::system_clock::now().time_since_epoch().count());

	std::string targetString = "Hello, World!";

	if (argc > 1)
		targetString = argv[1];

	String::maxStrLen = 2*targetString.length();

	Population<1000> population(targetString);

	while (population.getBestString().getString() != targetString) {
		population.update();
		std::cout << "Gen: " << std::setw(3) << std::right << population.gen()
		          << " | Error: " << std::setw(5) << std::right << population.getBestString().getError()
		          << " | \"" << population.getBestString().getString() << "\"\n";
	}

	std::cout << population.getBestString().getString() << std::endl;

	return 0;
}
