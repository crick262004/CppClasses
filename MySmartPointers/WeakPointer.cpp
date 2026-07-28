#include <iostream>
#include <utility>

// Shared Control Block to track both strong (shared) and weak reference counts.
struct ControlBlock {
    int shared_count = 0;
    int weak_count = 0;
};

// Forward declaration
template <typename T>
class WeakPtr;

// Custom SharedPtr required to support WeakPtr functionality
template <typename T>
class SharedPtr {
private:
    T* ptr_ = nullptr;
    ControlBlock* cb_ = nullptr;

    // Private constructor used internally by WeakPtr::lock()
    SharedPtr(T* ptr, ControlBlock* cb) noexcept : ptr_(ptr), cb_(cb) {
        if (cb_) {
            cb_->shared_count++;
        }
    }

    friend class WeakPtr<T>;

public:
    constexpr SharedPtr() noexcept = default;

    explicit SharedPtr(T* ptr) {
        if (ptr) {
            ptr_ = ptr;
            cb_ = new ControlBlock{1, 0};
        }
    }

    ~SharedPtr() {
        release();
    }

    // Copy semantics
    SharedPtr(const SharedPtr& other) noexcept : ptr_(other.ptr_), cb_(other.cb_) {
        if (cb_) {
            cb_->shared_count++;
        }
    }

    SharedPtr& operator=(const SharedPtr& other) noexcept {
        if (this != &other) {
            release();
            ptr_ = other.ptr_;
            cb_ = other.cb_;
            if (cb_) {
                cb_->shared_count++;
            }
        }
        return *this;
    }

    // Move semantics
    SharedPtr(SharedPtr&& other) noexcept : ptr_(other.ptr_), cb_(other.cb_) {
        other.ptr_ = nullptr;
        other.cb_ = nullptr;
    }

    SharedPtr& operator=(SharedPtr&& other) noexcept {
        if (this != &other) {
            release();
            ptr_ = other.ptr_;
            cb_ = other.cb_;
            other.ptr_ = nullptr;
            other.cb_ = nullptr;
        }
        return *this;
    }

    T& operator*() const noexcept { return *ptr_; }
    T* operator->() const noexcept { return ptr_; }
    T* get() const noexcept { return ptr_; }
    
    int use_count() const noexcept {
        return cb_ ? cb_->shared_count : 0;
    }

    explicit operator bool() const noexcept {
        return ptr_ != nullptr;
    }

private:
    void release() noexcept {
        if (!cb_) return;

        cb_->shared_count--;
        if (cb_->shared_count == 0) {
            delete ptr_;  // Free the managed object
            ptr_ = nullptr;
            
            // Delete control block only if no weak references exist
            if (cb_->weak_count == 0) {
                delete cb_;
            }
        }
        cb_ = nullptr;
    }
};

// Custom WeakPtr Implementation
template <typename T>
class WeakPtr {
private:
    T* ptr_ = nullptr;
    ControlBlock* cb_ = nullptr;

public:
    constexpr WeakPtr() noexcept = default;

    // Construct WeakPtr from SharedPtr
    WeakPtr(const SharedPtr<T>& shared) noexcept : ptr_(shared.ptr_), cb_(shared.cb_) {
        if (cb_) {
            cb_->weak_count++;
        }
    }

    ~WeakPtr() {
        release();
    }

    // Copy semantics
    WeakPtr(const WeakPtr& other) noexcept : ptr_(other.ptr_), cb_(other.cb_) {
        if (cb_) {
            cb_->weak_count++;
        }
    }

    WeakPtr& operator=(const WeakPtr& other) noexcept {
        if (this != &other) {
            release();
            ptr_ = other.ptr_;
            cb_ = other.cb_;
            if (cb_) {
                cb_->weak_count++;
            }
        }
        return *this;
    }

    // Assign from SharedPtr
    WeakPtr& operator=(const SharedPtr<T>& shared) noexcept {
        release();
        ptr_ = shared.ptr_;
        cb_ = shared.cb_;
        if (cb_) {
            cb_->weak_count++;
        }
        return *this;
    }

    // Move semantics
    WeakPtr(WeakPtr&& other) noexcept : ptr_(other.ptr_), cb_(other.cb_) {
        other.ptr_ = nullptr;
        other.cb_ = nullptr;
    }

    WeakPtr& operator=(WeakPtr&& other) noexcept {
        if (this != &other) {
            release();
            ptr_ = other.ptr_;
            cb_ = other.cb_;
            other.ptr_ = nullptr;
            other.cb_ = nullptr;
        }
        return *this;
    }

    // Observers
    bool expired() const noexcept {
        return !cb_ || cb_->shared_count == 0;
    }

    int use_count() const noexcept {
        return cb_ ? cb_->shared_count : 0;
    }

    // Convert back to a SharedPtr safely
    SharedPtr<T> lock() const noexcept {
        if (expired()) {
            return SharedPtr<T>();
        }
        return SharedPtr<T>(ptr_, cb_);
    }

    void reset() noexcept {
        release();
    }

private:
    void release() noexcept {
        if (!cb_) return;

        cb_->weak_count--;
        // If no strong pointers OR weak pointers remain, clean up the control block
        if (cb_->weak_count == 0 && cb_->shared_count == 0) {
            delete cb_;
        }
        cb_ = nullptr;
        ptr_ = nullptr;
    }
};

// Test class with console logging
struct TestObj {
    int value;
    TestObj(int v) : value(v) { std::cout << "  [Constructed] TestObj(" << value << ")\n"; }
    ~TestObj() { std::cout << "  [Destroyed]   TestObj(" << value << ")\n"; }
    void doSomething() const { std::cout << "  [Method Call] TestObj value = " << value << "\n"; }
};

int main() {
    std::cout << "=== 1. Construction & Expiration ===\n";
    WeakPtr<TestObj> weak;
    {
        SharedPtr<TestObj> shared(new TestObj(100));
        weak = shared;

        std::cout << "shared.use_count(): " << shared.use_count() << "\n";
        std::cout << "weak.use_count(): " << weak.use_count() << "\n";
        std::cout << "weak.expired(): " << (weak.expired() ? "true" : "false") << "\n";
    } // shared goes out of scope here -> TestObj is destroyed!

    std::cout << "After scope end:\n";
    std::cout << "weak.expired(): " << (weak.expired() ? "true" : "false") << "\n";

    std::cout << "\n=== 2. Safe Access via lock() ===\n";
    {
        SharedPtr<TestObj> shared2(new TestObj(200));
        WeakPtr<TestObj> weak2 = shared2;

        std::cout << "Locking valid WeakPtr...\n";
        if (SharedPtr<TestObj> locked = weak2.lock()) {
            locked->doSomething();
            std::cout << "shared count inside lock scope: " << locked.use_count() << "\n";
        }

        std::cout << "Locking expired WeakPtr...\n";
        shared2 = nullptr; // Explicit reset / reassignment
        
        if (SharedPtr<TestObj> locked = weak2.lock()) {
            locked->doSomething();
        } else {
            std::cout << "Failed to lock: object has already been destroyed.\n";
        }
    }

    std::cout << "\n=== 3. Copy & Move Semantics for WeakPtr ===\n";
    {
        SharedPtr<TestObj> shared3(new TestObj(300));
        WeakPtr<TestObj> w1(shared3);
        
        // Copy construction
        WeakPtr<TestObj> w2(w1);
        
        // Move construction
        WeakPtr<TestObj> w3(std::move(w1));

        std::cout << "w1 expired after move? " << (w1.expired() ? "true" : "false") << "\n";
        std::cout << "w3 locks value: " << w3.lock()->value << "\n";
    }

    std::cout << "\n=== All Tests Executed Successfully ===\n";
    return 0;
}
