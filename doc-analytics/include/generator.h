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
            "debugging", "version", "control", "agile", "deployment", "refactoring"},
            {"cloud", "computing", "virtual", "container", "docker", "kubernetes",
            "microservices", "scalability", "infrastructure", "deployment", "serverless", "aws"},
            {"algorithm", "complexity", "optimization", "sorting", "searching", "graph",
            "tree", "dynamic", "programming", "recursive", "hashing", "efficiency"},
            {"mobile", "application", "android", "ios", "native", "responsive",
            "touch", "interface", "user", "experience", "design", "platform"},
            {"blockchain", "cryptocurrency", "bitcoin", "ethereum", "decentralized",
            "smart", "contract", "mining", "ledger", "consensus", "token", "wallet"},
            {"artificial", "intelligence", "cognitive", "reasoning", "expert", "system",
            "knowledge", "inference", "pattern", "recognition", "vision", "processing"},
            {"network", "protocol", "tcp", "ip", "routing", "switching",
            "bandwidth", "latency", "packet", "transmission", "ethernet", "wireless"},
            {"compiler", "interpreter", "parsing", "lexical", "syntax", "semantic",
            "optimization", "code", "generation", "assembly", "bytecode", "runtime"},
            {"operating", "system", "kernel", "process", "thread", "scheduling",
            "memory", "management", "file", "system", "driver", "synchronization"},
            {"graphics", "rendering", "shader", "texture", "polygon", "lighting",
            "animation", "gpu", "opengl", "directx", "rasterization", "pipeline"},
            {"testing", "unit", "integration", "automation", "coverage", "assertion",
            "mock", "stub", "regression", "continuous", "quality", "validation"}
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
