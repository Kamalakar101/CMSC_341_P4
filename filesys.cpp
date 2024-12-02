// CMSC 341 - Fall 2024 - Project 4
#include "filesys.h"
FileSys::FileSys(int size, hash_fn hash, prob_t probing = DEFPOLCY){
    if (size > MAXPRIME) {
        m_currentCap = MAXPRIME;
    }
    else if (size < MINPRIME) {
        m_currentCap = MINPRIME;
    }

    if (!isPrime(size)) {
        m_currentCap = findNextPrime(size);
    }
    else {
        m_currentCap = size;
    }
    
    m_hash = hash;
    m_currProbing = probing;
    m_currentTable = new File*[m_currentCap];
}

FileSys::~FileSys(){
    for (int i = 0; i < m_currentCap; i++) {
        delete m_currentTable[i];
    }
    delete[] m_currentTable;

    if (m_oldTable != nullptr) {
        for (int i = 0; i < m_oldCap; i++) {
            delete m_oldTable[i];
        }
        delete[] m_oldTable;
    }
}

void FileSys::changeProbPolicy(prob_t policy){
    m_newPolicy = policy;
}

bool FileSys::insert(File file){
    int hashIndex = m_hash(file.m_name) % m_currentCap;
    int index = -1;
    bool isInserted = false;

    //If file has an invalid disblock number or is a duplicate
    if (file.m_diskBlock < DISKMIN || file.m_diskBlock > DISKMAX || getFile(file.m_name, file.m_diskBlock) == file) {
        return false;
    }
    //If the hashIndex is available
    else if (m_currentTable[hashIndex] == nullptr) {
        m_currentTable[hashIndex] = new File(file);
        m_currentSize++;
        isInserted = true;
    }
    //Collision handling
    else if (m_currProbing == QUADRATIC) {
        index = quadraticProbing(hashIndex, m_currentCap, m_currentTable);
    }
    else if (m_currProbing == DOUBLEHASH) {
        index = doubleHashing(hashIndex, m_currentCap, m_currentTable);
    }
    else if (m_currProbing == LINEAR) {
        index = linearProbing(hashIndex, m_currentCap, m_currentTable);
    }

    //Insert if index is valid
    if (index != -1) {
        delete m_currentTable[index];
        m_currentTable[index] = new File(file);
        m_currentSize++;
        isInserted = true;
    }

    //If load factor is greater than 50 percent or deleted ratio is greater than 80 percent
    if (lambda() > 0.5 || deletedRatio() > 0.8) {
        rehash();
    }
    return isInserted;
}

bool FileSys::remove(File file){
    int currIndex = checkForFileInCurrent(file.m_name, file.m_diskBlock);
    int oldIndex = checkForFileInOld(file.m_name, file.m_diskBlock);
    bool isTagged = false;
    if (oldIndex != -1) {
        m_oldTable[oldIndex]->m_used = false;
        m_oldNumDeleted--;
        isTagged = true;
    }
    else if (currIndex != 1) {
        m_currentTable[currIndex]->m_used = false;
        m_currNumDeleted--;
        isTagged = true;
    }

    //If load factor is greater than 50 percent or deleted ratio is greater than 80 percent
    if (lambda() > 0.5 || deletedRatio() > 0.8) {
        rehash();
    }
    return isTagged;
}

const File FileSys::getFile(string name, int block) const {
    int currIndex = checkForFileInCurrent(name, block);
    int oldIndex = checkForFileInOld(name, block);
    if (oldIndex != -1 && m_oldTable[oldIndex]->m_used == true) {
        return *m_oldTable[oldIndex];
    }
    else if (currIndex != 1 && m_currentTable[currIndex]->m_used == true) {
        return *m_currentTable[currIndex];
    }
    return File();
}

bool FileSys::updateDiskBlock(File file, int block) {
    int currIndex = checkForFileInCurrent(file.m_name, file.m_diskBlock);
    int oldIndex = checkForFileInOld(file.m_name, file.m_diskBlock);
    if (oldIndex != -1) {
        m_oldTable[oldIndex]->m_diskBlock = block;
        return true;
    }
    else if (currIndex != 1) {
        m_currentTable[currIndex]->m_diskBlock = block;
        return true;
    }
    return false;
}

float FileSys::lambda() const {
    return m_currentSize / m_currentSize;
}

float FileSys::deletedRatio() const {
    return m_currNumDeleted / m_currentSize;
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

int FileSys::quadraticProbing(int hashIndex, int capacity, File ** table, string name, int block, bool toFindItem) const {
    int index = 0;
    for (int i = 0; i < capacity; i++) {
        index = (hashIndex + i * i) % capacity;
        if (toFindItem == true && table[index] != nullptr && table[index]->m_used == true && table[index]->m_name == name && table[index]->m_diskBlock == block) {
            return index;
        }
        else if (toFindItem == false && (table[index] == nullptr || (table[index] != nullptr && table[index]->m_used == false))) {
            return index;
        }
    }
    return -1;
}

int FileSys::doubleHashing(int hashIndex, int capacity, File ** table, string name, int block, bool toFindItem) const {
    int index = 0;
    for (int i = 0; i < capacity; i++) {
        index = (hashIndex + (11 - (hashIndex % 11)) * i) % capacity;
        if (toFindItem == true && table[index] != nullptr && table[index]->m_used == true && table[index]->m_name == name && table[index]->m_diskBlock == block) {
            return index;
        }
        else if (toFindItem == false && (table[index] == nullptr || (table[index] != nullptr && table[index]->m_used == false))) {
            return index;
        }
    }
    return -1;
}

int FileSys::linearProbing(int hashIndex, int capacity, File ** table, string name, int block, bool toFindItem) const {
    int index = 0;
    for (int i = 1; i < capacity; i++) {
        index = (hashIndex + i) % capacity;
        if (toFindItem == true && table[index] != nullptr && table[index]->m_used == true && table[index]->m_name == name && table[index]->m_diskBlock == block) {
            return index;
        }
        else if (toFindItem == false && (table[index] == nullptr || (table[index] != nullptr && table[index]->m_used == false))) {
            return index;
        }
    }
    return -1;
}

int FileSys::findFileIndex(int hashIndex, int capacity, File **table, prob_t policy, string name, int block) const {
    //If the hashIndex is available
    if (table[hashIndex] != nullptr && table[hashIndex]->m_name == name && table[hashIndex]->m_diskBlock == block) {
        return hashIndex;
    }
    //Searching via probing policy
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

int FileSys::checkForFileInOld(string name, int block) const {
    if (m_oldTable == nullptr) {
        return -1;
    }
    return findFileIndex((m_hash(name) % m_oldCap), m_oldCap, m_oldTable, m_oldProbing, name, block);
}

int FileSys::checkForFileInCurrent(string name, int block) const {
    return findFileIndex((m_hash(name) % m_currentCap), m_currentCap, m_currentTable, m_currProbing, name, block);
}

void FileSys::rehash() {
    if (m_transferIndex == 0) {
        m_oldCap = m_currentCap;
        m_oldNumDeleted = m_currNumDeleted;
        m_oldProbing = m_currProbing;
        m_oldSize = m_currentSize;
        m_oldTable = m_currentTable;

        m_currentCap = findNextPrime((m_currentSize - m_currNumDeleted) * 4);
        m_currentTable = m_currentTable = new File*[m_currentCap];
        m_currentSize = 0;
        m_currNumDeleted = 0;
        if (m_newPolicy != m_currProbing) {
            m_currProbing = m_newPolicy;
        }
    }
    int oldTransferIndex = m_transferIndex;
    for (m_transferIndex; m_transferIndex < m_oldCap; m_transferIndex++) {
        if (floor((m_transferIndex - oldTransferIndex) * 100 / m_currentCap) == 25) {
            break;
        }
        else if (m_oldTable[m_transferIndex] != nullptr && m_oldTable[m_transferIndex]->m_used == true) {
            insert(*m_oldTable[m_transferIndex]);
            m_oldTable[m_transferIndex]->m_used = false;
            m_oldNumDeleted++;
        }
    }

    if (m_transferIndex + 1 == m_oldCap) {
        for (int i = 0; i < m_oldCap; i++) {
            delete m_oldTable[i];
        }
        delete[] m_oldTable;

        m_oldCap = 0;
        m_oldNumDeleted = 0;
        m_oldSize = 0;
    }
}
