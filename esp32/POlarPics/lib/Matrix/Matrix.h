#pragma once

#include "esp32-hal.h" // used to allow ps_malloc

#include <algorithm>
#include <functional>

struct PIXELFORMAT_RGB {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    PIXELFORMAT_RGB(uint8_t r_, uint8_t g_, uint8_t b_) : r{r_}, g{g_}, b{b_} {}
} ;

template <class T >
class Matrix {
private:
    size_t cols_;
	size_t rows_;
	T* data_;
	
public:
	Matrix() : cols_{0}, rows_{0}, data_{nullptr} {}

	Matrix(size_t c, size_t r) : cols_{c}, rows_{r} {
        data_ = (T*) ps_malloc(cols_ * rows_ * sizeof(T));
	}

	Matrix(size_t c, size_t r, T fillVal) : Matrix(c, r) {
        std::fill(data_, data_ + rows_ * cols_, fillVal);
	}

	Matrix(const Matrix<T>& rhs) : Matrix(rhs.cols_, rhs.rows_) {
       std::copy(rhs.begin(), rhs.end(), data_);
	}

    Matrix(Matrix<T>&& mat) : cols_{mat.cols_}, rows_{mat.rows_}, data_{mat.data_} {
        mat.data_ = nullptr;
    }

    ~Matrix() {
        if(data_ != nullptr)
            free(data_);
    }

    // getters
    inline T* data() { return data_; }
    inline const T* data() const { return data_; }
	inline size_t width() const { return cols_; }
	inline size_t height() const { return rows_; }
	inline size_t len() const { return rows_ * cols_; }

    T* begin() { return data_; }
    const T* begin() const { return data_; }
    T* end() { return begin() + len(); }
    const T* end() const { return begin() + len(); }

    inline T& operator[](const size_t& id) { return data_[id]; }
    inline const T& operator[](const size_t& id) const { return data_[id]; }

    inline T& operator()(const size_t& r, const size_t& c) { return data_[r * cols_ + c]; }
    inline const T& operator()(const size_t& r, const size_t& c) const { return data_[r * cols_ + c]; }
    inline T& operator()(const size_t& id) { return data_[id]; }
    inline const T& operator()(const size_t& id) const { return data_[id]; }

    Matrix& operator=(const Matrix& rhs) {
        if (this != &rhs) {
            if(rhs.rows_ != rows_ || rhs.cols_ != cols_) {
                if(data_ != nullptr)
                    free(data_);
                rows_ = rhs.rows_;
                cols_ = rhs.cols_;
                data_ = (T*) ps_malloc(len() * sizeof(T));
            }
            
            std::copy(rhs.begin(), rhs.end(), begin());
        }
        return *this;
    }
    
    Matrix& operator=(Matrix&& rhs) {
        if (this != &rhs) {
            if(data_ != nullptr)
                free(data_);

            data_ = rhs.data_;
            rhs.data_ = nullptr;
            rows_ = rhs.rows_;
            rhs.rows_ = 0;
            cols_ = rhs.cols_;
            rhs.cols_ = 0;
        }
        return *this;
    }

	bool operator==(const Matrix<T>& rhs) const {
        if (this != &rhs)
            return true;

		if(rhs.rows_ != rows_ || rhs.cols_ != cols_)
			return false;

		return std::equal(begin(), end(), rhs.begin());
	}

    
    Matrix& operator+(const Matrix& rhs) const {
        Matrix m(*this);
		return m += rhs;
    }

    Matrix& operator+(const T& rhs) {
        Matrix m(*this);
		return m += rhs;
    }

    Matrix& operator+=(const Matrix& rhs) {
        if(rhs.rows_ != rows_ || rhs.cols_ != cols_)
			throw std::invalid_argument("Matrix without the same size can't be added.");

        std::transform(begin(), end(), rhs.begin(), begin(), std::plus<T>());
    }

    Matrix& operator+=(const T& rhs) {
        if(rhs.rows_ != rows_ || rhs.cols_ != cols_)
			throw std::invalid_argument("Matrix without the same size can't be added.");

        std::transform(begin(), end(), begin(), std::bind(std::plus<T>(), std::placeholders::_1, rhs));
    }
};

template<typename T>
Matrix<T>& operator+(const T& lhs, const Matrix<T>& rhs) {
    return rhs + lhs;
}