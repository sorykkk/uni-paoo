#ifndef TABLE_H
#define TABLE_H
#include <vector>
#include <string>
#include <memory>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <mutex>
#include <fstream>

// namespace for CSV as database
namespace csvdb {
using Row = std::vector<std::string>;
using Table = std::vector<Row>;

class CSVTable {
protected:
    std::shared_ptr<Table> data;
    std::vector<std::string> headers;
    
    CSVTable() : data(std::make_shared<Table>()) {}
    int getColumnIndex(const std::string& column);
    Row parseCSVLine(const std::string& line);

public:
    virtual bool loadFromFile(const std::string & filepath) = 0;
    virtual std::shared_ptr<CSVTable> sort(const std::string & column, bool ascending) = 0;
    virtual std::shared_ptr<CSVTable> filter(const std::string& column, const std::string& value) = 0;
    virtual std::shared_ptr<CSVTable> join(const std::shared_ptr<CSVTable>& other, const std::string& left_col, const std::string& right_col) = 0;
    virtual bool saveToFile(const std::string& filename) const = 0;
    
    void print(size_t max_rows = 0);
    size_t getRowCount() const;
    size_t getColumnCount() const;
};

class FastCSVDatabase : public CSVTable {
private:
    std::mutex data_mutex;
    size_t num_threads;
public:
    FastCSVDatabase(size_t threads = 4) 
        : CSVTable(), num_threads(threads) {}

    bool saveToFile(const std::string& filename) const override;
    bool loadFromFile(const std::string& filename) override;
    std::shared_ptr<CSVTable> sort(const std::string& column, bool ascending = true) override;
    std::shared_ptr<CSVTable> filter(const std::string& column, const std::string& value) override;
    std::shared_ptr<CSVTable> join(const std::shared_ptr<CSVTable>& other, const std::string& left_col, const std::string& right_col) override;

private:
    template <typename Comp>
    void mergesortParallel(std::shared_ptr<Table>& table, size_t left, size_t right, Comp comp, size_t depth = 0, size_t max_depth = 4);
    template <typename Comp>
    void merge(std::shared_ptr<Table>& table, size_t left, size_t mid, size_t right, Comp comp);
};

class CSVDatabase : public CSVTable {
public:
    CSVDatabase() : CSVTable() {}

    bool saveToFile(const std::string& filename) const override;
    bool loadFromFile(const std::string& filename) override;
    std::shared_ptr<CSVTable> sort(const std::string& column, bool ascending = true) override;
    std::shared_ptr<CSVTable> filter(const std::string& column, const std::string& value) override;
    std::shared_ptr<CSVTable> join(const std::shared_ptr<CSVTable>& other, const std::string& left_col, const std::string& right_col) override;
};

} // namespace csvdb
#endif // TABLE_H