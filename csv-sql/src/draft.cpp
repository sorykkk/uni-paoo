// For real performance boost run with this -O2 command
// g++ -std=c++17 -pthread -O2 -o csv_ops csv_ops.cpp
// ./csv_ops

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <memory>
#include <thread>
#include <mutex>
#include <algorithm>
#include <map>
#include <string>
#include <chrono>

using Row = std::vector<std::string>;
using Table = std::vector<Row>;

class CSVTable {
private:
    std::shared_ptr<Table> data;
    std::vector<std::string> headers;
    std::mutex data_mutex;

public:
    CSVTable() : data(std::make_shared<Table>()) {}

    // Sequential file loading
    bool loadFromFileSequential(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Error opening file: " << filename << std::endl;
            return false;
        }

        std::string line;
        bool first_line = true;

        while (std::getline(file, line)) {
            Row row = parseCSVLine(line);
            if (first_line) {
                headers = row;
                first_line = false;
            } else {
                data->push_back(row);
            }
        }
        return true;
    }

    // Multithreaded file loading
    bool loadFromFileMultithreaded(const std::string& filename, size_t num_threads = 4) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Error opening file: " << filename << std::endl;
            return false;
        }

        // First pass: read all lines
        std::vector<std::string> lines;
        std::string line;
        bool first_line = true;
        while (std::getline(file, line)) {
            if (first_line) {
                headers = parseCSVLine(line);
                first_line = false;
            } else {
                lines.push_back(line);
            }
        }

        // Second pass: parse lines in parallel
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

    // Sequential sort
    std::shared_ptr<CSVTable> sortSequential(const std::string& column, bool ascending = true) {
        auto result = std::make_shared<CSVTable>();
        result->headers = headers;
        result->data = std::make_shared<Table>(*data);

        int col_idx = getColumnIndex(column);
        if (col_idx == -1) {
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

    // Multithreaded sort using merge sort
    std::shared_ptr<CSVTable> sortMultithreaded(const std::string& column, bool ascending = true, size_t num_threads = 4) {
        auto result = std::make_shared<CSVTable>();
        result->headers = headers;
        result->data = std::make_shared<Table>(*data);

        int col_idx = getColumnIndex(column);
        if (col_idx == -1) {
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

    // Sequential filter
    std::shared_ptr<CSVTable> filterSequential(const std::string& column, const std::string& value) {
        auto result = std::make_shared<CSVTable>();
        result->headers = headers;

        int col_idx = getColumnIndex(column);
        if (col_idx == -1) {
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

    // Multithreaded filter
    std::shared_ptr<CSVTable> filterMultithreaded(const std::string& column, const std::string& value, size_t num_threads = 4) {
        auto result = std::make_shared<CSVTable>();
        result->headers = headers;

        int col_idx = getColumnIndex(column);
        if (col_idx == -1) {
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

        for (const auto& thread_result : thread_results) {
            for (const auto& row : thread_result) {
                result->data->push_back(row);
            }
        }

        return result;
    }

    // Sequential join
    std::shared_ptr<CSVTable> joinSequential(const std::shared_ptr<CSVTable>& other,
                                              const std::string& left_col,
                                              const std::string& right_col) {
        auto result = std::make_shared<CSVTable>();
        result->headers = headers;
        result->headers.insert(result->headers.end(), other->headers.begin(), other->headers.end());

        int left_idx = getColumnIndex(left_col);
        int right_idx = other->getColumnIndex(right_col);

        if (left_idx == -1 || right_idx == -1) {
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

    // Multithreaded join
    std::shared_ptr<CSVTable> joinMultithreaded(const std::shared_ptr<CSVTable>& other,
                                                 const std::string& left_col,
                                                 const std::string& right_col,
                                                 size_t num_threads = 4) {
        auto result = std::make_shared<CSVTable>();
        result->headers = headers;
        result->headers.insert(result->headers.end(), other->headers.begin(), other->headers.end());

        int left_idx = getColumnIndex(left_col);
        int right_idx = other->getColumnIndex(right_col);

        if (left_idx == -1 || right_idx == -1) {
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

        for (const auto& thread_result : thread_results) {
            for (const auto& row : thread_result) {
                result->data->push_back(row);
            }
        }

        return result;
    }

    void print(size_t max_rows = 0) {
        std::cout << "Headers: ";
        for (const auto& h : headers) {
            std::cout << h << " | ";
        }
        std::cout << "\n" << std::string(100, '-') << "\n";

        size_t rows_to_print = (max_rows == 0) ? data->size() : std::min(max_rows, data->size());
        for (size_t i = 0; i < rows_to_print; ++i) {
            for (const auto& cell : (*data)[i]) {
                std::cout << cell << " | ";
            }
            std::cout << "\n";
        }

        if (max_rows > 0 && data->size() > max_rows) {
            std::cout << "... (" << (data->size() - max_rows) << " more rows)\n";
        }
    }

    size_t getRowCount() const { return data->size(); }
    size_t getColumnCount() const { return headers.size(); }

private:
    int getColumnIndex(const std::string& column) {
        auto it = std::find(headers.begin(), headers.end(), column);
        if (it == headers.end()) return -1;
        return std::distance(headers.begin(), it);
    }

    Row parseCSVLine(const std::string& line) {
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

    template <typename Comp>
    void mergesortParallel(std::shared_ptr<Table>& table, size_t left, size_t right, Comp comp, size_t depth = 0, size_t max_depth = 4) {
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
    void merge(std::shared_ptr<Table>& table, size_t left, size_t mid, size_t right, Comp comp) {
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
};

int main() {
    // Create sample CSV file
    {
        std::ofstream file("employees.csv");
        file << "ID,Name,Department,Salary\n";
        for (int i = 1; i <= 1000; ++i) {
            file << i << ",Employee" << i << ",Dept" << (i % 10) << "," << (50000 + rand() % 50000) << "\n";
        }
    }

    {
        std::ofstream file("departments.csv");
        file << "DeptID,DeptName,Budget\n";
        file << "1,Engineering,500000\n";
        file << "5,Sales,300000\n";
        file << "9,HR,150000\n";
    }

    std::cout << "============================================\n";
    std::cout << "CSV SQL Operations - Sequential vs Multithreaded\n";
    std::cout << "============================================\n\n";

    // Test 1: File Loading
    std::cout << "=== TEST 1: FILE LOADING ===\n";

    auto emp_seq = std::make_unique<CSVTable>();
    auto start = std::chrono::high_resolution_clock::now();
    emp_seq->loadFromFileSequential("employees.csv");
    auto end = std::chrono::high_resolution_clock::now();
    auto seq_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    auto emp_mt = std::make_unique<CSVTable>();
    start = std::chrono::high_resolution_clock::now();
    emp_mt->loadFromFileMultithreaded("employees.csv", 4);
    end = std::chrono::high_resolution_clock::now();
    auto mt_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    std::cout << "Sequential: " << seq_time << " µs\n";
    std::cout << "Multithreaded (4 threads): " << mt_time << " µs\n";
    std::cout << "Speedup: " << (double)seq_time / mt_time << "x\n";
    std::cout << "Rows loaded: " << emp_seq->getRowCount() << "\n\n";

    // Test 2: Sorting
    std::cout << "=== TEST 2: SORTING (by Salary) ===\n";

    start = std::chrono::high_resolution_clock::now();
    auto sorted_seq = emp_seq->sortSequential("Salary", false);
    end = std::chrono::high_resolution_clock::now();
    seq_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    start = std::chrono::high_resolution_clock::now();
    auto sorted_mt = emp_mt->sortMultithreaded("Salary", false, 4);
    end = std::chrono::high_resolution_clock::now();
    mt_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    std::cout << "Sequential: " << seq_time << " µs\n";
    std::cout << "Multithreaded (4 threads): " << mt_time << " µs\n";
    std::cout << "Speedup: " << (double)seq_time / mt_time << "x\n";
    std::cout << "Top 5 by Salary:\n";
    sorted_seq->print(5);
    std::cout << "\n";

    // Test 3: Filtering
    std::cout << "=== TEST 3: FILTERING (Department = Dept5) ===\n";

    start = std::chrono::high_resolution_clock::now();
    auto filtered_seq = emp_seq->filterSequential("Department", "Dept5");
    end = std::chrono::high_resolution_clock::now();
    seq_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    start = std::chrono::high_resolution_clock::now();
    auto filtered_mt = emp_mt->filterMultithreaded("Department", "Dept5", 4);
    end = std::chrono::high_resolution_clock::now();
    mt_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    std::cout << "Sequential: " << seq_time << " µs\n";
    std::cout << "Multithreaded (4 threads): " << mt_time << " µs\n";
    std::cout << "Speedup: " << (double)seq_time / mt_time << "x\n";
    std::cout << "Results: " << filtered_seq->getRowCount() << " rows\n";
    std::cout << "First 5:\n";
    filtered_seq->print(5);
    std::cout << "\n";

    // Test 4: Join
    std::cout << "=== TEST 4: JOIN (Employees with Departments) ===\n";

    auto depts = std::make_unique<CSVTable>();
    depts->loadFromFileSequential("departments.csv");

    auto emp_for_join_seq = std::make_unique<CSVTable>();
    emp_for_join_seq->loadFromFileSequential("employees.csv");

    auto emp_for_join_mt = std::make_unique<CSVTable>();
    emp_for_join_mt->loadFromFileMultithreaded("employees.csv", 4);

    start = std::chrono::high_resolution_clock::now();
    auto joined_seq = emp_for_join_seq->joinSequential(depts, "Department", "DeptID");
    end = std::chrono::high_resolution_clock::now();
    seq_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    start = std::chrono::high_resolution_clock::now();
    auto joined_mt = emp_for_join_mt->joinMultithreaded(depts, "Department", "DeptID", 4);
    end = std::chrono::high_resolution_clock::now();
    mt_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    std::cout << "Sequential: " << seq_time << " µs\n";
    std::cout << "Multithreaded (4 threads): " << mt_time << " µs\n";
    std::cout << "Speedup: " << (double)seq_time / mt_time << "x\n";
    std::cout << "Results: " << joined_seq->getRowCount() << " rows\n";
    std::cout << "First 3:\n";
    joined_seq->print(3);

    std::cout << "\n============================================\n";
    std::cout << "Summary: Multithreading benefits scale with data size\n";
    std::cout << "============================================\n";

    return 0;
}