#include "tf-idf.h"

void DocumentCollection::addDocument(std::shared_ptr<DocumentStats> doc) {
    std::lock_guard<std::mutex> lock(mtx);
    documents.push_back(doc);
    
    // Build global vocabulary
    for (const auto& [term, freq] : doc->termFrequency) {
        vocabulary.insert(term);
    }
}

size_t DocumentCollection::getDocumentCount() const { 
    return documents.size(); 
}

const std::set<std::string>& DocumentCollection::getVocabulary() const { 
    return vocabulary; 
}

const std::vector<std::shared_ptr<DocumentStats>>& DocumentCollection::getDocuments() const {
    return documents;
}

int DocumentCollection::getDocumentFrequency(const std::string& term) const {
    int count = 0;
    for (const auto& doc : documents) {
        if (doc->termFrequency.find(term) != doc->termFrequency.end()) {
            count++;
        }
    }
    return count;
}

std::string DocumentProcessor::cleanWord(const std::string& word) {
    std::string cleaned;
    for (char c : word) {
        if (std::isalnum(c)) {
            cleaned += std::tolower(c);
        }
    }
    return cleaned;
}

DocumentProcessor::DocumentProcessor(const std::string& path, 
                    std::shared_ptr<DocumentCollection> coll)
    : filepath(path), collection(coll) {}

void DocumentProcessor::process() {
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


double TFIDFMatrix::calculateTF(int termFreq, int totalTerms) {
    return static_cast<double>(termFreq) / totalTerms;
}

double TFIDFMatrix::calculateIDF(int docFreq, int totalDocs) {
    return std::log(static_cast<double>(totalDocs) / docFreq);
}

TFIDFMatrix::TFIDFMatrix(std::shared_ptr<DocumentCollection> coll) 
    : collection(coll) {}

void TFIDFMatrix::compute() {
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

void TFIDFMatrix::printTopTermsPerDocument(int topN) {
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

void TFIDFMatrix::printMatrix(int maxTerms) {
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

void TFIDFMatrix::exportToCSV(const std::string& filename) {
    // Create directory if it doesn't exist
    fs::path filepath(filename);
    if (filepath.has_parent_path()) {
        fs::create_directories(filepath.parent_path());
    }
    
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not create file " << filename << "\n";
        return;
    }
    
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
