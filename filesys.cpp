// CMSC 341 - Fall 2024 - Project 4
#include "filesys.h"

//Constructor
//Makes a FileSys object
FileSys::FileSys(int size, hash_fn hash, prob_t probing = DEFPOLCY) {
    //If the size is not prime, it finds the next smallest prime greater than the size
    if (isPrime(size)) {
        m_currentCap = size;
    }
    //Makes the size as the capacity
    else {
        m_currentCap = findNextPrime(size);
    }
    
    //Initializes the hash table and member variables
    m_hash = hash;
    m_currProbing = probing;
    m_currentTable = new File*[m_currentCap]();
    m_currNumDeleted = 0;
    m_currentSize = 0;
    m_oldCap = 0;
    m_oldNumDeleted = 0;
    m_oldSize = 0;
    m_oldTable = nullptr;
    m_newPolicy = probing;
    m_transferIndex = 0;
}

//Destructor
//Deallocates the tables
FileSys::~FileSys(){
    //Deallocates the current table
    for (int i = 0; i < m_currentCap; i++) {
        delete m_currentTable[i];
    }
    delete[] m_currentTable;

    //Deallocates the old table
    if (m_oldTable != nullptr) {
        for (int i = 0; i < m_oldCap; i++) {
            delete m_oldTable[i];
        }
        delete[] m_oldTable;
    }
}

//changeProbPolicy()
//Stores the user's request for the new policy
void FileSys::changeProbPolicy(prob_t policy){
    m_newPolicy = policy;
}

//insert()
//Inserts the file into the current table
bool FileSys::insert(File file){
    int hashIndex = m_hash(file.m_name) % m_currentCap;
    int index = -1;
    bool isNewFile = file.m_used;
    bool isInserted = false;

    //If file has an invalid disblock number or is a duplicate
    if (file.m_diskBlock < DISKMIN || file.m_diskBlock > DISKMAX || getFile(file.m_name, file.m_diskBlock) == file) {
        return false;
    }
    //If the hashIndex of the file is available
    else if (m_currentTable[hashIndex] == nullptr || (m_currentTable[hashIndex] != nullptr && m_currentTable[hashIndex]->m_used == false)) {
        index = hashIndex;
    }
    //Collision handling
    //Aquires the index based on the probing policy of the current table
    else if (m_currProbing == QUADRATIC) {
        index = quadraticProbing(hashIndex, m_currentCap, m_currentTable, file.m_name);
    }
    else if (m_currProbing == DOUBLEHASH) {
        index = doubleHashing(hashIndex, m_currentCap, m_currentTable, file.m_name);
    }
    else if (m_currProbing == LINEAR) {
        index = linearProbing(hashIndex, m_currentCap, m_currentTable, file.m_name);
    }

    //Inserts if the index is valid
    if (index != -1) {
        //Inserts the file
        delete m_currentTable[index];
        file.m_used = true;
        m_currentTable[index] = new File(file);
        m_currentSize++;
        isInserted = true;
    }
    
    //Makes sure the file was not from the old table
    if (isNewFile) {
        //Checks if a rehash is possible
        checkForRehash();
    }
    return isInserted;
}

//remove()
//Tags the file for deletion in either the old or current table
bool FileSys::remove(File file){
    //Checks for the index of the file in the old and current tables
    int currIndex = checkForFileInCurrent(file.m_name, file.m_diskBlock);
    int oldIndex = checkForFileInOld(file.m_name, file.m_diskBlock);
    bool isTagged = false;

    //Tags the file as deleted if found in the old table
    if (oldIndex != -1) {
        m_oldTable[oldIndex]->m_used = false;
        m_oldNumDeleted++;
        isTagged = true;
    }
    //Tags the file as deleted if found in the current table
    else if (currIndex != -1) {
        m_currentTable[currIndex]->m_used = false;
        m_currNumDeleted++;
        isTagged = true;
    }
    //Checks if a rehash is possible
    checkForRehash();
    return isTagged;
}

//getFile()
//Returns the file if found and an empty file if not
const File FileSys::getFile(string name, int block) const {
    //Checks for the index of the file in the old and current tables
    int currIndex = checkForFileInCurrent(name, block);
    int oldIndex = checkForFileInOld(name, block);
    
    //Returns the file from the old table (if found)
    if (oldIndex != -1 && m_oldTable[oldIndex]->m_used == true) {
        return *m_oldTable[oldIndex];
    }
    //Returns the file from the current table (if found)
    else if (currIndex != -1 && m_currentTable[currIndex]->m_used == true) {
        return *m_currentTable[currIndex];
    }
    return File();
}

//updateDiskBlock()
//Updates a file's diskBlock
bool FileSys::updateDiskBlock(File file, int block) {
    //Attempts to update only if block is within DISKMIN and DISKMAX
    if (!(block < DISKMIN || block > DISKMAX)) {
        //Checks for the index of the file in the old and current tables
        int currIndex = checkForFileInCurrent(file.m_name, file.m_diskBlock);
        int oldIndex = checkForFileInOld(file.m_name, file.m_diskBlock);

        //Updates the file's diskBlock from the old table (if found)
        if (oldIndex != -1) {
            m_oldTable[oldIndex]->m_diskBlock = block;
            return true;
        }
        //Updates the file's diskBlock from the current table (if found)
        else if (currIndex != 1) {
            m_currentTable[currIndex]->m_diskBlock = block;
            return true;
        }
    }
    return false;
}

//lambda()
//Returns the load factor
float FileSys::lambda() const {
    return (float) m_currentSize / m_currentCap;
}

//deletedRatio()
//Returns the deletedRatio
float FileSys::deletedRatio() const {
    return (float) m_currNumDeleted / m_currentSize;
}

//dump()
//Outputs the Hash Table
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

//isPrime()
//Checks if a number is a prime number
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

//findNextPrime
//Finds the next biggest prime number
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

//quadraticProbing()
//Searches for and returns an index using quadratic probing
int FileSys::quadraticProbing(int hashIndex, int capacity, File ** table, string name, int block, bool toFindItem) const {
    //Loops until the capacity of the table is reached
    for (int i = 0; i < capacity; i++) {
        //calculates index
        unsigned int index = (hashIndex + i * i) % capacity;
        //Returns the index if a file is found (purpose of retrieving the index of a file)
        if (toFindItem == true && table[index] != nullptr && table[index]->m_used == true && table[index]->m_name == name && table[index]->m_diskBlock == block) {
            return index;
        }
        //Returns the index of a potential place to store the file (purpose of retrieving the index of a potential storage spot)
        else if (toFindItem == false && (table[index] == nullptr || (table[index] != nullptr && table[index]->m_used == false))) {
            return index;
        }
    }
    return -1;
}

//doubleHashing()
//Searches for and returns an index using double hashing
int FileSys::doubleHashing(int hashIndex, int capacity, File ** table, string name, int block, bool toFindItem) const {
    //Loops until the capacity of the table is reached
    for (int i = 0; i < capacity; i++) {
        //calculates index
        int index = ((m_hash(name) % 101) + i * (11 - (m_hash(name) % 11))) % 101;
        //Returns the index if a file is found (purpose of retrieving the index of a file)
        if (toFindItem == true && table[index] != nullptr && table[index]->m_used == true && table[index]->m_name == name && table[index]->m_diskBlock == block) {
            return index;
        }
        //Returns the index of a potential place to store the file (purpose of retrieving the index of a potential storage spot)
        else if (toFindItem == false && (table[index] == nullptr || (table[index] != nullptr && table[index]->m_used == false))) {
            return index;
        }
    }
    return -1;
}

//linearProbing()
//Searches for and returns an index using linear probing
int FileSys::linearProbing(int hashIndex, int capacity, File ** table, string name, int block, bool toFindItem) const {
    //Loops until the capacity of the table is reached
    for (int i = 1; i < capacity; i++) {
        //calculates index
        int index = (hashIndex + i) % capacity;
        //Returns the index if a file is found (purpose of retrieving the index of a file)
        if (toFindItem == true && table[index] != nullptr && table[index]->m_used == true && table[index]->m_name == name && table[index]->m_diskBlock == block) {
            return index;
        }
        //Returns the index of a potential place to store the file (purpose of retrieving the index of a potential storage spot)
        else if (toFindItem == false && (table[index] == nullptr || (table[index] != nullptr && table[index]->m_used == false))) {
            return index;
        }
    }
    return -1;
}

//findFileIndex()
//Returns the index of a file
int FileSys::findFileIndex(int hashIndex, int capacity, File **table, prob_t policy, string name, int block) const {
    //If the hashIndex has the file
    if (table[hashIndex] != nullptr && (table[hashIndex]->m_name == name) && table[hashIndex]->m_diskBlock == block) {
        return hashIndex;
    }
    //Searching via probing policy and returns the returned index
    else if (policy == QUADRATIC) {
        return quadraticProbing(hashIndex, capacity, table, name, block, true);
    }
    else if (policy == DOUBLEHASH) {
        return doubleHashing(hashIndex, capacity, table, name, block, true);
    }
    else if (policy == LINEAR) {
        return linearProbing(hashIndex, capacity, table, name, block, true);
    }
    return -1;
}

//checkForFileInOld()
//Checks for a file in the old table and returns an index if found
int FileSys::checkForFileInOld(string name, int block) const {
    //Returns an invalid index if there is no old table
    if (m_oldTable == nullptr) {
        return -1;
    }
    return findFileIndex((m_hash(name) % m_oldCap), m_oldCap, m_oldTable, m_oldProbing, name, block);
}

//checkForFileInCurrent()
//Checks for a file in the current table and returns an index if found
int FileSys::checkForFileInCurrent(string name, int block) const {
    return findFileIndex((m_hash(name) % m_currentCap), m_currentCap, m_currentTable, m_currProbing, name, block);
}

//checkForRehash()
//Rehashes if certain criteria is met
void FileSys::checkForRehash() {
    //If load factor is greater than 50 percent or deleted ratio is greater than 80 percent or there is a rehash in progress
    if (lambda() > 0.5f || deletedRatio() > 0.8f || m_transferIndex != 0) {
        rehash();
    }
}

//rehash()
//Flips current to old and rehashes old files into a new current table
void FileSys::rehash() {
    //Starts a new rehash if transferIndex is 0
    if (m_transferIndex == 0) {
        //Transfers current member variables into the old member variables
        m_oldCap = m_currentCap;
        m_oldNumDeleted = m_currNumDeleted;
        m_oldProbing = m_currProbing;
        m_oldSize = m_currentSize;
        m_oldTable = m_currentTable;

        //Initializes a new table
        m_currentCap = findNextPrime((m_currentSize - m_currNumDeleted) * 4);
        m_currentTable = new File*[m_currentCap]();
        m_currentSize = 0;
        m_currNumDeleted = 0;
        //If there is a new policy request
        if (m_newPolicy != m_currProbing) {
            m_currProbing = m_newPolicy;
        }
    }
    
    //Attempts to loop through all of the old table until 25% progress has been made or the whole table has been scanned
    for (int i = 0; i < ((m_oldCap / 4) + 1); i++) {
        if (m_transferIndex == m_oldCap) {
            break;
        }
        //Transfers any live nodes to the current table by insertion
        else if ((m_transferIndex + i) < m_oldCap && m_oldTable[m_transferIndex + i] != nullptr && m_oldTable[m_transferIndex + i]->m_used == true) {
            m_oldTable[m_transferIndex + i]->m_used = false;
            insert(*m_oldTable[m_transferIndex + i]);
            m_oldNumDeleted++;
        }
    }
    m_transferIndex += (m_oldCap / 4);

    //When the transfer process has gone through the whole table
    if (m_transferIndex >= m_oldCap) {
        //Deallocates the old table
        for (int i = 0; i < m_oldCap; i++) {
            delete m_oldTable[i];
        }
        delete[] m_oldTable;

        //Sets old member variables to default values
        m_transferIndex = 0;
        m_oldCap = 0;
        m_oldNumDeleted = 0;
        m_oldSize = 0;
        m_oldTable = nullptr;
    }
}