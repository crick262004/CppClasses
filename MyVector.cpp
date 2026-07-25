#include <iostream>
#include <utility>
#include <algorithm>

template <typename T>
class Vector {
private:
    T* arr = nullptr;
    size_t m_size = 0;
    size_t m_capacity = 0;

    // Internal helper to handle memory growth
    void resize(size_t new_cap) {
        // 1. Allocate the new larger array
        T* new_arr = new T[new_cap];
        
        // 2. Safely move elements from the old array to the new one
        for (size_t i = 0; i < m_size; ++i) {
            new_arr[i] = std::move(arr[i]);
        }
        
        // 3. Clean up the old array and update pointers
        delete[] arr;
        arr = new_arr;
        m_capacity = new_cap;
    }

public:
    // 1. Default Constructor
    Vector() : arr(nullptr), m_size(0), m_capacity(0) {}

    // 2. Sized Constructor
    Vector(size_t cap) : arr(cap > 0 ? new T[cap] : nullptr), m_size(0), m_capacity(cap) {}

    // 3. Destructor 
    ~Vector() {
        delete[] arr;
    }

    // Helper function for the Copy-and-Swap idiom
    void swap(Vector& other) noexcept {
        std::swap(arr, other.arr);
        std::swap(m_size, other.m_size);
        std::swap(m_capacity, other.m_capacity);
    }

    // 4. Copy Constructor (Deep Copy)
    Vector(const Vector& other) 
        : arr(other.m_capacity > 0 ? new T[other.m_capacity] : nullptr), 
          m_size(other.m_size), 
          m_capacity(other.m_capacity) {
        
        std::copy(other.arr, other.arr + other.m_size, arr);
    }

    // 5. Copy Assignment Operator (Exception-safe Copy-and-Swap)
    Vector& operator=(const Vector& other) {
        if (this != &other) {
            // Create a temporary local copy of 'other'
            Vector temp(other);
            // Swap our current state with the temp copy. 
            swap(temp);
        }
        return *this;
    }

    // 6. Move Constructor (Resource Pilfering)
    Vector(Vector&& other) noexcept : Vector() {
        // Swap our empty state with other's populated state
        swap(other);
    }

    // 7. Move Assignment Operator
    Vector& operator=(Vector&& other) noexcept {
        if (this != &other) {
            // First, free our own existing resources
            delete[] arr;

            // Second, steal other's pointers and data
            arr = other.arr;
            m_size = other.m_size;
            m_capacity = other.m_capacity;

            // Third, reset 'other' to a safe, default state so its destructor does nothing
            other.arr = nullptr;
            other.m_size = 0;
            other.m_capacity = 0;
        }
        return *this;
    }

    // --- Core API ---

    // Push Back for L-values (Copies the item)
    void push_back(const T& val) {
        if (m_size == m_capacity) {
            resize(m_capacity == 0 ? 1 : m_capacity * 2);
        }
        arr[m_size++] = val;
    }

    // Push Back for R-values (Moves the item efficiently)
    void push_back(T&& val) {
        if (m_size == m_capacity) {
            resize(m_capacity == 0 ? 1 : m_capacity * 2);
        }
        arr[m_size++] = std::move(val);
    }

    void pop_back() {
        if (m_size > 0) {
            // Explicitly reset the element to its default state if needed
            arr[m_size - 1] = T(); 
            m_size--;
        }
    }

    // Element Access (Non-const for modification, Const for read-only)
    T& operator[](size_t index) { return arr[index]; }
    const T& operator[](size_t index) const { return arr[index]; }

    size_t size() const { return m_size; }
    size_t capacity() const { return m_capacity; }
};
