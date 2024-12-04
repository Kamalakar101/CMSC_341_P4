// CMSC 341 - Fall 2024 - Project 4
#include "filesys.h"
#include <math.h>
#include <algorithm>
#include <random>
#include <vector>
using namespace std;
enum RANDOM {UNIFORMINT, UNIFORMREAL, NORMAL, SHUFFLE};
class Random {
public:
    Random(){}
    Random(int min, int max, RANDOM type=UNIFORMINT, int mean=50, int stdev=20) : m_min(min), m_max(max), m_type(type)
    {
        if (type == NORMAL){
            //the case of NORMAL to generate integer numbers with normal distribution
            m_generator = std::mt19937(m_device());
            //the data set will have the mean of 50 (default) and standard deviation of 20 (default)
            //the mean and standard deviation can change by passing new values to constructor 
            m_normdist = std::normal_distribution<>(mean,stdev);
        }
        else if (type == UNIFORMINT) {
            //the case of UNIFORMINT to generate integer numbers
            // Using a fixed seed value generates always the same sequence
            // of pseudorandom numbers, e.g. reproducing scientific experiments
            // here it helps us with testing since the same sequence repeats
            m_generator = std::mt19937(10);// 10 is the fixed seed value
            m_unidist = std::uniform_int_distribution<>(min,max);
        }
        else if (type == UNIFORMREAL) { //the case of UNIFORMREAL to generate real numbers
            m_generator = std::mt19937(10);// 10 is the fixed seed value
            m_uniReal = std::uniform_real_distribution<double>((double)min,(double)max);
        }
        else { //the case of SHUFFLE to generate every number only once
            m_generator = std::mt19937(m_device());
        }
    }
    void setSeed(int seedNum){
        // we have set a default value for seed in constructor
        // we can change the seed by calling this function after constructor call
        // this gives us more randomness
        m_generator = std::mt19937(seedNum);
    }
    void init(int min, int max){
        m_min = min;
        m_max = max;
        m_type = UNIFORMINT;
        m_generator = std::mt19937(10);// 10 is the fixed seed value
        m_unidist = std::uniform_int_distribution<>(min,max);
    }
    void getShuffle(vector<int> & array){
        // this function provides a list of all values between min and max
        // in a random order, this function guarantees the uniqueness
        // of every value in the list
        // the user program creates the vector param and passes here
        // here we populate the vector using m_min and m_max
        for (int i = m_min; i<=m_max; i++){
            array.push_back(i);
        }
        shuffle(array.begin(),array.end(),m_generator);
    }

    void getShuffle(int array[]){
        // this function provides a list of all values between min and max
        // in a random order, this function guarantees the uniqueness
        // of every value in the list
        // the param array must be of the size (m_max-m_min+1)
        // the user program creates the array and pass it here
        vector<int> temp;
        for (int i = m_min; i<=m_max; i++){
            temp.push_back(i);
        }
        std::shuffle(temp.begin(), temp.end(), m_generator);
        vector<int>::iterator it;
        int i = 0;
        for (it=temp.begin(); it != temp.end(); it++){
            array[i] = *it;
            i++;
        }
    }

    int getRandNum(){
        // this function returns integer numbers
        // the object must have been initialized to generate integers
        int result = 0;
        if(m_type == NORMAL){
            //returns a random number in a set with normal distribution
            //we limit random numbers by the min and max values
            result = m_min - 1;
            while(result < m_min || result > m_max)
                result = m_normdist(m_generator);
        }
        else if (m_type == UNIFORMINT){
            //this will generate a random number between min and max values
            result = m_unidist(m_generator);
        }
        return result;
    }

    double getRealRandNum(){
        // this function returns real numbers
        // the object must have been initialized to generate real numbers
        double result = m_uniReal(m_generator);
        // a trick to return numbers only with two deciaml points
        // for example if result is 15.0378, function returns 15.03
        // to round up we can use ceil function instead of floor
        result = std::floor(result*100.0)/100.0;
        return result;
    }

    string getRandString(int size){
        // the parameter size specifies the length of string we ask for
        // to use ASCII char the number range in constructor must be set to 97 - 122
        // and the Random type must be UNIFORMINT (it is default in constructor)
        string output = "";
        for (int i=0;i<size;i++){
            output = output + (char)getRandNum();
        }
        return output;
    }
    
    int getMin(){return m_min;}
    int getMax(){return m_max;}
    private:
    int m_min;
    int m_max;
    RANDOM m_type;
    std::random_device m_device;
    std::mt19937 m_generator;
    std::normal_distribution<> m_normdist;//normal distribution
    std::uniform_int_distribution<> m_unidist;//integer uniform distribution
    std::uniform_real_distribution<double> m_uniReal;//real uniform distribution

};

unsigned int hashCode(const string str) {
   unsigned int val = 0 ;
   const unsigned int thirtyThree = 33 ;  // magic number from textbook
   for (unsigned int i = 0 ; i < str.length(); i++)
      val = val * thirtyThree + str[i] ;
   return val ;
}

string alphabet[] = {"a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z", 
        "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z"};

class Tester{
    public:
        bool testInsertionNormal(vector<File> &nonCollidingFiles, prob_t policy);
        bool testFindError(vector<File> &nonCollidingFiles, prob_t policy);
        bool testFindNonColliding(vector<File> &nonCollidingFiles, prob_t policy);
        bool testFindColliding(vector<File> &collidingFiles, prob_t policy);
        bool testRemoveNonColliding(vector<File> &nonCollidingFiles, prob_t policy);
        bool testRemoveColliding(vector<File> &collidingFiles, prob_t policy);
        bool testTriggerRehashDataInsertion(vector<File> &nonCollidingFiles, prob_t policy);
        bool testRehashCompletionLoadFactor(vector<File> &files, prob_t policy);
        bool testTriggerRehashDataRemoval(vector<File> &nonCollidingFiles, prob_t policy);
        bool testRehashCompletionDeleteRatio(vector<File> &nonCollidingFiles, prob_t policy);
};

int main() {
    Tester test;
    vector<File> nonCollidingFiles;
    vector<File> collidingFiles;
    Random RndID(DISKMIN,DISKMAX);
    Random RndAlphaAll(0,51);
    Random RndAlphaSmall(0,25);
    
    for (int i = 0; i < 52; i++){
        // generating random data
        File file = File(alphabet[i], RndID.getRandNum(), true);
        // saving data for later use
        nonCollidingFiles.push_back(file);
    }

    for (int i = 0; i < 20; i++){
        for (int j = 0; j < 3; j++) {
            // generating random data
            File file = File(alphabet[i], RndID.getRandNum(), true);
            // saving data for later use
            collidingFiles.push_back(file);
        }
    }

        //*********************** NORMAL INSERTION ***********************
    cout << "*********************** NORMAL INSERTION ***********************" << endl;
    cout << "Normal Insertion: ";
    if(test.testInsertionNormal(nonCollidingFiles, QUADRATIC) && test.testInsertionNormal(nonCollidingFiles, DOUBLEHASH) && test.testInsertionNormal(nonCollidingFiles, LINEAR)) {
        cout << "Inserted Properly! (PASSED)" << endl;
    }
    else {
        cout << "Did Not Insert Properly! (FAILED)" << endl;
    }
    cout << endl;

    //*********************** FIND ERROR ***********************
    cout << "*********************** FIND ERROR ***********************" << endl;
    cout << "Find File That Does Not Exist: ";
    if(test.testFindError(nonCollidingFiles, QUADRATIC) && test.testFindError(nonCollidingFiles, DOUBLEHASH) && test.testFindError(nonCollidingFiles, LINEAR)) {
        cout << "Not Found! (PASSED)" << endl;
    }
    else {
        cout << "Error! (FAILED)" << endl;
    }
    cout << endl;

    //*********************** FIND FILE NON-COLLIDING ***********************
    cout << "*********************** FIND FILE NON-COLLIDING ***********************" << endl;
    cout << "Find File Non-Colliding: ";
    if(test.testFindNonColliding(nonCollidingFiles, QUADRATIC) && test.testFindNonColliding(nonCollidingFiles, DOUBLEHASH) && test.testFindNonColliding(nonCollidingFiles, LINEAR)) {
        cout << "Found! (PASSED)" << endl;
    }
    else {
        cout << "Not Found! (FAILED)" << endl;
    }
    cout << endl;

    //*********************** FIND FILE COLLIDING ***********************
    cout << "*********************** FIND FILE COLLIDING ***********************" << endl;
    cout << "Find File Colliding: ";
    if(test.testFindColliding(collidingFiles, QUADRATIC) && test.testFindColliding(collidingFiles, DOUBLEHASH) && test.testFindColliding(collidingFiles, LINEAR)) {
        cout << "Found! (PASSED)" << endl;
    }
    else {
        cout << "Not Found! (FAILED)" << endl;
    }
    cout << endl;

    //*********************** REMOVE FILE NON-COLLIDING ***********************
    cout << "*********************** REMOVE FILE NON-COLLIDING ***********************" << endl;
    cout << "Remove File Non-Colliding: ";
    if(test.testRemoveNonColliding(nonCollidingFiles, QUADRATIC) && test.testRemoveNonColliding(nonCollidingFiles, DOUBLEHASH) && test.testRemoveNonColliding(nonCollidingFiles, LINEAR)) {
        cout << "Removed! (PASSED)" << endl;
    }
    else {
        cout << "Not Removed! (FAILED)" << endl;
    }
    cout << endl;

    //*********************** REMOVE FILE COLLIDING ***********************
    cout << "*********************** REMOVE FILE COLLIDING ***********************" << endl;
    cout << "Remove File Colliding: ";
    if(test.testRemoveColliding(collidingFiles, QUADRATIC) && test.testRemoveColliding(collidingFiles, DOUBLEHASH) && test.testRemoveColliding(collidingFiles, LINEAR)) {
        cout << "Removed! (PASSED)" << endl;
    }
    else {
        cout << "Not Removed! (FAILED)" << endl;
    }
    cout << endl;

    //*********************** TRIGGER REHASH DATA INSERTION ***********************
    cout << "*********************** TRIGGER REHASH DATA INSERTION ***********************" << endl;
    cout << "Trigger Rehash Data Insertion: ";
    if(test.testTriggerRehashDataInsertion(nonCollidingFiles, QUADRATIC) && test.testTriggerRehashDataInsertion(nonCollidingFiles, DOUBLEHASH) && test.testTriggerRehashDataInsertion(nonCollidingFiles, LINEAR)) {
        cout << "Triggered Rehash! (PASSED)" << endl;
    }
    else {
        cout << "Rehash Did Not Trigger! (FAILED)" << endl;
    }
    cout << endl;

    //*********************** REHASH COMPLETION LOAD FACTOR ***********************
    cout << "*********************** REHASH COMPLETION LOAD FACTOR ***********************" << endl;
    cout << "Rehash Completion Load Factor: ";
    if(test.testRehashCompletionLoadFactor(collidingFiles, QUADRATIC) && test.testRehashCompletionLoadFactor(collidingFiles, DOUBLEHASH) && test.testRehashCompletionLoadFactor(collidingFiles, LINEAR)) {
        cout << "Rehash Completed! (PASSED)" << endl;
    }
    else {
        cout << "Rehash Not Completed! (FAILED)" << endl;
    }
    cout << endl;

    //*********************** TRIGGER REHASH DATA REMOVAL ***********************
    cout << "*********************** TRIGGER REHASH DATA REMOVAL ***********************" << endl;
    cout << "Trigger Rehash Data Removal: ";
    if(test.testTriggerRehashDataRemoval(nonCollidingFiles, QUADRATIC) && test.testTriggerRehashDataRemoval(nonCollidingFiles, DOUBLEHASH) && test.testTriggerRehashDataRemoval(nonCollidingFiles, LINEAR)) {
        cout << "Triggered Rehash! (PASSED)" << endl;
    }
    else {
        cout << "Rehash Did Not Trigger! (FAILED)" << endl;
    }
    cout << endl;

    //*********************** REHASH COMPLETION DELETE RATIO ***********************
    cout << "*********************** REHASH COMPLETION DELETE RATIO ***********************" << endl;
    cout << "Rehash Completion Delete Ratio: ";
    if(test.testRehashCompletionDeleteRatio(nonCollidingFiles, QUADRATIC) && test.testRehashCompletionDeleteRatio(nonCollidingFiles, DOUBLEHASH) && test.testRehashCompletionDeleteRatio(nonCollidingFiles, LINEAR)) {
        cout << "Rehash Completed! (PASSED)" << endl;
    }
    else {
        cout << "Rehash Not Completed! (FAILED)" << endl;
    }
    cout << endl;

    return 0;
}

bool Tester::testInsertionNormal(vector<File> &nonCollidingFiles, prob_t policy) {
    FileSys database(MINPRIME, hashCode, policy);
    for (int i = 0; i < 26; i++) {
        database.insert(nonCollidingFiles.at(i));
        int hashIndex = hashCode(nonCollidingFiles.at(i).m_name) % MINPRIME;
        if (!((database.m_currentTable[hashIndex] != nullptr &&  *database.m_currentTable[hashIndex] == nonCollidingFiles.at(i)) && database.m_currentSize == (i + 1))) {
            return false;
        }
    }
    return true;
}

bool Tester::testFindError(vector<File> &nonCollidingFiles, prob_t policy) {
    FileSys database(MINPRIME, hashCode, policy);
    for (int i = 0; i < 26; i++) {
        database.insert(nonCollidingFiles.at(i));
    }
    if (database.getFile("IMABOT", DISKMAX) == File()) {
        return true;
    }
    return false;
}

bool Tester::testFindNonColliding(vector<File> &nonCollidingFiles, prob_t policy) {
    FileSys database(MINPRIME, hashCode, policy);
    for (int i = 0; i < 26; i++) {
        database.insert(nonCollidingFiles.at(i));
    }

    for (int i = 0; i < 26; i++) {
        if (!(database.getFile(nonCollidingFiles.at(i).m_name, nonCollidingFiles.at(i).m_diskBlock) == nonCollidingFiles.at(i))) {
            return false;
        }
    }
    return true;
}

bool Tester::testFindColliding(vector<File> &collidingFiles, prob_t policy) {
    FileSys database(MINPRIME, hashCode, policy);
    for (int i = 0; i < 26; i++) {
        database.insert(collidingFiles.at(i));
    }

    for (int i = 0; i < 26; i++) {
        if (!(database.getFile(collidingFiles.at(i).m_name, collidingFiles.at(i).m_diskBlock) == collidingFiles.at(i))) {
            return false;
        }
    }
    return true;
}

bool Tester::testRemoveNonColliding(vector<File> &nonCollidingFiles, prob_t policy) {
    FileSys database(MINPRIME, hashCode, policy);
    for (int i = 0; i < 26; i++) {
        database.insert(nonCollidingFiles.at(i));
    }

    for (int i = 0; i < 5; i++) {
        if (!database.remove(nonCollidingFiles.at(i)) && database.m_currNumDeleted == (i + 1)) {
            return false;
        }
    }
    return true;
}

bool Tester::testRemoveColliding(vector<File> &collidingFiles, prob_t policy) {
    FileSys database(MINPRIME, hashCode, policy);
    for (int i = 0; i < 26; i++) {
        database.insert(collidingFiles.at(i));
    }

    for (int i = 0; i < 5; i++) {
        if (!database.remove(collidingFiles.at(i)) && database.m_currNumDeleted == (i + 1)) {
            return false;
        }
    }
    return true;
}

bool Tester::testTriggerRehashDataInsertion(vector<File> &nonCollidingFiles, prob_t policy) {
    FileSys database(MINPRIME, hashCode, policy);
    File ** table = database.m_currentTable;
    for (int i = 0; i < 52; i++) {
        database.insert(nonCollidingFiles.at(i));
    }

    if (table != database.m_currentTable && database.m_transferIndex != 0 && database.m_currentCap > MINPRIME) {
        return true;
    }
    return false;
}

bool Tester::testRehashCompletionLoadFactor(vector<File> &files, prob_t policy) {
    FileSys database(MINPRIME, hashCode, policy);
    bool allFilesFound = false;
    for (int i = 0; i < 60; i++) {
        database.insert(files.at(i));
    }

    for (int i = 0; i < (int) files.size(); i++) {
        if (database.getFile(files.at(i).m_name, files.at(i).m_diskBlock) == files.at(i)) {
            allFilesFound = true;
        }
    }

    if (database.m_oldTable == nullptr && database.m_transferIndex == 0 && database.m_currentCap > MINPRIME && allFilesFound) {
        return true;
    }
    return false;
}

bool Tester::testTriggerRehashDataRemoval(vector<File> &nonCollidingFiles, prob_t policy) {
    FileSys database(MINPRIME, hashCode, policy);
    File ** table = database.m_currentTable;
    for (int i = 0; i < 50; i++) {
        database.insert(nonCollidingFiles.at(i));
    }

    for (int i = 0; i < 41; i++) {
        database.remove(nonCollidingFiles.at(i));
    }

    if (table != database.m_currentTable && database.m_transferIndex != 0) {
        return true;
    }
    return false;
}

bool Tester::testRehashCompletionDeleteRatio(vector<File> &nonCollidingFiles, prob_t policy) {
    FileSys database(MINPRIME, hashCode, policy);
    bool allFilesFound = false;
    for (int i = 0; i < 50; i++) {
        database.insert(nonCollidingFiles.at(i));
    }

    for (int i = 0; i < 45; i++) {
        database.remove(nonCollidingFiles.at(i));
    }

    for (int i = 45; i < 50; i++) {
        if (database.getFile(nonCollidingFiles.at(i).m_name, nonCollidingFiles.at(i).m_diskBlock) == nonCollidingFiles.at(i)) {
            allFilesFound = true;
        }
    }

    if (database.m_oldTable == nullptr && database.m_transferIndex == 0 && allFilesFound) {
        return true;
    }
    return false;
}
