#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include <string>
#include <fstream>
#include <unordered_map>
#include <omp.h>
using namespace std;

vector<string> docList;
unsigned* result;
unordered_map<string, int> dictMap;
ifstream dictFile;
ofstream outFile;
int i;
double startTime, endTime;
int numThreads = 12;

int main(int argc, char* argv[])
{
	dictFile.open("dict.txt");
	int i = 0;
	while (!dictFile.fail()) {
		string s;
		dictFile >> s;
		if (!s.empty()) dictMap[s] = i;
		i++;
	}
	dictFile.close();

	struct _finddata_t file;
	intptr_t handle;
	if ((handle = _findfirst(".\\test\\*", &file)) != -1) {
		do {
			if ((file.attrib & _A_SUBDIR)) continue;
			string s(".\\test\\");
			s += file.name;
			docList.push_back(s);
		} while (_findnext(handle, &file) == 0);
		_findclose(handle);
	}

	result = (unsigned*)malloc(sizeof(unsigned) * docList.size() * dictMap.size());
	memset(result, 0, sizeof(unsigned) * docList.size() * dictMap.size());

	startTime = omp_get_wtime();

#pragma omp parallel num_threads(numThreads)
#pragma omp for schedule (dynamic)
	for (i = 0; i < docList.size(); i++) {
		ifstream docFile(docList[i]);
		while (!docFile.fail()) {
			string s;
			getline(docFile, s, ' ');
			if (dictMap.find(s) == dictMap.end()) continue;
			result[i * dictMap.size() + dictMap[s]]++;
		}
	}

	endTime = omp_get_wtime();

	outFile.open("out.txt");
	for (i = 0; i < docList.size(); i++) {
		for (int j = 0; j < dictMap.size(); j++) {
			outFile << result[i * dictMap.size() + j] << " ";
		}
		outFile << endl;
	}
	outFile.close();

	printf("Thread Num: %d, Time: %fs\n", numThreads, endTime - startTime);

	return 0;
}