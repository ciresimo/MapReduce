#include <unordered_map>
#include <thread>
#include <vector>
#include <mutex>
#include <mr.h>
#include <ctpl.h>

using namespace std;

std::string outputDirectory = "../output/"; //TODO: change to something like outputPath.string()
const std::filesystem::path outputPath{"../output/"};
std::mutex output_mutex;
const int threadsTotalNumber = 4;
const int bucketsTotalNumber = 4;

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
            storeWordInFile(cleanedWord, threadNumber);
        }
    }

    file.close();
}

void processFile(const int threadNumber, const std::string& fileName)
{
    // std::lock_guard<std::mutex> lock(output_mutex);
    std::cout << "Thread " << threadNumber << " is processing file: " << fileName << std::endl;
    readWordsFromFile(fileName, threadNumber);
    return;
}

void storeWordInFile(const std::string& word, const int threadNumber)
{
    const char firstLetter = word[0];  // Get the first letter
    int bucket = int(firstLetter - 'a') % bucketsTotalNumber;    // Compute bucket index M normalized to range 0-25

    std::string fileName = outputDirectory + "mr-" + std::to_string(threadNumber) + "-" + std::to_string(bucket) + ".txt";
    std::ofstream file(fileName, std::ios_base::app);
    if (!file)
    {
        std::cerr  << __func__ << " | Error: Unable to open file " << fileName << std::endl;
        return;
    }
    // Add the word to the file as a new line
    file << word + "\n";
    file.close();
    return;
}

void createMap(const std::string& fileName, std::unordered_map<string,int>& wordMap)
{
    std::ifstream inputFile(fileName);
    if (!inputFile)
    {
        std::cerr  << __func__ << " | Error: Unable to open inputFile " << fileName << std::endl;
        return;
    }

    std::string word;
    while (inputFile >> word)
    {
        int occurrence = 1;

        // This first part is important only during the reduce phase. In this case the "word" is composed by both the actual word and the occurence
        // Find the position of the first comma (in order to use the same function also during the reduce phase)
        size_t commaPos = word.find(',');
        if (commaPos != std::string::npos)
        {
            try 
            {
                occurrence = std::stoi(word.substr(commaPos + 1)); // Convert the part after the comma to an integer
            } 
            catch (const std::invalid_argument& e) 
            {
                std::cerr << "Warning: Invalid occurrence value in word: " << word << std::endl;
                occurrence = 1; // Fallback to default occurrence
            }
            word = word.substr(0, commaPos); // Keep the portion before the comma (which is the actual word)
        }

        // If the word is in the map (and so there are more occurency of it), add to the occurence the value already stored
        if(wordMap.find(word) != wordMap.end())
        {
            occurrence += wordMap[word];
        }
        wordMap.insert_or_assign(word, occurrence);

    }
    // Close the file after reading
    inputFile.close();
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

void createMapAndCount(const std::string& fileName)
{
    std::unordered_map<std::string,int> wordMap;

    // Create a word map for the specified file
    createMap(fileName, wordMap);

    // Write out the word map in the same file
    writeMap(fileName,wordMap);
    
    return;
}

void map(const int threadNumber, const std::string& fileName)
{
    // std::lock_guard<std::mutex> lock(output_mutex);
    std::cout << "Thread " << threadNumber << " is mapping file: " << fileName << std::endl;
    createMapAndCount(fileName);
    return;
}

void reduceBucketFiles(const int bucketNumber)
{
    std::unordered_map<std::string,int> wordMap;

    // Create a unique map for all the mr-i-bucketNumber files
    for(int i = 0; i < threadsTotalNumber; i++)
    {
        std::string fileName = outputDirectory + "mr-" + std::to_string(i) + "-" + std::to_string(bucketNumber) + ".txt";
        std::cout << "Adding to the map file: " << fileName << std::endl;
        createMap(fileName, wordMap);
    }

    // Write it down in a single file
    std::string outputFileName = outputDirectory + "out-" + std::to_string(bucketNumber) + ".txt";
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

    
    // Extract all words from input files
    ctpl::thread_pool pool1(threadsTotalNumber);
    for (int i = 1; i < argc; i++)
    {  
        pool1.push(processFile,argv[i]);
    }

    // Wait for all threads to complete the work
    pool1.stop(true);

    // MAP
    ctpl::thread_pool pool2(threadsTotalNumber);

    // Iterate through the files in the output directory
    for(const auto& entry : std::filesystem::directory_iterator{outputPath})
    {
        // Only add files (not directories) to the list
        if (entry.is_regular_file())
        {
            pool2.push(map,entry.path().string());
        }
        else
        {
            std::cout << "File not valid for mapping: " << entry.path().string() << std::endl;
        }
    }
    pool2.stop(true);

    // REDUCE
    ctpl::thread_pool pool3(bucketsTotalNumber);

    for(int i = 0; i < bucketsTotalNumber; i++)
    {
        pool3.push(reduce,i);        
    }
    pool3.stop(true);

    return 0;
}
