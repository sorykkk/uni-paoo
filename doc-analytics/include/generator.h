#ifndef DOC_GENERATOR_H_
#define DOC_GENERATOR_H_

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <filesystem>

namespace fs = std::filesystem;

class DataGenerator {
public:
    // Sample document generator
    static void generateSampleDocuments(const std::string& dirPath) {
        fs::create_directories(dirPath);
        
        std::vector<std::vector<std::string>> docTopics = {
            {"machine", "learning", "algorithm", "neural", "network", "training", 
            "model", "data", "prediction", "classification", "deep", "artificial"},
            {"database", "query", "sql", "table", "index", "optimization", 
            "transaction", "relational", "storage", "data", "schema", "normalization"},
            {"web", "server", "http", "client", "request", "response", 
            "api", "rest", "protocol", "network", "browser", "html"},
            {"security", "encryption", "authentication", "password", "cryptography", 
            "attack", "vulnerability", "protection", "firewall", "threat", "malware"},
            {"software", "development", "programming", "code", "testing", 
            "debugging", "version", "control", "agile", "deployment", "refactoring"}
        };
        
        std::cout << "Generating sample documents...\n";
        
        for (size_t i = 0; i < docTopics.size(); ++i) {
            std::string filename = dirPath + "/document_" + std::to_string(i + 1) + ".txt";
            std::ofstream file(filename);
            
            // Generate 100 lines per document
            for (int line = 0; line < 100; ++line) {
                for (int word = 0; word < 15; ++word) {
                    // 70% chance of topic word, 30% chance of common word
                    if (rand() % 100 < 70) {
                        file << docTopics[i][rand() % docTopics[i].size()] << " ";
                    } else {
                        std::vector<std::string> common = {"the", "and", "for", "with", "system"};
                        file << common[rand() % common.size()] << " ";
                    }
                }
                file << "\n";
            }
            file.close();
            std::cout << "  Created: " << filename << "\n";
        }
    }

};

#endif // DOC_GENERATOR_H_
