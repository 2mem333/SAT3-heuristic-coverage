#include <iostream>
#include "math.h"

#include <ctime>
#include <chrono>

#include "HeuristicCoverage.h"
#include "ParseCNF.h"

#define OUTPUT_DEBUGG
int main()
{
	//------PARAMETERS-----------
	double MAXTRY = 30;
	double SIGMA = 0.1f;
	double SIGMA_CONSTANT = 0.05f;
	//---------------------------

	int solved = 0;
	int clauseCount;
	int variableCount;
	int* problem;

	double totalFileParsingTime = 0;
	double totalAlgorithmTime = 0;

	using clock = std::chrono::steady_clock;

	for (int start = 1; start <= 300; ++start)
	{
		auto t0 = clock::now();
		std::string num;
		num = std::to_string(start);

		std::string cnfPath = "C:\\Users\\negan\\Desktop\\benchmark\\r3sat\\d10_p20(random)\\e" + num;
		if (!readCNF(cnfPath, problem, clauseCount, variableCount))
		{
			std::cout << "Error parsing file: " << start << "!\n";
			continue;
		}
		auto t1 = clock::now();

		int result = 0;
		int cycle = 0;

		//TRY TO SOLVE WITH BP RULE.
		while (result == 0 && cycle < MAXTRY)
		{
			result = solve_sat_problem(problem, clauseCount,variableCount,
				1, 1, SIGMA + cycle * SIGMA_CONSTANT);

			cycle++;
		}

		//IF STILL THERE IS STILL NO SOLUTION, DISABLE BP RULE AND TRY AGAIN
		if (result == 0)
		{	cycle = 0;
			while (result == 0 && cycle < MAXTRY)
			{
				result = solve_sat_problem(problem, clauseCount, variableCount,
					1, 0, SIGMA + cycle * SIGMA_CONSTANT);

				cycle++;
			}
		}
		solved += result;
		auto t2 = clock::now();

		double fileParsingTime = std::chrono::duration<double, std::milli>(t1 - t0).count();
		double algorithmTime = std::chrono::duration<double, std::milli>(t2 - t1).count();

		totalFileParsingTime += fileParsingTime;
		totalAlgorithmTime += algorithmTime;

#ifdef OUTPUT_DEBUG
		{
			if (result)
				std::cout << "SAT: " << start;
			else
				std::cout << "UNSAT: " << start;
			std::cout << "  File parsing time (ms): " << fileParsingTime << "  Algorithm time (ms): " << algorithmTime << "\n";
		}
#endif
	}

	std::cout << "\n-------------------------\n";
	std::cout << "File parsing time: (ms) " << totalFileParsingTime << "\n";
	std::cout << "Total algorithm execution time (ms): " << totalAlgorithmTime << "\n";

	std::cout << "Total solved: " << solved << " / 300\n";
	std::cout << "Success Rate: " << float(solved / 3) << "%\n";
	return 0;
}