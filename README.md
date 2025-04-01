# Parallel Word Frequency Counter Using Memory-Based MapReduce

## Introduction
This project implements a parallel word frequency counter using a memory-based MapReduce approach. The program processes multiple text files concurrently, maps words into designated memory banks based on their first letter, and then reduces the data to compute the total occurrences of each word.
Master branch uses intermediate files, while no-file optimizes memory usage by storing data in an in-memory hash table.
Implementation Details
The program consists of the following major components:

- File Processing: Text files are read concurrently by multiple threads using a thread pool. Each word is extracted, cleaned (removing non-alphanumeric characters and converting to lowercase), and processed.

- Mapping Phase: Words are assigned to specific buckets based on the modulo operation of their first letter and the total number of buckets (M). Instead of writing to intermediate files, the program stores word counts in an unordered_map inside a global memoryBanks structure.

- Reduction Phase: Once all words are mapped, the reducers aggregate word counts from multiple memory banks and output the final frequency count for each word.

- Threading and Synchronization: The program uses ctpl::thread_pool to efficiently manage threads for both mapping and reducing stages.

 -Performance Optimization: The use of std::unordered_map ensures fast lookups, insertions, and updates. By eliminating intermediate files, the program significantly reduces I/O overhead.
 
Parallel computing is enhanced through the use of the [CTPL library](https://github.com/vit-vit/CTPL). It allows setting up pools with a fixed amount of threads with automatic task assignment, handling all concurrency issues

## Building and running the code
- `make build` installs the necessary cmake dependancies and builds the code (if not working straight away, try by manually removing the build directory)
- `make run` runs the code, giving in input all the text files

## Results and Performance
The program successfully processes multiple input files in parallel, efficiently distributing work across available CPU cores.
Performance improvements are evident compared to single-threaded implementations:
- no threads: 2908 ms
- threads + intermediate files: 1420 ms
- threads without intermediate files: 452 ms

Performance improvement aorund 85%

