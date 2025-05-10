#pragma once

#include <unistd.h>

#include <stdexcept>

#include "vec2i.hpp"

template <typename T>
class Array2D {
   private:
    T **container;
    vec2i _size;

    static T **allocate(size_t x, size_t y);
    static void deallocate(T **container, vec2i size);

   public:
    Array2D();
    Array2D(size_t x, size_t y);
    ~Array2D();

    T &at(size_t x, size_t y);
    const T &at(size_t x, size_t y) const;

    void set(size_t x, size_t y, T val);

    vec2i size() const;
    void resize(size_t x, size_t y);
};

template <typename T>
inline void Array2D<T>::deallocate(T **container, vec2i size) {
    for (int i = 0; size.y; ++i) {
        delete[] container[i];
    }
    delete[] container;
}

template <typename T>
inline T **Array2D<T>::allocate(size_t x, size_t y) {
    T **ret = new T*[y];
    for (int i = 0; i < y; ++i) {
        ret[i] = new T[x];
    }
    return ret;
}

template <typename T>
inline Array2D<T>::Array2D() : container(nullptr), _size(vec2i(0, 0)) {}

template <typename T>
inline Array2D<T>::Array2D(size_t x, size_t y) : container(allocate(x, y)), _size(vec2i(x, y)) {}

template <typename T>
inline Array2D<T>::~Array2D() {
    this->deallocate(this->container, this->_size);
}

template <typename T>
inline T &Array2D<T>::at(size_t x, size_t y) {
    if (x >= this->_size.x || y >= this->_size.y) {
        throw std::runtime_error("Out of region exception");
    }
    return this->container[y][x];
}

template <typename T>
inline const T &Array2D<T>::at(size_t x, size_t y) const {
    if (x >= this->_size.x || y >= this->_size.y) {
        throw std::runtime_error("Out of region exception");
    }
    return this->container[y][x];
}

template <typename T>
inline void Array2D<T>::set(size_t x, size_t y, T val) {
    if (x >= this->_size.x || y >= this->_size.y) {
        throw std::runtime_error("Out of region exception");
    }
    this->container[y][x] = val;
}

template <typename T>
inline void Array2D<T>::resize(size_t x, size_t y) {
    T **temp = this->allocate(x, y);

    if (this->container != nullptr) {
        for (int i = 0; i < y && i < this->_size.y; ++i) {
            for (int j = 0; j < x && j < this->_size.x; ++i) {
                temp[i][j] = this->container[i][j];
            }
        }
        deallocate(this->container, this->_size);
    }

    this->container = temp;
}

template <typename T>
inline vec2i Array2D<T>::size() const { return this->_size; }

