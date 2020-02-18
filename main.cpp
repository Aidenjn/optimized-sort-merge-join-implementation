#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <sstream>
#include <iterator>
#include <map>
#include <bitset>
#include <math.h>

// Constants
const int BUFFER_SIZE = 22;

using namespace std;

// Struct for holding department records
struct deptRecord {
  int did; // 4 bytes
  string dname; // 40 bytes 
  double budget; // 8 bytes
  string managerid; // 4 bytes
};

// Struct for holding employee records
struct empRecord {
  int eid; // 4 bytes
  string ename; // 40 bytes 
  int age; // 4 bytes
  double salary; // 8 bytes
};

// Global Variables
//int buckets; // amount of buckets currently in the index
//int bitRep; // amount of bits currently being used in the keys
//int totalSpaceUsed; // amount of bytes the records are taking up
int empRecordCount;
int deptRecordCount;


// Function prototypes
// void processRecords(string csvName);
void runSortEmpRecords(int m); // Sorts files into runs of m length
void runSortDeptRecords(int m); // Sorts files into runs of m length  
void merge(int m); // Merges files that are sorted into runs of m length

struct empRecord getEmpRecord(int index);

int countLinesOfFile(string filename);
string getLineOfFile(string fileName, int lineNumber);
void addLineToEOF(string filename, string newLine);

// Old prototypes !!! DELETE !!!

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

    // Calculate M
    int m = BUFFER_SIZE - 2; // -2 because reserving space for output and double buffer

    // Sort Dept records
    runSortDeptRecords(m);
    
    // Sort Emp records
    runSortEmpRecords(m);
    
    // Merge on Dept.managerid = Emp.eid
    merge(m);

    return 0;
}

bool compareEid(const empRecord &a, const empRecord &b) {
    return a.eid < b.eid;
}

void runSortEmpRecords(int m) {
    //struct empRecord curRec = getNextEmpRecord();
    string oldFilename = "Emp.csv";
    string newFilename = "sortingEmp.csv";
    ofstream output(newFilename);
    struct empRecord outRec;
    int curRecNum = 0;
    vector<struct empRecord> run = vector<struct empRecord>();

    int empLines = countLinesOfFile(oldFilename);

    while (curRecNum < empLines) {
        // load run into buffer
        for (int i=0; i<m && curRecNum < empLines; i++) { 
            // Retrieve and place record in buffer block
            run.push_back(getEmpRecord(curRecNum));
            curRecNum++;
        }

        // sort run
        sort(run.begin(), run.end(), compareEid);

        // write run to file
        for (int i=0; i<run.size(); i++) {
            outRec = run[i];
            cout << to_string(outRec.eid) + ", " + outRec.ename + "\n";
            addLineToEOF(newFilename, to_string(outRec.eid) + "," + outRec.ename + "," + to_string(outRec.age) + "," + to_string(outRec.salary));
        }
        run.clear();
    }
    // Unsorted file is replaced with the newly sorted file
    rename(newFilename.c_str(), oldFilename.c_str()); 
}

void runSortDeptRecords(int m) {

}

void merge(int m) {

}

struct empRecord createEmpRecord(string eid, string ename, string age, string salary) {
    struct empRecord returnRecord;
    returnRecord.eid = stoi(eid);
    returnRecord.ename = ename;
    returnRecord.age = stoi(age);
    returnRecord.salary = stof(salary);
    return returnRecord;
}

/*
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
*/

// Indexed at 0
struct empRecord getEmpRecord(int index) {
    vector<string> csvAttributes;
    string attribute; // For tokenizing csv lines
    string line = getLineOfFile("Emp.csv", index + 1); // File lines start at 1
    stringstream linestream(line);
    while (getline(linestream, attribute, ','))
    {
        csvAttributes.push_back(attribute);
    }
    
    // Create and return records from values extracted from the line
    return createEmpRecord(csvAttributes[0], csvAttributes[1], csvAttributes[2], csvAttributes[3]);
}



/*
void runSortEmpRecords(int m) {
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
*/
/*
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
*/

/*
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
*/

int countLinesOfFile(string filename ) {
    string line;
    int count = 0;
    ifstream file {filename};

    while (getline(file, line)) {
        count++;
    }

    file.close();
    return count;
}

string getLineOfFile(string fileName, int lineNumber) {
    string line;
    ifstream file {fileName};

    for (int i=0; i < lineNumber-1; i++) {
        if (!getline(file, line)) {
            cerr << "ERROR: File does not extend to line " << lineNumber << endl;
        }
    }

    getline(file, line);
    file.close();
    return line;
}

void addLineToEOF(string filename, string newLine) {
    string line;
    //ifstream indexfile {"EmployeeIndex"};
    //ofstream newIndexFile {"NewEmployeeIndex"};
    
    string newFilename = "new" + filename;

    ifstream oldFile {filename};
    ofstream newFile {newFilename};

    // write lines of old file
    while (getline(oldFile, line)) {
        newFile<< line + "\n";
    }

    // insert new line
    newFile<< newLine + "\n";

    oldFile.close();
    newFile.close();

    // Delete old index
    if (remove(filename.c_str())!=0) {
        cerr << "ERROR: Could not delete old index file" << endl;
    }
    // Rename new index
    rename(newFilename.c_str(), filename.c_str()); 

}
