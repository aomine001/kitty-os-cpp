//
// Created by Piotr on 03.05.2024.
//

#include "kstring.hpp"

namespace kstd
{
    size_t strlen(const char* s)
    {
        size_t len = 0;
        while (*s++ != 0)
        {
            len++;
        }
        return len;
    }
    void* memset(void* ptr, int v, size_t num)
    {
        char* _Ptr = static_cast<char*>(ptr);
        char _Value = static_cast<char>(v); // Cast v to char

        while (num-- > 0)
        {
            *_Ptr++ = _Value; // Increment _Ptr to move to the next memory location
        }

        return _Ptr; // Return the end pointer
    }
    void* memcpy(void* dest, const void* src, size_t num)
    {
        char* _Dest = static_cast<char*>(dest);
        const char* _Src = static_cast<const char*>(src);

        // Copy byte by byte
        for (size_t i = 0; i < num; ++i) {
            _Dest[i] = _Src[i];
        }

        return dest;
    }
    void* memmove(void* dest, const void* src, size_t num)
    {
        char* _Dest = static_cast<char*>(dest);
        const char* _Src = static_cast<const char*>(src);

        // If source and destination addresses overlap
        if (_Dest > _Src && _Dest < _Src + num) {
            // Copy from end to start to avoid overwriting
            for (size_t i = num; i != 0; --i)
            {
                _Dest[i - 1] = _Src[i - 1];
            }
        } else {
            // Copy from start to end
            for (size_t i = 0; i < num; ++i)
            {
                _Dest[i] = _Src[i];
            }
        }

        return dest;
    }
    int memcmp(const void* ptr1, const void* ptr2, size_t num)
    {
        const unsigned char* p1 = static_cast<const unsigned char*>(ptr1);
        const unsigned char* p2 = static_cast<const unsigned char*>(ptr2);

        for (size_t i = 0; i < num; ++i)
        {
            if (p1[i] != p2[i])
            {
                return (p1[i] < p2[i]) ? -1 : 1;
            }
        }
        return 0;
    }
    void* memchr(const void* ptr, int value, size_t num)
    {
        const unsigned char* p = static_cast<const unsigned char*>(ptr);
        for (size_t i = 0; i < num; ++i)
        {
            if (p[i] == value)
            {
                return const_cast<void*>(static_cast<const void*>(p + i));
            }
        }
        return nullptr;
    }
    void* memrchr(const void* ptr, int value, size_t num)
    {
        const unsigned char* p = static_cast<const unsigned char*>(ptr);
        for (size_t i = num; i > 0; --i)
        {
            if (p[i - 1] == value)
            {
                return const_cast<void*>(static_cast<const void*>(p + i - 1));
            }
        }
        return nullptr;
    }
    void* memccpy(void* dest, const void* src, int c, size_t num)
    {
        unsigned char* d = static_cast<unsigned char*>(dest);
        const unsigned char* s = static_cast<const unsigned char*>(src);

        for (size_t i = 0; i < num; ++i)
        {
            d[i] = s[i];
            if (s[i] == static_cast<unsigned char>(c))
            {
                return d + i + 1;
            }
        }
        return nullptr;
    }
    void memswap(void* ptr1, void* ptr2, size_t num)
    {
        unsigned char* p1 = static_cast<unsigned char*>(ptr1);
        unsigned char* p2 = static_cast<unsigned char*>(ptr2);

        for (size_t i = 0; i < num; ++i)
        {
            unsigned char temp = p1[i];
            p1[i] = p2[i];
            p2[i] = temp;
        }
    }

    char* strcpy(char* destination, const char* source) {
        char* originalDestination = destination;

        // Copy characters until null terminator is encountered
        while (*source != '\0') {
            *destination = *source;
            ++destination;
            ++source;
        }

        // Null-terminate the destination string
        *destination = '\0';

        return originalDestination;
    }

    char* strcat(char* destination, const char* source) {
        char* originalDestination = destination;

        // Move the destination pointer to the end of the string
        while (*destination != '\0') {
            ++destination;
        }

        // Copy characters from source to the end of destination
        while (*source != '\0') {
            *destination = *source;
            ++destination;
            ++source;
        }

        // Null-terminate the concatenated string
        *destination = '\0';

        return originalDestination;
    }

}

kstd::string::string(const char* str)
{
    length = kstd::strlen(str);
    data = new char[length + 1]; // +1 for termination
    kstd::strcpy(data, str);
}

kstd::string::~string()
{
    delete[] data;
}

kstd::string::string(const kstd::string& other)
{
    length = other.length;
    data = new char[length + 1]; // +1 for termination
    kstd::strcpy(data, other.data);
}

kstd::string& kstd::string::operator=(const kstd::string &other)
{
    if (this != &other) {
        delete[] data;
        length = other.length;
        data = new char[length + 1]; // +1 for null terminator
        kstd::strcpy(data, other.data);
    }
    return *this;
}

size_t kstd::string::size() const {
    return length;
}

const char* kstd::string::c_str() const {
    return data;
}

kstd::string& kstd::string::operator+=(const kstd::string &other) {
    size_t newLength = length + other.length;
    char* newData = new char[newLength + 1]; // +1 for null terminator
    kstd::strcpy(newData, data);
    kstd::strcat(newData, other.data);
    delete[] data;
    data = newData;
    length = newLength;
    return *this;
}


kstd::string& kstd::string::operator+=(const char other) {
    char* new_data = new char[length + 2];
    strcpy(new_data, data);
    new_data[length] = other;
    new_data[length + 1] = '\0'; // Null terminator
    delete[] data;
    data = new_data;
    ++length;
    return *this;
}

bool kstd::string::empty() const {
    return length == 0;
}

void kstd::string::clear() {
    delete[] data;
    data = nullptr;
    length = 0;
}

void kstd::string::reserve(size_t newCapacity)
{
    if (newCapacity <= length) return; // No need to reallocate

    char* newData = new char[newCapacity];
    if (data) {
        kstd::memcpy(newData, data, length); // Copy existing data
        delete[] data; // Delete old data
    }
    data = newData;
    length = newCapacity;
}