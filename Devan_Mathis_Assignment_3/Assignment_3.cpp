/* Program Name:   EECS 348 Assignment 3
 *
 * Brief Description:   C++ program that prioritizes emails for a busy company CEO.
 *                      Emails are stored in a MaxHeap
 *
 * Inputs:         Name of a test file, given on the command line:
 *                     ./Assignment_3 testfile.txt
 *                 or typed at a prompt if no command-line argument is given.
 *                 The program reads the file line by line. Each line is one
 *                 of these commands:
 *                 EMAIL <sender category>,<subject line>,<date MM-DD-YYYY>
 *                 NEXT
 *                 READ
 *                 COUNT
 *
 * Output:         Terminal output only:
 *                 - COUNT prints "There are N emails to read."
 *                 - NEXT prints the Sender, Subject and Date of the email
 *                   with the highest priority (without removing it)
 *
 * Collaborators:  [List every collaborator here, or write "None"]
 *
 * Other Sources:  1. Claude (Anthropic AI assistant): wrote a C++ MaxHeap
 *                    email prioritizer and combined it with the Gemini code
 *                    below, and added the comments in this file.
 *                 2. Gemini code supplied by the author (Date struct, Email
 *                    struct, MaxHeap with upHeap/downHeap, trim and
 *                    processCommand). Origin of that code:
 *                    Google Gemini
 *
 * Author:         Devan Mathis
 * Creation Date:  10-01-2026
 * Revision Date:  10-01-2026
 * Revisions:      Hybrid of two versions. Removed <iomanip> (unused).
 *                 Replaced the hard-coded sample input with file input using
 *                 <fstream>: the test file name comes from the command line
 *                 or from a prompt, so any test file can be used.
 *                 Subordinate, Peer and ImportantPerson share one priority
 *                 tier and are ordered by newest date within that tier.
 */

#include <iostream>  // std::cin, std::cout, std::cerr for the prompt, output and error messages
#include <fstream>   // std::ifstream for reading the test file
#include <string>    // std::string and std::getline for handling text
#include <vector>    // std::vector, the list that stores the heap
#include <sstream>   // std::stringstream for splitting and parsing strings

/*
 * Block: Date struct
 * Source: Gemini, used unchanged.
 * Stores the month, day and year of an email so dates can be compared as
 * numbers instead of as text.
 */
struct Date {
    int month;  // month number parsed from the MM part of the date
    int day;    // day number parsed from the DD part of the date
    int year;   // year number parsed from the YYYY part of the date

    // Returns true if this date is strictly newer than the other date
    bool isNewerThan(const Date& other) const {
        if (year != other.year) return year > other.year;      // different years: the later year is newer
        if (month != other.month) return month > other.month;  // same year, different months: the later month is newer
        return day > other.day;                                // same year and month: the later day is newer
    }
};

/*
 * Block: getCategoryPriority
 * Source: Combination. Structure from the Gemini code, with the shared
 * middle tier for Subordinate / Peer / ImportantPerson added by Claude.
 * Converts a sender category string into a number (bigger = read sooner).
 */
int getCategoryPriority(const std::string& category) {
    if (category == "Boss") return 3;         // Boss is read first
    if (category == "Subordinate") return 2;  // "read next" tier
    if (category == "Peer") return 2;         // "read next" tier (same as Subordinate)
    if (category == "ImportantPerson") return 2;  // "read next" tier (same as Subordinate)
    if (category == "OtherPerson") return 1;  // OtherPerson is read last
    return 0;                                 // unknown category: lowest priority
}

/*
 * Block: Email struct
 * Source: Combination. Fields and constructor from the Gemini code,
 * comparison operator combined with Claude's priority comparison.
 * Holds one email and knows how to compare its priority to another email.
 */
struct Email {
    std::string senderCategory;  // Boss, Subordinate, Peer, ImportantPerson or OtherPerson
    std::string subject;         // subject line of the email (may contain spaces)
    std::string dateStr;         // original date text, kept for printing (MM-DD-YYYY)
    Date parsedDate;             // date broken into numbers for comparing
    int priorityRank;            // number form of the sender category

    // Constructor: builds an email and works out its rank and parsed date
    Email(std::string cat, std::string subj, std::string dStr)
        : senderCategory(cat), subject(subj), dateStr(dStr) {  // copy the three text fields in
        priorityRank = getCategoryPriority(cat);  // look up the category's priority number

        std::stringstream ss(dStr);  // treat the date text as a stream we can read numbers from
        char dash1, dash2;           // places to throw away the two '-' characters
        ss >> parsedDate.month >> dash1 >> parsedDate.day >> dash2 >> parsedDate.year;  // read MM, '-', DD, '-', YYYY
    }

    // Returns true if this email should be read before 'other'
    bool operator>(const Email& other) const {
        if (priorityRank != other.priorityRank) {  // categories are in different tiers
            return priorityRank > other.priorityRank;  // higher tier wins
        }
        return parsedDate.isNewerThan(other.parsedDate);  // same tier: the newest email wins
    }
};

/*
 * Block: MaxHeap class
 * Source: Combination. Class layout and recursive downHeap from the pasted
 * code, insert / extractMax logic matches Claude's version.
 * A list-based binary max heap. The vector holds the heap with the root at
 * index 0, the children of index i at 2i+1 and 2i+2, and the parent of
 * index i at (i-1)/2. The highest priority email is always at index 0.
 */
class MaxHeap {
private:
    std::vector<Email> heap;  // the list that stores the heap

    // Moves the item at 'index' up until its parent has higher priority
    void upHeap(size_t index) {
        while (index > 0) {  // stop once we reach the root
            size_t parentIndex = (index - 1) / 2;  // find the parent's position
            if (heap[index] > heap[parentIndex]) {  // child has higher priority than parent
                std::swap(heap[index], heap[parentIndex]);  // swap child and parent
                index = parentIndex;  // continue checking from the parent's position
            } else {
                break;  // heap order is correct, stop moving up
            }
        }
    }

    // Moves the item at 'index' down until both children have lower priority
    void downHeap(size_t index) {
        size_t largest = index;        // assume the current item is the highest priority
        size_t left = 2 * index + 1;   // position of the left child
        size_t right = 2 * index + 2;  // position of the right child
        size_t size = heap.size();     // number of items currently in the heap

        if (left < size && heap[left] > heap[largest]) {  // left child exists and beats current largest
            largest = left;  // remember the left child as the largest
        }
        if (right < size && heap[right] > heap[largest]) {  // right child exists and beats current largest
            largest = right;  // remember the right child as the largest
        }

        if (largest != index) {  // a child has higher priority than this item
            std::swap(heap[index], heap[largest]);  // swap with that child
            downHeap(largest);  // keep moving down from the child's position
        }
    }

public:
    MaxHeap() {}  // empty constructor: the vector starts out empty

    // Returns how many emails are in the heap
    size_t size() const {
        return heap.size();
    }

    // Returns true if there are no emails in the heap
    bool isEmpty() const {
        return heap.empty();
    }

    // Adds an email, then moves it up to its correct place
    void insert(const Email& email) {
        heap.push_back(email);  // place the new email at the end of the list
        upHeap(heap.size() - 1);  // move it up until heap order is restored
    }

    // Returns a pointer to the highest priority email without removing it (nullptr if empty)
    const Email* peek() const {
        if (isEmpty()) return nullptr;  // nothing to look at
        return &heap[0];  // the root is the highest priority email
    }

    // Removes the highest priority email (does nothing if empty)
    void extractMax() {
        if (isEmpty()) return;  // nothing to remove

        if (heap.size() == 1) {  // only the root exists
            heap.pop_back();  // remove it and we are done
            return;
        }

        heap[0] = heap.back();  // move the last email into the root
        heap.pop_back();  // remove the now-duplicate last slot
        downHeap(0);  // move the new root down to its correct place
    }
};

/*
 * Block: trim
 * Source: Gemini code, used unchanged.
 * Removes spaces, tabs and line endings from both ends of a string.
 */
std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");  // position of the first non-whitespace character
    if (first == std::string::npos) return "";  // string is all whitespace
    size_t last = str.find_last_not_of(" \t\r\n");  // position of the last non-whitespace character
    return str.substr(first, (last - first + 1));  // keep only the part between them
}

/*
 * Block: processCommand
 * Source: Combination. Parsing and command layout from the Gemini code,
 * output wording adjusted by Claude to match the assignment's sample output.
 * Reads one line from the test file and carries out EMAIL, COUNT, NEXT or READ.
 */
void processCommand(const std::string& line, MaxHeap& heap) {
    std::string trimmedLine = trim(line);  // remove extra whitespace around the command
    if (trimmedLine.empty()) return;  // ignore blank lines

    if (trimmedLine.rfind("EMAIL ", 0) == 0) {  // line starts with "EMAIL "
        std::string payload = trimmedLine.substr(6);  // keep everything after "EMAIL "
        std::stringstream ss(payload);  // stream used to split on commas

        std::string sender, subject, dateStr;  // the three comma-separated fields
        if (std::getline(ss, sender, ',') &&    // read text up to the first comma
            std::getline(ss, subject, ',') &&   // read text up to the second comma
            std::getline(ss, dateStr)) {        // read the rest as the date

            heap.insert(Email(trim(sender), trim(subject), trim(dateStr)));  // build the email and add it to the heap
        }
    }
    else if (trimmedLine == "COUNT") {  // report how many unread emails remain
        std::cout << "There are " << heap.size() << " emails to read.\n\n";  // print the count and a blank line
    }
    else if (trimmedLine == "NEXT") {  // show the highest priority email
        const Email* email = heap.peek();  // look at the top email without removing it
        if (email != nullptr) {  // there is at least one email
            std::cout << "Next email:\n";  // heading line
            std::cout << "    Sender: " << email->senderCategory << "\n";  // sender category
            std::cout << "    Subject: " << email->subject << "\n";  // subject line
            std::cout << "    Date: " << email->dateStr << "\n\n";  // date followed by a blank line
        } else {
            std::cout << "No emails to display.\n\n";  // inbox is empty
        }
    }
    else if (trimmedLine == "READ") {  // CEO has read and dealt with the top email
        heap.extractMax();  // remove it (does nothing if the inbox is empty)
    }
}

/*
 * Block: main
 * Source: Claude. Replaces the hard-coded sample input of the Gemini code
 * with file input, so the grader's test file can be used. The file name is
 * taken from the command line (./ceo_email_hybrid testfile.txt) or, if none
 * is given, typed in at a prompt.
 */
int main(int argc, char* argv[]) {
    std::string filename;  // name of the test file to read
    if (argc > 1) {  // a file name was given on the command line
        filename = argv[1];  // use the first command-line argument
    } else {
        std::cout << "Test file name: ";  // ask the user for the file name
        std::getline(std::cin, filename);  // read the whole typed line as the file name
        filename = trim(filename);  // remove stray spaces or line endings around it
    }

    std::ifstream inFile(filename);  // open the test file for reading
    if (!inFile) {  // the file could not be opened
        std::cerr << "Could not open " << filename << "\n";  // tell the user which file failed
        return 1;  // exit with an error code
    }

    MaxHeap emailHeap;  // the CEO's inbox, stored as a max heap

    std::string line;  // holds one line of the file at a time
    while (std::getline(inFile, line)) {  // keep reading until the end of the file
        processCommand(line, emailHeap);  // carry out the command on this line
    }

    inFile.close();  // close the test file
    return 0;  // program finished successfully
}
