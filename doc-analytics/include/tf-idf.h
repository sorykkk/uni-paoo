#ifndef TF_IDF_H_
#define TF_IDF_H_

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <memory>
#include <thread>
#include <mutex>
#include <sstream>
#include <algorithm>
#include <iomanip>
#include <cmath>
#include <filesystem>

namespace fs = std::filesystem;

// Represents a single document's term frequencies
struct DocumentStats {
    std::string docName;
    std::map<std::string, int> termFrequency;
    int totalTerms = 0;
};

// Thread-safe document collection manager
class DocumentCollection {
private:
    std::mutex mtx;
    std::vector<std::shared_ptr<DocumentStats>> documents;
    std::set<std::string> vocabulary;
    
public:
    void addDocument(std::shared_ptr<DocumentStats> doc);
    size_t getDocumentCount() const;
    const std::set<std::string>& getVocabulary() const;
    const std::vector<std::shared_ptr<DocumentStats>>& getDocuments() const;
    int getDocumentFrequency(const std::string& term) const;
};

// Processes a single document
class DocumentProcessor {
private:
    std::string filepath;
    std::shared_ptr<DocumentCollection> collection;
    
    std::string cleanWord(const std::string& word);
    
public:
    DocumentProcessor(const std::string& path, 
                     std::shared_ptr<DocumentCollection> coll);
    
    void process();
};

// TF-IDF Matrix generator
class TFIDFMatrix {
private:
    std::shared_ptr<DocumentCollection> collection;
    std::map<std::string, std::map<std::string, double>> matrix; // [term][document] = score
    
    double calculateTF(int termFreq, int totalTerms);
    double calculateIDF(int docFreq, int totalDocs);
    
public:
    TFIDFMatrix(std::shared_ptr<DocumentCollection> coll);
    
    void compute();
    void printTopTermsPerDocument(int topN = 10);
    void printMatrix(int maxTerms = 20);
    void exportToCSV(const std::string& filename);
};

#endif // TF_IDF_H_


