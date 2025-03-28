#include <iostream>
#include <fstream>
#include <string>
#include <cctype>
#include <filesystem>

void readWordsFromFile(const std::string& filename);
void storeWordInFile(const std::string& word, const int thread_number);
void sumWordsCounts();
void createMapAndCount(const std::string& filename);


