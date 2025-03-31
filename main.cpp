#include <unordered_map>
#include <thread>
#include <vector>
#include <mutex>
#include <mr.h>
#include <ctpl.h>

using namespace std;

const std::filesystem::path outputPath{"../output/"};
const int threadsTotalNumber = 4;
const int bucketsTotalNumber = 4;

std::unordered_map<std::string, std::unordered_map<std::string, int>> memoryBanks;

void readWordsFromFile(const std::string& fileName, const int threadNumber)
{
    std::ifstream file(fileName);
    if (!file)
    {
        std::cerr << __func__ << " | Error: Unable to open file " << fileName << std::endl;
        return;
    }

    std::string word;
    while (file >> word)
    {
        // Remove non-alphanumeric characters from the word
        std::string cleanedWord;
        for (char ch : word)
        {
            if (std::isalnum(ch) && !std::isdigit(ch))
            {
                // Make sure that all letters are low case
                cleanedWord += std::tolower(ch);
            }
        }
        
        if (!cleanedWord.empty())
        {
            // storeWordInFile(cleanedWord, threadNumber);
            storeWordInMap(cleanedWord, threadNumber);
            
        }
    }

    file.close();
}

void processFile(const int threadNumber, const std::string& fileName)
{
    std::cout << "Thread " << threadNumber << " is processing file: " << fileName << std::endl;
    readWordsFromFile(fileName, threadNumber);
    return;
}

void storeWordInMap(const std::string& word, const int threadNumber)
{
    const char firstLetter = word[0];  // Get the first letter
    int bucket = int(firstLetter - 'a') % bucketsTotalNumber;    // Compute bucket index M normalized to range 0-25

    std::string fileName = outputPath.string() + "mr-" + std::to_string(threadNumber) + "-" + std::to_string(bucket) + ".txt";
    
    // Check if the fileName already exist in memoryBanks
    if(memoryBanks.find(fileName) != memoryBanks.end())
    {
        int occurrence = 1;
        // Chek if the word already exists in the "file". In case increase the current occurrence
        if(memoryBanks[fileName].find(word) != memoryBanks[fileName].end())
        {
            occurrence += memoryBanks[fileName][word];
        }
        memoryBanks[fileName].insert_or_assign(word, occurrence);
    }
    else
    {
        // The "file", doesn't exists. So insert a new element in memoryBanks and immediately add the word
        memoryBanks[fileName].emplace(word, 1);

    }
    return;
}

void writeMap(const std::string& fileName, std::unordered_map<string,int>& wordMap)
{
    // Open the file in output mode to store the map. Make sure to clear the file before writing
    std::ofstream outFile(fileName, std::ios::trunc);
    if (!outFile)
    {
        std::cerr << __func__ << " | Error: Unable to open file " << fileName << " for writing." << std::endl;
        return;
    }
    std::cout << "Creating hash map for file:" << fileName << std::endl;

    // Write the map contents to the file (word and its count)
    for (const auto& entry : wordMap)
    {
        outFile << entry.first << "," << entry.second << "\n";
    }

    outFile.close();
    return;
}

void reduceBucketFiles(const int bucketNumber)
{
    std::unordered_map<std::string,int> wordMap;

    // Create a unique map for all the mr-i-bucketNumber "files" (the maps associated to fileName)
    for(int i = 0; i < threadsTotalNumber; i++)
    {
        std::string fileName = outputPath.string() + "mr-" + std::to_string(i) + "-" + std::to_string(bucketNumber) + ".txt";
        // Check that the "file" actually exists
        if(memoryBanks.find(fileName) != memoryBanks.end())
        {
            // Score through the entire "file": add each word to the new map, making sure to increase the occurrence if already exists
            for (const auto& entry : memoryBanks[fileName])
            {
                std::string word = entry.first;
                int occurence = entry.second;
                if(wordMap.find(word) != wordMap.end())
                {
                    wordMap[word] += occurence;
                }
                else
                {
                    wordMap.insert_or_assign(word, occurence);
                }

            }
        }
        else
        {
            std::cerr << __func__ << "Error when trying to access memoryBanks" << std::endl;
            return;
        }

    }

    // Write it down in a single file
    std::string outputFileName = outputPath.string() + "out-" + std::to_string(bucketNumber) + ".txt";
    writeMap(outputFileName, wordMap); 
    return;
}

void reduce(const int threadNumber, const int bucketNumber)
{
    std::cout << "Thread " << threadNumber << " is reducing files from bucket: " << bucketNumber << std::endl;
    reduceBucketFiles(bucketNumber);
    return;  
}

int main(int argc, char* argv[])
{
    if (argc < 9)
    {
        std::cerr << "Error: Please provide at least 8 input files. Only " << argc << " recieved" << std::endl;
        return 1;
    }

    // Make sure that output folder exists and is empty
    if (!std::filesystem::exists(outputPath))
    {
        std::filesystem::create_directory(outputPath);
    }
    else
    {
        // Clear the folder if it exists
        for (const auto& entry : std::filesystem::directory_iterator(outputPath))
        {
            std::filesystem::remove(entry);
        }
    }

    // MAP

    // Extract all words from input files and store them in the maps
    ctpl::thread_pool pool1(threadsTotalNumber);
    for (int i = 1; i < argc; i++)
    {  
        pool1.push(processFile,argv[i]);
    }

    // Wait for all threads to complete the work
    pool1.stop(true);

    // REDUCE
    ctpl::thread_pool pool2(bucketsTotalNumber);

    for(int i = 0; i < bucketsTotalNumber; i++)
    {
        pool2.push(reduce,i);        
    }
    pool2.stop(true);

    return 0;
}
