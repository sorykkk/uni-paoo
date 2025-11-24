#include <chrono>
#include "table.h"

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