/**
 * Smart Pointer & Thread Safety Demonstration
 * 
 * This demo shows UNSAFE versions of DocumentCollection
 * to demonstrate why mutex, unique_ptr, and shared_ptr are needed.
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <set>
#include <string>
#include <memory>
#include <thread>
#include <mutex>
#include <filesystem>
#include <algorithm>
#include "generator.h"
#include "tf-idf.h"

namespace fs = std::filesystem;

class UnsafeDocumentCollection {
private:
    std::vector<DocumentStats*> documents;  // Raw pointers - manual management
    std::set<std::string> vocabulary;
    
public:
    ~UnsafeDocumentCollection() {
        for (auto* doc : documents) {
            delete doc;
        }
    }
    
    void addDocument(DocumentStats* doc) {
        // Race condition
        documents.push_back(doc);
        
        // Multiple threads can modify vocabulary simultaneously - unsafe
        for (const auto& [term, freq] : doc->termFrequency) {
            vocabulary.insert(term);
        }
    }
    
    size_t getDocumentCount() const { return documents.size(); }
    size_t getVocabularySize() const { return vocabulary.size(); }
};

class UnsafeDocumentProcessor {
private:
    std::string filepath;
    UnsafeDocumentCollection* collection;  // Raw pointer - no ownership semantics
    
    std::string cleanWord(const std::string& word) {
        std::string cleaned;
        for (char c : word) {
            if (std::isalnum(c)) {
                cleaned += std::tolower(c);
            }
        }
        return cleaned;
    }
    
public:
    UnsafeDocumentProcessor(const std::string& path, UnsafeDocumentCollection* coll)
        : filepath(path), collection(coll) {}
    
    void process() {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            std::cerr << "Warning: Could not open " << filepath << "\n";
            return;
        }
        
        // Manual memory management - must remember to delete
        DocumentStats* docStats = new DocumentStats();
        docStats->docName = fs::path(filepath).filename().string();
        
        std::string line, word;
        while (std::getline(file, line)) {
            std::istringstream iss(line);
            while (iss >> word) {
                std::string cleaned = cleanWord(word);
                if (!cleaned.empty() && cleaned.length() > 2) {
                    docStats->termFrequency[cleaned]++;
                    docStats->totalTerms++;
                }
            }
        }
        
        file.close();
        collection->addDocument(docStats);
    }
};

void runUnsafeDemo(const std::vector<std::string>& filePaths) {
    std::cout << "\n" << std::string(70, '=') << "\n";
    std::cout << "UNSAFE VERSION (Raw pointers, NO mutex)\n";
    std::cout << std::string(70, '=') << "\n\n";
    
    UnsafeDocumentCollection collection;
    std::vector<UnsafeDocumentProcessor*> processors;
    
    // Manual memory management - must track all pointers
    for (const auto& path : filePaths) {
        processors.push_back(new UnsafeDocumentProcessor(path, &collection));
    }
    
    std::cout << "Processing " << filePaths.size() << " documents in parallel...\n";
    
    std::vector<std::thread> threads;
    for (auto* processor : processors) {
        threads.emplace_back([processor]() {
            processor->process();
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    std::cout << "\nResults:\n";
    std::cout << "  Expected documents: " << filePaths.size() << "\n";
    std::cout << "  Actual documents:   " << collection.getDocumentCount() << "\n";
    std::cout << "  Vocabulary size:    " << collection.getVocabularySize() << "\n";
    
    // usually at this point it will have segmentaiton fault because of vector allocating 
    // memory concurrently, but if not we can at least check if we lost documents
    if (collection.getDocumentCount() != filePaths.size()) {
        std::cout << "\nRACE CONDITION DETECTED!\n";
        std::cout << "   Lost " << (filePaths.size() - collection.getDocumentCount()) 
                  << " documents due to concurrent access!\n";
    } else {
        std::cout << "\nGot lucky this time, race conditions are non-deterministic.\n";
    }
    
    // Manual cleanup - easy to forget!
    for (auto* processor : processors) {
        delete processor;
    }
}

int main(int argc, char* argv[]) {
    std::string docDirectory = "sample_docs";
    
    if (argc > 1) {
        docDirectory = argv[1];
    }
    
    // Generate sample documents if needed
    if (!fs::exists(docDirectory)) {
        std::cout << "Generating sample documents...\n";
        DataGenerator::generateSampleDocuments(docDirectory);
    }
    
    // Collect all text files
    std::vector<std::string> filePaths;
    for (const auto& entry : fs::directory_iterator(docDirectory)) {
        if (entry.path().extension() == ".txt") {
            filePaths.push_back(entry.path().string());
        }
    }
    
    if (filePaths.empty()) {
        std::cerr << "Error: No .txt files found in " << docDirectory << "\n";
        return 1;
    }
    
    std::cout << "\nFound " << filePaths.size() << " documents to process\n";
    std::cout << "Running with " << std::thread::hardware_concurrency() << " hardware threads\n";
    
    // Run unsafe version multiple times to increase chance of seeing race condition
    std::cout << "\n\nRunning UNSAFE version 5 times to demonstrate race conditions...\n";
    for (int i = 0; i < 5; i++) {
        std::cout << "\n--- Run " << (i + 1) << " ---";
        runUnsafeDemo(filePaths);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    return 0;
}
