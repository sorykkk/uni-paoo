#include <iostream>
#include "generator.h"
#include "tf-idf.h"

int main(int argc, char* argv[]) {
    std::string docDirectory = "sample_docs";
    
    if (argc > 1) {
        docDirectory = argv[1];
    }
    
    // Generate sample documents if directory doesn't exist
    if (!fs::exists(docDirectory)) {
        DataGenerator::generateSampleDocuments(docDirectory);
        std::cout << "\n";
    }
    
    // Collect all text files
    std::vector<std::string> filePaths;
    for (const auto& entry : fs::directory_iterator(docDirectory)) {
        std::cout << "Found: " << entry.path() << " (ext: " << entry.path().extension() << ")\n";
        if (entry.path().extension() == ".txt") {
            filePaths.push_back(entry.path().string());
        }
    }
    
    if (filePaths.empty()) {
        std::cerr << "Error: No .txt files found in " << docDirectory << "\n";
        return 1;
    }
    
    std::cout << "Found " << filePaths.size() << " documents\n";
    std::cout << "Processing with " << std::thread::hardware_concurrency() 
              << " threads...\n\n";
    
    // Create shared collection
    auto collection = std::make_shared<DocumentCollection>();
    
    // Process documents in parallel
    std::vector<std::thread> threads;
    std::vector<std::unique_ptr<DocumentProcessor>> processors;
    
    for (const auto& path : filePaths) {
        processors.push_back(std::make_unique<DocumentProcessor>(path, collection));
    }
    
    for (auto& processor : processors) {
        threads.emplace_back([&processor]() {
            processor->process();
        });
    }
    
    // Wait for all processing to complete
    for (auto& t : threads) {
        t.join();
    }
    
    std::cout << "Processed " << collection->getDocumentCount() << " documents\n";
    std::cout << "Vocabulary size: " << collection->getVocabulary().size() << " unique terms\n\n";
    
    // Compute TF-IDF matrix
    TFIDFMatrix tfidf(collection);
    tfidf.compute();
    
    // Display results
    tfidf.printTopTermsPerDocument(10);
    tfidf.printMatrix(15);
    tfidf.exportToCSV("output/tfidf_matrix.csv");
    
    return 0;
}