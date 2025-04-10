// Stalford, Michael.
// Version:
//    4/05/2025 -     Gonna get it working today haha.
//    3/17/2025 -     Took a look. Did some fiddling. 
//    3/3/2025  -     File created. Basic structure created.
//                    Added some functionality based on the
//                    resources below. Doesn't quite work.
// Resources:
// https://www.geeksforgeeks.org/finite-automata-algorithm-for-pattern-searching/

// This file is an example of the Finite Automata pattern running with 8 threads on the lenovo slim 3
// recognition algorithm to be used for CEC470, Spring 2025.

// Imports -------------------------------------------------
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <thread>
#include <chrono>
#include <mutex>

// Defines -------------------------------------------------
#define CHAR_CNT 256
#define TEXT_FILENAME "file/war_and_peace.txt"
#define PATTERN "moscow"
#define CASE_SENSITIVE false

#define TOTALLINES 26579
#define SECBASE (26579 / 8)
#define SEC1 SECBASE
#define SEC2 (SECBASE*2)
#define SEC3 (SECBASE*3)
#define SEC4 (SECBASE*4)
#define SEC5 (SECBASE*5)
#define SEC6 (SECBASE*6)
#define SEC7 (SECBASE*7)
#define SEC8 (TOTALLINES) // Final line of file

#define TESTAMOUNT 20

// Function prototypes -------------------------------------
void buildTransitionTable(const std::string& pattern, std::vector<std::vector<int>>& transitionTable);
void matchFinAut(const std::string& pattern, const std::string& text, const std::vector<std::vector<int>>& transitionTable, int& totalCount);
int getNextState(const std::string& pattern, int state, int x);
std::string getText(const std::string& filepath, int startline, int endline);
void search(const std::string& text, const std::string& pattern, int& totalCount);
void createThread(const std::string filepath, int startline, int endline, int& totalCount);
void launchThreads(const std::string filename);
void runTests();


std::mutex coutMutex; // tracks the logging of the threads that process each section and allows the cout not to get overwritten while threads write at the same time

int main(void) {
    runTests();

    return 0;
}

// Functions ----------------------------------------------------------------------------------
void runTests(){
    std::vector<int> avgVec;
    int totalSum=0;

    for(int i=0; i< TESTAMOUNT; i++){ 

        std::cout<<"\nTEST "<<i+1<<"\n";
        auto start = std::chrono::high_resolution_clock::now(); // Timing the function
    
        launchThreads("war_and_peace.txt");
    
        auto end = std::chrono::high_resolution_clock::now();
    
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end-start);
        std::cout<<"\tTotal time taken Lenovo Slim 3: "<<duration.count()<<" ms"<<"\n";
        avgVec.push_back(duration.count());
        }

        for(int i=0; i<avgVec.size(); i++){
            totalSum += avgVec[i];
        }
    
        std::cout<<"\nTotal Average Time for Lenovo Slim 3 over "<<TESTAMOUNT<<" tests: "<<totalSum/TESTAMOUNT<<" ms";
        std::cout<<"\n\n";
}

void launchThreads(const std::string filename){
    int tc1 = 0, tc2 = 0, tc3 = 0, tc4 = 0, tc5 = 0, tc6 = 0, tc7 = 0, tc8 = 0;

    std::thread t1(createThread, filename, 0, SEC1, std::ref(tc1));
    std::thread t2(createThread, filename, SEC1, SEC2, std::ref(tc2));
    std::thread t3(createThread, filename, SEC2, SEC3, std::ref(tc3));
    std::thread t4(createThread, filename, SEC3, SEC4, std::ref(tc4));
    std::thread t5(createThread, filename, SEC4, SEC5, std::ref(tc5));
    std::thread t6(createThread, filename, SEC5, SEC6, std::ref(tc6));
    std::thread t7(createThread, filename, SEC6, SEC7, std::ref(tc7));
    std::thread t8(createThread, filename, SEC7, SEC8, std::ref(tc8));

    t1.join();
    t2.join();
    t3.join();
    t4.join();
    t5.join();
    t6.join();
    t7.join();
    t8.join();
    /*************************************************************** */

    /* calculate final count */
    int finalCount = tc1 + tc2 + tc3 + tc4 + tc5 + tc6 + tc7 + tc8;

    std::cout << "\tFINAL COUNT: " << finalCount << "\n";

}

void createThread(const std::string filepath, int startline, int endline, int& totalCount){

    auto threadStart = std::chrono::high_resolution_clock::now(); // start logging the time for each thread

    std::string text = getText(TEXT_FILENAME, startline, endline);


    if (!CASE_SENSITIVE) {
        for (int i = 0; i < text.size(); ++i) {
            text[i] = tolower(text[i]);
        }
    }


    // Search!
    search(text, PATTERN, totalCount);

    auto threadEnd = std::chrono::high_resolution_clock::now();
    auto threadDuration = std::chrono::duration_cast<std::chrono::milliseconds>(threadEnd - threadStart); // track the time for each thread
    {
        std::lock_guard<std::mutex> lock(coutMutex); // using mutex again to print out the duration for each thread
        std::cout << "Thread ID: " << std::this_thread::get_id()<< " finished in " << threadDuration.count() << " ms\n";
    }

}

std::string getText(const std::string& filepath, int startline, int endline) {
    // Initialize file I/O vars
    std::ifstream fp(filepath);
    std::stringstream buffer;
    std::string line;

     // Check file status
    if (!fp) {
        throw std::runtime_error("Failed to open file: " + filepath);
    }

    // Read in the text file to a variable
    int lineCount = 0;

    while(std::getline(fp, line)){
        ++lineCount;

        if (lineCount > startline && lineCount <= endline) {
            buffer << line << "\n";
        }
        if (lineCount > endline) {
            break;
        }
    }


    // Clean-up
    fp.close();

    return buffer.str();
}

void search(const std::string& text, const std::string& pattern, int& totalCount) {
    // Build transition table
    std::vector<std::vector<int>> transitionTable(pattern.size(), std::vector<int>(CHAR_CNT));
    buildTransitionTable(pattern, transitionTable);

    // Perform mattern matching
    matchFinAut(pattern, text, transitionTable, totalCount);
}

int getNextState(const std::string& pattern, int state, int x) {
    int i;

    // First sequence is only the first char
    if (state < pattern.size() && x == pattern[state]) {
        // Convert from index to value (char)
        return state + 1;
    }

    for (int ns = state; ns > 0; --ns) {
        if (pattern[ns - 1] == x) {
            for (i = 0; i < ns - 1; ++i) {
                if (pattern[i] != pattern[state - ns + 1 + i]) {
                    break;
                }
            }

            if (i == ns - 1) {
                return ns;
            }
        }
    }

    return 0;
}

void buildTransitionTable(const std::string& pattern, std::vector<std::vector<int>>& transitionTable) {
    for (int state = 0; state < pattern.size(); ++state) {
        for (int x = 0; x < CHAR_CNT; ++x) {
            transitionTable[state][x] = getNextState(pattern, state, x);
        }
    }
}

void matchFinAut(const std::string& pattern, const std::string& text, const std::vector<std::vector<int>>& transitionTable, int& totalCount) {
    int state = 0; // current state
    int patternCount = 0; // For debug
    char currentChar;

    // Process the text with the transition table
    for (int i = 0; i < text.size(); ++i) {
        currentChar = text[i];

        // Discard chars that aren't in our 256 char alphabet
        if (currentChar >= 0 && currentChar < CHAR_CNT) {
            // Reference our transition table to find the transition
            state = transitionTable[state][currentChar];
        }

        // If we have reached the final state, we have a match
        if (state == pattern.size()) {
            //std::cout << "Pattern found at index " << i - pattern.size() + 1 << std::endl;
            ++patternCount;
            state = 0;
        }
    }
    totalCount = patternCount;

    //std::cout << "Pattern count: " << patternCount << "." << std::endl;
}