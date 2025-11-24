#include "table.h"
#include <fstream>
#include <thread>

namespace csvdb {

// Static mutex for console I/O protection
static std::mutex console_mutex;

void CSVTable::print(size_t max_rows) {
    std::lock_guard<std::mutex> lock(console_mutex);
    std::cout << "Headers: ";
    for(const auto& h: headers) {
        std::cout << h << " | ";
    }
    std::cout << "\n" << std::string(100, '-') << "\n";

    size_t rows_to_print = (max_rows == 0)? data->size() : std::min(max_rows, data->size());
    for(size_t i = 0; i < rows_to_print; i++) {
        for(const auto& cell : (*data)[i]) {
            std::cout << cell << " | ";
        }
        std::cout << "\n";
    }

    if(max_rows > 0 && data->size() > max_rows) {
        std::cout << "... (" << (data->size() - max_rows) << " more rows)\n";
    }
}

size_t CSVTable::getRowCount() const { return data->size(); }
size_t CSVTable::getColumnCount() const { return headers.size(); }

bool CSVTable::saveToFile(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::lock_guard<std::mutex> lock(console_mutex);
        std::cerr << "Error opening file for writing: " << filename << std::endl;
        return false;
    }

    // Write headers
    for (size_t i = 0; i < headers.size(); ++i) {
        file << headers[i];
        if (i < headers.size() - 1) file << ",";
    }
    file << "\n";

    // Write data rows
    for (const auto& row : *data) {
        for (size_t i = 0; i < row.size(); ++i) {
            file << row[i];
            if (i < row.size() - 1) file << ",";
        }
        file << "\n";
    }

    file.close();
    return true;
}

int CSVTable::getColumnIndex(const std::string& column) {
        auto it = std::find(headers.begin(), headers.end(), column);
        if (it == headers.end()) return -1;
        return std::distance(headers.begin(), it);
    }

Row CSVTable::parseCSVLine(const std::string& line) {
    Row row;
    std::stringstream ss(line);
    std::string cell;

    while (std::getline(ss, cell, ',')) {
        cell.erase(0, cell.find_first_not_of(" \t"));
        cell.erase(cell.find_last_not_of(" \t") + 1);
        row.push_back(cell);
    }

    return row;
}

bool CSVDatabase::loadFromFile(const std::string& filename) override {
    std::ifstream file(filename);
    if(!file.is_open()) {
        std::lock_guard<std::mutex> lock(console_mutex);
        std::cerr << "Error opening file: " << filename << std::endl;
        return false;
    }

    std::string line;
    bool first_line = true;

    while(std::getline(file, line)) {
        Row row = parseCSVLine(line);
        if(first_line) {
            headers = row;
            first_line = false;
        } else {
            data->push_back(row);
        }
    }
    return true;
}

bool FastCSVDatabase::loadFromFile(const std::string& filename) override {
    std::ifstream file(filename);
    if(!file.is_open()) {
        std::lock_guard<std::mutex> lock(console_mutex);
        std::cerr << "Error opening file: " << filename << std::endl;
        return false;
    }

    // first pass: read all lines
    std::vector<std::string> lines;
    std::string line;
    bool first_line = true;
    while(std::getline(file, line)) {
        if(first_line) {
            headers = parseCSVLine(line);
            first_line = false;
        } else {
            lines.push_back(line);
        }
    }
    // second pass: parse lines in parallel
    data->resize(lines.size());
    size_t lines_per_thread = (lines.size() + num_threads - 1) / num_threads; 
    std::vector<std::thread> threads;

    for (size_t t = 0; t < num_threads; ++t) {
            threads.emplace_back([this, &lines, t, lines_per_thread](size_t thread_id) {
                size_t start = thread_id * lines_per_thread;
                size_t end = std::min(start + lines_per_thread, lines.size());

                for (size_t i = start; i < end; ++i) {
                    (*data)[i] = parseCSVLine(lines[i]);
                }
            }, t);
        }

        for (auto& t : threads) {
            t.join();
        }

        return true;

}

std::shared_ptr<CSVTable> CSVDatabase::sort(const std::string& column, bool ascending) override {
    // Sequential sort implementation
    auto result = std::make_shared<CSVTable>();
    result->headers = headers;
    result->data = std::make_shared<Table>(*data);

    int col_idx = getColumnIndex(column);
    if (col_idx == -1) {
        std::lock_guard<std::mutex> lock(console_mutex);
        std::cerr << "Column not found for sort" << std::endl;
        return result;
    }

    std::sort(result->data->begin(), result->data->end(),
                  [col_idx, ascending](const Row& a, const Row& b) {
                      try {
                          double a_val = std::stod(a[col_idx]);
                          double b_val = std::stod(b[col_idx]);
                          return ascending ? a_val < b_val : a_val > b_val;
                      } catch (...) {
                          return ascending ? a[col_idx] < b[col_idx] : a[col_idx] > b[col_idx];
                      }
                  });

    return result;
}

template <typename Comp>
void FastCSVDatabase::mergesortParallel(std::shared_ptr<Table>& table, size_t left, size_t right, Comp comp, size_t depth = 0, size_t max_depth = 4) {
    if (left < right) {
        if (depth < max_depth) {
            size_t mid = left + (right - left) / 2;

            std::thread t1([this, &table, left, mid, &comp, depth, max_depth]() {
                mergesortParallel(table, left, mid, comp, depth + 1, max_depth);
            });

            std::thread t2([this, &table, mid, right, &comp, depth, max_depth]() {
                mergesortParallel(table, mid + 1, right, comp, depth + 1, max_depth);
            });

            t1.join();
            t2.join();

            merge(table, left, mid, right, comp);
        } else {
            std::sort(table->begin() + left, table->begin() + right + 1, comp);
        }
    }
}

template <typename Comp>
void FastCSVDatabase::merge(std::shared_ptr<Table>& table, size_t left, size_t mid, size_t right, Comp comp) {
    std::vector<Row> temp;
    size_t i = left, j = mid + 1;

    while (i <= mid && j <= right) {
        if (comp((*table)[i], (*table)[j])) {
            temp.push_back((*table)[i++]);
        } else {
            temp.push_back((*table)[j++]);
        }
    }

    while (i <= mid) temp.push_back((*table)[i++]);
    while (j <= right) temp.push_back((*table)[j++]);

    for (size_t i = 0; i < temp.size(); ++i) {
        (*table)[left + i] = temp[i];
    }
}

std::shared_ptr<CSVTable> FastCSVDatabase::sort(const std::string& column, bool ascending) override {
    auto result = std::make_shared<CSVTable>();
    result->headers = headers;
    result->data = std::make_shared<Table>(*data);

    int col_idx = getColumnIndex(column);
    if (col_idx == -1) {
        std::lock_guard<std::mutex> lock(console_mutex);
        std::cerr << "Column not found for sort" << std::endl;
        return result;
    }

    auto comparator = [col_idx, ascending](const Row& a, const Row& b) {
        try {
            double a_val = std::stod(a[col_idx]);
            double b_val = std::stod(b[col_idx]);
            return ascending ? a_val < b_val : a_val > b_val;
        } catch (...) {
            return ascending ? a[col_idx] < b[col_idx] : a[col_idx] > b[col_idx];
        }
    };

    mergesortParallel(result->data, 0, result->data->size() - 1, comparator, num_threads);

    return result;
}

std::shared_ptr<CSVTable> CSVDatabase::filter(const std::string& column, const std::string& value) override {
    auto result = std::make_shared<CSVTable>();
    result->headers = headers;

    int col_idx = getColumnIndex(column);
    if (col_idx == -1) {
        std::lock_guard<std::mutex> lock(console_mutex);
        std::cerr << "Column not found for filter" << std::endl;
        return result;
    }

    for (const auto& row : *data) {
        if (row[col_idx] == value) {
            result->data->push_back(row);
        }
    }

    return result;
}

std::shared_ptr<CSVTable> FastCSVDatabase::filter(const std::string& column, const std::string& value) override {
    auto result = std::make_shared<CSVTable>();
    result->headers = headers;

    int col_idx = getColumnIndex(column);
    if (col_idx == -1) {
        std::lock_guard<std::mutex> lock(console_mutex);
        std::cerr << "Column not found for filter" << std::endl;
        return result;
    }

    std::vector<std::vector<Row>> thread_results(num_threads);
    size_t rows_per_thread = (data->size() + num_threads - 1) / num_threads;
    std::vector<std::thread> threads;

    for (size_t t = 0; t < num_threads; ++t) {
        threads.emplace_back([this, &thread_results, col_idx, &value, t, rows_per_thread](size_t thread_id) {
            size_t start = thread_id * rows_per_thread;
            size_t end = std::min(start + rows_per_thread, data->size());

            for (size_t i = start; i < end; ++i) {
                if ((*data)[i][col_idx] == value) {
                    thread_results[thread_id].push_back((*data)[i]);
                }
            }
        }, t);
    }

    for (auto& t : threads) {
        t.join();
    }

    // Protect result data assembly
    {
        std::lock_guard<std::mutex> lock(data_mutex);
        for (const auto& thread_result : thread_results) {
            for (const auto& row : thread_result) {
                result->data->push_back(row);
            }
        }
    }

    return result;
}

std::shared_ptr<CSVTable> CSVDatabase::join(const std::shared_ptr<CSVTable>& other,
                                              const std::string& left_col,
                                              const std::string& right_col) override {
    auto result = std::make_shared<CSVTable>();
    result->headers = headers;
    result->headers.insert(result->headers.end(), other->headers.begin(), other->headers.end());

    int left_idx = getColumnIndex(left_col);
    int right_idx = other->getColumnIndex(right_col);

    if (left_idx == -1 || right_idx == -1) {
        std::lock_guard<std::mutex> lock(console_mutex);
        std::cerr << "Column not found for join" << std::endl;
        return result;
    }

    for (const auto& left_row : *data) {
        for (const auto& right_row : *(other->data)) {
            if (left_row[left_idx] == right_row[right_idx]) {
                Row joined_row = left_row;
                joined_row.insert(joined_row.end(), right_row.begin(), right_row.end());
                result->data->push_back(joined_row);
            }
        }
    }

    return result;
}

std::shared_ptr<CSVTable> FastCSVDatabase::join(const std::shared_ptr<CSVTable>& other,
                                                 const std::string& left_col,
                                                 const std::string& right_col) override {
    auto result = std::make_shared<CSVTable>();
    result->headers = headers;
    result->headers.insert(result->headers.end(), other->headers.begin(), other->headers.end());

    int left_idx = getColumnIndex(left_col);
    int right_idx = other->getColumnIndex(right_col);

    if (left_idx == -1 || right_idx == -1) {
        std::lock_guard<std::mutex> lock(console_mutex);
        std::cerr << "Column not found for join" << std::endl;
        return result;
    }

    std::vector<std::vector<Row>> thread_results(num_threads);
    size_t rows_per_thread = (data->size() + num_threads - 1) / num_threads;
    std::vector<std::thread> threads;

    for (size_t t = 0; t < num_threads; ++t) {
        threads.emplace_back([this, &other, &thread_results, left_idx, right_idx, t, rows_per_thread](size_t thread_id) {
            size_t start = thread_id * rows_per_thread;
            size_t end = std::min(start + rows_per_thread, data->size());

            for (size_t i = start; i < end; ++i) {
                for (const auto& right_row : *(other->data)) {
                    if ((*data)[i][left_idx] == right_row[right_idx]) {
                        Row joined_row = (*data)[i];
                        joined_row.insert(joined_row.end(), right_row.begin(), right_row.end());
                        thread_results[thread_id].push_back(joined_row);
                    }
                }
            }
        }, t);
    }

    for (auto& t : threads) {
        t.join();
    }

    // Protect result data assembly
    {
        std::lock_guard<std::mutex> lock(data_mutex);
        for (const auto& thread_result : thread_results) {
            for (const auto& row : thread_result) {
                result->data->push_back(row);
            }
        }
    }

    return result;
}


} // namespace csvdb