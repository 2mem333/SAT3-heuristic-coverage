#include <iostream>
#include "math.h"


int main()
{
	int totalCoveredClauses = 0;
	int clauseCount;
	int variableCount;
	int *problem;

	//readCNF("cnf//uf20-06.cnf", problem, clauseCount, variableCount);

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


	//for (int c = 0; c < clauseCount; c++)
	//{
	//	std::cout << "(";
	//	for (int l = 0; l < variableCount; l++)
	//	{
	//		if (problem[c * variableCount + l] == 0) continue;
	//		if (problem[c * variableCount + l] == -1)
	//		{
	//			std::cout << "'" << l << " + ";
	//		}
	//		else
	//		{
	//			std::cout << l << " + ";
	//		}
	//	}
	//	std::cout << ").";
	//}
	//std::cout << "\n";

	int it = 0;
start:
	//std::cout << "iteration : " << ++it << "\n";
	//if (it > 1000)
	//{
	//	for (int c = 0; c < clauseCount; c++)
	//{
	//	std::cout << "(";
	//	for (int l = 0; l < variableCount; l++)
	//	{
	//		if (problem[c * variableCount + l] == 0) continue;
	//		if (problem[c * variableCount + l] == -1)
	//		{
	//			std::cout << "'" << l << " + ";
	//		}
	//		else
	//		{
	//			std::cout << l << " + ";
	//		}
	//	}
	//	std::cout << ").";
	//}
	//std::cout << "\n";
	//	return -1;
	//}

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
			std::cout << "Impossible to solve problem.\n";

			std::cout << totalCoveredClauses << " clauses are covered.\n";
			for (int i = 0; i < variableCount; i++)
			{
				std::cout << solution[i] << " ";
			}
			return 1;
		}

		if (priorityScore == 1)
		{
			for (int v = 0; v < variableCount; v++)
			{
				if (problem[c * variableCount + v] != 0)
					variableWeight[v] += 100 * problem[c * variableCount + v];
			}
		}
		else if (priorityScore == 2)
		{
			for (int v = 0; v < variableCount; v++)
			{
				if (problem[c * variableCount + v] != 0)
					variableWeight[v] += 50 * problem[c * variableCount + v];
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
								deltaW -= 0.154f;
							}
						}
					}
					if (problem[c * variableCount + v] == -1)
					{
						if (problem[c * variableCount + v2] == -1)
						{
							if (variableDensity[v2] < 0)
							{
								deltaW += 0.154f; //negative effect on weights of literals negotiated.

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
		if (abs(variableWeight[i]) > maxVal)
		{
			maxVal = abs(variableWeight[i]);
			selectedLit = i;
		}
	}

	int direction;
	variableWeight[selectedLit] > 0 ? direction = 1: direction = -1;
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
	
	goto start;



}
