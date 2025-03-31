#include <iostream>
#include <fstream>
#include <string>
#include <cctype>
#include <filesystem>

void readWordsFromFile(const std::string& fileName, const int threadNumber);
void processFile(int threadNumber, const std::string& fileName);
void storeWordInFile(const std::string& word, const int threadNumber);
void createMap(const std::string& fileName, std::unordered_map<std::string,int>& wordMap);
void writeMap(const std::string& fileName, std::unordered_map<std::string,int>& wordMap);
void sumWordsCounts();
void createMapAndCount(const std::string& fileName);
void map(int threadNumber, const std::string& fileName);
void reduceBucketFiles(const int bucketNumber);
void reduce(const int threadNumber, const int bucketNumber);






