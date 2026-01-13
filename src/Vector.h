#ifndef VECTOR_H
#define VECTOR_H

#include <algorithm> // std::random_access_iterator_tag
#include <cstddef> // size_t
#include <stdexcept> // std::out_of_range
#include <type_traits> // std::is_same
#include <iostream> // for testing
#include <shared_mutex> // for thread locking
#include <functional>

template <class T>
class Vector {
public:
    class iterator;
private:
    T* array;
    size_t _capacity, _size;
    mutable std::shared_mutex mtx; // this is our traffic controller

    // doubles the vector capacity when we need more capacity
    void grow() {
        this->_capacity = this->_capacity == 0 ? 1 : this->_capacity * 2;
        T* newArray = new T[this->_capacity];
        for (size_t i = 0; i < this->_size; i++)
            newArray[i] = std::move(this->array[i]);
        delete[] this->array;
        this->array = newArray;
    }

public:


    // ================== CONSTRUCTORS ==================
    Vector() noexcept : array(nullptr), _capacity(0), _size(0) {} //constructor
    Vector(size_t count, const T& value) :  array(new T[count]()), _capacity(count), _size(count) { // initialization constructor
        for (size_t i = 0; i < count; i++) this->array[i] = value;
    }
    explicit Vector(size_t count) : array(new T[count]()), _capacity(count), _size(count) {} // default inserted constructor
    Vector(const Vector& other) : array(new T[other.capacity()]), _capacity(other.capacity()), _size(other.size()) { //copy constructor
        for (size_t i = 0; i < this->_size; i++) this->array[i] = other.array[i];
    }
    Vector(Vector&& other) noexcept : array(other.array), _capacity(other.capacity()), _size(other.size()) { //move constructor
        other.array = nullptr;
        other._size = 0;
        other._capacity = 0;
    }

    // dihstructor 🥀
    ~Vector() {
        this->clear();
    }



    // ================== PUBLIC ==================
    Vector& operator=(const Vector& other) { // copy assignment
        std::unique_lock<std::shared_mutex> lock(mtx);
        if (this != &other) {
            delete[] this->array;
            _size = other.size();
            _capacity = other.capacity();
            this->array = new T[this->_capacity]();
            for (size_t i = 0; i < this->_size; i++) this->array[i] = other[i];
        }
        return *this;
    }
    Vector& operator=(Vector&& other) noexcept { // move assignment
        std::unique_lock<std::shared_mutex> lock(mtx);
        if (this != &other) {
            delete[] this->array;
            _size = other.size();
            _capacity = other.capacity();
            this->array = other.array;
            
            other.array = nullptr;
            other._size = 0;
        }
        return *this;
    }

    iterator begin() noexcept {
        std::shared_lock<std::shared_mutex> lock(mtx);
        return iterator(this->array);
    }
    iterator end() noexcept {
        std::shared_lock<std::shared_mutex> lock(mtx);
        return iterator(this->array + this->_size);
    }

    [[nodiscard]] bool empty() const noexcept {
        std::shared_lock<std::shared_mutex> lock(mtx);
        return _size == 0;
    }
    size_t size() const noexcept {
        std::shared_lock<std::shared_mutex> lock(mtx);
        return this->_size;
    }
    size_t capacity() const noexcept {
        std::shared_lock<std::shared_mutex> lock(mtx);
        return this->_capacity;
    }

    T& at(size_t pos) {
        std::shared_lock<std::shared_mutex> lock(mtx);
        if (pos >= this->_size) throw std::out_of_range("Out of bounds");
        return this->array[pos];
    }
    const T& at(size_t pos) const {
        std::shared_lock<std::shared_mutex> lock(mtx);
        if (pos >= this->_size) throw std::out_of_range("Out of bounds");
        return this->array[pos];
    }
    T& operator[](size_t pos) {
        std::shared_lock<std::shared_mutex> lock(mtx);
        if (pos >= this->_size) throw std::out_of_range("Out of bounds");
        return this->array[pos];
    }
    const T& operator[](size_t pos) const {
        std::shared_lock<std::shared_mutex> lock(mtx);
        if (pos >= this->_size) throw std::out_of_range("Out of bounds");
        return this->array[pos];
    }
    T& front() {
        std::shared_lock<std::shared_mutex> lock(mtx);
        return this->array[0];
    }
    const T& front() const {
        std::shared_lock<std::shared_mutex> lock(mtx);
        return this->array[0];
    }
    T& back() {
        std::shared_lock<std::shared_mutex> lock(mtx);
        return this->array[this->_size - 1];
    }
    const T& back() const {
        std::shared_lock<std::shared_mutex> lock(mtx);
        return this->array[this->_size - 1];
    }

    void push_back(const T& value) {
        std::unique_lock<std::shared_mutex> lock(mtx);
        if (this->_size == this->_capacity) this->grow();
        this->array[this->_size] = value;
        this->_size++;
    }
    void push_back(T&& value) {
        std::unique_lock<std::shared_mutex> lock(mtx);
        if (this->_size == this->_capacity) this->grow();
        this->array[this->_size] = std::move(value);
        this->_size++;
    }
    void pop_back() {
        std::unique_lock<std::shared_mutex> lock(mtx);
        this->_size--;
    }

    iterator insert(iterator pos, const T& value) { // insert
        std::unique_lock<std::shared_mutex> lock(mtx);
        size_t index = pos - this->begin();
        if (this->_size == this->_capacity) this->grow();
        for (size_t i = this->_size - 1; i >= index; i--) {this->array[i + 1] = std::move(this->array[i]);}
        this->array[index] = value;
        _size++;
        return this->begin() + index;
    }
    iterator insert(iterator pos, T&& value) { // insert move
        std::unique_lock<std::shared_mutex> lock(mtx);
        size_t index = pos - this->begin();
        if (this->_size == this->_capacity) this->grow();
        for (size_t i = this->_size - 1; i >= index; i--) {this->array[i + 1] = std::move(this->array[i]);}
        this->array[index] = std::move(value);
        _size++;
        return this->begin() + index;
    }
    iterator insert(iterator pos, size_t count, const T& value) { // insert multiple
        std::unique_lock<std::shared_mutex> lock(mtx);
        size_t index = pos - this->begin();
        while (this->_size + count >= this->_capacity) {this->grow();}
        for (size_t i = this->_size - 1; i >= index; i--) {
            this->array[i + count] = std::move(this->array[i]);
            if (i == 0) break;
        }
        for (size_t i = 0; i < count; i++) {this->array[index + i] = value;}
        this->_size += count;
        return this->begin() + index;
    }
    iterator erase(iterator pos) {
        std::unique_lock<std::shared_mutex> lock(mtx);
        size_t index = pos - this->begin();
        for (size_t i = index; i < this->_size - 1; i++) {this->array[i] = std::move(this->array[i + 1]);}
        this->_size--;
        return this->begin() + index;
    }
    iterator erase(iterator first, iterator last) {
        std::unique_lock<std::shared_mutex> lock(mtx);
        size_t current = first - this->begin();
        size_t transfer = last - this->begin();
        size_t diff = transfer - current;
        size_t returnIndex = current;

        while (this->_size - 1 - transfer >= 0) { 
            this->array[current] = std::move(this->array[transfer]);
            if (this->_size - 1 - transfer == 0) break;
            current++; transfer++;
        }
        this->_size -= diff;
        return this->begin() + returnIndex;
    }
    void execute_safely(std::function<void(T*,T*)> func) {
        std::unique_lock<std::shared_mutex> lock(mtx);
        func(this->array, this->array + this->_size);
    }

    class iterator {
    public:
        using iterator_category = std::random_access_iterator_tag;
        using value_type        = T;
        using difference_type   = ptrdiff_t;
        using pointer           = T*;
        using reference         = T&;
    private:
        // Points to some element in the vector (or nullptr)
        T* _ptr;

    public:
        iterator() : _ptr(nullptr) {}
        explicit iterator(T* ptr) : _ptr(ptr) {}

        // This assignment operator is done for you, please do not add more
        iterator& operator=(const iterator&) noexcept = default;

        [[nodiscard]] reference operator*() const noexcept {return *(this->_ptr);}
        [[nodiscard]] pointer operator->() const noexcept {return this->_ptr;}

        // Prefix Increment: ++a
        iterator& operator++() noexcept {++this->_ptr; return *this;}
        // Postfix Increment: a++
        iterator operator++(int) noexcept {
            iterator copy = iterator(this->_ptr);
            this->_ptr++;
            return copy;
        }
        // Prefix Decrement: --a
        iterator& operator--() noexcept {--this->_ptr; return *this;}
        // Postfix Decrement: a--
        iterator operator--(int) noexcept {
            iterator copy = iterator(this->_ptr);
            this->_ptr--;
            return copy;
        }

        iterator& operator+=(difference_type offset) noexcept {this->_ptr += offset; return *this;}
        [[nodiscard]] iterator operator+(difference_type offset) const noexcept {
            iterator copy = iterator(this->_ptr);
            copy += offset;
            return copy;
        }
        
        iterator& operator-=(difference_type offset) noexcept {*this += (-offset); return *this;}
        [[nodiscard]] iterator operator-(difference_type offset) const noexcept {return *this + (-offset);}
        [[nodiscard]] difference_type operator-(const iterator& rhs) const noexcept {return this->_ptr - rhs._ptr;}

        [[nodiscard]] reference operator[](difference_type offset) const noexcept {return *(this->_ptr + offset);}

        [[nodiscard]] bool operator==(const iterator& rhs) const noexcept {return this->_ptr == rhs._ptr;}
        [[nodiscard]] bool operator!=(const iterator& rhs) const noexcept {return this->_ptr != rhs._ptr;}
        [[nodiscard]] bool operator<(const iterator& rhs) const noexcept {return this->_ptr < rhs._ptr;}
        [[nodiscard]] bool operator>(const iterator& rhs) const noexcept {return this->_ptr > rhs._ptr;}
        [[nodiscard]] bool operator<=(const iterator& rhs) const noexcept {return this->_ptr <= rhs._ptr;}
        [[nodiscard]] bool operator>=(const iterator& rhs) const noexcept {return this->_ptr >= rhs._ptr;}
    };


    void clear() noexcept {
        if (this->array != nullptr) {
            delete[] this->array;
            this->array = nullptr;
        }
        this->_size = 0;
    }
};

// This ensures at compile time that the deduced argument _Iterator is a Vector<T>::iterator
// There is no way we know of to back-substitute template <typename T> for external functions
// because it leads to a non-deduced context
namespace {
    template <typename _Iterator>
    using is_vector_iterator = std::is_same<typename Vector<typename _Iterator::value_type>::iterator, _Iterator>;
}

template <typename _Iterator, bool _enable = is_vector_iterator<_Iterator>::value>
[[nodiscard]] _Iterator operator+(typename _Iterator::difference_type offset, _Iterator const& iterator) noexcept {return iterator + offset;}

#endif
