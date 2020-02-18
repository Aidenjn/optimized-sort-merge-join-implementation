#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <sstream>
#include <iterator>
#include <map>
#include <bitset>
#include <math.h>

using namespace std;

// Struct for holding records
struct record {
  string id; // Fixed at 8 bytes
  string name; // Max of 200 bytes
  string bio; // Max of 500 bytes
  string manager_id; // Fixed at 8 bytes
};

// Global Variables
int buckets; // amount of buckets currently in the index
int bitRep; // amount of bits currently being used in the keys
int totalSpaceUsed; // amount of bytes the records are taking up

// Function prototypes
void processRecords(string csvName);
void createIndex();
void lookupId(string id);
struct record* add_record(string id, string name, string bio, string manager_id); 
int binaryStrToDecimal(string str);
string idToBitString(string id);
string getLastIBits(string key, int i);
string flipBit(string key);
string toBinary(int n);
void reorderTable();
void insertRecord(struct record rec);
string getLineOfIndex(int lineNumber);
int countLinesOfIndex();
void changeLineOfIndex(int lineNumber, string newLine);
void addLineToIndex(string newLine);
int getIndexLineNumber(string index);
vector<struct record> readRecordsAtIndex(string index);
void writeRecordsAtIndex(string index, vector<struct record> records);
string idToIndexKey(string id);
void initIndex();
void handleFlippedKeyValues(string key);

int main(int argc, char* argv[])
{
    if (argc < 2) {
        cerr << "Usage: " << endl;
        cerr << "For tuple lookup: " << argv[0] << " -L [ID]" << endl;
        cerr << "Tuple lookup example: " << argv[0] << " -L 4" << endl;
        cerr << "For index creation: " << argv[0] << " -C" << endl;
        return 1;
    }
    cout << "Correct number of arguments given" << endl;

    string mode = argv[1];

    if (mode ==  "-C") { // Create table mode
        createIndex();
    }
    else if (mode ==  "-L") { // Lookup mode 
        string id = argv[2];
        lookupId(id);
    }

    return 0;
}

void createIndex() {
    cout << "Creating index from csv file..." << endl;

    // initialize with two buckets
    initIndex();
    bitRep = 1;
    buckets = 2;
    totalSpaceUsed = 0;

    processRecords("Employee.csv");
    cout << "Index Created!" << endl;
}

void lookupId(string id) {
    cout << "Looking up " << id << " in index data..." << endl;
    bool foundID = false;
    
    // Find amount of bits used to represent keys
    int keySize = 0;
    string firstLine = getLineOfIndex(1); // get first line of index
    char c;
    do {
        c = firstLine[keySize];
        keySize++;
    } while (c != '|');
    keySize--; // don't include delimeter in character count
    bitRep = keySize;
    buckets = countLinesOfIndex();

    // createkey from id
    string key = idToIndexKey(id);

    // Print info
    cout << "Key: " << key << endl;
    cout << "Line of EmployeeIndex: " << getIndexLineNumber(key) << endl;

    // lookup records with key
    vector<struct record> records = readRecordsAtIndex(key);

    for (int i = 0; i < records.size(); i++) {
        if (records[i].id == id) {
            cout << "Record Found ->" << endl;
            cout << "\tID: " << records[i].id << endl;
            cout << "\tName: " << records[i].name << endl;
            cout << "\tBio: " << records[i].bio << endl;
            cout << "\tManager ID: " << records[i].manager_id << endl;
            foundID = true;
        }
    }

    if (foundID == false) {
        cout << "Could not find record." << endl;
    }

}

int getRecordSize(struct record r) {
    return r.id.length() + r.name.length() + r.bio.length() + r.manager_id.length();
}

struct record* add_record(string id, string name, string bio, string manager_id) {

    // Check to see if attributes are correct byte lengths
    if (id.length() != 8) {
        cerr << "ERROR: id value [" << id << "] not 8 bytes" << endl;
    }
    else if (name.length() < 1 || name.length() > 200) {
        cerr << "ERROR: name value [" << name << "] not within range of 1-200 bytes" << endl;
    }
    else if (bio.length() < 1 || bio.length() > 500) {
        cerr << "ERROR: bio value [" << bio << "] not within range of 1-200 bytes" << endl;
    }
    if (manager_id.length() != 8) {
        cerr << "ERROR: manager_id value [" << manager_id << "] not 8 bytes" << endl;
    }

    // Create new record
    struct record* returnRecord= new struct record;

    returnRecord->id = id;
    returnRecord->name = name;
    returnRecord->bio = bio;
    returnRecord->manager_id = manager_id;

    return returnRecord; 
}

void processRecords(string csvName){
    vector<string> csvAttributes;
    string attribute; // For tokenizing csv lines
    ifstream infile(csvName);
    

    for(string line; getline(infile, line);)
    {
        stringstream linestream(line);
        while (getline(linestream, attribute, ','))
        {
            csvAttributes.push_back(attribute);
        }
        
        // Add record to index
        insertRecord(*add_record(csvAttributes[0], csvAttributes[1], csvAttributes[2], csvAttributes[3]));
        csvAttributes.clear();
    }
}

vector<struct record> readRecordsAtIndex(string index){
    vector<struct record> records;
    vector<string> attributes;
    string record; // For tokenizing the line
    string attribute; // For tokenizing the line
    ifstream infile("EmployeeIndex");

    int linenumber = getIndexLineNumber(index);
    string line = getLineOfIndex(linenumber);
    line = line.substr(index.length() + 1, line.length()); // remove key from front of line

    stringstream linestream(line);

    while (getline(linestream, record, '|'))
    {
        stringstream recordstream(record);
        while (getline(recordstream, attribute, ',')) {
            attributes.push_back(attribute);
        }
        records.push_back(*add_record(attributes[0], attributes[1], attributes[2], attributes[3]));
        attributes.clear();
    }
    return records;
}

void writeRecordsAtIndex(string index, vector<struct record> records) {
    vector<string> attributes;
    string record; // For tokenizing the line
    string attribute; // For tokenizing the line
    ifstream infile("EmployeeIndex");

    int linenumber = getIndexLineNumber(index);
    string line = getLineOfIndex(linenumber);
    string key = line.substr(0, bitRep + 1); // get key from front of line
    string writeline = key;

    for (int i=0; i < records.size(); i++) {
        writeline = writeline + records[i].id + "," + records[i].name + "," + records[i].bio + "," + records[i].manager_id;
        if (i != records.size() - 1) {
            writeline = writeline + "|";
        }
    }

    changeLineOfIndex(linenumber, writeline);
}

int binaryStrToDecimal(string str) {
    int num = 0;
    for (int i=0; i < str.length(); i++) {
        if (str[i] == '1') {
            num += pow(2, str.length() - i - 1);
        }
    }
    return num;
}

// Code refrenced from stack overflow user user3478487
// Reference link: https://stackoverflow.com/questions/22746429/c-decimal-to-binary-converting/22746526
string toBinary(int n)
{
    std::string r;
    while(n!=0) {r=(n%2==0 ?"0":"1")+r; n/=2;}
    return r;
}


string idToBitString(string id) {
    string bitstring = "";
    for (char& letter : id) {
        bitstring += bitset<8>(letter).to_string();
    }
    return bitstring;
}

string getLastIBits(string key, int i) {
    return key.substr(key.length()-i);
}

string flipBit(string key) {
    if (key[0] == '0') {
            key[0] = '1';
    }
    else if (key[0] == '1') {
        key[0] = '0';
    }
    return key;
}

string idToIndexKey(string id) {
    string key = idToBitString(id);
    key = getLastIBits(key, bitRep);

    // Flip bits if decimal rep is bigger than bucket amount
    if (binaryStrToDecimal(key) >= buckets) {
        key = flipBit(key);
    }

    return key;
}


void insertRecord(struct record rec) {
    string key = idToIndexKey(rec.id);

    // Find bucket based on key
    vector<struct record> bucketRecords = readRecordsAtIndex(key);
    bucketRecords.push_back(rec);
    // Add to space count
    totalSpaceUsed += getRecordSize(rec);

    writeRecordsAtIndex(key, bucketRecords);

    
    // Trigger extension if overload occurs
    if (totalSpaceUsed / 4096 > 0.8) { // if space goes over load factor
        // Extend if necessary
        if (buckets + 1 > pow(2, bitRep)) {
           bitRep++;
           reorderTable(); // Increase indexes and reorder
        }
        buckets++;

        string newBucketKey = toBinary(buckets - 1);
        addLineToIndex(newBucketKey + "|");

        // put unfliped values in proper bucket
        handleFlippedKeyValues(newBucketKey);
        
    }
}

void handleFlippedKeyValues(string key) {
    vector<struct record> recordsAtOldBucket;
    string intendedKey;
    struct record moveRecord;
    vector<struct record> recordsAtBucketToMoveTo;
    vector<int> recordsToErase;

    string flipKey = flipBit(key);

    recordsAtOldBucket = readRecordsAtIndex(flipKey);

    for (int j = 0; j < recordsAtOldBucket.size(); j++) {
        intendedKey = idToIndexKey(recordsAtOldBucket[j].id);
        
        if (intendedKey != flipKey) {
            moveRecord = recordsAtOldBucket[j];
            recordsToErase.push_back(j);
            recordsAtBucketToMoveTo = readRecordsAtIndex(intendedKey);
            recordsAtBucketToMoveTo.push_back(moveRecord);
            writeRecordsAtIndex(intendedKey, recordsAtBucketToMoveTo);
        }
    }

    // Erase moved buckets at original values to prevent duplicates
    for (int j = 0; j < recordsToErase.size(); j++) {
        recordsAtOldBucket.erase(recordsAtOldBucket.begin()+recordsToErase[j]);
        writeRecordsAtIndex(flipKey, recordsAtOldBucket);
    }
    
    recordsToErase.clear();
}

void reorderTable() {
    int count = 0;
    string bucketKey;
    string line;
    vector<struct record> recordsAtBucket;
    string intendedKey;
    struct record moveRecord;
    vector<struct record> recordsAtBucketToMoveTo;
    vector<int> recordsToErase;
    
    // Change keys
    for (int i = 0; i < buckets; i++) {
        bucketKey = toBinary(i);
        int zeros = bitRep - bucketKey.length();
        for (int j = 0; j < zeros; j++) {
            bucketKey = "0" + bucketKey;
        }
        line = getLineOfIndex(i + 1);
        line = bucketKey + line.substr(bitRep - 1, line.length());
        changeLineOfIndex(i + 1, line);
    }

    // Reorder records to new keys
    for (int i = 0; i < buckets; i++) {
        bucketKey = toBinary(i);
        int zeros = bitRep - bucketKey.length();
        for (int j = 0; j < zeros; j++) {
            bucketKey = "0" + bucketKey;
        }
        recordsAtBucket = readRecordsAtIndex(bucketKey);

        for (int j = 0; j < recordsAtBucket.size(); j++) {
            intendedKey = idToIndexKey(recordsAtBucket[j].id);
            
            if (intendedKey != bucketKey) {
                moveRecord = recordsAtBucket[j];
                recordsToErase.push_back(j);
                recordsAtBucketToMoveTo = readRecordsAtIndex(intendedKey);
                recordsAtBucketToMoveTo.push_back(moveRecord);
                writeRecordsAtIndex(intendedKey, recordsAtBucketToMoveTo);
            }
        }

        // Erase moved buckets at original values to prevent duplicates
        for (int j = 0; j < recordsToErase.size(); j++) {
            recordsAtBucket.erase(recordsAtBucket.begin()+recordsToErase[j]);
            writeRecordsAtIndex(bucketKey, recordsAtBucket);
        }
        
        recordsToErase.clear();
    }
}

int countLinesOfIndex() {
    string line;
    int count = 0;
    ifstream indexfile {"EmployeeIndex"};

    while (getline(indexfile, line)) {
        count++;
    }
    indexfile.close();
    return count;
}

string getLineOfIndex(int lineNumber) {
    string line;
    ifstream indexfile {"EmployeeIndex"};

    for (int i=0; i < lineNumber-1; i++) {
        if (!getline(indexfile, line)) {
            cerr << "ERROR: File does not extend to line " << lineNumber << endl;
        }
    }
    getline(indexfile, line);
    indexfile.close();
    return line;
}

void changeLineOfIndex(int lineNumber, string newLine) {
    string line;
    ifstream indexfile {"EmployeeIndex"};
    ofstream newIndexFile {"NewEmployeeIndex"};
    for (int i=0; i < lineNumber-1; i++) {
        if (!getline(indexfile, line)) {
            cerr << "ERROR: File does not extend to line " << lineNumber << endl;
        }
        newIndexFile << line + "\n";
    }

    
    // insert new line
    newIndexFile << newLine + "\n";

    // discard old line
    getline(indexfile, line);

    // write remaining lines of old file
    while (getline(indexfile, line)) {
        newIndexFile << line + "\n";
    }

    indexfile.close();
    newIndexFile.close();
    // Delete old index
    if (remove("EmployeeIndex")!=0) {
        cerr << "ERROR: Could not delete old index file" << endl;
    }
    // Rename new index
    rename("NewEmployeeIndex", "EmployeeIndex");
}

void addLineToIndex(string newLine) {
    string line;
    ifstream indexfile {"EmployeeIndex"};
    ofstream newIndexFile {"NewEmployeeIndex"};

    // write lines of old file
    while (getline(indexfile, line)) {
        newIndexFile << line + "\n";
    }

    // insert new line
    newIndexFile << newLine + "\n";

    indexfile.close();
    newIndexFile.close();

    // Delete old index
    if (remove("EmployeeIndex")!=0) {
        cerr << "ERROR: Could not delete old index file" << endl;
    }
    // Rename new index
    rename("NewEmployeeIndex", "EmployeeIndex");

    
}

int getIndexLineNumber(string index) {
    string line;
    ifstream indexfile {"EmployeeIndex"};
    int lineNumber = 0;

    while (getline(indexfile, line)) {
        lineNumber++;
        if (index == line.substr(0, bitRep)) {
            indexfile.close();
            return lineNumber;
        }
    }
    indexfile.close();
    cerr << "ERROR: Could not find line number for index" << endl;
    return -5;
}

void initIndex() {
    
    if (remove("EmployeeIndex")!=0) {
        //cerr << "ERROR: Could not delete old index file" << endl;
    }
    
    ofstream indexfile {"EmployeeIndex"};

    indexfile << "0|\n";
    indexfile << "1|\n";

    indexfile.close();
}


