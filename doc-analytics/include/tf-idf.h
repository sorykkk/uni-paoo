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
    void addDocument(std::shared_ptr<DocumentStats> doc) {
        std::lock_guard<std::mutex> lock(mtx);
        documents.push_back(doc);
        
        // Build global vocabulary
        for (const auto& [term, freq] : doc->termFrequency) {
            vocabulary.insert(term);
        }
    }
    
    size_t getDocumentCount() const { return documents.size(); }
    
    const std::set<std::string>& getVocabulary() const { return vocabulary; }
    
    const std::vector<std::shared_ptr<DocumentStats>>& getDocuments() const {
        return documents;
    }
    
    int getDocumentFrequency(const std::string& term) const {
        int count = 0;
        for (const auto& doc : documents) {
            if (doc->termFrequency.find(term) != doc->termFrequency.end()) {
                count++;
            }
        }
        return count;
    }
};

// Processes a single document
class DocumentProcessor {
private:
    std::string filepath;
    std::shared_ptr<DocumentCollection> collection;
    
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
    DocumentProcessor(const std::string& path, 
                     std::shared_ptr<DocumentCollection> coll)
        : filepath(path), collection(coll) {}
    
    void process() {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            std::cerr << "Warning: Could not open " << filepath << "\n";
            return;
        }
        
        auto docStats = std::make_shared<DocumentStats>();
        docStats->docName = fs::path(filepath).filename().string();
        
        std::string line, word;
        while (std::getline(file, line)) {
            std::istringstream iss(line);
            while (iss >> word) {
                std::string cleaned = cleanWord(word);
                if (!cleaned.empty() && cleaned.length() > 2) { // Filter very short words
                    docStats->termFrequency[cleaned]++;
                    docStats->totalTerms++;
                }
            }
        }
        
        file.close();
        collection->addDocument(docStats);
    }
};

// TF-IDF Matrix generator
class TFIDFMatrix {
private:
    std::shared_ptr<DocumentCollection> collection;
    std::map<std::string, std::map<std::string, double>> matrix; // [term][document] = score
    
    double calculateTF(int termFreq, int totalTerms) {
        return static_cast<double>(termFreq) / totalTerms;
    }
    
    double calculateIDF(int docFreq, int totalDocs) {
        return std::log(static_cast<double>(totalDocs) / docFreq);
    }
    
public:
    TFIDFMatrix(std::shared_ptr<DocumentCollection> coll) 
        : collection(coll) {}
    
    void compute() {
        std::cout << "Computing TF-IDF matrix...\n";
        
        const auto& vocabulary = collection->getVocabulary();
        const auto& documents = collection->getDocuments();
        int totalDocs = documents.size();
        
        for (const auto& term : vocabulary) {
            int docFreq = collection->getDocumentFrequency(term);
            double idf = calculateIDF(docFreq, totalDocs);
            
            for (const auto& doc : documents) {
                auto it = doc->termFrequency.find(term);
                if (it != doc->termFrequency.end()) {
                    double tf = calculateTF(it->second, doc->totalTerms);
                    double tfidf = tf * idf;
                    matrix[term][doc->docName] = tfidf;
                }
            }
        }
        
        std::cout << "TF-IDF computation complete!\n";
    }
    
    void printTopTermsPerDocument(int topN = 10) {
        const auto& documents = collection->getDocuments();
        
        std::cout << "\n=== Top " << topN << " Terms per Document ===\n";
        
        for (const auto& doc : documents) {
            std::cout << "\n" << std::string(60, '=') << "\n";
            std::cout << "Document: " << doc->docName << "\n";
            std::cout << "Total terms: " << doc->totalTerms << "\n";
            std::cout << std::string(60, '-') << "\n";
            
            // Collect TF-IDF scores for this document
            std::vector<std::pair<std::string, double>> scores;
            for (const auto& [term, docScores] : matrix) {
                auto it = docScores.find(doc->docName);
                if (it != docScores.end()) {
                    scores.push_back({term, it->second});
                }
            }
            
            // Sort by TF-IDF score
            std::sort(scores.begin(), scores.end(),
                [](const auto& a, const auto& b) { return a.second > b.second; });
            
            // Print top N
            for (int i = 0; i < std::min(topN, (int)scores.size()); ++i) {
                std::cout << std::setw(3) << (i + 1) << ". "
                         << std::setw(20) << std::left << scores[i].first
                         << " : " << std::fixed << std::setprecision(4) 
                         << scores[i].second << "\n";
            }
        }
    }
    
    void printMatrix(int maxTerms = 20) {
        const auto& documents = collection->getDocuments();
        const auto& vocabulary = collection->getVocabulary();
        
        std::cout << "\n=== TF-IDF Matrix (showing top " << maxTerms << " terms) ===\n";
        
        // Get top terms by average TF-IDF
        std::vector<std::pair<std::string, double>> termAvgScores;
        for (const auto& term : vocabulary) {
            double avgScore = 0;
            int count = 0;
            for (const auto& [t, docScores] : matrix) {
                if (t == term) {
                    for (const auto& [doc, score] : docScores) {
                        avgScore += score;
                        count++;
                    }
                }
            }
            if (count > 0) {
                termAvgScores.push_back({term, avgScore / count});
            }
        }
        
        std::sort(termAvgScores.begin(), termAvgScores.end(),
            [](const auto& a, const auto& b) { return a.second > b.second; });
        
        // Print header
        std::cout << std::setw(15) << "Term";
        for (const auto& doc : documents) {
            std::cout << std::setw(12) << doc->docName.substr(0, 10);
        }
        std::cout << "\n" << std::string(15 + documents.size() * 12, '-') << "\n";
        
        // Print matrix rows
        for (int i = 0; i < std::min(maxTerms, (int)termAvgScores.size()); ++i) {
            const auto& term = termAvgScores[i].first;
            std::cout << std::setw(15) << term;
            
            for (const auto& doc : documents) {
                auto it = matrix[term].find(doc->docName);
                if (it != matrix[term].end()) {
                    std::cout << std::setw(12) << std::fixed 
                             << std::setprecision(4) << it->second;
                } else {
                    std::cout << std::setw(12) << "0.0000";
                }
            }
            std::cout << "\n";
        }
    }
    
    void exportToCSV(const std::string& filename) {
        std::ofstream file(filename);
        const auto& documents = collection->getDocuments();
        const auto& vocabulary = collection->getVocabulary();
        
        // Header
        file << "term";
        for (const auto& doc : documents) {
            file << "," << doc->docName;
        }
        file << "\n";
        
        // Data rows
        for (const auto& term : vocabulary) {
            file << term;
            for (const auto& doc : documents) {
                auto it = matrix[term].find(doc->docName);
                if (it != matrix[term].end()) {
                    file << "," << it->second;
                } else {
                    file << ",0";
                }
            }
            file << "\n";
        }
        
        file.close();
        std::cout << "\nTF-IDF matrix exported to: " << filename << "\n";
    }
};

#endif // TF_IDF_H_


