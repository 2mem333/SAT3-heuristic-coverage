#include "math.h"
#include <random>
#include <cstring>

static std::mt19937 rng(std::random_device{}());
static std::bernoulli_distribution coin(0.5); 

int solve_sat_problem(int* problemPtr, int clauseCount, int variableCount,
bool rule1, bool rule2, double sigma, float MAXEPOCH = 10000000) //INFINITY.
{
	int* problem = new int[clauseCount*variableCount];
	std::memcpy(problem, problemPtr, clauseCount * variableCount * sizeof(int));

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

	double epoch = 0;
	while (epoch < MAXEPOCH) {
		epoch++;
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
				return 0;

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
			/*std::cout << "Finished, all clauses covered.\n";
			for (int i = 0; i < variableCount; i++)
			{
				std::cout << solution[i] << " ";
			}*/
			return 1;
		}
	}
}