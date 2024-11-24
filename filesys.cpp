// CMSC 341 - Fall 2024 - Project 4
#include "filesys.h"
FileSys::FileSys(int size, hash_fn hash, prob_t probing = DEFPOLCY){
    
}

FileSys::~FileSys(){
    
}

void FileSys::changeProbPolicy(prob_t policy){
    
}

bool FileSys::insert(File file){
    
}

bool FileSys::remove(File file){
    
}

const File FileSys::getFile(string name, int block) const{
    
}

bool FileSys::updateDiskBlock(File file, int block){
    
}

float FileSys::lambda() const {
    
}

float FileSys::deletedRatio() const {
    
}

void FileSys::dump() const {
    cout << "Dump for the current table: " << endl;
    if (m_currentTable != nullptr)
        for (int i = 0; i < m_currentCap; i++) {
            cout << "[" << i << "] : " << m_currentTable[i] << endl;
        }
    cout << "Dump for the old table: " << endl;
    if (m_oldTable != nullptr)
        for (int i = 0; i < m_oldCap; i++) {
            cout << "[" << i << "] : " << m_oldTable[i] << endl;
        }
}

bool FileSys::isPrime(int number){
    bool result = true;
    for (int i = 2; i <= number / 2; ++i) {
        if (number % i == 0) {
            result = false;
            break;
        }
    }
    return result;
}

int FileSys::findNextPrime(int current){
    //we always stay within the range [MINPRIME-MAXPRIME]
    //the smallest prime starts at MINPRIME
    if (current < MINPRIME) current = MINPRIME-1;
    for (int i=current; i<MAXPRIME; i++) { 
        for (int j=2; j*j<=i; j++) {
            if (i % j == 0) 
                break;
            else if (j+1 > sqrt(i) && i != current) {
                return i;
            }
        }
    }
    //if a user tries to go over MAXPRIME
    return MAXPRIME;
}