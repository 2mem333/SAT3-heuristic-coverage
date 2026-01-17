#include <iostream>
#include "math.h"

#include <fstream>
#include <string>
#include <cstdlib>  

#include <ctime>
#include <chrono>

#include <random>
static std::mt19937 rng(std::random_device{}());
static std::bernoulli_distribution coin(0.5);

void readCNF(std::string filename,
	int*& problem,
	int& clauseCount,
	int& variableCount)
{
	std::ifstream in(filename);
	if (!in) return;

	std::string s;

	while (in >> s) {
		if (s == "c") {
			std::getline(in, s); // comment satırını atla
		}
		else if (s == "p") {
			in >> s; // "cnf"
			in >> variableCount >> clauseCount;
			break;
		}
	}

	// matrix allocate
	problem = new int[clauseCount * variableCount];
	for (int i = 0; i < clauseCount * variableCount; i++)
		problem[i] = 0;

	int lit, clause = 0;
	while (in >> lit && clause < clauseCount) {
		if (lit == 0) {
			clause++;
			continue;
		}

		int var = abs(lit) - 1;
		int sign = (lit > 0) ? 1 : -1;

		problem[clause * variableCount + var] = sign;
	}

	in.close();
}

bool isSolutionValid(const int* problem,
	int clauseCount,
	int variableCount,
	const int* solution)
{
	for (int c = 0; c < clauseCount; ++c) {
		bool satisfied = false;

		for (int v = 0; v < variableCount; ++v) {
			int lit = problem[c * variableCount + v];
			if (lit == 0) continue;        // clause'ta yok
			if (solution[v] == 2) continue; 

			if (solution[v] == lit) {
				satisfied = true;
				break;
			}
		}

		if (!satisfied) {
			std::cout << "no: " << c << "\n";
			return false;
		}
	}

	return true;
}


int solve_sat_problem(std::string cnfPath, bool rule1, bool rule2, double sigma)
{
	int clauseCount;
	int variableCount;
	int* problem;

	readCNF(cnfPath, problem, clauseCount, variableCount);

	int totalCoveredClauses = 0;
	int* variableDensity = new int[variableCount];
	int* solution = new int[variableCount];
	bool* coveredClauses = new bool[clauseCount];
	float* variableWeight = new float[variableCount];

	for (int i = 0; i < clauseCount; i++)
		coveredClauses[i] = 0;
	for (int i = 0; i < variableCount; i++) {
		variableDensity[i] = 0;
		solution[i] = 2;
	}


	while (true) {
		//--------------------INITILIZE VARS-------------------
		for (int i = 0; i < variableCount; i++) {
			variableDensity[i] = 0;
			variableWeight[i] = 0;
		}

		//--------------------APPLY UNIT & BINARY PROPORGATION------------
		for (int c = 0; c < clauseCount; c++)
		{
			if (coveredClauses[c] == 1)
				continue;

			int priorityScore = 0;

			for (int v = 0; v < variableCount; v++)
			{
				//handle with density calcs along the way
				variableDensity[v] += problem[c * variableCount + v];

				if (problem[c * variableCount + v] != 0)
					priorityScore++;
			}

			if (priorityScore == 0)
			{
				//std::cout << "Impossible to solve problem.\n";

				//std::cout << totalCoveredClauses << " clauses are covered.\n";
				//for (int i = 0; i < variableCount; i++)
				//{
				//	std::cout << solution[i] << " ";
				//}
				return 0;
			}

			if (priorityScore == 1 && rule1)
			{
				for (int v = 0; v < variableCount; v++)
				{
					if (problem[c * variableCount + v] != 0)
						variableWeight[v] += clauseCount * problem[c * variableCount + v];
				}
			}
			else if (priorityScore == 2 && rule2)
			{
				for (int v = 0; v < variableCount; v++)
				{
					if (problem[c * variableCount + v] != 0)
						variableWeight[v] += clauseCount * problem[c * variableCount + v] / 2;
				}
			}
		}

		//--------------------CALCULATE WEIGHTS FOR VARIABLES--
		for (int v = 0; v < variableCount; v++)
		{
			float deltaW = 0;

			for (int c = 0; c < clauseCount; c++)
			{
				if (coveredClauses[c] == 1)
					continue;

				if (problem[c * variableCount + v] == 0)
					continue;

				deltaW += problem[c * variableCount + v];

				for (int v2 = 0; v2 < variableCount; v2++)
				{
					if (v2 == v) //could be disabled
						continue;

					if (problem[c * variableCount + v] == 1)
					{
						if (problem[c * variableCount + v2] == 1)
						{
							if (variableDensity[v2] > 0)
							{
								deltaW -= sigma;
							}
						}
					}
					if (problem[c * variableCount + v] == -1)
					{
						if (problem[c * variableCount + v2] == -1)
						{
							if (variableDensity[v2] < 0)
							{
								deltaW += sigma; //negative effect on weights of literals negotiated.

							}
						}
					}

				}
			}
			variableWeight[v] += deltaW;
		}

		//--------------------CHOOSE LITERAL TO BE ASSIGNED----
		//tie breaking rule = select the first indexed biggets

		int selectedLit = 0;
		float maxVal = -1;

		for (int i = 0; i < variableCount; i++)
		{
			if (solution[i] == 2) {
				if (abs(variableWeight[i]) > maxVal)
				{
					maxVal = abs(variableWeight[i]);
					selectedLit = i;
				}
				else if (abs(variableWeight[i]) == maxVal && coin(rng)) //TIE BREAKING RULE IS BEING APPLYING.
				{
					maxVal = abs(variableWeight[i]);
					selectedLit = i;
				}
			}
		}

		int direction;
		variableWeight[selectedLit] > 0 ? direction = 1 : direction = -1;
		solution[selectedLit] = direction;

		//--------------------REMOVE COVERED CLAUSES-----
		for (int c = 0; c < clauseCount; c++)
		{
			if (problem[c * variableCount + selectedLit] == direction)
			{
				totalCoveredClauses++;
				coveredClauses[c] = 1; //MEANS THAT WE COVERED 
				for (int v = 0; v < variableCount; v++)
				{
					problem[c * variableCount + v] = 0;
				}
			}
		}

		//--------------------REMOVE VARIABLE FROM CLAUSES-----
		for (int c = 0; c < clauseCount; c++)
			problem[c * variableCount + selectedLit] = 0;

		//------------CHECK IF FINISHED---------------
		if (totalCoveredClauses >= clauseCount)
		{
			std::cout << "Finished, all clauses covered.\n";
			for (int i = 0; i < variableCount; i++)
			{
				std::cout << solution[i] << " ";
			}
			//readCNF("problem.txt", problem, clauseCount, variableCount);
			//std::cout << "\nIs really sat?: " << isSolutionValid(problem, clauseCount, variableCount, solution);

			return 1;
		}
	}
}

int main(int argc, char* argv[])
{
	if (argc < 2) {
		std::cerr << "Usage: " << argv[0] << " <cnf_file_path>" << std::endl;
		return 1;
	}

	const char* cnfPath = argv[1];

	int clauseCount;
	int variableCount;
	int* problem;
	int maxtry = 30;

	auto zamanbaslangic = std::chrono::high_resolution_clock::now();

	int result = 0;
	int cycle = 0;

	while (result == 0 && cycle < maxtry)
	{
		result = solve_sat_problem(cnfPath, 1, 1, 0.1f + cycle * 0.05f);
		cycle++;
	}

	//IF STILL THERE IS NO SOLUTION, DISABLE BP RULE AND TRY AGAIN
	cycle = 0;
	if (result == 0)
	{
		while (result == 0 && cycle < maxtry)
		{
			result = solve_sat_problem(cnfPath, 1, 0, 0.1f + cycle * 0.05f);
			cycle++;
		}
	}


	auto zamanbitis = std::chrono::high_resolution_clock::now();
	auto toplam_zaman = std::chrono::duration_cast<std::chrono::nanoseconds>(zamanbitis - zamanbaslangic);
	double islemsuresi = toplam_zaman.count() * 0.000000001;
	std::cout << "\nTotal time: " << islemsuresi << "\n";

	return 0;
}
