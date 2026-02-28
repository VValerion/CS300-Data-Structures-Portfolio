/*
 * ProjectTwo.cpp
 * ABCU Advising Assistance Program
 *
 * Author: Vonatarius Valerion
 *
 * Single-file C++ program (no CSVParser / no extra headers) that:
 *  - Prompts for a course data CSV filename
 *  - Loads courses into an unordered_map (hash table)
 *  - Prints an alphanumeric course list
 *  - Prints a single course with prerequisite numbers + titles
 */

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

struct Course {
    string number;                 // e.g., "CSCI200"
    string title;                  // e.g., "Data Structures"
    vector<string> prerequisites;  // e.g., {"CSCI100", "MATH201"}
};

static inline string trim(const string& s) {
    size_t start = 0;
    while (start < s.size() && isspace(static_cast<unsigned char>(s[start]))) {
        start++;
    }
    size_t end = s.size();
    while (end > start && isspace(static_cast<unsigned char>(s[end - 1]))) {
        end--;
    }
    return s.substr(start, end - start);
}

static inline string toUpper(string s) {
    for (char& ch : s) {
        ch = static_cast<char>(toupper(static_cast<unsigned char>(ch)));
    }
    return s;
}

// Split a line by commas (simple CSV for this project: no quoted commas).
static vector<string> splitCSVLine(const string& line) {
    vector<string> fields;
    string field;
    stringstream ss(line);

    while (getline(ss, field, ',')) {
        fields.push_back(trim(field));
    }

    // Handle trailing comma case (rare): "A,B," should produce empty last field.
    if (!line.empty() && line.back() == ',') {
        fields.push_back("");
    }

    return fields;
}

static bool loadCoursesFromFile(const string& filename, unordered_map<string, Course>& courseTable) {
    ifstream file(filename);
    if (!file.is_open()) {
        cout << "Error: Could not open file \"" << filename << "\".\n";
        return false;
    }

    unordered_map<string, Course> temp; // load into temp first; swap on success
    string line;
    size_t lineNumber = 0;

    while (getline(file, line)) {
        lineNumber++;
        line = trim(line);
        if (line.empty()) {
            continue;
        }

        vector<string> fields = splitCSVLine(line);
        if (fields.size() < 2) {
            cout << "Warning: Skipping invalid line " << lineNumber
                << " (expected at least course number and title).\n";
            continue;
        }

        string courseNum = toUpper(trim(fields[0]));
        string courseTitle = trim(fields[1]);

        if (courseNum.empty() || courseTitle.empty()) {
            cout << "Warning: Skipping line " << lineNumber
                << " (missing course number or title).\n";
            continue;
        }

        Course c;
        c.number = courseNum;
        c.title = courseTitle;

        // Remaining fields (if any) are prerequisites
        for (size_t i = 2; i < fields.size(); i++) {
            string pre = toUpper(trim(fields[i]));
            if (!pre.empty()) {
                c.prerequisites.push_back(pre);
            }
        }

        temp[c.number] = c; // overwrite duplicates by course number (last one wins)
    }

    courseTable.swap(temp);
    cout << "Loaded " << courseTable.size() << " courses.\n";
    return true;
}

static vector<string> getSortedCourseNumbers(const unordered_map<string, Course>& courseTable) {
    vector<string> numbers;
    numbers.reserve(courseTable.size());

    for (const auto& kv : courseTable) {
        numbers.push_back(kv.first);
    }

    sort(numbers.begin(), numbers.end());
    return numbers;
}

static void printCourseList(const unordered_map<string, Course>& courseTable) {
    vector<string> sorted = getSortedCourseNumbers(courseTable);

    cout << "\nHere is a sample schedule:\n";
    for (const string& num : sorted) {
        auto it = courseTable.find(num);
        if (it != courseTable.end()) {
            cout << it->second.number << ", " << it->second.title << "\n";
        }
    }
    cout << "\n";
}

static void printCourseInfo(const unordered_map<string, Course>& courseTable, const string& userCourseNumber) {
    string key = toUpper(trim(userCourseNumber));
    auto it = courseTable.find(key);

    if (it == courseTable.end()) {
        cout << "Error: Course \"" << key << "\" not found.\n\n";
        return;
    }

    const Course& c = it->second;
    cout << "\n" << c.number << ", " << c.title << "\n";

    if (c.prerequisites.empty()) {
        cout << "Prerequisites: None\n\n";
        return;
    }

    cout << "Prerequisites: ";

    for (size_t i = 0; i < c.prerequisites.size(); i++) {
        const string& preNum = c.prerequisites[i];

        // Try to print prereq title as well, if found
        auto preIt = courseTable.find(preNum);
        if (preIt != courseTable.end()) {
            cout << preIt->second.number << " (" << preIt->second.title << ")";
        }
        else {
            // If prerequisite isn’t in the loaded file, still show the number
            cout << preNum;
        }

        if (i + 1 < c.prerequisites.size()) {
            cout << ", ";
        }
    }

    cout << "\n\n";
}

static void printMenu() {
    cout << "1. Load Data Structure.\n";
    cout << "2. Print Course List.\n";
    cout << "3. Print Course.\n";
    cout << "9. Exit\n\n";
    cout << "What would you like to do? ";
}

int main() {
    unordered_map<string, Course> courseTable;
    bool isLoaded = false;

    cout << "Welcome to the course planner.\n\n";

    int choice = 0;

    while (true) {
        printMenu();

        if (!(cin >> choice)) {
            // Non-numeric input handling
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Error: Please enter a valid menu option number.\n\n";
            continue;
        }

        cin.ignore(numeric_limits<streamsize>::max(), '\n'); // consume rest of line

        if (choice == 1) {
            cout << "Enter the file name: ";
            string filename;
            getline(cin, filename);
            filename = trim(filename);

            if (filename.empty()) {
                cout << "Error: Filename cannot be empty.\n\n";
                continue;
            }

            isLoaded = loadCoursesFromFile(filename, courseTable);
            cout << "\n";
        }
        else if (choice == 2) {
            if (!isLoaded) {
                cout << "Error: Please load the data structure first (Option 1).\n\n";
                continue;
            }
            printCourseList(courseTable);
        }
        else if (choice == 3) {
            if (!isLoaded) {
                cout << "Error: Please load the data structure first (Option 1).\n\n";
                continue;
            }

            cout << "What course do you want to know about? ";
            string courseNum;
            getline(cin, courseNum);

            if (trim(courseNum).empty()) {
                cout << "Error: Course number cannot be empty.\n\n";
                continue;
            }

            printCourseInfo(courseTable, courseNum);
        }
        else if (choice == 9) {
            cout << "Thank you for using the course planner!\n";
            break;
        }
        else {
            cout << "Error: " << choice << " is not a valid option.\n\n";
        }
    }

    return 0;
}

