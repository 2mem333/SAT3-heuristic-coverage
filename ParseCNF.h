#include <fstream>
#include <string>
#include <cstdlib>   

int readCNF(std::string filename,
	int*& problem,
	int& clauseCount,
	int& variableCount)
{
	std::ifstream in(filename);
	if (!in) return 0;

	std::string s;

	while (in >> s) {
		if (s == "c") {
			std::getline(in, s); 
		}
		else if (s == "p") {
			in >> s; 
			in >> variableCount >> clauseCount;
			break;
		}
	}

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
	return 1;
}