#include "../include/common/data.h"

Data::Data() : data_(nullptr), length_(0) {}

Data::Data(size_t length) : data_(std::make_unique<uint8_t[]>(length)), length_(length) {}

Data::Data(const void* ptr, size_t length) : Data(length) {
    if (ptr && length > 0) {
        std::memcpy(data_.get(), ptr, length);
    }
}

Data::Data(const Data& other) : Data(other.data_.get(), other.length_) {}

Data::Data(Data&& other) noexcept 
    : data_(std::move(other.data_)), length_(other.length_) {
    other.length_ = 0;
}

Data::~Data() = default;

Data& Data::operator=(const Data& other) {
    if (this != &other) {
        *this = Data(other);
    }
    return *this;
}

Data& Data::operator=(Data&& other) noexcept {
    if (this != &other) {
        data_ = std::move(other.data_);
        length_ = other.length_;
        other.length_ = 0;
    }
    return *this;
}

