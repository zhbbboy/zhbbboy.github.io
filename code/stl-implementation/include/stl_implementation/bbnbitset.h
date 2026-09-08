#pragma once
#include<vector>
#include <stdexcept> // 用于抛出异常
#include<string>

namespace bbn
{
	// 模板参数N：位图覆盖的整数范围（0 ~ N-1）
	template<size_t N>
	class bitset
	{
	public:
		// 构造函数：初始化位图空间，所有比特位默认置0
		bitset();

		// 析构函数：默认即可（vector自动管理内存，无需手动释放）
		~bitset() = default;

		// 禁止拷贝构造和赋值（可选，避免大对象拷贝开销）
		bitset(const bitset&) = delete;
		bitset& operator=(const bitset&) = delete;

		// 允许移动构造和赋值（优化性能）
		bitset(bitset&&) noexcept = default;
		bitset& operator=(bitset&&) noexcept = default;

		// 核心操作接口
		void set(size_t x);// 标记x存在（置1）
		void reset(size_t x);// 取消标记x（置0）
		void reset();		// 重载：所有比特位置0
		bool test(size_t x)const;// 检查x是否存在（返回bool）
		void flip(size_t x);// 翻转x对应的比特位状态
		bool any() const;		// 判断是否存在至少一个置1的比特位
		bool none() const;     // 判断是否所有比特位都为0
		size_t count() const;  // 统计置1的比特位个数
		size_t size() const;   // 返回位图总比特位个数（即N）
	private:
		std::vector<size_t> _bits;// 底层存储：size_t数组，每个元素管理32位

	};

	template<size_t N>
	bitset<N>::bitset()
	{
		// 计算数组大小：向上取整，确保覆盖0~N-1所有整数
		// (N + 31) >> 5 等价于 (N + 31) / 32（整数除法）
		size_t arr_size = (N + 31) >> 5;
		_bits.resize(arr_size, 0);
	}

	template<size_t N>
	void bitset<N>::set(size_t x)
	{
		if (x >= N)
		{
			throw std::out_of_range("bitset::set: x超出位图范围，x=" + std::to_string(x) + ", 最大范围=" + std::to_string(N - 1));
		}
		// 定位下标和位置
		size_t idx = x >> 5;
		size_t pos = x % 32;
		//构造掩码
		size_t mask = 1ULL << pos;
		_bits[idx] |= mask;
	}

	template<size_t N>
	void bitset<N>::reset(size_t x)
	{
		if (x >= N)
		{
			throw std::out_of_range("bitset::reset: x超出位图范围，x=" + std::to_string(x) + ", 最大范围=" + std::to_string(N - 1));
		}

		size_t idx = x >> 5;
		size_t pos = x % 32;

		size_t mask = ~(1ULL << pos);
		_bits[idx] &= mask;
	}

	template<size_t N>
	void bitset<N>::reset()
	{
		std::fill(_bits.begin(), _bits.end(), 0);
	}

	template<size_t N>
	bool bitset<N>::test(size_t x) const
	{
		if (x >= N)
		{
			throw std::out_of_range("bitset::test: x超出位图范围，x=" + std::to_string(x) + ", 最大范围=" + std::to_string(N - 1));
		}

		size_t idx = x >> 5;
		size_t pos = x % 32;
		size_t mask = 1ULL << pos;

		return (_bits[idx] & mask) != 0;
	}

	template<size_t N>
	void bitset<N>::flip(size_t x)
	{
		if (x >= N) {
			throw std::out_of_range("bitset::flip: x超出位图范围，x=" + std::to_string(x) + ", 最大范围=" + std::to_string(N - 1));
		}
		size_t idx = x >> 5;
		size_t pos = x % 32;
		size_t mask = 1ULL << pos;

		_bits[idx] ^= mask;
	}

	//判断是否存在至少一个置 1 的比特位
	template<size_t N>
	bool bitset<N>::any() const
	{
		for (size_t val : _bits)
		{
			if (val != 0)
			{
				return true;
			}
		}
		return false;
	}

	template<size_t N>
	bool bitset<N>::none() const {
		// 与any相反，所有元素都为0则返回true
		return !any();
	}

	template<size_t N>
	size_t bitset<N>::count() const
	{
		size_t cut = 0;
		for (size_t val : _bits)
		{
			while (val != 0)
			{
				val &= val - 1;
				cut++;
			}
		}
		return cut;
	}

	template<size_t N>
	size_t bitset<N>::size() const
	{
		return N;
	}
}
