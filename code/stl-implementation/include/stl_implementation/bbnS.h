#pragma once

#include <cstddef>
#include <iosfwd>

namespace bbn {

class string
{
public:
    using iterator = char*;
    using const_iterator = const char*;
    static constexpr std::size_t npos = static_cast<std::size_t>(-1);

    string(const char* str = "");
    string(const string& other);
    string& operator=(string other);
    ~string();

    iterator begin();
    iterator end();
    const_iterator begin() const;
    const_iterator end() const;

    std::size_t size() const;
    std::size_t capacity() const;
    void reserve(std::size_t n);
    void resize(std::size_t n, char ch = '\0');
    bool empty() const;

    void push_back(char ch);
    void append(const char* str);
    string& operator+=(char ch);
    string& operator+=(const char* str);
    string& insert(std::size_t pos, char ch);
    string& insert(std::size_t pos, const char* str);
    string& erase(std::size_t pos, std::size_t len);
    void clear();
    void swap(string& other);
    const char* c_str() const;

    char& operator[](std::size_t i);
    const char& operator[](std::size_t i) const;
    std::size_t find(char ch, std::size_t pos = 0) const;
    std::size_t find(const char* str, std::size_t pos = 0) const;
    std::size_t rfind(char ch, std::size_t pos = npos) const;
    std::size_t rfind(const char* str, std::size_t pos = npos) const;

    bool operator>(const string& other) const;
    bool operator>=(const string& other) const;
    bool operator<(const string& other) const;
    bool operator<=(const string& other) const;
    bool operator==(const string& other) const;
    bool operator!=(const string& other) const;

private:
    char* _str;
    std::size_t _size;
    std::size_t _capacity;
};

std::istream& operator>>(std::istream& in, string& value);
std::ostream& operator<<(std::ostream& out, const string& value);
std::istream& getline(std::istream& in, string& value);

} // namespace bbnS
