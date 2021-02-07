#pragma once

#include "esp32-hal.h" // used to allow ps_malloc

#include <algorithm>
#include <functional>

class PrinterMatrix {
private:
    size_t cols_;
	size_t rows_;

    size_t size_; // internal size

	uint8_t* data_;
	
public:
	PrinterMatrix() : cols_{0}, rows_{0}, size_{0}, data_{nullptr} {}

	PrinterMatrix(size_t c, size_t r) : cols_{c}, rows_{r} {
        size_ = (cols_*rows_+7)/8;
        data_ = (uint8_t*) ps_malloc(size_ * sizeof(uint8_t));
	}

	PrinterMatrix(size_t c, size_t r, bool fillVal) : PrinterMatrix(c, r) {
        std::fill(data_, data_ + size_, fillVal ? 255 : 0);
	}

    PrinterMatrix(PrinterMatrix&& mat) : cols_{mat.cols_}, rows_{mat.rows_}, size_{mat.size_}, data_{mat.data_} {
        mat.data_ = nullptr;
    }

    ~PrinterMatrix() {
        if(data_ != nullptr) free(data_);
    }

    PrinterMatrix& operator=(PrinterMatrix&& rhs) {
        if (this != &rhs) {
            if(data_ != nullptr) free(data_);

            data_ = rhs.data_;
            rhs.data_ = nullptr;
            rows_ = rhs.rows_;
            rhs.rows_ = 0;
            cols_ = rhs.cols_;
            rhs.cols_ = 0;
            size_ = rhs.size_;
            rhs.size_ = 0;
        }
        return *this;
    }

    inline uint8_t* data() { return data_; }
    inline const uint8_t* data() const { return data_; }
	inline size_t width() const { return cols_; }
	inline size_t height() const { return rows_; }
	inline size_t len() const { return cols_*  rows_; }

    inline uint8_t& operator[](const size_t& id) { return data_[id]; }
    inline const uint8_t& operator[](const size_t& id) const { return data_[id]; }

    inline bool operator()(const size_t& r, const size_t& c) {
        const size_t id = r * cols_ + c;
        return data_[id/8] >> (7-id%8) && 1;
    }
    
    inline bool operator()(const size_t& r, const size_t& c) const {
        const size_t id = r * cols_ + c;
        return data_[id/8] >> (7-id%8) && 1;
    }

    inline void set(const size_t& r, const size_t& c, const bool value) {
        const size_t id = r * cols_ + c;
        const uint8_t mask = 1 << (7-id%8);
        data_[id/8] = value ? data_[id/8] | mask : data_[id/8] & ~mask;
    }
};