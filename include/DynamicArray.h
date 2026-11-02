#pragma once

#include <cstddef>
#include <utility>

// Growable array that manages its own heap memory with new[] / delete[].
// When full, it doubles its capacity and moves the elements across.
template <typename T>
class DynamicArray {
public:
    DynamicArray() : items(new T[kInitialCapacity]), capacity(kInitialCapacity) {}
    ~DynamicArray() { delete[] items; }

    // Owns raw memory, so copying is disabled to prevent a double delete.
    DynamicArray(const DynamicArray&) = delete;
    DynamicArray& operator=(const DynamicArray&) = delete;

    void push_back(T value) {
        if (count == capacity) grow();
        items[count++] = std::move(value);
    }

    void removeAt(std::size_t index) {
        for (std::size_t i = index; i + 1 < count; ++i) items[i] = std::move(items[i + 1]);
        --count;
    }

    void clear() { count = 0; }

    [[nodiscard]] std::size_t size() const { return count; }
    [[nodiscard]] bool empty() const { return count == 0; }

    T& operator[](std::size_t i) { return items[i]; }
    const T& operator[](std::size_t i) const { return items[i]; }

    // Allow range-based for loops.
    T* begin() { return items; }
    T* end() { return items + count; }
    const T* begin() const { return items; }
    const T* end() const { return items + count; }

private:
    static constexpr std::size_t kInitialCapacity = 4;

    void grow() {
        std::size_t newCapacity = capacity * 2;
        T* bigger = new T[newCapacity];
        for (std::size_t i = 0; i < count; ++i) bigger[i] = std::move(items[i]);
        delete[] items;
        items = bigger;
        capacity = newCapacity;
    }

    T* items;
    std::size_t count = 0;
    std::size_t capacity;
};
