#include <core/host/python.hpp>

#include <span>
#include <filesystem>

#include <core/engine/card.hpp>
#include <core/engine/effect.hpp>
#include <core/engine/api.hpp>

#include <spdlog/spdlog.h>

#include <pybind11/embed.h>
#include <pybind11/stl.h>
#include <pybind11/native_enum.h>

namespace py = pybind11;

namespace cg::host
{
	namespace
	{
		enum class PrototypeKind : std::uint8_t
		{
			MONSTER,
			RITUAL_MONSTER,
			FUSION_MONSTER,
			SYNCHRO_MONSTER,
			XYZ_MONSTER,
			PENDULUM_MONSTER,
			LINK_MONSTER,
			SPELL,
			TRAP,
		};

		// 获取原型的类型
		[[nodiscard]] auto detect_prototype_kind(const py::object& prototype) noexcept(false) -> std::optional<PrototypeKind>
		{
			const auto cls = prototype.attr("__class__");
			const auto mro = cls.attr("__mro__");

			for (const auto base: mro)
			{
				if (!py::hasattr(base, "_prototype_kind"))
				{
					continue;
				}

				return base.attr("_prototype_kind").cast<PrototypeKind>();
			}

			return std::nullopt;
		}

		// 原型必填字段名
		[[nodiscard]] auto required_field_names(const PrototypeKind kind) noexcept -> std::span<const std::string_view>
		{
			using std::string_view_literals::operator""sv;

			// 怪兽
			constexpr static std::array monster
			{
					// 卡牌类型
					"card_type"sv,
					// 属性
					"attribute"sv,
					// 种族
					"race"sv,
					// 等级
					"level"sv,
					// 攻击力
					"attack"sv,
					// 防御力
					"defense"sv,
			};
			// 超量怪兽
			constexpr static std::array xyz_monster
			{
					// 卡牌类型
					"card_type"sv,
					// 属性
					"attribute"sv,
					// 种族
					"race"sv,
					// 阶级
					"rank"sv,
					// 攻击力
					"attack"sv,
					// 防御力
					"defense"sv,
			};
			// 灵摆怪兽
			constexpr static std::array pendulum_monster
			{
					// 卡牌类型
					"card_type"sv,
					// 属性
					"attribute"sv,
					// 种族
					"race"sv,
					// 等级
					"level"sv,
					// 攻击力
					"attack"sv,
					// 防御力
					"defense"sv,
					// 灵摆刻度
					"left_pendulum"sv,
					"right_pendulum"sv,
			};
			// 连接怪兽
			constexpr static std::array link_monster
			{
					// 卡牌类型
					"card_type"sv,
					// 属性
					"attribute"sv,
					// 种族
					"race"sv,
					// 连接箭头
					"link_marker"sv,
					// 攻击力
					"attack"sv,
			};
			// 魔法陷阱
			constexpr static std::array spell_trap
			{
					// 卡牌类型
					"card_type"sv,
			};

			switch (kind)
			{
				// 怪兽
				case PrototypeKind::MONSTER:
				// 仪式怪兽
				case PrototypeKind::RITUAL_MONSTER:
				// 融合怪兽
				case PrototypeKind::FUSION_MONSTER:
				// 同调怪兽
				case PrototypeKind::SYNCHRO_MONSTER:
				{
					return monster;
				}
				// 超量怪兽
				case PrototypeKind::XYZ_MONSTER:
				{
					return xyz_monster;
				}
				// 灵摆怪兽
				case PrototypeKind::PENDULUM_MONSTER:
				{
					return pendulum_monster;
				}
				// 连接怪兽
				case PrototypeKind::LINK_MONSTER:
				{
					return link_monster;
				}
				case PrototypeKind::SPELL:
				case PrototypeKind::TRAP:
				{
					return {};
				}
				default: // NOLINT(clang-diagnostic-covered-switch-default)
				{
					std::unreachable();
				}
			}
		}

		// 原型选填字段名
		[[nodiscard]] auto optional_field_names(const PrototypeKind kind) noexcept -> std::span<const std::string_view>
		{
			using std::string_view_literals::operator""sv;

			// 怪兽
			constexpr static std::array monster
			{
					// 规范卡密 -- 可以没有规范卡密(原卡)
					"canonical_code"sv,
					// 字段 -- 可以没有字段
					"series"sv,
			};
			// 魔法陷阱
			// 无

			switch (kind)
			{
				case PrototypeKind::MONSTER:
				case PrototypeKind::RITUAL_MONSTER:
				case PrototypeKind::FUSION_MONSTER:
				case PrototypeKind::SYNCHRO_MONSTER:
				case PrototypeKind::XYZ_MONSTER:
				case PrototypeKind::PENDULUM_MONSTER:
				case PrototypeKind::LINK_MONSTER:
				{
					return monster;
				}
				case PrototypeKind::SPELL:
				case PrototypeKind::TRAP:
				{
					return {};
				}
				default: // NOLINT(clang-diagnostic-covered-switch-default)
				{
					std::unreachable();
				}
			}
		}

		// 获取原型的所有字段名
		[[nodiscard]] auto declared_field_names(const py::object& prototype) noexcept(false) -> std::vector<std::string>
		{
			std::vector<std::string> names{};

			const auto cls = prototype.attr("__class__");
			const auto dict = static_cast<py::dict>(cls.attr("__dict__"));

			for (const auto [key, value]: dict)
			{
				auto name = key.cast<std::string>();
				if (name.empty() || name.front() == '_')
				{
					continue;
				}

				if (py::isinstance<py::function>(value) || py::isinstance<py::type>(value))
				{
					continue;
				}

				names.push_back(std::move(name));
			}

			return names;
		}

		// 读取字段
		template<typename T>
		[[nodiscard]] auto read_field(const py::object& prototype, const std::string_view field) noexcept(false) -> T
		{
			return prototype.attr(field.data()).cast<T>(); // NOLINT(bugprone-suspicious-stringview-data-usage)
		}

		// 转换原型
		[[nodiscard]] auto build_prototype(engine::Prototype& out_prototype, const py::object& prototype, const std::vector<std::string>& fields) noexcept -> bool
		{
			try
			{
				for (const auto& field: fields)
				{
					// 规范卡密
					if (field == "canonical_code")
					{
						out_prototype.set_canonical_code(read_field<domain::CardCode>(prototype, field));
					}
					// 字段
					else if (field == "series")
					{
						const auto list = prototype.attr("series").cast<py::list>();
						if (list.size() >= engine::Prototype::max_series_count)
						{
							SPDLOG_ERROR("字段过多: 最多{}个,实际{}个", engine::Prototype::max_series_count, list.size());
							return false;
						}

						engine::Prototype::series_type series{};
						for (std::size_t i = 0; i < list.size(); ++i)
						{
							const auto& item = list[i];
							series[i] = item.cast<domain::SeriesCode>();
						}
					}
					// 卡牌类型
					else if (field == "card_type")
					{
						out_prototype.set_card_type(read_field<domain::CardType>(prototype, field));
					}
					// 属性
					else if (field == "attribute")
					{
						out_prototype.set_attribute(read_field<domain::Attribute>(prototype, field));
					}
					// 种族
					else if (field == "race")
					{
						out_prototype.set_race(read_field<domain::Race>(prototype, field));
					}
					// 等级
					else if (field == "level")
					{
						out_prototype.set_level(read_field<domain::Level>(prototype, field));
					}
					// 阶级
					else if (field == "rank")
					{
						out_prototype.set_rank(read_field<domain::Rank>(prototype, field));
					}
					// 链接箭头
					else if (field == "link_marker")
					{
						out_prototype.set_link_marker(read_field<domain::LinkMarker>(prototype, field));
					}
					// 攻击力
					else if (field == "attack")
					{
						out_prototype.set_attack(read_field<domain::attack_defense_value_type>(prototype, field));
					}
					// 防御力
					else if (field == "defense")
					{
						out_prototype.set_defense(read_field<domain::attack_defense_value_type>(prototype, field));
					}
					// 灵摆刻度
					else if (field == "left_pendulum")
					{
						out_prototype.set_left_pendulum(read_field<domain::PendulumScale>(prototype, field));
					}
					else if (field == "right_pendulum")
					{
						out_prototype.set_right_pendulum(read_field<domain::PendulumScale>(prototype, field));
					}
					else
					{
						// 这应该不可能发生
						SPDLOG_ERROR("未知字段: {}", field);
						return false;
					}
				}
			}
			catch (const py::error_already_set& e)
			{
				SPDLOG_ERROR("构建原型失败: {}", e.what());
				PyErr_Clear();
				return false;
			}

			return true;
		}

		// 校验数据范围
		[[nodiscard]] auto validate_ranges(const engine::Prototype& prototype, const PrototypeKind kind) noexcept -> bool
		{
			const auto validate_attack_defense = [&]() noexcept -> bool
			{
				const auto valid = [](const domain::attack_defense_value_type value) noexcept -> bool
				{
					return value >= 0 || value == domain::unknown_attack_defense_value;
				};

				return valid(prototype.attack()) && valid(prototype.defense());
			};
			const auto validate_level = [&prototype]() noexcept -> bool
			{
				const auto valid = [](const domain::Level l) noexcept -> bool
				{
					constexpr auto min = std::to_underlying(domain::Level::L1);
					constexpr auto max = std::to_underlying(domain::Level::L12);
					const auto current = std::to_underlying(l);

					return current >= min && current <= max;
				};

				return valid(prototype.level());
			};
			const auto validate_rank = [&prototype]() noexcept -> bool
			{
				const auto valid = [](const domain::Rank r) noexcept -> bool
				{
					constexpr auto min = std::to_underlying(domain::Rank::R1);
					constexpr auto max = std::to_underlying(domain::Rank::R12);
					const auto current = std::to_underlying(r);

					return current >= min && current <= max;
				};

				return valid(prototype.rank());
			};
			const auto validate_pendulum = [&prototype]() noexcept -> bool
			{
				const auto valid = [](const domain::PendulumScale s) noexcept -> bool
				{
					constexpr auto min = std::to_underlying(domain::PendulumScale::PS0);
					constexpr auto max = std::to_underlying(domain::PendulumScale::PS13);
					const auto current = std::to_underlying(s);

					return current >= min && current <= max;
				};

				return valid(prototype.left_pendulum()) && valid(prototype.right_pendulum());
			};

			switch (kind)
			{
				case PrototypeKind::MONSTER:
				case PrototypeKind::RITUAL_MONSTER:
				case PrototypeKind::FUSION_MONSTER:
				case PrototypeKind::SYNCHRO_MONSTER:
				{
					return validate_level() && validate_attack_defense();
				}
				case PrototypeKind::XYZ_MONSTER:
				{
					return validate_rank() && validate_attack_defense();
				}
				case PrototypeKind::PENDULUM_MONSTER:
				{
					return validate_pendulum() && validate_attack_defense();
				}
				case PrototypeKind::LINK_MONSTER:
				case PrototypeKind::SPELL:
				case PrototypeKind::TRAP:
				{
					return true;
				}
				default: // NOLINT(clang-diagnostic-covered-switch-default)
				{
					std::unreachable();
				}
			}
		}

		thread_local py::module_ g_module;
		thread_local engine::Api* g_api;
	}

	auto PythonHost::do_initialize(std::string_view script_root) noexcept -> bool
	{
		try
		{
			// 检查路径是否存在?
			if (!std::filesystem::exists(script_root))
			{
				SPDLOG_ERROR("脚本目录 {} 不存在!", script_root);
				return false;
			}

			// 将脚本目录添加到 Python 模块搜索路径中
			const auto sys = py::module_::import("sys");
			const auto path = sys.attr("path");
			// ReSharper disable once CppExpressionWithoutSideEffects
			path.attr("append")(script_root);

			// 下面的PYBIND11_EMBEDDED_MODULE(_cg, m)
			g_module = py::module_::import("_cg");
		}
		catch (const py::error_already_set& e)
		{
			SPDLOG_ERROR("Python宿主初始化失败: {}", e.what());
			return false;
		}

		return true;
	}

	auto PythonHost::do_bind_api(engine::Api& api) noexcept -> void
	{
		g_api = &api;
	}

	auto PythonHost::do_load_prototype(const domain::CardCode code) noexcept -> std::optional<engine::Prototype>
	{
		// 拼接脚本文件名
		const auto script_filename = std::format("c{}", std::to_underlying(code));

		try
		{
			// 载入脚本
			const auto script_module = py::module_::import(script_filename.c_str());
			// 获取原型数据
			const auto prototype_data = script_module.attr("Prototype")();
			// 获取原型类型
			const auto kind = detect_prototype_kind(prototype_data);

			if (!kind.has_value())
			{
				// 非规范脚本格式,拒绝载入
				return std::nullopt;
			}

			// 必选字段
			const auto required = required_field_names(*kind);
			// 已检查必选项
			std::vector<std::size_t> required_check{};
			required_check.reserve(required.size());
			// 选填
			const auto optional = optional_field_names(*kind);
			// 获取原型字段
			const auto fields = declared_field_names(prototype_data);

			for (const auto& field: fields)
			{
				if (const auto it = std::ranges::find(required, field);
					it != required.end())
				{
					// 是必选字段
					const auto index = std::ranges::distance(required.begin(), it);
					required_check.push_back(index);
					continue;
				}

				if (const auto it = std::ranges::find(optional, field);
					it != optional.end())
				{
					// 是选填字段
					continue;
				}

				// 未知字段
				SPDLOG_WARN("脚本{}包含未知字段: {}", script_filename, field);
			}

			// 所有必选字段都必须存在
			if (required_check.size() != required.size())
			{
				SPDLOG_ERROR("脚本{}缺少必选字段: ", script_filename);

				// 检查下标先排序
				std::ranges::sort(required_check);
				// 打印缺少的必选字段
				for (std::size_t check = 0, i = 0; i < required.size(); ++i)
				{
					if (i == required_check[check])
					{
						check += 1;
					}
					else
					{
						SPDLOG_ERROR("    - {}", required[i]);
					}
				}

				return std::nullopt;
			}

			// 构建原型
			engine::Prototype prototype{code};
			if (!build_prototype(prototype, prototype_data, fields))
			{
				return std::nullopt;
			}

			// 校验数值
			if (!validate_ranges(prototype, *kind))
			{
				return std::nullopt;
			}

			// 返回原型
			return prototype;
		}
		catch (const py::error_already_set& e)
		{
			SPDLOG_ERROR("载入原型[{}]的脚本{}失败: {}", std::to_underlying(code), script_filename, e.what());
			PyErr_Clear();
			return std::nullopt;
		}
	}

	PythonHost::PythonHost() noexcept
	{
		try
		{
			py::initialize_interpreter();
		}
		catch (const std::exception& e)
		{
			SPDLOG_ERROR("初始化Python解释器失败: {}", e.what());
		}
	}

	PythonHost::~PythonHost() noexcept
	{
		g_module = {};
		g_api = nullptr;

		py::finalize_interpreter();
	}
}

// ==============================================================================

namespace
{
	using namespace cg;

	auto bind_prototype_kind(const py::module_& m) noexcept -> void
	{
		using host::PrototypeKind;

		auto e = py::native_enum<PrototypeKind>(m, "PrototypeKind", "enum.Enum");

		e //
				.value("MONSTER", PrototypeKind::MONSTER) //
				.value("RITUAL_MONSTER", PrototypeKind::RITUAL_MONSTER) //
				.value("FUSION_MONSTER", PrototypeKind::FUSION_MONSTER) //
				.value("SYNCHRO_MONSTER", PrototypeKind::SYNCHRO_MONSTER) //
				.value("XYZ_MONSTER", PrototypeKind::XYZ_MONSTER) //
				.value("PENDULUM_MONSTER", PrototypeKind::PENDULUM_MONSTER) //
				.value("LINK_MONSTER", PrototypeKind::LINK_MONSTER) //
				.value("SPELL", PrototypeKind::SPELL) //
				.value("TRAP", PrototypeKind::TRAP) //
				;

		e.finalize();
	}

	auto bind_card_type(const py::module_& m) noexcept -> void
	{
		using domain::CardType;

		auto e = py::native_enum<CardType>(m, "CardType", "enum.Flag");

		e //
				.value("NONE", CardType::NONE) //
				.value("MONSTER", CardType::MONSTER) //
				.value("SPELL", CardType::SPELL) //
				.value("TRAP", CardType::TRAP) //
				.value("FLIP_SUMMON", CardType::FLIP_SUMMON) //
				.value("SPECIAL_SUMMON", CardType::SPECIAL_SUMMON) //
				.value("NORMAL", CardType::NORMAL) //
				.value("EFFECT", CardType::EFFECT) //
				.value("FUSION", CardType::FUSION) //
				.value("RITUAL", CardType::RITUAL) //
				.value("SYNCHRO", CardType::SYNCHRO) //
				.value("XYZ", CardType::XYZ) //
				.value("PENDULUM", CardType::PENDULUM) //
				.value("LINK", CardType::LINK) //
				.value("TRAP_MONSTER", CardType::TRAP_MONSTER) //
				.value("TOKEN_MONSTER", CardType::TOKEN_MONSTER) //
				.value("SPIRIT", CardType::SPIRIT) //
				.value("UNION", CardType::UNION) //
				.value("DUAL", CardType::DUAL) //
				.value("TUNER", CardType::TUNER) //
				.value("CARTOON", CardType::CARTOON) //
				.value("CONTINUOUS", CardType::CONTINUOUS) //
				.value("QUICK_PLAY", CardType::QUICK_PLAY) //
				.value("EQUIP", CardType::EQUIP) //
				.value("FIELD", CardType::FIELD) //
				.value("COUNTER", CardType::COUNTER) //
				;

		e.finalize();
	}

	auto bind_attribute(const py::module_& m) noexcept -> void
	{
		using domain::Attribute;

		auto e = py::native_enum<Attribute>(m, "Attribute", "enum.Enum");

		e //
				.value("DARK", Attribute::DARK) //
				.value("LIGHT", Attribute::LIGHT) //
				.value("EARTH", Attribute::EARTH) //
				.value("WATER", Attribute::WATER) //
				.value("FIRE", Attribute::FIRE) //
				.value("WIND", Attribute::WIND) //
				.value("DIVINE", Attribute::DIVINE) //
				;

		e.finalize();
	}

	auto bind_race(const py::module_& m) noexcept -> void
	{
		using domain::Race;

		auto e = py::native_enum<Race>(m, "Race", "enum.Enum");

		e //
				.value("WARRIOR", Race::WARRIOR) //
				.value("SPELLCASTER", Race::SPELLCASTER) //
				.value("DRAGON", Race::DRAGON) //
				.value("FAIRY", Race::FAIRY) //
				.value("FIEND", Race::FIEND) //
				.value("ZOMBIE", Race::ZOMBIE) //
				.value("MACHINE", Race::MACHINE) //
				.value("AQUA", Race::AQUA) //
				.value("PYRO", Race::PYRO) //
				.value("ROCK", Race::ROCK) //
				.value("WINGED_BEAST", Race::WINGED_BEAST) //
				.value("PLANT", Race::PLANT) //
				.value("INSECT", Race::INSECT) //
				.value("THUNDER", Race::THUNDER) //
				.value("BEAST", Race::BEAST) //
				.value("BEAST_WARRIOR", Race::BEAST_WARRIOR) //
				.value("DINOSAUR", Race::DINOSAUR) //
				.value("FISH", Race::FISH) //
				.value("SEA_SERPENT", Race::SEA_SERPENT) //
				.value("REPTILE", Race::REPTILE) //
				.value("PSYCHIC", Race::PSYCHIC) //
				.value("DIVINE_BEAST", Race::DIVINE_BEAST) //
				.value("CYBERSE", Race::CYBERSE) //
				.value("WYRM", Race::WYRM) //
				.value("ILLUSION", Race::ILLUSION) //
				;

		e.finalize();
	}

	auto bind_level(const py::module_& m) noexcept -> void
	{
		using domain::Level;

		auto e = py::native_enum<Level>(m, "Level", "enum.Enum");

		e //
				.value("L1", Level::L1) //
				.value("L2", Level::L2) //
				.value("L3", Level::L3) //
				.value("L4", Level::L4) //
				.value("L5", Level::L5) //
				.value("L6", Level::L6) //
				.value("L7", Level::L7) //
				.value("L8", Level::L8) //
				.value("L9", Level::L9) //
				.value("L10", Level::L10) //
				.value("L11", Level::L11) //
				.value("L12", Level::L12) //
				;

		e.finalize();
	}

	auto bind_rank(const py::module_& m) noexcept -> void
	{
		using domain::Rank;

		auto e = py::native_enum<Rank>(m, "Rank", "enum.Enum");

		e //
				.value("R1", Rank::R1) //
				.value("R2", Rank::R2) //
				.value("R3", Rank::R3) //
				.value("R4", Rank::R4) //
				.value("R5", Rank::R5) //
				.value("R6", Rank::R6) //
				.value("R7", Rank::R7) //
				.value("R8", Rank::R8) //
				.value("R9", Rank::R9) //
				.value("R10", Rank::R10) //
				.value("R11", Rank::R11) //
				.value("R12", Rank::R12) //
				;

		e.finalize();
	}

	auto bind_link_marker(const py::module_& m) noexcept -> void
	{
		using domain::LinkMarker;

		auto e = py::native_enum<LinkMarker>(m, "LinkMarker", "enum.Flag");

		e //
				.value("TOP_LEFT", LinkMarker::TOP_LEFT) //
				.value("TOP", LinkMarker::TOP) //
				.value("TOP_RIGHT", LinkMarker::TOP_RIGHT) //
				.value("LEFT", LinkMarker::LEFT) //
				.value("RIGHT", LinkMarker::RIGHT) //
				.value("BOTTOM_LEFT", LinkMarker::BOTTOM_LEFT) //
				.value("BOTTOM", LinkMarker::BOTTOM) //
				.value("BOTTOM_RIGHT", LinkMarker::BOTTOM_RIGHT) //
				;

		e.finalize();
	}

	auto bind_pendulum_scale(const py::module_& m) noexcept -> void
	{
		using domain::PendulumScale;

		auto e = py::native_enum<PendulumScale>(m, "PendulumScale", "enum.Enum");

		e //
				.value("PS0", PendulumScale::PS0) //
				.value("PS1", PendulumScale::PS1) //
				.value("PS2", PendulumScale::PS2) //
				.value("PS3", PendulumScale::PS3) //
				.value("PS4", PendulumScale::PS4) //
				.value("PS5", PendulumScale::PS5) //
				.value("PS6", PendulumScale::PS6) //
				.value("PS7", PendulumScale::PS7) //
				.value("PS8", PendulumScale::PS8) //
				.value("PS9", PendulumScale::PS9) //
				.value("PS10", PendulumScale::PS10) //
				.value("PS11", PendulumScale::PS11) //
				.value("PS12", PendulumScale::PS12) //
				.value("PS13", PendulumScale::PS13) //
				;

		e.finalize();
	}

	auto bind_duel_type(py::module_& m) noexcept -> void
	{
		auto duel = m.def_submodule("Duel");

		// 转发给API
		// todo: 等待API完工
	}
}

PYBIND11_EMBEDDED_MODULE(_cg, m)
{
	m.doc() = "Python bindings for cg";

	// 原型类型
	bind_prototype_kind(m);

	// 卡类型
	bind_card_type(m);
	// 属性
	bind_attribute(m);
	// 种族
	bind_race(m);
	// 等级
	bind_level(m);
	// 阶级
	bind_rank(m);
	// 连接箭头
	bind_link_marker(m);
	// 灵摆刻度
	bind_pendulum_scale(m);

	// Duel
	bind_duel_type(m);
}
