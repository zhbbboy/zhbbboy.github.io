#include "stl_implementation/bbnS.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstring>
#include <iostream>
#include <utility>

namespace bbnS {

namespace {

const char* safe_c_str(const char* str)
{
    // 辅助函数：strlen、memcpy 等 C 字符串函数不能接收 nullptr。
    // 若调用者传入 nullptr，则把它当成空字符串 "" 使用。
    return str == nullptr ? "" : str;
}

} // namespace

string::string(const char* str)
    : _str(nullptr)
    , _size(0)
    , _capacity(0)
{
    const char* source = safe_c_str(str);
    _size = std::strlen(source);
    _capacity = _size;
    _str = new char[_capacity + 1];

    // std::memcpy(目标地址, 源地址, 字节数)：原样复制指定数量的字节。
    // 这里复制 _size + 1 个字节，把字符串结尾的 '\0' 也复制过去。
    std::memcpy(_str, source, _size + 1);
}

string::string(const string& other)
    : string(other._str)
{
}

string& string::operator=(string other)
{
    // 参数 other 按值传递：进入函数时会先复制出一份数据。
    // 交换后，旧数据被交给 other，离开函数时由 other 的析构函数释放。
    // 这种写法称为“拷贝并交换（copy-and-swap）”。
    swap(other);
    return *this;
}

string::~string()
{
    delete[] _str;
}

string::iterator string::begin()
{
    return _str;
}

string::iterator string::end()
{
    return _str + _size;
}

string::const_iterator string::begin() const
{
    return _str;
}

string::const_iterator string::end() const
{
    return _str + _size;
}

std::size_t string::size() const
{
    return _size;
}

std::size_t string::capacity() const
{
    return _capacity;
}

void string::reserve(std::size_t n)
{
    // n 是期望的字符容量，不包含末尾的 '\0'。
    // 容量足够时不重新分配，避免不必要的内存申请。
    if (n <= _capacity)
    {
        return;
    }

    char* replacement = new char[n + 1];
    // memcpy(目标, 源, 字节数)：把旧字符和末尾 '\0' 一起搬到新空间。
    std::memcpy(replacement, _str, _size + 1);
    delete[] _str;
    _str = replacement;
    _capacity = n;
}

void string::resize(std::size_t n, char ch)
{
    // n 是调整后的字符串长度；当 n 大于原长度时，用 ch 填充新增的位置。
    if (n > _capacity)
    {
        reserve(n);
    }

    if (n > _size)
    {
        // std::fill(起始迭代器, 结束迭代器, 填充值)：
        // 将半开区间 [_str + _size, _str + n) 内的每个字符都改为 ch。
        // 结束位置不参与填充，因此不会覆盖后面预留给 '\0' 的位置。
        std::fill(_str + _size, _str + n, ch);
    }

    _size = n;
    _str[_size] = '\0';
}

bool string::empty() const
{
    return _size == 0;
}

void string::push_back(char ch)
{
    if (_size == _capacity)
    {
        reserve(_capacity == 0 ? 4 : _capacity * 2);
    }

    _str[_size++] = ch;
    _str[_size] = '\0';
}

void string::append(const char* str)
{
    const char* source = safe_c_str(str);
    if (source == _str)
    {
        string copy(source);
        append(copy.c_str());
        return;
    }

    const std::size_t length = std::strlen(source);
    if (length == 0)
    {
        return;
    }

    reserve(_size + length);
    // 复制 length 个实际字符和紧跟的 '\0'，使追加后的字符串仍正确结束。
    std::memcpy(_str + _size, source, length + 1);
    _size += length;
}

string& string::operator+=(char ch)
{
    push_back(ch);
    return *this;
}

string& string::operator+=(const char* str)
{
    append(str);
    return *this;
}

string& string::insert(std::size_t pos, char ch)
{
    assert(pos <= _size);
    if (pos > _size)
    {
        return *this;
    }

    if (_size == _capacity)
    {
        reserve(_capacity == 0 ? 4 : _capacity * 2);
    }

    // std::memmove(目标, 源, 字节数) 与 memcpy 相似，但允许源、目标区域重叠。
    // 把 [pos, _size] 整段右移一格，包含 '\0'，从而为 ch 腾出位置。
    std::memmove(_str + pos + 1, _str + pos, _size - pos + 1);
    _str[pos] = ch;
    ++_size;
    return *this;
}

string& string::insert(std::size_t pos, const char* str)
{
    assert(pos <= _size);
    if (pos > _size)
    {
        return *this;
    }

    const char* source = safe_c_str(str);
    if (source == _str)
    {
        string copy(source);
        return insert(pos, copy.c_str());
    }

    const std::size_t length = std::strlen(source);
    if (length == 0)
    {
        return *this;
    }

    reserve(_size + length);
    // 源区和目标区位于同一个 _str 数组，彼此会重叠，所以必须使用 memmove。
    // 将旧的 [pos, _size]（包含 '\0'）右移 length 格，为新子串留出空位。
    std::memmove(_str + pos + length, _str + pos, _size - pos + 1);
    std::memcpy(_str + pos, source, length);
    _size += length;
    return *this;
}

string& string::erase(std::size_t pos, std::size_t len)
{
    assert(pos <= _size);
    if (pos >= _size)
    {
        return *this;
    }

    // std::min(a, b) 返回较小值。删除长度不能超过 pos 之后剩余的字符数。
    const std::size_t count = std::min(len, _size - pos);
    // 将删除区间后的内容（包含 '\0'）左移，覆盖被删除的字符。
    std::memmove(_str + pos, _str + pos + count, _size - pos - count + 1);
    _size -= count;
    return *this;
}

void string::clear()
{
    _size = 0;
    _str[0] = '\0';
}

void string::swap(string& other)
{
    // std::swap(a, b) 交换两个对象的值。
    // 这里交换指针、长度和容量，就能以常数时间交换两个字符串的数据所有权。
    std::swap(_str, other._str);
    std::swap(_size, other._size);
    std::swap(_capacity, other._capacity);
}

const char* string::c_str() const
{
    return _str;
}

char& string::operator[](std::size_t i)
{
    assert(i < _size);
    return _str[i];
}

const char& string::operator[](std::size_t i) const
{
    assert(i < _size);
    return _str[i];
}

std::size_t string::find(char ch, std::size_t pos) const
{
    for (std::size_t i = pos; i < _size; ++i)
    {
        if (_str[i] == ch)
        {
            return i;
        }
    }

    return npos;
}

std::size_t string::find(const char* str, std::size_t pos) const
{
    if (pos > _size || str == nullptr)
    {
        return npos;
    }

    const std::size_t length = std::strlen(str);
    if (length == 0)
    {
        return pos;
    }

    if (length > _size - pos)
    {
        return npos;
    }

    // std::strstr(主串起始地址, 子串)：返回子串首次出现的位置；找不到返回 nullptr。
    const char* result = std::strstr(_str + pos, str);
    return result == nullptr ? npos : static_cast<std::size_t>(result - _str);
}

std::size_t string::rfind(char ch, std::size_t pos) const
{
    if (_size == 0)
    {
        return npos;
    }

    const std::size_t start = pos == npos || pos >= _size ? _size - 1 : pos;
    for (std::size_t i = start + 1; i > 0; --i)
    {
        const std::size_t index = i - 1;
        if (_str[index] == ch)
        {
            return index;
        }
    }

    return npos;
}

std::size_t string::rfind(const char* str, std::size_t pos) const
{
    if (str == nullptr)
    {
        return npos;
    }

    const std::size_t length = std::strlen(str);
    if (length == 0)
    {
        return pos == npos || pos > _size ? _size : pos;
    }

    if (length > _size)
    {
        return npos;
    }

    const std::size_t last = _size - length;
    const std::size_t start = pos == npos || pos > last ? last : pos;
    for (std::size_t i = start + 1; i > 0; --i)
    {
        const std::size_t index = i - 1;
        // std::strncmp(字符串1, 字符串2, 字符数) 最多比较 length 个字符；
        // 返回 0 表示这 length 个字符完全相同。
        if (std::strncmp(_str + index, str, length) == 0)
        {
            return index;
        }
    }

    return npos;
}

bool string::operator>(const string& other) const
{
    // std::strcmp(字符串1, 字符串2)：返回值大于/等于/小于 0，分别表示前者大于/等于/小于后者。
    return std::strcmp(_str, other._str) > 0;
}

bool string::operator>=(const string& other) const
{
    return !(*this < other);
}

bool string::operator<(const string& other) const
{
    return std::strcmp(_str, other._str) < 0;
}

bool string::operator<=(const string& other) const
{
    return !(*this > other);
}

bool string::operator==(const string& other) const
{
    return _size == other._size && std::strcmp(_str, other._str) == 0;
}

bool string::operator!=(const string& other) const
{
    return !(*this == other);
}

std::istream& operator>>(std::istream& in, string& value)
{
    value.clear();

    // std::ws 是输入流操纵器：跳过开头连续的空白字符。
    in >> std::ws;

    char ch = '\0';
    // in.get(ch) 每次从输入流读取一个字符到 ch。
    // std::isspace 判断字符是否为空白；转换为 unsigned char 可避免负 char 值的未定义行为。
    while (in.get(ch) && !std::isspace(static_cast<unsigned char>(ch)))
    {
        value.push_back(ch);
    }

    return in;
}

std::ostream& operator<<(std::ostream& out, const string& value)
{
    return out << value.c_str();
}

std::istream& getline(std::istream& in, string& value)
{
    value.clear();

    char ch = '\0';
    while (in.get(ch) && ch != '\n')
    {
        value.push_back(ch);
    }

    return in;
}

} // namespace bbnS
