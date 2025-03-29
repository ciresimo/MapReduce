#include <iostream>
#include <fstream>
#include <string>
#include <cctype>
#include <filesystem>

void readWordsFromFile(const std::string& fileName, const int threadNumber);
void storeWordInFile(const std::string& word, const int threadNumber);
void sumWordsCounts();
void createMapAndCount(const std::string& fileName);
void map(const std::string& fileName, const int threadNumber);
void processFile(int threadNumber, const std::string& fileName);



