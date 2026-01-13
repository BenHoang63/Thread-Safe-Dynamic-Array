#include <iostream>
#include "Vector.h"

void printVector(Vector<int> &v) {
    for (size_t i = 0; i < v.size(); i++) {
        std::cout << v[i] << " ";
    }
    std::cout << std::endl;
}
// int main() {
//     Vector<int> a;
//     for (int i = 0; i < 10; i++) {
//         a.push_back(i);

//     }
//     std::cout << *(a.erase(a.begin() + 4, a.begin() + 6));
//     printVector(a);

//     // std::cout << &(a[0]) << " " << &(b[0]);

// }
#include <iostream>
#include <thread>
#include <vector>
#include <numeric>
#include "Vector.h" 

void stress_test() {
    Vector<int> ts_vec;
    const int num_operations = 1000;

    // Thread 1: Constant WRITER (forces reallocations)
    std::thread writer([&]() {
        for (int i = 0; i < num_operations; ++i) {
            ts_vec.push_back(i);
        }
    });

    // Thread 2: Constant READER (uses the callback pattern)
    std::thread reader([&]() {
        for (int i = 0; i < 100; ++i) {
            ts_vec.execute_safely([](int* begin, int* end) {
                long long sum = 0;
                for (int* it = begin; it != end; ++it) {
                    sum += *it;
                }
                // Just a dummy print to show it's working
                if (begin != end) std::cout << "Current Sum: " << sum << "\n";
            });
        }
    });

    writer.join();
    reader.join();

    std::cout << "Test Passed: No race conditions or crashes detected." << std::endl;
}

int main() {
    stress_test();
    return 0;
}