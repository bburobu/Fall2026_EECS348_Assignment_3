//
//Name of program: 3228668_Warren_assignment3.c
//Discription: A c++ program that takes in a list of commands holding emails,
//adds them to a max heap by priority, and completes each list of commands.
//commands able to be used:
//EMAIL (category, subject, date)
//READ
//COUNT
//NEXT
//
//Inputs: .txt file where each line contains only one of any of the above mentioned commands.
// Output: all commands completed (printed statements that commands were done)
// collaborators: none
//other sources: Google Gemini's code, chatgpt's code for pros and cons
//Author: Christopher Warren
//Creation date: 10/1/2026
// Revision date: 10/1/2026
// Revisions:1. added comments, 2: moved command if statements to functon runcommands(), 3: email category removed, only uses priority enum, 4: fixed print statements


#include <iostream> //input/output
#include <fstream> //reading/writing files
#include <sstream> //insertion/extraction
#include <string> //strings 
#include <vector> //vectors


// Enumeration for sender priorities 
// (Higher value = Higher priority)
// Boss is highest, Other Person is lowest
enum SenderPriority {
    OTHER_PERSON = 1,
    IMPORTANT_PERSON = 2,
    PEER = 3,
    SUBORDINATE = 4,
    BOSS = 5
};

// Map string category to enum priority
SenderPriority getPriority(const std::string& category) {
    if (category == "Boss") return BOSS;
    if (category == "Subordinate") return SUBORDINATE;
    if (category == "Peer") return PEER;
    if (category == "ImportantPerson") return IMPORTANT_PERSON;
    return OTHER_PERSON;
}

//Email object Structure to represent an Email
// Contains category, subject, and date inclueded in commands
// adds global counter value for extra priority

struct Email {
    //std::string category;
    std::string subject;
    std::string date;
    SenderPriority priority;
    long long sequenceId; // Ensures "newest" breaks ties for the same sender type

    // Overloaded operators for heap comparisons
    bool operator<(const Email& other) const {
        //Returns if one has higher priority than the other
        if (this->priority != other.priority) {
            return this->priority < other.priority;
        }
        // If priorities are equal, the newer email (higher sequence ID) takes precedence
        return this->sequenceId < other.sequenceId;
    }

    bool operator>(const Email& other) const {
        if (this->priority != other.priority) {
            return this->priority > other.priority;
        }
        return this->sequenceId > other.sequenceId;
    }
};

std::string prioritytostr(SenderPriority sp){
    if (sp == BOSS) return "Boss";
    if (sp == SUBORDINATE) return "Subordinate";
    if (sp == PEER) return "Peer";
    if (sp == IMPORTANT_PERSON) return "ImportantPerson";
    return "OtherPerson";
}

// Max Heap implementation using a vector/list
class EmailMaxHeap {
private:
    std::vector<Email> heap; // heap is kept as a vector

    //heapify up to correct heap structure
    //uses for while loop instead of recursion like last time
    //inputs: index of email
    //void - no output
    void heapifyUp(int index) {
        while (index > 0) { // while not at root
            int parent = (index - 1) / 2; //get parent
            if (heap[index] > heap[parent]) { //if the child is higher priority than parent
                std::swap(heap[index], heap[parent]); //swap
                index = parent; //move index to where the child went
            } else {
                break; //end loop
            }
        }
    }
    //heapify down (while loop)
    //input: index of email
    //no output
    void heapifyDown(int index) {
        int size = heap.size(); //get size of heap
        while (2 * index + 1 < size) { // while there is at least once child
            int leftChild = 2 * index + 1; //left child
            int rightChild = 2 * index + 2; // right child
            int largest = index; // parent starts as largest

            if (heap[leftChild] > heap[largest]) {
                largest = leftChild; //we start with left child to check if laargest.
                //if it is larger than parent, we mark it the largest
            }
            if (rightChild < size && heap[rightChild] > heap[largest]) {
                largest = rightChild; //now the right child is checked if largest
                // if it is larger than the oarent, we mark it as the largest
            }

            if (largest != index) { // if either of the children were made to be larger
                std::swap(heap[index], heap[largest]); //swap
                index = largest; //move the index to the smaller value that was swapped and repeat
            } else {
                break; //list is sorted
            }
        }
    }

public:
    // insert function
    //inserts an email object to heap and calls heapify up to correct list
    // input: email object
    // no output, adds and fixes heap with new email
    void insert(const Email& email) {
        heap.push_back(email); //appends email to heap at the end
        heapifyUp(heap.size() - 1); //heapify up at the last index (where new email is)
    }

    bool isEmpty() const { // checks if heap is empty (bool)
        return heap.empty(); // if empty, true
    }

    Email getMax() const { //returns root email object (highest priority), no input
        if (!heap.empty()) {
            return heap[0];
        }
        throw std::runtime_error("Heap is empty"); // if heap is empty, throw error (BAD)
    }

    void extractMax() { // returns root email object and removes it
        if (heap.empty()) return;
        heap[0] = heap.back(); // takes the last email in list amd makes it to first
        heap.pop_back(); // removes duplicate last obj at the end
        if (!heap.empty()) {
            heapifyDown(0); // heapify down  once the last object was added to the first
        }
    }

    // returns heap size
    size_t size() const {
        return heap.size();
    }
};

// Helper function to trim whitespaces from parsed CSV strings
std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1)); //output is string without whitespaces
}

//process command handles if statements for the commands in the user file
//inputs: command pointer
// ss pointer (separate parts of line)
// email heap pointer
//global counter pointer
//no outputs but processes commands EMAIL, NEXT, READ, and COUNT
void processCommand(const std::string& command, 
                    std::stringstream& ss, 
                    EmailMaxHeap& emailHeap, 
                    long long& globalSequenceCounter){
        if (command == "EMAIL") { // email actions
            std::string remaining, category, subject, date; // declaring everything included in email line
            std::getline(ss, remaining); // Get the rest of the line after "EMAIL "
            
            std::stringstream csvStream(remaining); // we separate everything after email to categorize
            std::getline(csvStream, category, ','); // category here, will become priority later 
            std::getline(csvStream, subject, ','); // subject header here
            std::getline(csvStream, date, ','); // date here

            category = trim(category); //get priority for this email
            subject = trim(subject);
            date = trim(date);

            Email newEmail = { //we make a new email object for this information
                //category, 
                subject, 
                date, 
                getPriority(category),   //get priority for this email
                ++globalSequenceCounter  //global counter value for extra priority
            };
            emailHeap.insert(newEmail); //once complete, put email in heap

        } else if (command == "NEXT") { //next command displays what is in the root of the maxheap
            if (emailHeap.isEmpty()) { //does not show root if there is nothing in it
                std::cout << "No emails in the queue.\n\n";
            } else {
                Email top = emailHeap.getMax();
                std::cout << "Next Priority Email: \n\tSender: " << prioritytostr(top.priority) << "\n\tSubject: " 
                          << top.subject << "\n\tDate: " << top.date << "\n\n"; // displays info
            }

        } else if (command == "READ") { //the boss will read the next email
            if (emailHeap.isEmpty()) { // if no emails, do not read and remove
                std::cout << "No emails to read.\n\n";
            } else {
                Email top = emailHeap.getMax(); //get the top email
                emailHeap.extractMax(); //take it out of the list using extractmax function
            }

        } else if (command == "COUNT") { //tracks how many email is in the list
            std::cout << "There are " << emailHeap.size() << " Emails to read.\n\n";
        }

                    }

//main function
// main function takes in user inputted command list file
//checks if it exists,
// and runs through each command here. 
//once done, close file and end

//no input, outputs 0 to end main or 1 if an error
int main() { //main function
    std::string filename; //user file
    std::cout << "Enter the command text file name: ";
    std::cin >> filename; //takes in user input for file

    std::ifstream file(filename);
    if (!file.is_open()) { //checks if file can be opened
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return 1; //end early if cannot
    }

    EmailMaxHeap emailHeap; // creates email heap
    long long globalSequenceCounter = 0; //global counter 
    std::string line; //for the lines of the user files

    while (std::getline(file, line)) { // while there are sill lines in the file
        if (line.empty()) continue; // skip if nothing in that line

        std::stringstream ss(line); //get line as stream for individual parts (ss)
        std::string command; // gets command from first part
        ss >> command; // get command

        //call command function with all variables needed to perform commands
        processCommand(command, ss, emailHeap, globalSequenceCounter);
        
    } //end of while loop

    file.close(); //always close file when done
    return 0; //end main
} // end of main
