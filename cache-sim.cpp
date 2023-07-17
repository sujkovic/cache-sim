#include <iostream>
#include <stdlib.h>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cmath>
#include <queue>

#include "PLRUTree.cpp"

using namespace std;

//  Global variables

vector<char> instructions;
vector<unsigned long long> addresses;
vector<string> outputVec;
struct cacheLine {
    unsigned long long tag;
    int lru;
  };

//  Helper Functions

void readFile(string inputArg) {
  //  Initialize variables
  unsigned long long target;
  string line, fixedInputArg;
  char behavior;
  fixedInputArg = inputArg;

  //  Read in traces file
  ifstream infile(fixedInputArg);
  while(getline(infile, line)) {
    stringstream s(line);
    s >> behavior >> std::hex >> target;
    // Now we can output it
    instructions.push_back(behavior);
    addresses.push_back(target);
  }
}

void outputToFile(string outputArg) {
  ofstream outputFile(outputArg, _S_trunc);
  for (unsigned int i = 0; i < outputVec.size(); i++) {
    outputFile << outputVec[i] << "\n";
  }
  outputFile.close();
}

//  Simulator Functions

int directMappedCache(int cacheSize) {
    int cacheLineSize = 32; 
    int numCacheLines = (cacheSize * 1024) / cacheLineSize;
    vector<unsigned long long> cache(numCacheLines, 0);
    int hits = 0; 
    for (unsigned int i = 0; i < addresses.size(); i++) {
      int cacheLineIndex = (addresses[i] / cacheLineSize) % numCacheLines;
      if (cache[cacheLineIndex] == (addresses[i] >> 5)) {
        hits++;
      }
      else {
        cache[cacheLineIndex] = addresses[i] >> 5;
      }
    }
    return hits;
}

int setAssociativeCache(int associativity) {
  int cacheLineSize = 32;
  int cacheSize = 16 * 1024;
  int numSets = cacheSize / (cacheLineSize * associativity);
  //  initialize cache
  vector<vector<cacheLine>> cache;
  for (int i = 0; i < numSets; i++) {
    vector<cacheLine> cacheLines;
    for (int j = 0; j < associativity; j++) {
      cacheLine cacheline = {0, j};
      cacheLines.push_back(cacheline);
    }
    cache.push_back(cacheLines);
  }
  int hits = 0;
  for (unsigned int i = 0; i < addresses.size(); i++) {
    int setNum = (addresses[i] / cacheLineSize) % numSets;
    unsigned long long tag = addresses[i] >> (unsigned long long)(5 + log2(numSets));
    bool hit = false;
    int indexOfLRU;
    int indexOfHit;
    int curLRUVal = -1;
    bool notLRUOrMRU = false;
    bool isMRU = false;
    //  Look for tag within the set
    for (int j = 0; j < associativity; j++) {
      //  cache hit
      if (cache[setNum][j].tag == tag && !hit) {
        if (cache[setNum][j].lru == (associativity - 1)) {
          isMRU = true;
        }
        else if (cache[setNum][j].lru != 0) {
          notLRUOrMRU = true;
          curLRUVal = cache[setNum][j].lru;
          indexOfHit = j;
        }
        hits++;
        hit = true;
      }
      if (cache[setNum][j].lru == 0) {
        indexOfLRU = j;
      }
    }

    //  updating lru case 1
    if (notLRUOrMRU && !isMRU) {
      for (int j = 0; j < associativity; j++) {
        if (j == indexOfHit) {
          cache[setNum][j].lru = associativity - 1;
        }
        else if (cache[setNum][j].lru > curLRUVal) {
          cache[setNum][j].lru--;
        }
      }
    }
    //  updating lru case 2
    if (!notLRUOrMRU && !isMRU) {
      for (int j = 0; j < associativity; j++) {
        //  set lru to mru
        if (j == indexOfLRU) {
          cache[setNum][j].lru = associativity - 1;
          cache[setNum][j].tag = tag;
        }
        //  decrement lru at each cacheLine by 1
        else {
          cache[setNum][j].lru--; 
        }
      }
     }

    /*
    //  testing  
     if (i == addresses.size() - 2) {
      for (int i = 0; i < numSets; i++) {
        cout << "\nSet " << i << "\n";
        for (int j = 0; j < associativity; j++) {
          cout << "(tag " << cache[i][j].tag << " lru " << cache[i][j].lru << ") ";
        }
      } 
     }
    */
  }
  return hits;
}

void NOTWORKINGfullyAssociativeCacheWithLRU() {
  //  init variables
  int cacheSize = 16 * 1024;
  int cacheLineSize = 32;
  int numCacheLines = cacheSize / cacheLineSize;
  vector<cacheLine> cache;
  for (int i = 0; i < numCacheLines; i++) {
    cacheLine cacheline = {0, i};
    cache.push_back(cacheline);
  }
  int hits = 0;
  //  loop through input file
  for (unsigned int i = 0; i < addresses.size(); i++) {
    //  init variables
    unsigned long long curTag = addresses[i] >> (unsigned long long)log2(cacheLineSize);
    bool specialCase = false;
    bool isMRU = false;
    bool hit = false;
    int indexOfLRU;
    int indexOfHit;
    int curLRUVal;
    //  look through the cache
    for (int j = 0; j < numCacheLines; j++) {
      cacheLine curCacheLine = cache[j];
      //  Cache hit
      if (curCacheLine.tag == curTag && !hit) {
        if (curCacheLine.lru == (numCacheLines - 1)) {
          isMRU = true;
        }
        else if (curCacheLine.lru != 0) {
          specialCase = true;
          indexOfHit = j;
          curLRUVal = curCacheLine.lru;
        }
        hits++;
        hit = true;
      }
      //  keep track of lru index
      if (curCacheLine.lru == 0) {
        indexOfLRU = j;
      }
    }

    //  Update LRU (case where its a hit and not the LRU)
      if (!specialCase && !isMRU) {
        for (int j = 0; j < numCacheLines; j++) {
          if (j == indexOfHit) {
            cache[j].lru = numCacheLines - 1;
          }
          else if (cache[j].lru > curLRUVal) {
            cache[j].lru--;
          }
        }
      }

      //  Update LRU (case where either its a miss, or its a hot and lru)
      if (specialCase && !isMRU) {
        for (int j = 0; j < numCacheLines; j++) {
          //  set lru to mru and set tag
          if (j == indexOfLRU) {
            cache[j].lru = numCacheLines - 1;
            cache[j].tag = curTag;
          }
          //  decrement lru at each cache line by one
          else {
            cache[j].lru--;
          }
        }
      }
      
      if (i == 3) {
        for (int j = 0; j < numCacheLines; j++) {
          cout << "(tag " << cache[j].tag << " lru " << cache[j].lru << ") \n";
        }
      }
  }
  string output = to_string(hits) + "," + to_string(addresses.size()) + ";";
  outputVec.push_back(output);
}

void fullyAssociativeCacheWithPLRU() {
  //  init vars
  int cacheLineSize = 32;
  int cacheSize = 16 * 1024;    
  int numCacheLines = cacheSize / cacheLineSize;
  int hits = 0;
  PLRUTree plru(numCacheLines);
  for (unsigned int i = 0; i < addresses.size(); i++) {
    unsigned long long curTag = addresses[i] << (unsigned long long)log2(cacheLineSize);
    //  cache hit
    bool hit = plru.searchCache(curTag);
    if (hit) {
      hits++;
    }
    plru.insert(curTag, hit);
    
  }
  string output = to_string(hits) + "," + to_string(addresses.size()) + ";";
  outputVec.push_back(output);
}

int setAssociativeCacheWithNoAllocationOnWriteMiss(int associativity) {
  //  init vars
  int cacheLineSize = 32;
  int cacheSize = 16 * 1024;
  int numSets = cacheSize / (cacheLineSize * associativity);
  //  initialize cache
  vector<vector<cacheLine>> cache;
  for (int i = 0; i < numSets; i++) {
    vector<cacheLine> cacheLines;
    for (int j = 0; j < associativity; j++) {
      cacheLine cacheline = {0, j};
      cacheLines.push_back(cacheline);
    }
    cache.push_back(cacheLines);
  }
  int hits = 0;
  for (unsigned int i = 0; i < addresses.size(); i++) {
    int setNum = (addresses[i] / cacheLineSize) % numSets;
    unsigned long long tag = addresses[i] >> (unsigned long long)(5 + log2(numSets));
    bool hit = false;
    int indexOfLRU;
    int indexOfHit;
    int curLRUVal = -1;
    bool notLRUOrMRU = false;
    bool isMRU = false;
    //  Look for tag within the set
    for (int j = 0; j < associativity; j++) {
      //  cache hit
      if (cache[setNum][j].tag == tag && !hit) {
        if (cache[setNum][j].lru == (associativity - 1)) {
          isMRU = true;
        }
        else if (cache[setNum][j].lru != 0) {
          notLRUOrMRU = true;
          curLRUVal = cache[setNum][j].lru;
          indexOfHit = j;
        }
        hits++;
        hit = true;
      }
      if (cache[setNum][j].lru == 0) {
        indexOfLRU = j;
      }
    }

    //  updating lru case 1
    if (notLRUOrMRU && !isMRU) {
      for (int j = 0; j < associativity; j++) {
        if (j == indexOfHit) {
          cache[setNum][j].lru = associativity - 1;
        }
        else if (cache[setNum][j].lru > curLRUVal) {
          cache[setNum][j].lru--;
        }
      }
    }
    //  updating lru case 2
    if (!notLRUOrMRU && !isMRU && !(!hit && (instructions[i] == 'S'))) {
      for (int j = 0; j < associativity; j++) {
        //  set lru to mru
        if (j == indexOfLRU) {
          cache[setNum][j].lru = associativity - 1;
          cache[setNum][j].tag = tag;
        }
        //  decrement lru at each cacheLine by 1
        else {
          cache[setNum][j].lru--; 
        }
      }
     }

    /*
    //  testing  
     if (i == addresses.size() - 2) {
      for (int i = 0; i < numSets; i++) {
        cout << "\nSet " << i << "\n";
        for (int j = 0; j < associativity; j++) {
          cout << "(tag " << cache[i][j].tag << " lru " << cache[i][j].lru << ") ";
        }
      } 
     }
    */
  }
  return hits;
}

int setAssociativeCacheWithNextLinePrefetching(int associativity) {
  int cacheLineSize = 32;
  int cacheSize = 16 * 1024;
  int numSets = cacheSize / (cacheLineSize * associativity);
  //  initialize cache
  vector<vector<cacheLine>> cache;
  for (int i = 0; i < numSets; i++) {
    vector<cacheLine> cacheLines;
    for (int j = 0; j < associativity; j++) {
      cacheLine cacheline = {0, j};
      cacheLines.push_back(cacheline);
    }
    cache.push_back(cacheLines);
  }
  int hits = 0;
  for (unsigned int i = 0; i < addresses.size(); i++) {
    int setNum = (addresses[i] / cacheLineSize) % numSets;
    unsigned long long tag = addresses[i] >> (unsigned long long)(5 + log2(numSets));
    bool hit = false;
    int indexOfLRU;
    int indexOfHit;
    int curLRUVal = -1;
    bool notLRUOrMRU = false;
    bool isMRU = false;
    //  Look for tag within the set
    for (int j = 0; j < associativity; j++) {
      //  cache hit
      if (cache[setNum][j].tag == tag && !hit) {
        if (cache[setNum][j].lru == (associativity - 1)) {
          isMRU = true;
        }
        else if (cache[setNum][j].lru != 0) {
          notLRUOrMRU = true;
          curLRUVal = cache[setNum][j].lru;
          indexOfHit = j;
        }
        hits++;
        hit = true;
      }
      if (cache[setNum][j].lru == 0) {
        indexOfLRU = j;
      }
    }

    //  updating lru case 1
    if (notLRUOrMRU && !isMRU) {
      for (int j = 0; j < associativity; j++) {
        if (j == indexOfHit) {
          cache[setNum][j].lru = associativity - 1;
        }
        else if (cache[setNum][j].lru > curLRUVal) {
          cache[setNum][j].lru--;
        }
      }
    }
    //  updating lru case 2
    if (!notLRUOrMRU && !isMRU) {
      for (int j = 0; j < associativity; j++) {
        //  set lru to mru
        if (j == indexOfLRU) {
          cache[setNum][j].lru = associativity - 1;
          cache[setNum][j].tag = tag;
        }
        //  decrement lru at each cacheLine by 1
        else {
          cache[setNum][j].lru--; 
        }
      }
     }

     //  PREFETCHING
     
     hit = false;
    notLRUOrMRU = false;
    isMRU = false;
     int nextSetNum = ((addresses[i] + cacheLineSize) / cacheLineSize) % numSets;
     unsigned long long nextTag = (addresses[i] + cacheLineSize) >> (unsigned long long)(5 + log2(numSets));
    for (int j = 0; j < associativity; j++) {
      //  cache hit
      if (cache[nextSetNum][j].tag == nextTag && !hit) {
        if (cache[nextSetNum][j].lru == (associativity - 1)) {
          isMRU = true;
        }
        else if (cache[nextSetNum][j].lru != 0) {
          notLRUOrMRU = true;
          curLRUVal = cache[nextSetNum][j].lru;
          indexOfHit = j;
        }
        hit = true;
      }
      if (cache[nextSetNum][j].lru == 0) {
        indexOfLRU = j;
      }
    }

    //  updating lru case 1
    if (notLRUOrMRU && !isMRU) {
      for (int j = 0; j < associativity; j++) {
        if (j == indexOfHit) {
          cache[nextSetNum][j].lru = associativity - 1;
        }
        else if (cache[nextSetNum][j].lru > curLRUVal) {
          cache[nextSetNum][j].lru--;
        }
      }
    }
    //  updating lru case 2
    if (!notLRUOrMRU && !isMRU) {
      for (int j = 0; j < associativity; j++) {
        //  set lru to mru
        if (j == indexOfLRU) {
          cache[nextSetNum][j].lru = associativity - 1;
          cache[nextSetNum][j].tag = nextTag;
        }
        //  decrement lru at each cacheLine by 1
        else {
          cache[nextSetNum][j].lru--; 
        }
      }
     }
  }
  return hits;
}

int setAssociativeCacheWithPrefetchOnMiss(int associativity) {
  int cacheLineSize = 32;
  int cacheSize = 16 * 1024;
  int numSets = cacheSize / (cacheLineSize * associativity);
  //  initialize cache
  vector<vector<cacheLine>> cache;
  for (int i = 0; i < numSets; i++) {
    vector<cacheLine> cacheLines;
    for (int j = 0; j < associativity; j++) {
      cacheLine cacheline = {0, j};
      cacheLines.push_back(cacheline);
    }
    cache.push_back(cacheLines);
  }
  int hits = 0;
  for (unsigned int i = 0; i < addresses.size(); i++) {
    int setNum = (addresses[i] / cacheLineSize) % numSets;
    unsigned long long tag = addresses[i] >> (unsigned long long)(5 + log2(numSets));
    bool hit = false;
    int indexOfLRU;
    int indexOfHit;
    int curLRUVal = -1;
    bool notLRUOrMRU = false;
    bool isMRU = false;
    //  Look for tag within the set
    for (int j = 0; j < associativity; j++) {
      //  cache hit
      if (cache[setNum][j].tag == tag && !hit) {
        if (cache[setNum][j].lru == (associativity - 1)) {
          isMRU = true;
        }
        else if (cache[setNum][j].lru != 0) {
          notLRUOrMRU = true;
          curLRUVal = cache[setNum][j].lru;
          indexOfHit = j;
        }
        hits++;
        hit = true;
      }
      if (cache[setNum][j].lru == 0) {
        indexOfLRU = j;
      }
    }

    //  updating lru case 1
    if (notLRUOrMRU && !isMRU) {
      for (int j = 0; j < associativity; j++) {
        if (j == indexOfHit) {
          cache[setNum][j].lru = associativity - 1;
        }
        else if (cache[setNum][j].lru > curLRUVal) {
          cache[setNum][j].lru--;
        }
      }
    }
    //  updating lru case 2
    if (!notLRUOrMRU && !isMRU) {
      for (int j = 0; j < associativity; j++) {
        //  set lru to mru
        if (j == indexOfLRU) {
          cache[setNum][j].lru = associativity - 1;
          cache[setNum][j].tag = tag;
        }
        //  decrement lru at each cacheLine by 1
        else {
          cache[setNum][j].lru--; 
        }
      }
     }

    //  PREFETCHING
     
    if (!hit) {
       hit = false;
    notLRUOrMRU = false;
    isMRU = false;
     int nextSetNum = ((addresses[i] + cacheLineSize) / cacheLineSize) % numSets;
     unsigned long long nextTag = (addresses[i] + cacheLineSize) >> (unsigned long long)(5 + log2(numSets));
    for (int j = 0; j < associativity; j++) {
      //  cache hit
      if (cache[nextSetNum][j].tag == nextTag && !hit) {
        if (cache[nextSetNum][j].lru == (associativity - 1)) {
          isMRU = true;
        }
        else if (cache[nextSetNum][j].lru != 0) {
          notLRUOrMRU = true;
          curLRUVal = cache[nextSetNum][j].lru;
          indexOfHit = j;
        }
        hit = true;
      }
      if (cache[nextSetNum][j].lru == 0) {
        indexOfLRU = j;
      }
    }

    //  updating lru case 1
    if (notLRUOrMRU && !isMRU) {
      for (int j = 0; j < associativity; j++) {
        if (j == indexOfHit) {
          cache[nextSetNum][j].lru = associativity - 1;
        }
        else if (cache[nextSetNum][j].lru > curLRUVal) {
          cache[nextSetNum][j].lru--;
        }
      }
    }
    //  updating lru case 2
    if (!notLRUOrMRU && !isMRU) {
      for (int j = 0; j < associativity; j++) {
        //  set lru to mru
        if (j == indexOfLRU) {
          cache[nextSetNum][j].lru = associativity - 1;
          cache[nextSetNum][j].tag = nextTag;
        }
        //  decrement lru at each cacheLine by 1
        else {
          cache[nextSetNum][j].lru--; 
        }
      }
     }
    }
  }
  return hits;
}

void callDirectMappedCaches() {
  string output = "";
  string curOutput;
  curOutput = to_string(directMappedCache(1)) + "," + to_string(addresses.size()) + "; ";
  output += curOutput;
  curOutput = to_string(directMappedCache(4)) + "," + to_string(addresses.size()) + "; ";
  output += curOutput;
  curOutput = to_string(directMappedCache(16)) + "," + to_string(addresses.size()) + "; ";
  output += curOutput;
  curOutput = to_string(directMappedCache(32)) + "," + to_string(addresses.size()) + ";";
  output += curOutput;
  outputVec.push_back(output);
}

void callSetAssociativeCaches() {
  string output = "";
  string curOutput;
  curOutput = to_string(setAssociativeCache(2)) + "," + to_string(addresses.size()) + "; ";
  output += curOutput;
  curOutput = to_string(setAssociativeCache(4)) + "," + to_string(addresses.size()) + "; ";
  output += curOutput;
  curOutput = to_string(setAssociativeCache(8)) + "," + to_string(addresses.size()) + "; ";
  output += curOutput;
  curOutput = to_string(setAssociativeCache(16)) + "," + to_string(addresses.size()) + ";";
  output += curOutput;
  outputVec.push_back(output);
}

void callFullyAssociativeCacheWithLRU() {
  string output = "";
  output = to_string(setAssociativeCache(512)) + "," + to_string(addresses.size()) + ";";
  outputVec.push_back(output);
}

void callSetAssociativeCacheWithNoAllocationOnWriteMiss() {
  string output = "";
  string curOutput;
  curOutput = to_string(setAssociativeCacheWithNoAllocationOnWriteMiss(2)) + "," + to_string(addresses.size()) + "; ";
  output += curOutput;
  curOutput = to_string(setAssociativeCacheWithNoAllocationOnWriteMiss(4)) + "," + to_string(addresses.size()) + "; ";
  output += curOutput;
  curOutput = to_string(setAssociativeCacheWithNoAllocationOnWriteMiss(8)) + "," + to_string(addresses.size()) + "; ";
  output += curOutput;
  curOutput = to_string(setAssociativeCacheWithNoAllocationOnWriteMiss(16)) + "," + to_string(addresses.size()) + ";";
  output += curOutput;
  outputVec.push_back(output);
}

void callSetAssociativeCacheWithNextLinePrefetching() {
  string output = "";
  string curOutput;
  curOutput = to_string(setAssociativeCacheWithNextLinePrefetching(2)) + "," + to_string(addresses.size()) + "; ";
  output += curOutput;
  curOutput = to_string(setAssociativeCacheWithNextLinePrefetching(4)) + "," + to_string(addresses.size()) + "; ";
  output += curOutput;
  curOutput = to_string(setAssociativeCacheWithNextLinePrefetching(8)) + "," + to_string(addresses.size()) + "; ";
  output += curOutput;
  curOutput = to_string(setAssociativeCacheWithNextLinePrefetching(16)) + "," + to_string(addresses.size()) + ";";
  output += curOutput;
  outputVec.push_back(output);
}

void callSetAssociativeCacheWithPrefetchOnMiss() {
  string output = "";
  string curOutput;
  curOutput = to_string(setAssociativeCacheWithPrefetchOnMiss(2)) + "," + to_string(addresses.size()) + "; ";
  output += curOutput;
  curOutput = to_string(setAssociativeCacheWithPrefetchOnMiss(4)) + "," + to_string(addresses.size()) + "; ";
  output += curOutput;
  curOutput = to_string(setAssociativeCacheWithPrefetchOnMiss(8)) + "," + to_string(addresses.size()) + "; ";
  output += curOutput;
  curOutput = to_string(setAssociativeCacheWithPrefetchOnMiss(16)) + "," + to_string(addresses.size()) + ";";
  output += curOutput;
  outputVec.push_back(output);
}

int main(int argc, char *argv[]) {
  //  Check if program executed correctly
  if (argc != 3) {
    cout << "Incorrect format. Correct format is ./cache-sim <input-trace-file> <output-file>\n";
    return 1;
  }

  //  Call all simulation functions here
  readFile(argv[1]);
  
  callDirectMappedCaches();
  callSetAssociativeCaches();
  callFullyAssociativeCacheWithLRU();
  fullyAssociativeCacheWithPLRU();    
  callSetAssociativeCacheWithNoAllocationOnWriteMiss();
  callSetAssociativeCacheWithNextLinePrefetching();
  callSetAssociativeCacheWithPrefetchOnMiss();

  //  Write to output
  outputToFile(argv[2]);
  return 0;
}
