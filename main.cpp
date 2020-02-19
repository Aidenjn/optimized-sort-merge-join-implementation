#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <sstream>
#include <iterator>
#include <map>
#include <bitset>
#include <math.h>
#include <algorithm>

// Constants
const int BUFFER_SIZE = 22;

using namespace std;

// Struct for holding department records
struct deptRecord {
  int did; // 4 bytes
  string dname; // 40 bytes 
  double budget; // 8 bytes
  int managerid; // 4 bytes
};

// Struct for holding employee records
struct empRecord {
  int eid; // 4 bytes
  string ename; // 40 bytes 
  int age; // 4 bytes
  double salary; // 8 bytes
};

// Dept Buffer Block for merge operation
struct deptBufferBlock {
    int elementCount;
    int currentElementLineNumber;
    struct deptRecord* blockRecord;
};

// Emp Buffer Block for merge operation
struct empBufferBlock {
    int elementCount;
    int currentElementLineNumber;
    struct empRecord* blockRecord;
};



// Function prototypes
// void processRecords(string csvName);
void runSortEmpRecords(int m); // Sorts files into runs of m length
void runSortDeptRecords(int m); // Sorts files into runs of m length  
void merge(int m); // Merges files that are sorted into runs of m length

struct empRecord getEmpRecord(int index);
struct deptRecord getDeptRecord(int index);

int countLinesOfFile(string filename);
string getLineOfFile(string fileName, int lineNumber);
void addLineToEOF(string filename, string newLine);

// Global Variables
int empRecordCount;
int deptRecordCount;


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

bool compareManagerid(const deptRecord &a, const deptRecord &b) {
    return a.managerid < b.managerid;
}

bool compareEid(const empRecord &a, const empRecord &b) {
    return a.eid < b.eid;
}

bool compareBuffManagerid(const deptBufferBlock &a, const deptBufferBlock &b) {
    return a.blockRecord->managerid < b.blockRecord->managerid;
}

bool compareBuffEid(const empBufferBlock &a, const empBufferBlock &b) {
    return a.blockRecord->eid < b.blockRecord->eid;
}

void runSortEmpRecords(int m) {
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
            //cout << to_string(outRec.eid) + ", " + outRec.ename + "\n";
            addLineToEOF(newFilename, to_string(outRec.eid) + "," + outRec.ename + "," + to_string(outRec.age) + "," + to_string(outRec.salary));
        }
        run.clear();
    }
    // Unsorted file is replaced with the newly sorted file
    rename(newFilename.c_str(), oldFilename.c_str()); 
}

void runSortDeptRecords(int m) {
    string oldFilename = "Dept.csv";
    string newFilename = "sortingDept.csv";
    ofstream output(newFilename);
    struct deptRecord outRec;
    int curRecNum = 0;
    vector<struct deptRecord> run = vector<struct deptRecord>();

    int deptLines = countLinesOfFile(oldFilename);

    while (curRecNum < deptLines) {
        // load run into buffer
        for (int i=0; i<m && curRecNum < deptLines; i++) { 
            // Retrieve and place record in buffer block
            run.push_back(getDeptRecord(curRecNum));
            curRecNum++;
        }

        // sort run
        sort(run.begin(), run.end(), compareManagerid);

        // write run to file
        for (int i=0; i<run.size(); i++) {
            outRec = run[i];
            //cout << to_string(outRec.managerid) + ", " + outRec.dname + "\n";
            addLineToEOF(newFilename, to_string(outRec.did) + "," + outRec.dname + "," + to_string(outRec.budget) + "," + to_string(outRec.managerid));
        }
        run.clear();
    }
    // Unsorted file is replaced with the newly sorted file
    rename(newFilename.c_str(), oldFilename.c_str()); 
}

void merge(int m) {
    ofstream output("creatingJoin.csv");

    struct empRecord* empOutRec;
    struct deptRecord* deptOutRec;
    //struct empRecord* empOutRec = new struct empRecord;
    //struct deptRecord* deptOutRec = new struct deptRecord;
    int curEmpRecNum = 0;
    int curDeptRecNum = 0;

    struct deptBufferBlock deptConstructor;
    struct empBufferBlock empConstructor;
    empConstructor.blockRecord = new struct empRecord;
    deptConstructor.blockRecord = new struct deptRecord;
    vector<struct empBufferBlock> empBuffs = vector<struct empBufferBlock>();
    vector<struct deptBufferBlock> deptBuffs = vector<struct deptBufferBlock>();

    int empLines = countLinesOfFile("Emp.csv");
    int deptLines = countLinesOfFile("Dept.csv");

    // Retrieve first tuples for each run
    
    // Dept records
    while (curDeptRecNum < deptLines) {
        // Get first block
        *(deptConstructor.blockRecord) = getDeptRecord(curDeptRecNum);
        deptConstructor.currentElementLineNumber = curDeptRecNum;
        if (curDeptRecNum + m > deptLines) {
            deptConstructor.elementCount = deptLines - curDeptRecNum; // TODO:Does this need to be -1 ?
        }
        else {
            deptConstructor.elementCount = m;
        }
        // add record to block
        deptBuffs.push_back(deptConstructor);

        // add m
        curDeptRecNum += m;
    }
    cout << "Dept records collected\n";

    // Emp records
    while (curEmpRecNum < empLines) {
        // Get first block
        *(empConstructor.blockRecord) = getEmpRecord(curEmpRecNum);
        empConstructor.currentElementLineNumber = curEmpRecNum;
        if (curEmpRecNum + m > empLines) {
            empConstructor.elementCount = empLines - curEmpRecNum; // TODO:Does this need to be -1 ?
        }
        else {
            empConstructor.elementCount = m;
        }
        // add record to block
        empBuffs.push_back(empConstructor);

        // add m
        curEmpRecNum += m;
    }
    cout << "Emp records collected\n";

    // Sort Merge Join
    while (empBuffs.size() > 0 && deptBuffs.size() > 0) {
        sort(empBuffs.begin(), empBuffs.end(), compareBuffEid);
        sort(deptBuffs.begin(), deptBuffs.end(), compareBuffManagerid);

        if (deptBuffs[0].blockRecord->managerid > empBuffs[0].blockRecord->eid) {
            // next tuple in emp
            if (empBuffs[0].elementCount != 1) {
                empBuffs[0].currentElementLineNumber += 1;
                empBuffs[0].elementCount -= 1;
                *empBuffs[0].blockRecord = getEmpRecord(empBuffs[0].currentElementLineNumber); 
            }
            else {
                empBuffs.erase(empBuffs.begin());
            }
        }
        else if (deptBuffs[0].blockRecord->managerid < empBuffs[0].blockRecord->eid) {
            // Next tuple in Dept
            if (deptBuffs[0].elementCount != 1) {
                deptBuffs[0].currentElementLineNumber += 1;
                deptBuffs[0].elementCount -= 1;
                *deptBuffs[0].blockRecord = getDeptRecord(deptBuffs[0].currentElementLineNumber); 
            }
            else {
                deptBuffs.erase(deptBuffs.begin());
            }
        }
        else {
            // Output all matching tuples
            while (deptBuffs[0].blockRecord->managerid == empBuffs[0].blockRecord->eid) {
                // Load tuples into output blocks
                deptOutRec = deptBuffs[0].blockRecord;
                empOutRec = empBuffs[0].blockRecord;

                // Output output
                addLineToEOF("creatingJoin.csv", 
                        to_string(deptOutRec->did) + "," + deptOutRec->dname + "," + to_string(deptOutRec->budget) + "," + to_string(deptOutRec->managerid) + "," +
                        to_string(empOutRec->eid) + "," + empOutRec->ename + "," + to_string(empOutRec->age) + "," + to_string(empOutRec->salary)
                        );

                // Next tuple in Dept
                if (deptBuffs[0].elementCount != 1) {
                    deptBuffs[0].currentElementLineNumber += 1;
                    deptBuffs[0].elementCount -= 1;
                    *deptBuffs[0].blockRecord = getDeptRecord(deptBuffs[0].currentElementLineNumber); 
                }
                else {
                    deptBuffs.erase(deptBuffs.begin());
                }

                // Next tuple in Emp
                if (empBuffs[0].elementCount != 1) {
                    empBuffs[0].currentElementLineNumber += 1;
                    empBuffs[0].elementCount -= 1;
                    *empBuffs[0].blockRecord = getEmpRecord(empBuffs[0].currentElementLineNumber); 
                }
                else {
                    empBuffs.erase(empBuffs.begin());
                }
            }
        }
    }
    rename("creatingJoin.csv", "join.csv"); 
}

struct empRecord createEmpRecord(string eid, string ename, string age, string salary) {
    struct empRecord returnRecord;
    returnRecord.eid = stoi(eid);
    returnRecord.ename = ename;
    returnRecord.age = stoi(age);
    returnRecord.salary = stof(salary);
    return returnRecord;
}

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

struct deptRecord createDeptRecord(string did, string dname, string budget, string managerid) {
    struct deptRecord returnRecord;
    returnRecord.did = stoi(did);
    returnRecord.dname = dname;
    returnRecord.budget = stof(budget);
    returnRecord.managerid = stoi(managerid);
    return returnRecord;
}

// Indexed at 0
struct deptRecord getDeptRecord(int index) {
    vector<string> csvAttributes;
    string attribute; // For tokenizing csv lines
    string line = getLineOfFile("Dept.csv", index + 1); // File lines start at 1
    stringstream linestream(line);
    while (getline(linestream, attribute, ','))
    {
        csvAttributes.push_back(attribute);
    }
    
    // Create and return records from values extracted from the line
    return createDeptRecord(csvAttributes[0], csvAttributes[1], csvAttributes[2], csvAttributes[3]);
}


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

    // Delete old file
    if (remove(filename.c_str())!=0) {
        cerr << "ERROR: Could not delete old file: " + filename << endl;
    }
    // Rename new index
    rename(newFilename.c_str(), filename.c_str()); 

}
