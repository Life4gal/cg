#pragma once

#include <concepts>
#include <random>
#include <span>

namespace cg::utility
{
	class Random final
	{
	public:
		using engine_type = std::mt19937;

		using result_type = engine_type::result_type;

		[[nodiscard]] constexpr static auto min() noexcept -> result_type
		{
			return engine_type::min();
		}

		[[nodiscard]] constexpr static auto max() noexcept -> result_type
		{
			return engine_type::max();
		}

	private:
		engine_type engine_;

	public:
		Random() noexcept = default; // NOLINT(cert-msc51-cpp)

		explicit Random(const result_type seed) noexcept
			: engine_{seed} {}

		explicit Random(const std::span<const result_type> seed_sequence) noexcept // NOLINT(cert-msc51-cpp)
		{
			seed(seed_sequence);
		}

		auto seed(const result_type seed) noexcept -> void
		{
			engine_.seed(seed);
		}

		auto seed(const std::span<const result_type> seed_sequence) noexcept -> void
		{
			std::seed_seq s{seed_sequence.begin(), seed_sequence.end()};
			engine_.seed(s);
		}

		// 用于例如`std::ranges::shuffle(range, random)`之类的调用
		[[nodiscard]] auto operator()() noexcept -> result_type
		{
			return engine_();
		}

		// 生成闭区间随机整数 [min_value, max_value]
		template<std::integral Integer>
		[[nodiscard]] auto int_inclusive(const Integer min_value, const std::type_identity_t<Integer> max_value) noexcept -> Integer
		{
			return std::uniform_int_distribution<Integer>{min_value, max_value}(engine_);
		}

		// 生成闭区间随机浮点数 [min_value, max_value]
		template<std::floating_point Float>
		[[nodiscard]] auto real_inclusive(const Float min_value, const std::type_identity_t<Float> max_value) noexcept -> Float
		{
			return std::uniform_real_distribution<Float>{min_value, max_value}(engine_);
		}

		// 生成正态分布随机浮点数 N(mean, standard_deviation)
		template<std::floating_point Float>
		[[nodiscard]] auto normal(const Float mean, const std::type_identity_t<Float> standard_deviation) noexcept -> Float
		{
			return std::normal_distribution<Float>{mean, standard_deviation}(engine_);
		}

		// 百分比判定: 例如 roll_percent(20) 表示 20% 成功率
		[[nodiscard]] auto roll_percent(const int percent) noexcept -> bool
		{
			if (percent <= 0)
			{
				return false;
			}
			if (percent >= 100)
			{
				return true;
			}

			return int_inclusive(0, 99) < percent;
		}

		// 概率判定: 概率范围 [0, 1]
		[[nodiscard]] auto chance(const float probability) noexcept -> bool
		{
			if (probability <= 0.f)
			{
				return false;
			}
			if (probability >= 1.f)
			{
				return true;
			}

			return std::bernoulli_distribution{static_cast<double>(probability)}(engine_);
		}
	};
}
