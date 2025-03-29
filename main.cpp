#include <unordered_map>
#include <thread>
#include <vector>
#include <mutex>
#include <mr.h>
#include <ctpl.h>

using namespace std;

std::string outputDirectory = "../output/";
std::mutex output_mutex;

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
                cleanedWord += std::tolower(ch);
            }
        }
        
        if (!cleanedWord.empty())
        {
            storeWordInFile(cleanedWord, threadNumber);
        }
    }

    file.close();
}

void storeWordInFile(const std::string& word, const int threadNumber)
{
    const char firstLetter = word[0];  // Get the first letter
    // const int M = 1; //TODO: get is an input when parallelizing stuff
    // int bucket = int(firstLetter) % M;    // Compute bucket index M
    int bucket = int(firstLetter);    // Compute bucket index M

    std::string fileName = outputDirectory + "mr-" + std::to_string(threadNumber) + "-" + std::to_string(bucket) + ".txt";
    std::ofstream file(fileName, std::ios_base::app); // TODO: maybe I should check if already open 
    if (!file)
    {
        std::cerr  << __func__ << " | Error: Unable to open file " << fileName << std::endl;
        return;
    }
    file << word + "\n";
    file.close();
    return;
}

void createMapAndCount(const std::string& fileName)
{
    std::ifstream inputFile(fileName);
    if (!inputFile)
    {
        std::cerr  << __func__ << " | Error: Unable to open inputFile " << fileName << std::endl;
        return;
    }

    std::cout << "Creating hash map for file:" << fileName << std::endl;
    std::unordered_map<string,int> wordMap;
    std::string word;
    while (inputFile >> word)
    {
        int occurrence = 1;
        // If the word is in the map (and so there are more occurency of it), add to the occurence the value already stored
        if(wordMap.find(word) != wordMap.end())
        {
            occurrence += wordMap[word];
        }
        wordMap.insert_or_assign(word, occurrence);

    }
    // Close the file after reading
    inputFile.close(); 

    // Now, open the file in output mode to store the map
    std::ofstream outFile(fileName, std::ios::trunc);  // Open in truncate mode to clear the file
    if (!outFile)
    {
        std::cerr << __func__ << " | Error: Unable to open file " << fileName << " for writing." << std::endl;
        return;
    }
    std::cout << "Creating hash map for file:" << fileName << std::endl;

    // Write the map contents to the file (word and its count)
    for (const auto& entry : wordMap)
    {
        outFile << entry.first << "," << entry.second << "\n";  // Write key and value
    }

    outFile.close();
    return;
}

void map(const std::string& fileName, const int threadNumber)
{
    readWordsFromFile(fileName, threadNumber);
    createMapAndCount(fileName);
    return;
}

void processFile(int threadNumber, const std::string& fileName)
{
    std::lock_guard<std::mutex> lock(output_mutex);
    std::cout << "Thread " << threadNumber << " is processing file: " << fileName << std::endl;
    map(fileName, threadNumber);
}



int main(int argc, char* argv[])
{
    if (argc < 9)
    {
        std::cerr << "Error: Please provide at least 8 input files. Only " << argc << " recieved" << std::endl;
        return 1;
    }

    // Ensure the output folder exists and is empty
    std::filesystem::path outputDir = std::filesystem::current_path() / "../output";
    
    // Make sure that output folder exists and is empty
    if (!std::filesystem::exists(outputDir))
    {
        std::filesystem::create_directory(outputDir);  // Create output folder if it doesn't exist
    }
    else
    {
        // Clear the folder if it exists
        for (const auto& entry : std::filesystem::directory_iterator(outputDir))
        {
            std::filesystem::remove(entry);  // Remove each file in the folder
        }
    }

    // List of input files
    std::vector<std::string> files;
    for (int i = 1; i < argc; i++)
    {  
        files.push_back(argv[i]);  
    }
    ctpl::thread_pool p(8);

    for(const auto& file: files)
    {
        p.push(processFile,file);
    }

    return 0;
}
