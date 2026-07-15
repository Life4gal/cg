#pragma once

#include <core/card.hpp>

namespace cg
{
	class MonsterSummonMaterial;
	class MonsterSummonMaterials;

	// 怪兽卡类型
	enum class MonsterCategory : std::uint8_t
	{
		// 通常怪兽(无效果)
		NONE,
		// 效果怪兽
		EFFECT,
	};

	// 怪兽卡能力(支持多个能力?)
	enum class MonsterAbility : std::uint8_t
	{
		NONE = 0,

		// ====================================

		// 调整(同调召唤素材)
		TUNER = 1 << 0,

		// ====================================

		// 联合/同盟
		UNION = 1 << 1,
		// 灵魂
		SPIRIT = 1 << 2,
		// 二重
		GEMINI = 1 << 3,
		// 反转
		FLIP = 1 << 4,
		// 卡通
		TOON = 1 << 5,
	};

	// 怪兽卡属性(支持多个属性)
	enum class MonsterAttribute : std::uint8_t
	{
		NONE = 0,

		// ====================================

		// 地属性
		EARTH = 1 << 0,
		// 水属性
		WATER = 1 << 1,
		// 火属性
		FIRE = 1 << 2,
		// 风属性
		WIND = 1 << 3,
		// 光属性
		LIGHT = 1 << 4,
		// 暗属性
		DARK = 1 << 5,
		// 神属性
		DIVINE = 6,
	};

	// 怪兽卡种族
	enum class MonsterRace : std::uint8_t
	{
		NONE = 0,

		// ====================================

		// 战士族
		WARRIOR,
		// 魔法师族
		SPELLCASTER,
		// 龙族
		DRAGON,
		// 天使族
		FAIRY,
		// 恶魔族
		FIEND,
		// 不死族
		ZOMBIE,
		// 机械族
		MACHINE,
		// 水族
		AQUA,
		// 炎族
		PYRO,
		// 岩石族
		ROCK,
		// 鸟兽族
		WINGED_BEAST,
		// 植物族
		PLANT,
		// 昆虫族
		INSECT,
		// 雷族
		THUNDER,
		// 兽族
		BEAST,
		// 兽战士族
		BEAST_WARRIOR,
		// 恐龙族
		DINOSAUR,
		// 鱼族
		FISH,
		// 海龙族
		SEA_SERPENT,
		// 爬虫类族
		REPTILE,
		// 念动力族
		PSYCHIC,
		// 幻神兽族
		DIVINE_BEAST,
		// 电子界族
		CYBERSE,
		// 幻龙族
		WYRM,
		// 幻想魔族
		ILLUSION,
	};

	// 怪兽卡等级
	enum class MonsterLevel : std::uint8_t
	{
		L0 = 0,
		L1 = 1,
		L2 = 2,
		L3 = 3,
		L4 = 4,
		L5 = 5,
		L6 = 6,
		L7 = 7,
		L8 = 8,
		L9 = 9,
		L10 = 10,
		L11 = 11,
		L12 = 12,
		L13 = 13,
	};

	// 怪兽卡阶级
	enum class MonsterRank : std::uint8_t
	{
		R0 = 0,
		R1 = 1,
		R2 = 2,
		R3 = 3,
		R4 = 4,
		R5 = 5,
		R6 = 6,
		R7 = 7,
		R8 = 8,
		R9 = 9,
		R10 = 10,
		R11 = 11,
		R12 = 12,
		R13 = 13,
	};

	// 怪兽卡灵摆刻度
	enum class MonsterPendulum : std::uint8_t
	{
		P0 = 0,
		P1 = 1,
		P2 = 2,
		P3 = 3,
		P4 = 4,
		P5 = 5,
		P6 = 6,
		P7 = 7,
		P8 = 8,
		P9 = 9,
		P10 = 10,
		P11 = 11,
		P12 = 12,
		P13 = 13,
	};

	// 怪兽卡链接箭头
	enum class MonsterLinkMarker : std::uint8_t
	{
		NONE = 0,

		// ====================================

		TOP_LEFT = 1 << 0,
		TOP = 1 << 1,
		TOP_RIGHT = 1 << 2,

		LEFT = 1 << 3,
		RIGHT = 1 << 4,

		BOTTOM_LEFT = 1 << 5,
		BOTTOM = 1 << 6,
		BOTTOM_RIGHT = 1 << 7,
	};

	// 怪兽卡
	class MonsterCard : public Card
	{
	public:
		//

	protected:
		MonsterCategory monster_category_;
		MonsterAbility monster_ability_;
		MonsterAttribute monster_attribute_;
		MonsterRace monster_race_;

		// todo: 记录召唤素材?

		MonsterCard(
			MonsterCategory category,
			MonsterAbility ability,
			MonsterAttribute attribute,
			MonsterRace race
		) noexcept;

	public:
		// 获取怪兽卡类型
		[[nodiscard]] auto monster_category() const noexcept -> MonsterCategory;

		// 获取怪兽卡能力
		[[nodiscard]] auto monster_ability() const noexcept -> MonsterAbility;

		// 获取怪兽卡属性
		[[nodiscard]] auto monster_attribute() const noexcept -> MonsterAttribute;

		// 获取怪兽卡种族
		[[nodiscard]] auto monster_race() const noexcept -> MonsterRace;

		// 检查能否召唤
		[[nodiscard]] virtual auto can_summon(const Dual& dual, const Player& owner, const MonsterSummonMaterials& materials) const noexcept -> bool = 0;

		// 尝试召唤
		virtual auto try_summon(Dual& dual, Player& owner, MonsterSummonMaterials& materials) noexcept -> void = 0;
	};

	// 普通怪兽卡
	class NormalMonsterCard final : public MonsterCard
	{
	public:
		//

	private:
		MonsterLevel monster_level_;
		MonsterLevel monster_level_current_;

	public:
		// todo: 构造
		NormalMonsterCard(
			MonsterCategory category,
			MonsterAbility ability,
			MonsterAttribute attribute,
			MonsterRace race,
			MonsterLevel level
		) noexcept;

		auto can_activate(const Dual& dual, const Player& owner) const noexcept -> bool override;

		auto try_activate(Dual& dual, Player& owner) noexcept -> void override;

		auto can_summon(const Dual& dual, const Player& owner, const MonsterSummonMaterials& materials) const noexcept -> bool override;

		auto try_summon(Dual& dual, Player& owner, MonsterSummonMaterials& materials) noexcept -> void override;

		[[nodiscard]] auto level_origin() const noexcept -> MonsterLevel;

		[[nodiscard]] auto level_current() const noexcept -> MonsterLevel;

		auto set_level(MonsterLevel new_level) noexcept -> void;

		// todo: 其他接口
	};

	// 仪式怪兽卡
	class RitualMonsterCard final : public MonsterCard
	{
	public:
		//

	private:
		MonsterLevel monster_level_;
		MonsterLevel monster_level_current_;

	public:
		// todo: 构造
		RitualMonsterCard(
			MonsterCategory category,
			MonsterAbility ability,
			MonsterAttribute attribute,
			MonsterRace race,
			MonsterLevel level
		) noexcept;

		auto can_activate(const Dual& dual, const Player& owner) const noexcept -> bool override;

		auto try_activate(Dual& dual, Player& owner) noexcept -> void override;

		auto can_summon(const Dual& dual, const Player& owner, const MonsterSummonMaterials& materials) const noexcept -> bool override;

		auto try_summon(Dual& dual, Player& owner, MonsterSummonMaterials& materials) noexcept -> void override;

		[[nodiscard]] auto level_origin() const noexcept -> MonsterLevel;

		[[nodiscard]] auto level_current() const noexcept -> MonsterLevel;

		auto set_level(MonsterLevel new_level) noexcept -> void;

		// todo: 其他接口
	};

	// 融合怪兽卡
	class FusionMonsterCard final : public MonsterCard
	{
	public:
		//

	private:
		MonsterLevel monster_level_;
		MonsterLevel monster_level_current_;

	public:
		// todo: 其他接口
		// todo: 构造
		FusionMonsterCard(
			MonsterCategory category,
			MonsterAbility ability,
			MonsterAttribute attribute,
			MonsterRace race,
			MonsterLevel level
		) noexcept;

		auto can_activate(const Dual& dual, const Player& owner) const noexcept -> bool override;

		auto try_activate(Dual& dual, Player& owner) noexcept -> void override;

		auto can_summon(const Dual& dual, const Player& owner, const MonsterSummonMaterials& materials) const noexcept -> bool override;

		auto try_summon(Dual& dual, Player& owner, MonsterSummonMaterials& materials) noexcept -> void override;

		[[nodiscard]] auto level_origin() const noexcept -> MonsterLevel;

		[[nodiscard]] auto level_current() const noexcept -> MonsterLevel;

		auto set_level(MonsterLevel new_level) noexcept -> void;
	};

	// 同调怪兽卡
	class SynchroMonsterCard final : public MonsterCard
	{
	public:
		//

	private:
		MonsterLevel monster_level_;
		MonsterLevel monster_level_current_;

	public:
		// todo: 构造
		SynchroMonsterCard(
			MonsterCategory category,
			MonsterAbility ability,
			MonsterAttribute attribute,
			MonsterRace race,
			MonsterLevel level
		) noexcept;

		auto can_activate(const Dual& dual, const Player& owner) const noexcept -> bool override;

		auto try_activate(Dual& dual, Player& owner) noexcept -> void override;

		auto can_summon(const Dual& dual, const Player& owner, const MonsterSummonMaterials& materials) const noexcept -> bool override;

		auto try_summon(Dual& dual, Player& owner, MonsterSummonMaterials& materials) noexcept -> void override;

		[[nodiscard]] auto level_origin() const noexcept -> MonsterLevel;

		[[nodiscard]] auto level_current() const noexcept -> MonsterLevel;

		auto set_level(MonsterLevel new_level) noexcept -> void;

		// todo: 其他接口
	};

	// 超量怪兽卡
	class XyzMonsterCard final : public MonsterCard
	{
	public:
		//

	private:
		MonsterRank monster_rank_;

	public:
		// todo: 构造
		XyzMonsterCard(
			MonsterCategory category,
			MonsterAbility ability,
			MonsterAttribute attribute,
			MonsterRace race,
			MonsterRank rank
		) noexcept;

		auto can_activate(const Dual& dual, const Player& owner) const noexcept -> bool override;

		auto try_activate(Dual& dual, Player& owner) noexcept -> void override;

		auto can_summon(const Dual& dual, const Player& owner, const MonsterSummonMaterials& materials) const noexcept -> bool override;

		auto try_summon(Dual& dual, Player& owner, MonsterSummonMaterials& materials) noexcept -> void override;

		[[nodiscard]] auto rank() const noexcept -> MonsterRank;

		// todo: 其他接口
	};

	// 灵摆怪兽卡
	class PendulumMonsterCard final : public MonsterCard
	{
	public:
		//

	private:
		MonsterLevel monster_level_;
		MonsterLevel monster_level_current_;
		MonsterPendulum left_monster_pendulum_;
		MonsterPendulum right_monster_pendulum_;

	public:
		// todo: 构造
		PendulumMonsterCard(
			MonsterCategory category,
			MonsterAbility ability,
			MonsterAttribute attribute,
			MonsterRace race,
			MonsterLevel level,
			MonsterPendulum left_pendulum,
			MonsterPendulum right_pendulum
		) noexcept;

		auto can_activate(const Dual& dual, const Player& owner) const noexcept -> bool override;

		auto try_activate(Dual& dual, Player& owner) noexcept -> void override;

		auto can_summon(const Dual& dual, const Player& owner, const MonsterSummonMaterials& materials) const noexcept -> bool override;

		auto try_summon(Dual& dual, Player& owner, MonsterSummonMaterials& materials) noexcept -> void override;

		[[nodiscard]] auto level_origin() const noexcept -> MonsterLevel;

		[[nodiscard]] auto level_current() const noexcept -> MonsterLevel;

		auto set_level(MonsterLevel new_level) noexcept -> void;

		[[nodiscard]] auto left_pendulum() const noexcept -> MonsterPendulum;

		[[nodiscard]] auto right_pendulum() const noexcept -> MonsterPendulum;

		// todo: 其他接口
	};

	// 链接怪兽卡
	class LinkMonsterCard final : public MonsterCard
	{
	public:
		//

	private:
		MonsterLinkMarker monster_link_marker_;

	public:
		// todo: 构造
		LinkMonsterCard(
			MonsterCategory category,
			MonsterAbility ability,
			MonsterAttribute attribute,
			MonsterRace race,
			MonsterLinkMarker link_marker
		) noexcept;

		auto can_activate(const Dual& dual, const Player& owner) const noexcept -> bool override;

		auto try_activate(Dual& dual, Player& owner) noexcept -> void override;

		auto can_summon(const Dual& dual, const Player& owner, const MonsterSummonMaterials& materials) const noexcept -> bool override;

		auto try_summon(Dual& dual, Player& owner, MonsterSummonMaterials& materials) noexcept -> void override;

		[[nodiscard]] auto link_marker() const noexcept -> MonsterLinkMarker;

		// todo: 其他接口
	};
}
