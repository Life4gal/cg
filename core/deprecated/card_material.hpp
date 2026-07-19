#pragma once

#include <memory>
#include <vector>

namespace cg
{
	class Card;

	// 召唤材料
	class SummonMaterialCard
	{
	public:
		using card_type = std::shared_ptr<Card>;

	private:
		card_type card_;
		// todo: 其他信息(例如位置信息)

	public:
		explicit SummonMaterialCard(card_type card) noexcept;

		SummonMaterialCard(const SummonMaterialCard&) noexcept = delete;
		auto operator=(const SummonMaterialCard&) noexcept -> SummonMaterialCard& = delete;

		SummonMaterialCard(SummonMaterialCard&&) noexcept;
		auto operator=(SummonMaterialCard&&) noexcept -> SummonMaterialCard&;

		~SummonMaterialCard() noexcept;

		// todo: 其他接口
	};

	// 召唤材料
	class SummonMaterialCards
	{
	public:
		using materials_type = std::vector<SummonMaterialCard>;

		using size_type = materials_type::size_type;

	private:
		materials_type materials_;

	public:
		SummonMaterialCards() noexcept;

		auto reserve(size_type capacity) noexcept -> void;

		auto add(SummonMaterialCard&& material) noexcept -> void;

		[[nodiscard]] auto size() const noexcept -> size_type;
	};

	// 超量素材
	class XyzMaterialCard
	{
	public:
		using card_type = std::shared_ptr<Card>;

	private:
		card_type card_;
		// todo: 其他信息

	public:
		explicit XyzMaterialCard(card_type card) noexcept;

		XyzMaterialCard(const XyzMaterialCard&) noexcept = delete;
		auto operator=(const XyzMaterialCard&) noexcept -> XyzMaterialCard& = delete;

		XyzMaterialCard(XyzMaterialCard&&) noexcept;
		auto operator=(XyzMaterialCard&&) noexcept -> XyzMaterialCard&;

		~XyzMaterialCard() noexcept;

		// todo: 其他接口
	};

	// 超量素材
	class XyzMaterialCards
	{
	public:
		using materials_type = std::vector<XyzMaterialCard>;

		using size_type = materials_type::size_type;

	private:
		materials_type materials_;

	public:
		XyzMaterialCards() noexcept;

		auto reserve(size_type capacity) noexcept -> void;

		auto add(XyzMaterialCard&& material) noexcept -> void;

		auto remove(size_type index) noexcept -> bool;

		[[nodiscard]] auto size() const noexcept -> size_type;
	};
}
