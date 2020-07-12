#pragma once

#include "img_converters.h"

#include <algorithm>
#include <functional>

struct PIXELFORMAT_RGB {
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

// using struct PIXELFORMAT_RGB PIXELFORMAT_RGB;
// using PIXELFORMAT_GRAYSCALE = uint8_t;

template <class T >
class Matrix
{
private:
	T* data_;
	size_t rows_;
	size_t cols_;

public:
	Matrix() : rows_(0), cols_(0) {}

	Matrix(size_t c, size_t r) : rows_{r}, cols_{0} {
		data_ = new T[rows_ * cols_];
	}

	Matrix(size_t c, size_t r, T fillVal) : Matrix(c, r) {
        std::fill(data_, data_ + r * c, fillVal);
		/*
        for (size_t i = 0; i < len(); i++) {
			data_[i] = fillVal;
		}
        */
	}

	Matrix(const Matrix& rhs) : Matrix(rhs.cols_, rhs.rows_) {
       std::copy(rhs.data_, rhs.data_ + rhs.len(), data_);
	}

    Matrix(Matrix&& mat) : data_{} {
        data_ = mat.data_;
        mat.data_ = nullptr;
    }

    Matrix(const camera_fb_t* fb) : rows_{fb->height}, cols_{fb->width} {
        // Load img and store it into our buffer
        if(!fmt2rgb888(fb->buf, fb->len, fb->format, (uint8_t*)(&(data_[0])))) {
            throw std::runtime_error("[Error] getImageMatrixFromJPEGBuffer: conversion to rgb888 failed.");
        }
    }

    ~Matrix() {
        if(data_ != nullptr)
            delete[] data_;
    }

    // getters
    inline T* data() const { return data_; }
	inline size_t width() const { return rows_; }
	inline size_t height() const { return cols_; }
	inline size_t len() const { return rows_ * cols_; }

    int begin() { return int(&data_[0]); }
    int end() { return int(&data_[rows_ * cols_]); }

    T& operator[](const size_t& id) { return data_[id]; }
    const T& operator[](const size_t& id) const { return data_[id]; }

    T& operator()(const size_t& r, const size_t& c) { return data_[r * cols_ + c]; }
    const T& operator()(const size_t& r, const size_t& c) const { return data_[r * cols_ + c]; }
    T& operator()(const size_t& id) { return data_[id]; }
    const T& operator()(const size_t& id) const { return data_[id]; }

    Matrix& operator=(const Matrix& rhs) {
        if (this != &rhs) {
            if(rhs.rows_ != rows_ || rhs.cols_ != cols_) {
                delete[] data;
                rows_ = rhs.rows_;
                cols_ = rhs.cols_;
                data_ = new T[rhs.len()];
            }
            
            std::copy(begin(), end(), begin());
        }
        return *this;
    }
    
    Matrix& operator=(Matrix&& rhs) {
        if (this != &rhs) {
            delete[] data_;
            data_ = rhs.data_;
            rhs.data_ = nullptr;
        }
        return *this;
    }

	bool operator==(const Matrix& rhs) const
	{
		if(rhs.rows_ != rows_ || rhs.cols_ != cols_)
			return false;

		return std::equal(begin(), end(), rhs.begin());
	}

    Matrix& operator+(const Matrix& rhs) const {
        Matrix m = *this;
		return m += rhs;
    }

    Matrix& operator+(const T& rhs) {
        Matrix m = *this;
		return m += rhs;
    }

    Matrix& operator+=(const Matrix& rhs) {

        if(rhs.rows_ != rows_ || rhs.cols_ != cols_)
			throw std::invalid_argument("Matrix without the same size can't be added.");

        std::transform(begin(), end(), rhs.begin(), begin(), std::plus<T>());
        return *this;
    }

    Matrix& operator+=(const T& rhs) {

        if(rhs.rows_ != rows_ || rhs.cols_ != cols_)
			throw std::invalid_argument("Matrix without the same size can't be added.");

        std::transform(begin(), end(), begin(), std::bind(std::plus<T>(), std::placeholders::_1, rhs));
        return *this;
    }
};

template<typename T>
Matrix<T>& operator+(const T& lhs, const Matrix<T>& rhs) {
    return rhs + lhs;
}

