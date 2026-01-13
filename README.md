# Thread-Safe C++ Template Library
A high-concurrency, memory-safe implementation of a dynamic array (Vector).

## Overview
This project is a custom implementation of a Vector container designed for multi-threaded environments. Standard C++ containers (like std::vector) are not thread-safe by default, which can lead to race conditions and memory corruption during concurrent operations. This library solves those issues using modern synchronization primitives and safe iteration patterns.

## Features
Multiple-Reader/Single-Writer: Optimized performance using std::shared_mutex, allowing multiple threads to read data simultaneously while ensuring exclusive access for modifications.

 • Template-Based: Supports any data type via C++ templates.

 • Safe Iteration API: Uses a functional callback pattern to prevent iterator invalidation—a common vulnerability where a thread accesses memory that was reallocated by another thread.

 • RAII Compliance: Leverages Resource Acquisition Is Initialization (RAII) for automatic lock management.

## Technical Implementation
The core synchronization logic uses the <shared_mutex> header:

 • Read Access: Managed by std::shared_lock, allowing high-throughput concurrent reads.

 • Write Access: Managed by std::unique_lock, blocking all other threads during reallocations or insertions.

## The Safe Iteration Pattern
To ensure the container is never modified while being iterated upon, the library provides an execute_safely method:

```C++
// Example of the safe iteration callback
vec.execute_safely([](auto* begin, auto* end) {
    for (auto* it = begin; it != end; ++it) {
        // Safe from concurrent push_back or reallocations
        process(*it); 
    }
});
```

## Testing & Validation
The project includes a stress-testing suite (test_vector.cpp) that simulates a high-load environment:

 • Writer Thread: Performs 1,000+ insertions to trigger frequent memory reallocations.

 • Reader Thread: Continuously calculates sums and iterates through the data.

Result: Zero data corruption or segmentation faults observed under simultaneous execution.