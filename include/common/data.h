#pragma once

#include <cstdint>
#include <cstring>
#include <memory>

class Data {
public:
    Data();
    explicit Data(size_t length);
    Data(const void* ptr, size_t length);
    Data(const Data& other);
    Data(Data&& other) noexcept;
    ~Data();
    
    Data& operator=(const Data& other);
    Data& operator=(Data&& other) noexcept;
    
    const void* ptr() const { return data_.get(); }
    void* ptr() { return data_.get(); }
    size_t length() const { return length_; }
    
private:
    std::unique_ptr<uint8_t[]> data_;
    size_t length_;
};

