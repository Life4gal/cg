#pragma once

#include <filesystem>

namespace cg::core
{
	class Card;
	class View;
	class Effect;

	class Bridge
	{
	public:
		Bridge() noexcept;

		Bridge(const Bridge&) noexcept = delete;
		auto operator=(const Bridge&) noexcept -> Bridge& = delete;

		Bridge(Bridge&&) noexcept;
		auto operator=(Bridge&&) noexcept -> Bridge&;

		virtual ~Bridge() noexcept;

		// 载入脚本
		[[nodiscard]] auto load_script(const std::filesystem::path& script_path) noexcept -> bool;

		// 注册卡片,返回注册是否成功
		auto register_card(const Card& card) noexcept -> bool;
		// 移除注册的卡片,返回移除是否成功
		auto unregister_card(const Card& card) noexcept -> bool;
		// 注册视图,返回注册是否成功
		auto register_view(const View& view) noexcept -> bool;
		// 移除注册的视图,返回移除是否成功
		auto unregister_view(const View& view) noexcept -> bool;
		// 注册效果,返回注册是否成功
		auto register_effect(const Effect& effect) noexcept -> bool;
		// 移除注册的效果,返回移除是否成功
		auto unregister_effect(const Effect& effect) noexcept -> bool;

	private:
		// 载入脚本
		[[nodiscard]] virtual auto do_load_script(const std::filesystem::path& script_path) noexcept -> bool = 0;

		// 注册卡片,返回注册是否成功
		virtual auto do_register_card(const Card& card) noexcept -> bool = 0;
		// 移除注册的卡片,返回移除是否成功
		virtual auto do_unregister_card(const Card& card) noexcept -> bool = 0;
		// 注册视图,返回注册是否成功
		virtual auto do_register_view(const View& view) noexcept -> bool = 0;
		// 移除注册的视图,返回移除是否成功
		virtual auto do_unregister_view(const View& view) noexcept -> bool = 0;
		// 注册效果,返回注册是否成功
		virtual auto do_register_effect(const Effect& effect) noexcept -> bool = 0;
		// 移除注册的效果,返回移除是否成功
		virtual auto do_unregister_effect(const Effect& effect) noexcept -> bool = 0;
	};
}
