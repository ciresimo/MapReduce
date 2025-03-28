#include <unordered_map>
#include <thread>
#include <vector>
#include "mr.h"
#include <ctpl.h>

using namespace std;

std::string outputDirectory = "../output/";

void readWordsFromFile(const std::string& filename)
{
    std::ifstream file(filename);
    if (!file)
    {
        std::cerr << __func__ << " | Error: Unable to open file " << filename << std::endl;
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
            storeWordInFile(cleanedWord, 1);
        }
    }

    file.close();
}

void storeWordInFile(const std::string& word, const int thread_number)
{
    const int M = 1; //TODO: get is an input when parallelizing stuff
    const char firstLetter = word[0];  // Get the first letter
    int bucket = int(firstLetter);    // Compute bucket index M

    std::string filename = outputDirectory + "mr-" + std::to_string(thread_number) + "-" + std::to_string(bucket) + ".txt";
    std::ofstream file(filename, std::ios_base::app); // TODO: maybe I should check if already open 
    if (!file)
    {
        std::cerr  << __func__ << " | Error: Unable to open file " << filename << std::endl;
        return;
    }
    file << word + "\n";
    file.close();
    return;
}


// // TODO: remove when not used anymore
// void sumWordsCounts()
// {
//     // TODO change this to include all the files
//     for(int i = 49; i < 122 ; i++)
//     {
//         std::string fileName = outputDirectory + "mr-1-" + std::to_string(i) + ".txt"; // TODO: include parallel computations
//         createMapAndCount(fileName);
//     }
//     return;
// }

void createMapAndCount(const std::string& filename)
{
    std::ifstream inputFile(filename);
    if (!inputFile)
    {
        std::cerr  << __func__ << " | Error: Unable to open inputFile " << filename << std::endl;
        return;
    }
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
    std::ofstream outFile(filename, std::ios::trunc);  // Open in truncate mode to clear the file
    if (!outFile)
    {
        std::cerr << __func__ << " | Error: Unable to open file " << filename << " for writing." << std::endl;
        return;
    }

    // Write the map contents to the file (word and its count)
    for (const auto& entry : wordMap)
    {
        outFile << entry.first << "," << entry.second << "\n";  // Write key and value
    }

    outFile.close();
    return;
}

void map(const std::string& filename)
{
    readWordsFromFile(filename);
    createMapAndCount(filename);
    return;
}

void processFile(int threadID, const std::string& filename) {
    std::cout << "Thread " << threadID << " is processing file: " << filename << '\n';
    map(filename);
}



int main(int argc, char* argv[])
{
    // TODO: for now I impose that the files are 8
    if (argc < 9)
    {
        std::cerr << "Error: Please provide at least 8 input files." << std::endl;
        return 1;
    }

    
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
    // Ensure the output folder exists and is empty
    std::filesystem::path outputDir = std::filesystem::current_path() / "../output";

    // List of input files
    std::vector<std::string> files;
    for (int i = 1; i < argc; ++i)
    {  
        files.push_back(argv[i]);  
    }

   ctpl::thread_pool p(4 /* two threads in the pool */);

   for(const auto& file: files)
   {
        p.push(processFile,file);
   }


    // std::string filename = "../pg-being_ernest.txt";

    // sumWordsCounts();
    return 0;
}
