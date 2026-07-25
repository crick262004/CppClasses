#include <iostream>
#include <cstring>
#include <utility>

class MyString {
private:
    char* m_data;
    std::size_t m_length;

public:
    // 1. Default Constructor
    // Allocation of 1 byte keeps m_data consistently valid and printable.
    MyString() : m_data(new char[1]{'\0'}), m_length(0) {}

    // 2. Parametric Constructor
    // Handles null pointers gracefully instead of crashing via strlen.
    MyString(const char* str) {
        if (str == nullptr) {
            m_data = new char[1]{'\0'};
            m_length = 0;
        } else {
            m_length = std::strlen(str);
            m_data = new char[m_length + 1];
            std::strcpy(m_data, str);
        }
    }

    // 3. Copy Constructor
    // Deep copy requirement fulfilled.
    MyString(const MyString& other) : m_length(other.m_length) {
        m_data = new char[m_length + 1];
        std::strcpy(m_data, other.m_data);
    }

    // 4. Move Constructor
    // Must be marked 'noexcept' so STL containers can use it during reallocations.
    MyString(MyString&& other) noexcept : m_data(other.m_data), m_length(other.m_length) {
        other.m_data = nullptr;
        other.m_length = 0;
    }

    // 5. Destructor
    // delete[] handles nullptr safely if the object was moved from.
    ~MyString() {
        delete[] m_data;
    }

    // Friend swap function - essential for the Copy-and-Swap idiom
    friend void swap(MyString& first, MyString& second) noexcept {
        using std::swap;
        swap(first.m_data, second.m_data);
        swap(first.m_length, second.m_length);
    }

    // 6. Copy Assignment Operator (Using Copy-and-Swap)
    // Provides strong exception safety. If allocation fails inside the 
    // implicit copy, 'this' object remains completely unmodified.
    MyString& operator=(const MyString& other) {
        if (this != &other) {
            MyString temp(other);
            swap(*this, temp);
        }
        return *this;
    }

    // 7. Move Assignment Operator
    MyString& operator=(MyString&& other) noexcept {
        if (this != &other) {
            delete[] m_data; // Free existing resource
            
            m_data = other.m_data; // Steal resources
            m_length = other.m_length;
            
            other.m_data = nullptr; // Leave other in valid state
            other.m_length = 0;
        }
        return *this;
    }

    // Accessors
    // Both marked const and noexcept for performance and guarantees.
    std::size_t size() const noexcept { 
        return m_length; 
    }
    
    const char* c_str() const noexcept { 
        return m_data ? m_data : ""; 
    }

    // Operators 
    // Compound assignment member operator
    MyString& operator+=(const MyString& rhs) {
        std::size_t new_length = m_length + rhs.m_length;
        char* new_data = new char[new_length + 1];
        
        if (m_data) std::strcpy(new_data, m_data);
        if (rhs.m_data) std::strcpy(new_data + m_length, rhs.m_data);
        
        delete[] m_data;
        m_data = new_data;
        m_length = new_length;
        return *this;
    }

    // Non-member symmetric addition operator
    // Passes lhs by value to take advantage of copy/move optimizations automatically.
    friend MyString operator+(MyString lhs, const MyString& rhs) {
        lhs += rhs;
        return lhs;
    }

    // Stream Insertion
    friend std::ostream& operator<<(std::ostream& os, const MyString& str) {
        os << str.c_str();
        return os;
    }
};
