#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using namespace std;

class HTnode {   // Huffman Tree Node
public:
    unsigned char c;     // Save the content of the character
    int weight;         // Record the weight of the character, i.e., frequency
    int p, lc, rc;      // Store the parent and left-right children
    int index;          // Save the index of the node for compression

    HTnode() {           // Initialize the node
        c = '\0';
        weight = 0;
        p = 0;
        lc = 0;
        rc = 0;
        index = 0;
    }
};

class Hcode {
public:
    unsigned char c;           // Save the current character
    vector<int> code;         // Save the Huffman code for the character
    Hcode() {
        c = '\0';
    }
};

class Com_Dec {
public:
    vector<HTnode> HT;       // Array to store the Huffman tree
    vector<Hcode> H_code;   // Array to store Huffman codes
    int H_types;            // Record the number of character types processed

    void compress();        // Compression function
    void decompress();      // Decompression function
    string getinfo();       // Read file information
    void createHT();        // Build the Huffman tree
    void createHcode();     // Build the Huffman code table
};

// For character variables, they should be stored as unsigned char types. 
// Both 'uc' and 'c' are 8 bits, but 'char' is signed and cannot represent up to 255.

int main() {

    Com_Dec *tar = new Com_Dec;
    int mode = 0;

    cout << "Option 1 - Compress a specific file" << endl
         << "Option 2 - Decompress a specific file" << endl
         << "Option 3 - Exit" << endl
         << "Please choose..." << endl;

    cin >> mode;

    while (1) {

        if (mode == 1) {
            tar->compress();
            cout << "Compression completed" << endl;
            mode = 0;
        }
        else if (mode == 2) {
            tar->decompress();
            cout << "Decompression completed" << endl;
            mode = 0;
        }
        else if (mode == 3) {
            break;
        }
        else {
            cout << "Please enter a valid option" << endl;
            cin >> mode;
        }
    }

    return 0;
}

void Com_Dec::compress() {

    string file = getinfo();
    if (file == "NULL") return;  // Compare 'file' with a string, add quotes around "NULL"

    createHT();
    createHcode();

    string compressedFile;
    cout << "Enter the path and name of the compressed result file: " << endl;
    cin >> compressedFile;
    ofstream out(compressedFile, ios::binary);

    if (!out) {
        cout << "Failed to open the (new) file" << endl;
        return;
    }

    out.write((char*)&H_types, sizeof(int));
    for (int i = 1; i <= H_types; i++) {
        out.write((char*)&(HT[i].c), sizeof(unsigned char));
        out.write((char*)&(HT[i].weight), sizeof(int));
    }

    ifstream in(file, ios::binary);  // Read the original file for the second time
    if (!in) {
        cout << "Failed to open the file" << endl;
        return;
    }

    unsigned char temp = '\0';  // An unsigned char as a reading unit
    string tmpstr;              // Store the current Huffman code
    while (1) {
        temp = in.get();  // Read a character from the file
        if (in.eof()) break;  // If EOF is reached, break out; otherwise, stop at EOF, and the last valid character will be recorded again
        for (int i = 1; i <= H_types; i++) {  // Extract the Huffman code for the current character
            if (H_code[i].c == temp) {
                for (auto elem : H_code[i].code) {
                    char x = elem - 0 + '0';
                    tmpstr.push_back(x);
                }
                break;
            }
        }
        while (tmpstr.length() >= 8) {  // When the current string is longer than 8 bits,
            temp = '\0';
            for (int i = 0; i < 8; i++) {
                temp = temp << 1;
                temp |= tmpstr[i] - '0';
            }
            auto p = tmpstr.begin();
            tmpstr.erase(p, p + 8);  // Remove the current 8 bits from the string
            out.write((char*)&temp, sizeof(unsigned char));  // Write the current 8 bits to the file
        }
    }
    if (tmpstr.length() > 0) {  // If there are remaining codes to be processed (shorter than the most frequent one)
        temp = '\0';
        for (auto i = 0; i < tmpstr.length(); i++) {
            temp = temp << 1;
            temp |= tmpstr[i] - '0';
        }
        for (auto i = 0; i < (8 - tmpstr.length()); i++) {
            temp = temp << 1;  // Pad the bits by shifting
        }
        out.write((char*)&temp, sizeof(unsigned char));
    }

    // Close streams, deallocate memory used by classes, and reset all data
    in.close();
    out.close();
    HT.resize(0);
    H_code.resize(0);
    H_types = 0;

    return;
}

void Com_Dec::createHT() {

    if (H_types == 1) {
        HT[1].lc = 1;
        HT[1].rc = 1;
    }

    int num = H_types + 1;
    int x = 0, y = 0;
    int min = 999999999, min2 = 0;

    while (num < 2 * H_types) {
        for (int i = 1; i < num; i++) {
            if (HT[i].p == 0 && HT[i].weight < min) {
                y = x;
                min2 = min;
                min = HT[i].weight;
                x = HT[i].index;
            }
            else if (HT[i].p == 0 && HT[i].weight < min2) {
                min2 = HT[i].weight;
                y = HT[i].index;
            }
            else;
        }

        // After creating a new node, restore the parameters to their original values
        HT[num].weight = HT[x].weight + HT[y].weight;
        HT[num].lc = x;
        HT[num].rc = y;
        HT[x].p = num;
        HT[y].p = num;
        num++;
        x = 0;
        y = 0;
        min = 999999999;
        min2 = 0;
    }
    HT[2 * H_types - 1].p = 0;

    return;
}

void Com_Dec::createHcode() {  // Generate Huffman codes

    Hcode element;
    H_code.push_back(element);  // Add the 0th element in advance
    if (H_types == 1) {
        H_code.push_back(element);
        H_code[1].c = HT[1].c;
        H_code[1].code.push_back(1);
    }

    for (int i = 1; i <= H_types; i++) {
        H_code.push_back(element);
        H_code[i].c = HT[i].c;
        int ptemp = HT[i].p;
        int indextemp = HT[i].index;
        auto pa = H_code[i].code.end();  // Add new content starting from the end of the string

        while (ptemp != 0) {
            if (HT[ptemp].lc == indextemp) {
                pa = H_code[i].code.insert(pa, 0);
            }
            else {
                pa = H_code[i].code.insert(pa, 1);
            }
            indextemp = HT[ptemp].index;
            ptemp = HT[ptemp].p;
        }
    }
    return;
}

string Com_Dec::getinfo() {  // Read file information

    cout << "Please enter the path of the file to be opened." << endl;

    string file;
    ifstream in;
    unsigned char cur = '\0';
    HTnode *buffer = new HTnode[256];

    cin >> file;
    in.open(file, ios::binary);
    if (!in) {
        cout << "Failed to open the file" << endl;
        return "NULL";  // Similarly, return a string
    }

    while (1) {
        cur = in.get();
        if (in.eof()) break;
        buffer[cur].c = cur;
        buffer[cur].weight++;
    }
    in.close();

    HTnode element;
    HT.push_back(element);
    H_types = 0;

    for (int i = 0; i < 256; i++) {
        if (buffer[i].weight != 0) {
            H_types++;
            HT.push_back(element);
            HT.back() = buffer[i];
        }
    }

    if (HT.size() == 1) {
        cout << "No valid data in the file." << endl;
        return "NULL";  // The valid return value must be the string "NULL" rather than NULL
    }

    delete[] buffer;  // Clean up the buffer

    for (int i = 1; i <= (H_types - 1); i++) {  // Add the obtained results to the tree
        HT.push_back(element);
    }

    for (int i = 0; i < 2 * H_types; i++) {  // Initialize indices
        HT[i].index = i;
    }

    return file;
}

void Com_Dec::decompress() {
    
    cout << "Please enter the path of the file to be decompressed:\n";
    string file;
    cin >> file;
    ifstream in;
    in.open(file, ios::binary);

    if (!in) {
        cout << "Failed to open the file" << endl;
        return;
    }

    in.read((char*)&H_types, sizeof(int));
    HTnode element;
    HT.push_back(element);

    for (int i = 1; i <= H_types; i++) {
        HT.push_back(element);
        in.read((char*)&(HT[i].c), sizeof(unsigned char));
        in.read((char*)&(HT[i].weight), sizeof(int));
    }
    for (int i = 1; i < H_types - 1; i++) {
        HT.push_back(element);
    }
    for (int i = 0; i < 2 * H_types; i++) {
        HT[i].index = i;
    }

    createHT();
    unsigned char temp = '\0';
    unsigned long long length = 0;
    for (int i = 1; i <= H_types; ++i)      // Calculate weight and get the original file length
        length = length + HT[i].weight;
    int top = HT[2 * H_types - 1].index;
    cout << "Please enter the path for the decompressed file:\n";
    string file2;
    cin >> file2;
    ofstream out(file2, ios::binary);
    if (!out) {
        cout << "Failed to open the file" << endl << endl;
        return;
    }
    while (length)                  // Continue looping until the entire file is read
    {
        temp = in.get();          // Read one character at a time, traverse the Huffman tree according to the code, output the character when reaching a leaf, and restart
        for (int i = 0; i < 8; i++) {
            if (temp & 128)                         // Choose to go left or right in the tree based on 0/1
                top = HT[top].rc;
            else
                top = HT[top].lc;
            
            if (top <= H_types) {
                out << HT[top].c;
                length--;
                if (length == 0)    break;          // Exit directly when the file is fully read
                top = HT[2 * H_types - 1].index;    // Reset top to the root of the Huffman tree
            }
            temp = temp << 1;
        }
    }
    
    // Clean up the allocated memory
    in.close();
    out.close();
    HT.resize(0);
    H_code.resize(0);
    H_types = 0;
    return;
}