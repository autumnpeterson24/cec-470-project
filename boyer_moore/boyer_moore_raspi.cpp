/*
    CEC470 Project: Comparing String Pattern Recognition Aglorithms with Lenovo Slim 3 and Rasberry Pi 5 Architectures

    boyer_moore_raspi.cpp: Using the Boyer-Moore algorithm to find the word 'war' in the book War and Peace by Leo Tolstoy this code will
    be run on the Raspberry Pi 5, 4 threads are launched for the 4 cores
    *full text found on https://www.gutenberg.org/files/2600/2600-h/2600-h.htm
    Author: Autumn Peterson
    Date: 3 Mar. 2025
*/

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>
#include <thread>
#include <mutex>
#include <chrono>

using namespace std;

#define TOTALLINES 26579
#define SECBASE (26579 / 4)
#define SEC1 SECBASE
#define SEC2 (SECBASE*2)
#define SEC3 (SECBASE*3)
#define SEC4 TOTALLINES // Final line of file
#define FILENAME "file/war_and_peace.txt"
#define PATTERN "moscow"


#define TESTAMOUNT 20

mutex coutMutex; // tracks the logging of the threads that process each section and allows the cout not to get overwritten while threads write at the same time


void processFileSection(const string &filename, int startLine, int endLine, int &totalCount);
int fileSecPartition(string filename);
void runMultipleTests();

// MAIN ============================================================

int main() {


   runMultipleTests();


    return 0;
}

void runMultipleTests(){
    /* runs multiple tests on the raspberry pi and get the overall average */
    int totalSum=0;
    vector<int> avgVec;

    for(int i=0; i< TESTAMOUNT; i++){
    cout<<"\nTEST "<<i+1<<"\n";
    auto start = chrono::high_resolution_clock::now(); // Timing the function

    fileSecPartition(FILENAME);

    auto end = chrono::high_resolution_clock::now();

    auto duration = chrono::duration_cast<chrono::milliseconds>(end-start);
    cout<<"\tTotal time taken Raspberry Pi 5: "<<duration.count()<<" ms"<<"\n";
    avgVec.push_back(duration.count());
    } 
    for(int i=0; i<avgVec.size(); i++){
        totalSum += avgVec[i];
    }

    cout<<"\nTotal Average Time for Raspberry Pi over "<<TESTAMOUNT<<" tests: "<<totalSum/TESTAMOUNT<<" ms";
    cout<<"\n\n";

}

// Function to partition file processing across multiple threads
int fileSecPartition(string filename) {
    int tc1 = 0, tc2 = 0, tc3 = 0, tc4 = 0;

    /* Creating 8 threads to process the file in parallel */
    thread t1(processFileSection, filename, 0, SEC1, ref(tc1));
    thread t2(processFileSection, filename, SEC1, SEC2, ref(tc2));
    thread t3(processFileSection, filename, SEC2, SEC3, ref(tc3));
    thread t4(processFileSection, filename, SEC3, SEC4, ref(tc4));

    /************************************************************* */

    /* Joining threads to make sure they finish before final count is found */
    t1.join();
    t2.join();
    t3.join();
    t4.join();
    /*************************************************************** */

    /* calculate final count */
    int finalCount = tc1 + tc2 + tc3 + tc4;

    cout << "\tFINAL COUNT: " << finalCount << "\n";

    return finalCount;
}

string toLowercase(string str) {
    /* transforms text to lowercase to find every iteration of word 
    Disclaimer: used GeeksForGeeks.com to help with this implementation
    link: https://www.geeksforgeeks.org/how-to-convert-std-string-to-lower-case-in-cpp/#using-stdtransform
    */

    transform(str.begin(), str.end(), str.begin(), ::tolower);
    return str;
}

bool isWholeWord(const string &text, int start, int length) {
    /* checks the boundary around the word to include the word that has punctuation
    *Disclaimer: had chatgpt.com help me with checking the boundaries around the word to count
    words with punctuation 
    */
    bool beforeIsBoundary = (start == 0 || !isalpha(text[start - 1]));
    bool afterIsBoundary = (start + length >= text.size() || !isalpha(text[start + length]));
    return beforeIsBoundary && afterIsBoundary;
}

vector<int> preprocessBadChar(const string &pattern) {
    /* Preprocess bad character heuristic for Boyer-Moore algorithm: finds the index of an unmatched
    character and stores in vector of ints to use for shifting in Boyer-Moore
    Disclaimer: had GeeksForGeeks.com help me with implementing this algorithm 
    link: https://www.geeksforgeeks.org/boyer-moore-algorithm-for-pattern-searching/
    */
    const int ASCI_ALPHABET_SIZE = 256;
    vector<int> badChar(ASCI_ALPHABET_SIZE, -1);
    for (int i = 0; i < pattern.size(); i++) {
        badChar[pattern[i]] = i;
    }
    return badChar;
}

int boyerMooreSearch(const string &text, const string &pattern) {
    /* Boyer-Moore search for word occurences: Searches for word occurence by using bad character heuristic
    to shift and find matches while skipping unnecessary comparisons
    Disclaimer: had GeeksForGeeks.com help me with implementing this algorithm 
    link: https://www.geeksforgeeks.org/boyer-moore-algorithm-for-pattern-searching/
    */

    int patternLen = pattern.size();
    int textLen = text.size();
    int count = 0;

    vector<int> badChar = preprocessBadChar(pattern);

    int shift = 0;
    while (shift <= (textLen - patternLen)) {
        int comp = patternLen - 1;

        while (comp >= 0 && pattern[comp] == text[shift + comp]) {
            comp--;
        }

        if (comp < 0) { 
            if (isWholeWord(text, shift, patternLen)) {
                count++;
            }
            shift += (shift + patternLen < textLen) ? patternLen : 1;
        } else {
            shift += max(1, comp - badChar[text[shift + comp]]);
        }
    }
    return count;
}

void processFileSection(const string &filename, int startLine, int endLine, int &totalCount) {
    /* Function used to create threads and section off War and Peace into 8 sections for 8 seperate threads*/

    auto threadStart = chrono::high_resolution_clock::now(); // start logging the time for each thread

    ifstream file(filename);
    if (!file.is_open()) {
        lock_guard<mutex> lock(coutMutex);
        cout << "Failed to open file.\n";
    }

    string line;
    int lineCount = 0;
    
    while (lineCount < startLine && getline(file, line)) { // Move to the correct start line
        lineCount++;
    }

    string pattern = PATTERN; //word want to find

    while (lineCount < endLine && getline(file, line)) {
        line = toLowercase(line);
        totalCount += boyerMooreSearch(line, pattern); //collect the totalCount from each section that is processed
        lineCount++;
    }

    file.close();

    auto threadEnd = chrono::high_resolution_clock::now();
    auto threadDuration = chrono::duration_cast<chrono::milliseconds>(threadEnd - threadStart); // track the time for each thread

    {
        lock_guard<mutex> lock(coutMutex); // using mutex again to print out the duration for each thread
        cout << "Thread ID: " << this_thread::get_id()<< " finished in " << threadDuration.count() << " ms\n";
    }
}
