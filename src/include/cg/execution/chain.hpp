#pragma once

#include <cg/domain/effect.hpp>

#include <cg/execution/context.hpp>

namespace cg::dsl
{
	class ActivatedEffect;
	class Effect;
}

namespace cg::execution
{
	// 一条连锁(激活时产生的绑定保存在context中)
	// - 手动发动(卡发动/启动/诱发即时)与触发发动共用此结构
	// - activated为空:仅卡的发动、没有效果体(如纯静态场地魔法)
	class ChainLink
	{
	public:
		//

	private:
		// 发动者
		domain::Player controller_;
		// 发动时的咒文速度(结算/展示用)
		domain::EffectSpeed speed_;
		// 效果来源卡
		state::CardInstanceId source_;

		// 发动的效果体
		const dsl::ActivatedEffect* activated_;
		// 效果定义(使用次数/互斥);无效果体时为空
		const dsl::Effect* effect_;

		// 发动时产生的绑定
		Context context_;

		// 结算时由无效原语写入
		bool activation_negated_;
		bool effect_negated_;

		// 次数回滚信息:是否由本 link 建立 mutex 选择
		bool mutex_inserted_;

	public:
		ChainLink(
			const domain::Player controller,
			const domain::EffectSpeed speed,
			const state::CardInstanceId source,
			const dsl::ActivatedEffect* activated,
			const dsl::Effect* effect,
			Context context,
			const bool mutex_inserted = false
		) noexcept
			: controller_{controller},
			  speed_{speed},
			  source_{source},
			  activated_{activated},
			  effect_{effect},
			  context_{std::move(context)},
			  activation_negated_{false},
			  effect_negated_{false},
			  mutex_inserted_{mutex_inserted} {}

		[[nodiscard]] auto controller() const noexcept -> domain::Player
		{
			return controller_;
		}

		[[nodiscard]] auto speed() const noexcept -> domain::EffectSpeed
		{
			return speed_;
		}

		[[nodiscard]] auto source() const noexcept -> state::CardInstanceId
		{
			return source_;
		}

		[[nodiscard]] auto activated_effect() const noexcept -> const dsl::ActivatedEffect*
		{
			return activated_;
		}

		[[nodiscard]] auto effect() const noexcept -> const dsl::Effect*
		{
			return effect_;
		}

		[[nodiscard]] auto context() noexcept -> Context&
		{
			return context_;
		}

		auto set_chain_position(Chain& chain, const std::size_t link_index) noexcept -> void
		{
			context_.chain = {.chain = chain, .link_index = link_index};
		}

		[[nodiscard]] auto activation_negated() const noexcept -> bool
		{
			return activation_negated_;
		}

		auto negate_activation() noexcept -> void
		{
			activation_negated_ = true;
		}

		[[nodiscard]] auto effect_negated() const noexcept -> bool
		{
			return effect_negated_;
		}

		auto negate_effect() noexcept -> void
		{
			effect_negated_ = true;
		}

		[[nodiscard]] auto mutex_inserted() const noexcept -> bool
		{
			return mutex_inserted_;
		}
	};

	// 连锁;下标 0 = Chain Link 1,结算时逆序
	class Chain : public std::vector<ChainLink> {};
}
