#include <iostream>
#include <fstream>
#include <string>
#include <cctype>
#include <filesystem>

void readWordsFromFile(const std::string& fileName, const int threadNumber);
void processFile(int threadNumber, const std::string& fileName);
void storeWordInMap(const std::string& word, const int threadNumber);
void writeMap(const std::string& fileName, std::unordered_map<std::string,int>& wordMap);
void reduceBucketFiles(const int bucketNumber);
void reduce(const int threadNumber, const int bucketNumber);






