#include <iostream>
#include <utility> // For std::move and std::swap

template <typename T>
class UniquePtr {
private:
    T* ptr_;

public:
    explicit UniquePtr(T* ptr = nullptr) noexcept : ptr_(ptr) {}

    ~UniquePtr() {
        delete ptr_;
    }

    // Unique ownership implies copying is impossible.
    UniquePtr(const UniquePtr&) = delete;
    UniquePtr& operator=(const UniquePtr&) = delete;

    UniquePtr(UniquePtr&& other) noexcept : ptr_(other.ptr_) {
        other.ptr_ = nullptr;
    }

    UniquePtr& operator=(UniquePtr&& other) noexcept {
        if (this != &other) {
            delete ptr_;          // Free existing managed object
            ptr_ = other.ptr_;    // Steal raw pointer from source
            other.ptr_ = nullptr; // Leave source in a valid, null state
        }
        return *this;
    }

    T& operator*() const {
        return *ptr_;
    }

    T* operator->() const {
        return ptr_;
    }

    T* get() const noexcept {
        return ptr_;
    }

    T* release() noexcept {
        T* temp = ptr_;
        ptr_ = nullptr;
        return temp;
    }

    void reset(T* ptr = nullptr) noexcept {
        T* old_ptr = ptr_;
        ptr_ = ptr;
        delete old_ptr; // Safe against self-reset (e.g., ptr1.reset(ptr1.get()))
    }

    explicit operator bool() const noexcept {
        return ptr_ != nullptr;
    }
};

struct TestObj {
    int value;
    TestObj(int v) : value(v) { std::cout << "  [Constructed] TestObj(" << value << ")\n"; }
    ~TestObj() { std::cout << "  [Destroyed]   TestObj(" << value << ")\n"; }
    void doSomething() const { std::cout << "  [Method Call] TestObj value = " << value << "\n"; }
};

int main() {
    std::cout << "=== 1. Construction & Destruction ===\n";
    {
        UniquePtr<TestObj> ptr1; // Default
        UniquePtr<TestObj> ptr2(new TestObj(10)); // Parameterized
    } // ptr2 destroyed here automatically

    std::cout << "\n=== 2. Operator Overloads (* and ->) ===\n";
    {
        UniquePtr<TestObj> ptr3(new TestObj(20));
        std::cout << "Dereferenced (*ptr3).value: " << (*ptr3).value << "\n";
        ptr3->doSomething();
    }

    std::cout << "\n=== 3. Move Semantics ===\n";
    {
        UniquePtr<TestObj> ptr4(new TestObj(30));
        
        std::cout << "Moving ptr4 into ptr5 via Move Construct...\n";
        UniquePtr<TestObj> ptr5(std::move(ptr4)); 
        std::cout << "ptr4 is " << (ptr4 ? "valid" : "null") << "\n";
        ptr5->doSomething();

        std::cout << "Moving ptr5 into ptr6 via Move Assignment...\n";
        UniquePtr<TestObj> ptr6;
        ptr6 = std::move(ptr5);
        std::cout << "ptr5 is " << (ptr5 ? "valid" : "null") << "\n";
        ptr6->doSomething();
    }

    std::cout << "\n=== 4. Utility Functions (get, release, reset) ===\n";
    {
        UniquePtr<TestObj> ptr7(new TestObj(40));
        
        // get()
        std::cout << "Raw pointer address from get(): " << ptr7.get() << "\n";

        // release()
        TestObj* raw = ptr7.release();
        std::cout << "ptr7 is " << (ptr7 ? "valid" : "null") << " after release()\n";
        delete raw; // Manual deletion required for released pointers

        // reset()
        UniquePtr<TestObj> ptr8(new TestObj(50));
        std::cout << "Resetting ptr8 with new TestObj(60):\n";
        ptr8.reset(new TestObj(60)); // Destroys 50, takes 60
        
        std::cout << "Resetting ptr8 to null:\n";
        ptr8.reset(); // Destroys 60
    }

    std::cout << "\n=== All Tests Executed Successfully ===\n";
    return 0;
}
