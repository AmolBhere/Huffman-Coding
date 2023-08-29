#include <iostream>
#include <fstream>
#include <queue>
#include <map>
#include <bitset>

struct HuffmanNode {
    char data;
    unsigned freq;
    HuffmanNode* left;
    HuffmanNode* right;
    
    HuffmanNode(char d, unsigned f) : data(d), freq(f), left(nullptr), right(nullptr) {}
};

struct CompareNodes {
    bool operator()(HuffmanNode* a, HuffmanNode* b) {
        return a->freq > b->freq;
    }
};

HuffmanNode* buildHuffmanTree(std::map<char, unsigned>& freqMap) {
    std::priority_queue<HuffmanNode*, std::vector<HuffmanNode*>, CompareNodes> pq;
    
    for (const auto& entry : freqMap) {
        pq.push(new HuffmanNode(entry.first, entry.second));
    }
    
    while (pq.size() > 1) {
        HuffmanNode* left = pq.top();
        pq.pop();
        
        HuffmanNode* right = pq.top();
        pq.pop();
        
        HuffmanNode* merged = new HuffmanNode('\0', left->freq + right->freq);
        merged->left = left;
        merged->right = right;
        
        pq.push(merged);
    }
    
    return pq.top();
}

void generateHuffmanCodes(HuffmanNode* root, std::string code, std::map<char, std::string>& codeMap) {
    if (!root)
        return;
    
    if (root->data != '\0') {
        codeMap[root->data] = code;
        return;
    }
    
    generateHuffmanCodes(root->left, code + "0", codeMap);
    generateHuffmanCodes(root->right, code + "1", codeMap);
}

void compressFile(const std::string& inputFilename, const std::string& outputFilename) {
    std::ifstream inputFile(inputFilename, std::ios::binary);
    std::ofstream outputFile(outputFilename, std::ios::binary);
    
    std::map<char, unsigned> freqMap;
    char ch;
    while (inputFile.get(ch)) {
        freqMap[ch]++;
    }
    
    HuffmanNode* huffmanTree = buildHuffmanTree(freqMap);
    std::map<char, std::string> huffmanCodes;
    generateHuffmanCodes(huffmanTree, "", huffmanCodes);
    
    // Write frequency map to output file (for decompression)
    for (const auto& entry : freqMap) {
        outputFile << entry.first << entry.second;
    }
    
    // Write compressed data to output file
    inputFile.clear();
    inputFile.seekg(0);
    std::string compressedData = "";
    while (inputFile.get(ch)) {
        compressedData += huffmanCodes[ch];
    }
    
    // Add padding and write compressed data
    std::bitset<8> padding(compressedData.length() % 8);
    outputFile << padding;
    for (size_t i = 0; i < compressedData.length(); i += 8) {
        std::bitset<8> byte(compressedData.substr(i, 8));
        outputFile << static_cast<char>(byte.to_ulong());
    }
    
    inputFile.close();
    outputFile.close();
}

void decompressFile(const std::string& inputFilename, const std::string& outputFilename) {
    std::ifstream inputFile(inputFilename, std::ios::binary);
    std::ofstream outputFile(outputFilename, std::ios::binary);
    
    std::map<char, unsigned> freqMap;
    char ch;
    while (inputFile.get(ch)) {
        freqMap[ch] = 0;
        inputFile.get(ch);
        freqMap[ch] = static_cast<unsigned>(ch);
    }
    
    HuffmanNode* huffmanTree = buildHuffmanTree(freqMap);
    
    char paddingChar;
    inputFile.get(paddingChar);
    std::bitset<8> padding(paddingChar);
    
    HuffmanNode* currentNode = huffmanTree;
    std::string decompressedData; // Initialize the variable to hold the decompressed data
    while (inputFile.get(ch)) {
        std::bitset<8> byte(static_cast<unsigned>(ch));
        for (size_t i = 0; i < 8; ++i) {
            if (currentNode->left == nullptr && currentNode->right == nullptr) {
                decompressedData += currentNode->data; // Append the decompressed character
                currentNode = huffmanTree;
            }
            if (byte[i] == 0) {
                currentNode = currentNode->left;
            } else {
                currentNode = currentNode->right;
            }
        }
    }
    
    // Handle remaining bits due to padding
     size_t remainingBits = decompressedData.length() % 8; // Corrected to decompressedData
    if (remainingBits > 0) {
        for (size_t i = 0; i < remainingBits; ++i) {
            if (currentNode->left == nullptr && currentNode->right == nullptr) {
                decompressedData += currentNode->data; // Append the decompressed character
                currentNode = huffmanTree;
            }
            if (padding[i] == 0) {
                currentNode = currentNode->left;
            } else {
                currentNode = currentNode->right;
            }
        }
    }
    
    // Write the entire decompressed data to the output file
    outputFile << decompressedData;
    
    inputFile.close();
    outputFile.close();
}


int main() {
    const std::string inputFilename = "input.txt";
    const std::string compressedFilename = "compressed.bin";
    const std::string decompressedFilename = "decompressed.txt";
    
    // Compress the input file
    compressFile(inputFilename, compressedFilename);
    
    // Decompress the compressed file
    decompressFile(compressedFilename, decompressedFilename);
    
    return 0;
}