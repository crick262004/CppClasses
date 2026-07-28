#include <iostream>
#include <utility>
#include <atomic> // Added for thread-safe reference counting

template <typename T>
class SharedPointer {
private:
    T* ptr = nullptr;
    std::atomic<int>* refCount = nullptr; // Made atomic for thread-safety

    // Helper to release resource
    void release() {
        if (refCount) {
            // Atomic decrement and check if it reached 0
            if (--(*refCount) == 0) {
                delete ptr;
                delete refCount;
                std::cout << "Resource freed\n";
            }
        }
    }

    // Friend swap function to enable the Copy-and-Swap idiom
    friend void swap(SharedPointer& first, SharedPointer& second) noexcept {
        using std::swap;
        swap(first.ptr, second.ptr);
        swap(first.refCount, second.refCount);
    }

public:
    // Constructor: Only allocate refCount if we are actually managing a pointer
    explicit SharedPointer(T* p = nullptr) 
        : ptr(p), refCount(p ? new std::atomic<int>(1) : nullptr) {
        if (refCount) {
            std::cout << "Constructor called, count = " << *refCount << "\n";
        } else {
            std::cout << "Default Constructor called (nullptr)\n";
        }
    }

    // Destructor
    ~SharedPointer() {
        release();
    }

    // Copy constructor: Safely handles null/moved-from states
    SharedPointer(const SharedPointer& other) : ptr(other.ptr), refCount(other.refCount) {
        if (refCount) {
            ++(*refCount);
            std::cout << "Copy constructor called, count = " << *refCount << "\n";
        }
    }

    // Move constructor: Utilizing initializer list
    SharedPointer(SharedPointer&& other) noexcept : ptr(other.ptr), refCount(other.refCount) {
        other.ptr = nullptr;
        other.refCount = nullptr;
        std::cout << "Move constructor called\n";
    }

    // Unified Assignment Operator (Copy-and-Swap Idiom)
    // This single operator safely replaces both Copy and Move assignment!
    SharedPointer& operator=(SharedPointer other) noexcept {
        std::cout << "Assignment (via Copy-and-Swap) executed\n";
        swap(*this, other);
        return *this;
    }

    // Dereference operators (with added const overloads)
    T& operator*() { return *ptr; }
    const T& operator*() const { return *ptr; }

    T* operator->() { return ptr; }
    const T* operator->() const { return ptr; }

    // Get current reference count
    int use_count() const {
        return (refCount ? refCount->load() : 0);
    }
};

// Test class
class Demo {
public:
    Demo() { std::cout << "Demo created\n"; }
    ~Demo() { std::cout << "Demo destroyed\n"; }
    void hello() { std::cout << "Hello from Demo!\n"; }
};

int main() {
    std::cout << "=== Copy Example ===\n";
    SharedPointer<Demo> sp1(new Demo());
    SharedPointer<Demo> sp2 = sp1;  
    SharedPointer<Demo> sp3;
    sp3 = sp1;                      
    std::cout << "sp1 count = " << sp1.use_count() << "\n";

    std::cout << "\n=== Move Example ===\n";
    SharedPointer<Demo> sp4 = std::move(sp1);   
    SharedPointer<Demo> sp5(new Demo());
    sp5 = std::move(sp4);                       
    std::cout << "sp5 count = " << sp5.use_count() << "\n";

    std::cout << "\n=== End of main ===\n";
    return 0;
}
