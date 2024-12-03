// CMSC 341 - Fall 2024 - Project 4
#ifndef FILESYS_H
#define FILESYS_H
#include <iostream>
#include <string>
#include "math.h"
using namespace std;
const int DISKMIN = 100000;
const int DISKMAX = 999999;
const int MINPRIME = 101;   // Min size for hash table
const int MAXPRIME = 99991; // Max size for hash table
typedef unsigned int (*hash_fn)(string); // declaration of hash function
enum prob_t {QUADRATIC, DOUBLEHASH, LINEAR}; // types of collision handling policy
#define DEFPOLCY QUADRATIC
class Grader;
class Tester;
class FileSys;
class File{
    public:
    friend class Grader;
    friend class Tester;
    friend class FileSys;
    File(string name="", int diskBlock=0, bool used=false){
        m_name = name; m_diskBlock = diskBlock; m_used = used;
    }
    string getName() const {return m_name;}
    int getDiskBlock() const {return m_diskBlock;}
    bool getUsed() const {return m_used;}
    void setName(string name) {m_name=name;}
    void setDiskBlock(int block) {m_diskBlock=block;}
    void setUsed(bool used) {m_used=used;}
    // the following function is a friend function
    friend ostream& operator<<(ostream& sout, const File *file ){
        if ((file != nullptr) && !(file->getName().empty()))
            sout << file->getName() << " (" << file->getDiskBlock() << ", "<< file->getUsed() <<  ")";
        else
            sout << "";
        return sout;
    }
    // the following function is a friend function
    friend bool operator==(const File& lhs, const File& rhs){
        // since the uniqueness of an object is defined by name and disk block
        // the equality operator considers only those two criteria
        return ((lhs.getName() == rhs.getName()) && (lhs.getDiskBlock() == rhs.getDiskBlock()));
    }
    // the following function is a class function
    bool operator==(const File* & rhs){
        // since the uniqueness of an object is defined by name and disk block
        // the equality operator considers only those two criteria
        return ((getName() == rhs->getName()) && (getDiskBlock() == rhs->getDiskBlock()));
    }
    // the following function is a class function
    const File& operator=(const File& rhs){
        if (this != &rhs){
            m_name = rhs.m_name;
            m_diskBlock = rhs.m_diskBlock;
            m_used = rhs.m_used;
        }
        return *this;
    }
    private:
    // m_name is the key of a File object and it is used for indexing
    string m_name;
    // m_diskBlock specifies the uniquness of a File object
    // It can hold a value in the range of [DISKMIN-DISKMAX]
    int m_diskBlock;
    // the following variable is used for lazy delete scheme in hash table
    // if it is set to false, it means the bucket in the hash table is free for insert
    // if it is set to true, it means the bucket contains live data, and we cannot overwrite it
    bool m_used;
};

class FileSys{
    public:
    friend class Grader;
    friend class Tester;
    FileSys(int size, hash_fn hash, prob_t probing);
    ~FileSys();
    // Returns Load factor of the new table
    float lambda() const;
    // Returns the ratio of deleted slots in the new table
    float deletedRatio() const;
    // insert only happens in the new table
    bool insert(File file);
    // remove can happen from either table
    bool remove(File file);
    // find can happen in either table
    const File getFile(string name, int block) const;
    // update the information
    bool updateDiskBlock(File file, int block);
    void changeProbPolicy(prob_t policy);
    void dump() const;
    private:
    hash_fn    m_hash;          // hash function
    prob_t     m_newPolicy;     // stores the change of policy request

    File**     m_currentTable;  // hash table
    int        m_currentCap;    // hash table size (capacity)
    int        m_currentSize;   // current number of entries
                                // m_currentSize includes deleted entries 
    int        m_currNumDeleted;// number of deleted entries
    prob_t     m_currProbing;   // collision handling policy

    File**     m_oldTable;      // hash table
    int        m_oldCap;        // hash table size (capacity)
    int        m_oldSize;       // current number of entries
                                // m_oldSize includes deleted entries
    int        m_oldNumDeleted; // number of deleted entries
    prob_t     m_oldProbing;    // collision handling policy

    int        m_transferIndex; // this can be used as a temporary place holder
                                // during incremental transfer to scanning the table

    //private helper functions
    bool isPrime(int number);
    int findNextPrime(int current);
    int quadraticProbing(int hashIndex, int capacity, File ** table, string name="", int block=0, bool toFindItem=false) const;
    int doubleHashing(int hashIndex, int capacity, File ** table, string name="", int block=0, bool toFindItem=false) const;
    int linearProbing(int hashIndex, int capacity, File ** table, string name="", int block=0, bool toFindItem=false) const;
    int findFileIndex(int hashIndex, int capacity, File ** table, prob_t policy, string name, int block) const;
    int checkForFileInOld(string name, int block) const;
    int checkForFileInCurrent(string name, int block) const;
    void checkForRehash();
    void rehash();

    /******************************************
    * Private function declarations go here! *
    ******************************************/

};

#endif