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