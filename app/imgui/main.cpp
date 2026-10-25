#include <algorithm>
#include <array>
#include <atomic>
#include <charconv>
#include <condition_variable>
#include <cstdio>
#include <filesystem>
#include <format>
#include <memory>
#include <mutex>
#include <numeric>
#include <optional>
#include <print>
#include <ranges>
#include <string>
#include <string_view>
#include <thread>
#include <utility>
#include <vector>

#include <cg/driver/local_session.hpp>
#include <cg/server/match_config.hpp>

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

namespace
{
	using namespace cg;

	[[nodiscard]] auto id_text(const state::CardInstanceId id) -> std::string
	{
		return std::format("#{}", id.raw());
	}

	[[nodiscard]] auto card_text(const driver::PlayerView& view, const state::CardInstanceId id) -> std::string
	{
		if (const auto it = view.card_codes.find(id.raw()); it != view.card_codes.end())
		{
			return std::format("{} [{}]", id_text(id), std::to_underlying(it->second));
		}

		return id_text(id);
	}

	[[nodiscard]] auto player_text(const domain::Player player) -> std::string_view
	{
		return utility::to_string(player);
	}

	// UI 线程只读取快照; 对局线程通过 condition_variable 等待玩家决定.
	class ImGuiController final : public driver::LocalController
	{
	public:
		class CardSelection
		{
		public:
			std::vector<state::CardInstanceId> candidates;
			std::size_t min{};
			std::size_t max{};
			std::vector<std::size_t> selected;
			bool confirmed{};
		};

		class LocationSelection
		{
		public:
			bool spell_trap{};
			std::vector<domain::FieldColumn> columns;
			std::optional<std::size_t> selected;
			bool confirmed{};
		};

		class TriggerSelection
		{
		public:
			class Candidate
			{
			public:
				state::CardInstanceId source;
			};

			std::vector<Candidate> candidates;
			bool optional{};
			std::vector<std::size_t> selected;
			bool confirmed{};
		};

		class Snapshot
		{
		public:
			std::optional<driver::PlayerView> view;
			std::optional<CardSelection> cards;
			std::optional<LocationSelection> locations;
			std::optional<TriggerSelection> triggers;
			bool waiting{};
			bool action_queued{};
			std::optional<std::string> error;
		};

	private:
		std::string name_;
		domain::Player player_;

		mutable std::mutex mutex_;
		std::condition_variable condition_;
		std::optional<driver::PlayerView> view_;
		std::optional<driver::LocalAction> action_;
		std::optional<CardSelection> cards_;
		std::optional<LocationSelection> locations_;
		std::optional<TriggerSelection> triggers_;
		std::optional<std::string> error_;
		std::size_t submitted_actions_{};
		bool waiting_{};
		bool shutdown_{};

	public:
		ImGuiController(std::string name, const domain::Player player)
			: name_{std::move(name)},
			  player_{player} {}

		ImGuiController(const ImGuiController&) = delete;
		auto operator=(const ImGuiController&) -> ImGuiController& = delete;
		ImGuiController(ImGuiController&&) = delete;
		auto operator=(ImGuiController&&) -> ImGuiController& = delete;

		~ImGuiController() override
		{
			shutdown();
		}

		[[nodiscard]] auto name() const noexcept -> std::string_view
		{
			return name_;
		}

		[[nodiscard]] auto player() const noexcept -> domain::Player
		{
			return player_;
		}

		[[nodiscard]] auto snapshot() const -> Snapshot
		{
			std::scoped_lock lock{mutex_};
			return
			{
					.view = view_,
					.cards = cards_,
					.locations = locations_,
					.triggers = triggers_,
					.waiting = waiting_,
					.action_queued = action_.has_value(),
					.error = error_,
			};
		}

		[[nodiscard]] auto submit(driver::LocalAction action) -> bool
		{
			std::scoped_lock lock{mutex_};
			if (
				!waiting_ ||
				cards_.has_value() ||
				locations_.has_value() ||
				triggers_.has_value() ||
				action_.has_value() ||
				shutdown_
			)
			{
				return false;
			}

			action_ = std::move(action);
			submitted_actions_ += 1;
			condition_.notify_one();
			return true;
		}

		[[nodiscard]] auto submitted_actions() const noexcept -> std::size_t
		{
			std::scoped_lock lock{mutex_};
			return submitted_actions_;
		}

		auto toggle_card(const std::size_t index) noexcept -> void
		{
			std::scoped_lock lock{mutex_};
			if (!cards_.has_value() || cards_->confirmed || shutdown_ || index >= cards_->candidates.size())
			{
				return;
			}

			auto& selected = cards_->selected;
			if (const auto it = std::ranges::find(selected, index); it != selected.end())
			{
				selected.erase(it);
			}
			else if (selected.size() < cards_->max)
			{
				selected.push_back(index);
			}
		}

		[[nodiscard]] auto confirm_cards() noexcept -> bool
		{
			std::scoped_lock lock{mutex_};
			if (!cards_.has_value() || cards_->confirmed || shutdown_)
			{
				return false;
			}
			if (cards_->selected.size() < cards_->min || cards_->selected.size() > cards_->max)
			{
				return false;
			}

			cards_->confirmed = true;
			condition_.notify_all();
			return true;
		}

		[[nodiscard]] auto confirm_location(const std::size_t index) noexcept -> bool
		{
			std::scoped_lock lock{mutex_};
			if (!locations_.has_value() || locations_->confirmed || shutdown_ || index >= locations_->columns.size())
			{
				return false;
			}

			locations_->selected = index;
			locations_->confirmed = true;
			condition_.notify_all();
			return true;
		}

		auto toggle_trigger(const std::size_t index) noexcept -> void
		{
			std::scoped_lock lock{mutex_};
			if (
				!triggers_.has_value() ||
				!triggers_->optional ||
				triggers_->confirmed ||
				shutdown_ ||
				index >= triggers_->candidates.size()
			)
			{
				return;
			}

			auto& selected = triggers_->selected;
			if (const auto it = std::ranges::find(selected, index); it != selected.end())
			{
				selected.erase(it);
			}
			else
			{
				selected.push_back(index);
			}
		}

		auto move_trigger(const std::size_t index, const int delta) noexcept -> void
		{
			std::scoped_lock lock{mutex_};
			if (!triggers_.has_value() || triggers_->confirmed || shutdown_)
			{
				return;
			}

			auto& selected = triggers_->selected;
			const auto it = std::ranges::find(selected, index);
			if (it == selected.end())
			{
				return;
			}

			const auto position = static_cast<std::ptrdiff_t>(it - selected.begin());
			if (delta < 0 && position > 0)
			{
				std::swap(selected[static_cast<std::size_t>(position)], selected[static_cast<std::size_t>(position - 1)]);
			}
			else if (delta > 0 && position + 1 < static_cast<std::ptrdiff_t>(selected.size()))
			{
				std::swap(selected[static_cast<std::size_t>(position)], selected[static_cast<std::size_t>(position + 1)]);
			}
		}

		[[nodiscard]] auto confirm_triggers() noexcept -> bool
		{
			std::scoped_lock lock{mutex_};
			if (!triggers_.has_value() || triggers_->confirmed || shutdown_)
			{
				return false;
			}

			if (!triggers_->optional)
			{
				if (triggers_->selected.size() != triggers_->candidates.size())
				{
					return false;
				}

				for (std::size_t index = 0; index < triggers_->candidates.size(); ++index)
				{
					if (!std::ranges::contains(triggers_->selected, index))
					{
						return false;
					}
				}
			}

			triggers_->confirmed = true;
			condition_.notify_all();
			return true;
		}

		auto set_error(std::string message) -> void
		{
			std::scoped_lock lock{mutex_};
			error_ = std::move(message);
		}

		auto shutdown() noexcept -> void
		{
			{
				std::scoped_lock lock{mutex_};
				shutdown_ = true;
			}
			condition_.notify_all();
		}

		[[nodiscard]] auto choose_action(const driver::PlayerView& view) -> std::optional<driver::LocalAction> override
		{
			std::unique_lock lock{mutex_};
			view_ = view;
			action_.reset();
			error_.reset();
			waiting_ = true;
			condition_.notify_all();

			condition_.wait(lock,
			                [this]() noexcept
			                {
				                return shutdown_ || action_.has_value();
			                });

			waiting_ = false;
			if (shutdown_)
			{
				return std::nullopt;
			}

			auto action = std::move(action_);
			action_.reset();
			return action;
		}

		[[nodiscard]] auto select_cards(
			[[maybe_unused]] domain::Player player,
			const execution::Group& candidates,
			const std::size_t min,
			const std::size_t max
		) noexcept -> execution::Group override
		{
			std::unique_lock lock{mutex_};
			if (shutdown_)
			{
				return {};
			}

			cards_ = CardSelection
			{
					.candidates = std::vector(candidates.begin(), candidates.end()),
					.min = min,
					.max = max,
					.selected = {},
					.confirmed = false,
			};
			waiting_ = true;
			condition_.notify_all();
			condition_.wait(
				lock,
				[this]() noexcept
				{
					return shutdown_ || (cards_.has_value() && cards_->confirmed);
				}
			);

			if (shutdown_)
			{
				cards_.reset();
				waiting_ = false;
				return {};
			}

			auto card_selection = std::move(*cards_);
			cards_.reset();
			waiting_ = false;

			execution::Group result{};
			for (const auto index: card_selection.selected)
			{
				if (index < candidates.size())
				{
					result.push_back(*(candidates.begin() + static_cast<std::ptrdiff_t>(index)));
				}
			}
			return result;
		}

		[[nodiscard]] auto select_monster_location(
			[[maybe_unused]] domain::Player player,
			[[maybe_unused]] state::CardInstanceId card,
			const std::span<const domain::MonsterLocation> candidates
		) noexcept -> std::optional<domain::MonsterLocation> override
		{
			if (candidates.empty())
			{
				return std::nullopt;
			}

			std::unique_lock lock{mutex_};
			if (shutdown_)
			{
				return std::nullopt;
			}

			LocationSelection selection;
			selection.spell_trap = false;
			selection.columns.reserve(candidates.size());
			for (const auto& candidate: candidates)
			{
				selection.columns.push_back(candidate.column);
			}

			locations_ = std::move(selection);
			waiting_ = true;
			condition_.notify_all();
			condition_.wait(
				lock,
				[this]() noexcept
				{
					return shutdown_ || (locations_.has_value() && locations_->confirmed);
				}
			);

			if (shutdown_)
			{
				locations_.reset();
				waiting_ = false;
				return std::nullopt;
			}

			auto location = std::move(*locations_);
			locations_.reset();
			waiting_ = false;
			if (!location.selected.has_value() || *location.selected >= candidates.size())
			{
				return std::nullopt;
			}

			return candidates[*location.selected];
		}

		[[nodiscard]] auto select_spell_trap_location(
			[[maybe_unused]] domain::Player player,
			[[maybe_unused]] state::CardInstanceId card,
			const std::span<const domain::SpellTrapLocation> candidates
		) noexcept -> std::optional<domain::SpellTrapLocation> override
		{
			if (candidates.empty())
			{
				return std::nullopt;
			}

			std::unique_lock lock{mutex_};
			if (shutdown_)
			{
				return std::nullopt;
			}

			LocationSelection selection;
			selection.spell_trap = true;
			selection.columns.reserve(candidates.size());
			for (const auto& candidate: candidates)
			{
				selection.columns.push_back(candidate.column);
			}

			locations_ = std::move(selection);
			waiting_ = true;
			condition_.notify_all();
			condition_.wait(
				lock,
				[this]() noexcept
				{
					return shutdown_ || (locations_.has_value() && locations_->confirmed);
				}
			);

			if (shutdown_)
			{
				locations_.reset();
				waiting_ = false;
				return std::nullopt;
			}

			auto location = std::move(*locations_);
			locations_.reset();
			waiting_ = false;
			if (!location.selected.has_value() || *location.selected >= candidates.size())
			{
				return std::nullopt;
			}

			return candidates[*location.selected];
		}

		[[nodiscard]] auto choose_triggers(const execution::TriggerDecision& decision) noexcept -> std::vector<std::size_t> override
		{
			if (decision.candidates.empty())
			{
				return {};
			}

			std::unique_lock lock{mutex_};
			if (shutdown_)
			{
				return {};
			}

			TriggerSelection selection;
			selection.optional = decision.optional;
			selection.candidates.reserve(decision.candidates.size());
			for (const auto& candidate: decision.candidates)
			{
				selection.candidates.push_back(TriggerSelection::Candidate{.source = candidate.source});
			}
			if (!decision.optional)
			{
				for (std::size_t index = 0; index < selection.candidates.size(); ++index)
				{
					selection.selected.push_back(index);
				}
			}

			triggers_ = std::move(selection);
			waiting_ = true;
			condition_.notify_all();
			condition_.wait(
				lock,
				[this]() noexcept
				{
					return shutdown_ || (triggers_.has_value() && triggers_->confirmed);
				}
			);

			if (shutdown_)
			{
				triggers_.reset();
				waiting_ = false;
				return {};
			}

			auto trigger_selection = std::move(*triggers_);
			triggers_.reset();
			waiting_ = false;
			return trigger_selection.selected;
		}
	};

	// LocalDuelSession 在独立线程里跑; controller 阻塞等待 UI 提交动作.
	class SessionRunner
	{
	public:
		using controllers_type = std::array<ImGuiController*, 2>;

	private:
		driver::LocalDuelSession& session_;
		controllers_type controllers_;
		std::thread worker_;
		std::atomic<bool> running_{};
		std::atomic<bool> finished_{};

		mutable std::mutex status_mutex_;
		std::string status_{"duel running"};

		auto set_status(std::string status) -> void
		{
			std::scoped_lock lock{status_mutex_};
			status_ = std::move(status);
		}

		auto loop() -> void
		{
			while (running_.load(std::memory_order_acquire))
			{
				const auto result = session_.run();
				if (!running_.load(std::memory_order_acquire))
				{
					break;
				}

				switch (result.status)
				{
					case driver::LocalDuelSession::Status::MATCH_OVER:
					{
						set_status(
							result.winner.has_value()
								? std::format("match over: {} wins", player_text(*result.winner))
								: std::string{"match over"}
						);
						finished_.store(true, std::memory_order_release);
						return;
					}
					case driver::LocalDuelSession::Status::NO_ACTION:
					{
						set_status(std::format("duel paused: {}", result.message));
						finished_.store(true, std::memory_order_release);
						return;
					}
					case driver::LocalDuelSession::Status::INVALID_ACTION:
					{
						set_status(std::format("action rejected: {}", result.message));
						for (auto* controller: controllers_)
						{
							controller->set_error(result.message);
						}
						break;
					}
					case driver::LocalDuelSession::Status::STEP_LIMIT:
					{
						set_status("step limit reached");
						finished_.store(true, std::memory_order_release);
						return;
					}
				}
			}
		}

	public:
		SessionRunner(driver::LocalDuelSession& session, const controllers_type controllers) noexcept
			: session_{session},
			  controllers_{controllers} {}

		SessionRunner(const SessionRunner&) = delete;
		auto operator=(const SessionRunner&) -> SessionRunner& = delete;
		SessionRunner(SessionRunner&&) = delete;
		auto operator=(SessionRunner&&) -> SessionRunner& = delete;

		~SessionRunner()
		{
			stop();
		}

		auto start() -> void
		{
			running_.store(true, std::memory_order_release);
			worker_ = std::thread{[this]()
			{
				loop();
			}};
		}

		auto stop() noexcept -> void
		{
			const auto was_running = running_.exchange(false, std::memory_order_acq_rel);
			for (auto* controller: controllers_)
			{
				controller->shutdown();
			}

			if (worker_.joinable())
			{
				worker_.join();
			}

			if (was_running && !finished_.load(std::memory_order_acquire))
			{
				set_status("session stopped");
			}
		}

		[[nodiscard]] auto finished() const noexcept -> bool
		{
			return finished_.load(std::memory_order_acquire);
		}

		[[nodiscard]] auto status() const -> std::string
		{
			std::scoped_lock lock{status_mutex_};
			return status_;
		}
	};

	class PlayerWindow
	{
		ImGuiController& controller_;
		std::string title_;
		GLFWwindow* window_{};
		ImGuiContext* context_{};
		int column_{};
		std::optional<state::CardInstanceId> tribute_card_;
		std::vector<state::CardInstanceId> tribute_cards_;
		bool shutdown_{};

		auto render_header(const driver::PlayerView& view) -> void
		{
			ImGui::Text(
				"Turn %llu  |  %s  |  LP %d - %d",
				static_cast<unsigned long long>(view.turn_id),
				std::string{utility::to_string(view.phase)}.c_str(),
				static_cast<int>(view.life_points[0]),
				static_cast<int>(view.life_points[1])
			);
			ImGui::SameLine();
			ImGui::TextDisabled(
				"| priority %s%s%s",
				std::string{player_text(view.priority_player)}.c_str(),
				view.pending_summon ? " | summon window" : "",
				view.pending_attack ? " | attack window" : ""
			);
		}

		static auto render_cards(
			const driver::PlayerView& view,
			const char* label,
			const std::vector<state::CardInstanceId>& cards,
			const bool show_empty
		) -> void
		{
			ImGui::TextDisabled("%s (%zu)", label, cards.size());
			if (cards.empty())
			{
				if (show_empty)
				{
					ImGui::TextDisabled("  (empty)");
				}
				return;
			}

			for (const auto card: cards)
			{
				ImGui::BulletText("%s", card_text(view, card).c_str());
			}
		}

		static auto submit_button(const char* label, ImGuiController& controller, driver::LocalAction action, const bool disabled) -> bool
		{
			ImGui::BeginDisabled(disabled);
			const auto pressed = ImGui::SmallButton(label);
			ImGui::EndDisabled();
			if (pressed)
			{
				return controller.submit(std::move(action));
			}
			return false;
		}

		auto render_tribute_panel(const driver::PlayerView& view, const bool queued) -> void
		{
			if (!tribute_card_.has_value())
			{
				return;
			}

			ImGui::SeparatorText("Tribute Summon");
			ImGui::Text("Tribute for %s in column %d", card_text(view, *tribute_card_).c_str(), column_ + 1);
			ImGui::TextDisabled("Level 5-6 requires 1 tribute; level 7+ requires 2.");

			for (const auto card: view.self_side.monsters)
			{
				const bool checked = std::ranges::contains(tribute_cards_, card);
				bool next = checked;
				ImGui::PushID(static_cast<int>(card.raw()));
				if (ImGui::Checkbox("##tribute", &next) && next != checked)
				{
					if (next)
					{
						tribute_cards_.push_back(card);
					}
					else if (const auto it = std::ranges::find(tribute_cards_, card); it != tribute_cards_.end())
					{
						tribute_cards_.erase(it);
					}
				}
				ImGui::SameLine();
				ImGui::Text("%s", card_text(view, card).c_str());
				ImGui::PopID();
			}

			const bool can_confirm = !queued && !tribute_cards_.empty();
			ImGui::BeginDisabled(!can_confirm);
			if (ImGui::Button("Confirm Tribute Summon"))
			{
				(void)controller_.submit(
					driver::LocalAction{
							driver::TributeSummonAction{
									*tribute_card_,
									static_cast<domain::FieldColumn>(column_),
									tribute_cards_,
							}
					}
				);
				tribute_card_.reset();
				tribute_cards_.clear();
			}
			ImGui::EndDisabled();
			ImGui::SameLine();
			if (ImGui::Button("Cancel Tribute"))
			{
				tribute_card_.reset();
				tribute_cards_.clear();
			}
		}

		auto render_actions(const driver::PlayerView& view, const ImGuiController::Snapshot& snapshot) -> void
		{
			ImGui::SeparatorText("Actions");
			if (!snapshot.waiting)
			{
				ImGui::TextDisabled("Waiting for the opponent or chain resolution...");
				return;
			}

			if (tribute_card_.has_value() && !std::ranges::contains(view.self_side.hand, *tribute_card_))
			{
				tribute_card_.reset();
				tribute_cards_.clear();
			}

			const bool queued = snapshot.action_queued;
			if (queued)
			{
				ImGui::TextColored(ImVec4{0.5f, 0.8f, 0.5f, 1.0f}, "Action queued; resolving...");
			}

			ImGui::BeginDisabled(queued);
			if (ImGui::Button("Pass Priority"))
			{
				(void)controller_.submit(driver::LocalAction{driver::PassAction{}});
			}
			ImGui::SameLine();
			if (ImGui::Button("Advance Phase"))
			{
				(void)controller_.submit(driver::LocalAction{driver::AdvancePhaseAction{}});
			}
			ImGui::SameLine();
			if (ImGui::Button("End Turn"))
			{
				(void)controller_.submit(driver::LocalAction{driver::EndTurnAction{}});
			}

			ImGui::SetNextItemWidth(90.0f);
			ImGui::Combo("Column", &column_, "1\0""2\0""3\0""4\0""5\0");
			const auto column = static_cast<domain::FieldColumn>(column_);

			ImGui::SeparatorText("Hand");
			if (view.self_side.hand.empty())
			{
				ImGui::TextDisabled("  (empty)");
			}
			for (const auto card: view.self_side.hand)
			{
				ImGui::PushID(static_cast<int>(card.raw()));
				ImGui::Text("%s", card_text(view, card).c_str());
				ImGui::SameLine();
				submit_button("Summon", controller_, driver::LocalAction{driver::NormalSummonAction{card, column}}, queued);
				ImGui::SameLine();
				if (ImGui::SmallButton("Tribute"))
				{
					tribute_card_ = card;
					tribute_cards_.clear();
				}
				ImGui::SameLine();
				submit_button("Set Monster", controller_, driver::LocalAction{driver::SetMonsterAction{card, column}}, queued);
				ImGui::SameLine();
				submit_button("Set S/T", controller_, driver::LocalAction{driver::SetSpellTrapAction{card, column}}, queued);
				ImGui::SameLine();
				submit_button("Activate", controller_, driver::LocalAction{driver::ActivateCardAction{card}}, queued);
				ImGui::PopID();
			}

			ImGui::SeparatorText("Your Monsters");
			if (view.self_side.monsters.empty())
			{
				ImGui::TextDisabled("  (empty)");
			}
			for (const auto card: view.self_side.monsters)
			{
				ImGui::PushID(static_cast<int>(card.raw()));
				ImGui::Text("%s", card_text(view, card).c_str());
				ImGui::SameLine();
				submit_button("Effect 0", controller_, driver::LocalAction{driver::ActivateEffectAction{card, 0}}, queued);
				ImGui::SameLine();
				submit_button("Attack Direct", controller_, driver::LocalAction{driver::DeclareAttackAction{card, std::nullopt}}, queued);
				for (const auto target: view.opponent_side.monsters)
				{
					ImGui::SameLine();
					const auto label = std::format("Attack {}", card_text(view, target));
					submit_button(label.c_str(), controller_, driver::LocalAction{driver::DeclareAttackAction{card, target}}, queued);
				}
				ImGui::PopID();
			}

			render_tribute_panel(view, queued);

			if (view.pending_replay)
			{
				ImGui::SeparatorText("Replay Target");
				submit_button("Direct Attack", controller_, driver::LocalAction{driver::ReplayAttackAction{std::nullopt}}, queued);
				for (const auto target: view.opponent_side.monsters)
				{
					ImGui::SameLine();
					const auto label = std::format("Replay {}", card_text(view, target));
					submit_button(label.c_str(), controller_, driver::LocalAction{driver::ReplayAttackAction{target}}, queued);
				}
			}
			ImGui::EndDisabled();
		}

		auto render_card_selection(const ImGuiController::CardSelection& selection) -> void
		{
			ImGui::SeparatorText("Select Cards");
			ImGui::TextDisabled("Select %zu to %zu card(s).", selection.min, selection.max);

			for (const auto [index, card]: selection.candidates | std::views::enumerate)
			{
				const bool checked = std::ranges::contains(selection.selected, index);
				bool next = checked;
				ImGui::PushID(static_cast<int>(index));
				if (ImGui::Checkbox(id_text(card).c_str(), &next) && next != checked)
				{
					controller_.toggle_card(index);
				}
				ImGui::PopID();
			}

			ImGui::Text(
				"Selected %zu / %zu",
				selection.selected.size(),
				selection.candidates.size()
			);

			const bool can_confirm =
					!selection.confirmed &&
					selection.selected.size() >= selection.min &&
					selection.selected.size() <= selection.max;
			ImGui::BeginDisabled(!can_confirm);
			if (ImGui::Button("Confirm Selection"))
			{
				(void)controller_.confirm_cards();
			}
			ImGui::EndDisabled();

			if (selection.confirmed)
			{
				ImGui::TextColored(ImVec4{0.5f, 0.8f, 0.5f, 1.0f}, "Selection queued; resolving...");
			}
		}

		auto render_location_selection(const ImGuiController::LocationSelection& selection) -> void
		{
			ImGui::SeparatorText(selection.spell_trap ? "Select Spell / Trap Zone" : "Select Monster Zone");
			ImGui::TextDisabled("Choose one zone.");

			for (const auto [index, column]: selection.columns | std::views::enumerate)
			{
				ImGui::PushID(static_cast<int>(index));
				ImGui::BeginDisabled(selection.confirmed);
				const auto label = std::format("Zone {}", std::to_underlying(column) + 1);
				if (ImGui::Button(label.c_str()))
				{
					(void)controller_.confirm_location(index);
				}
				ImGui::EndDisabled();
				ImGui::PopID();
				ImGui::SameLine();
			}
			ImGui::NewLine();

			if (selection.confirmed)
			{
				ImGui::TextColored(ImVec4{0.5f, 0.8f, 0.5f, 1.0f}, "Location queued; resolving...");
			}
		}

		auto render_trigger_selection(const driver::PlayerView& view, const ImGuiController::TriggerSelection& selection) -> void
		{
			ImGui::SeparatorText(selection.optional ? "Optional Triggers" : "Mandatory Trigger Order");
			if (selection.optional)
			{
				ImGui::TextDisabled("Select any subset; the list order is the resolution order.");
			}
			else
			{
				ImGui::TextDisabled("All mandatory triggers must resolve; choose their resolution order.");
			}

			for (std::size_t order = 0; order < selection.selected.size(); ++order)
			{
				const auto index = selection.selected[order];
				ImGui::PushID(static_cast<int>(index));
				ImGui::Text(
					"%zu. %s",
					order + 1,
					card_text(view, selection.candidates[index].source).c_str()
				);
				ImGui::SameLine();
				ImGui::BeginDisabled(selection.confirmed);
				if (ImGui::SmallButton("Up"))
				{
					controller_.move_trigger(index, -1);
				}
				ImGui::SameLine();
				if (ImGui::SmallButton("Down"))
				{
					controller_.move_trigger(index, 1);
				}
				if (selection.optional)
				{
					ImGui::SameLine();
					if (ImGui::SmallButton("Remove"))
					{
						controller_.toggle_trigger(index);
					}
				}
				ImGui::EndDisabled();
				ImGui::PopID();
			}

			if (selection.optional)
			{
				ImGui::SeparatorText("Available");
				for (std::size_t index = 0; index < selection.candidates.size(); ++index)
				{
					if (std::ranges::contains(selection.selected, index))
					{
						continue;
					}

					ImGui::PushID(static_cast<int>(index));
					ImGui::BeginDisabled(selection.confirmed);
					const auto label = std::format("Add {}", card_text(view, selection.candidates[index].source));
					if (ImGui::SmallButton(label.c_str()))
					{
						controller_.toggle_trigger(index);
					}
					ImGui::EndDisabled();
					ImGui::PopID();
				}
			}

			const bool mandatory_complete =
					selection.optional ||
					selection.selected.size() == selection.candidates.size();
			const bool can_confirm = !selection.confirmed && mandatory_complete;
			ImGui::BeginDisabled(!can_confirm);
			if (ImGui::Button("Confirm Triggers"))
			{
				(void)controller_.confirm_triggers();
			}
			ImGui::EndDisabled();

			if (selection.confirmed)
			{
				ImGui::TextColored(ImVec4{0.5f, 0.8f, 0.5f, 1.0f}, "Trigger selection queued; resolving...");
			}
		}

		auto render_view(const ImGuiController::Snapshot& snapshot) -> void
		{
			const auto* view = snapshot.view.has_value() ? &*snapshot.view : nullptr;
			if (view == nullptr)
			{
				ImGui::TextDisabled("Waiting for the first view...");
				return;
			}

			render_header(*view);
			if (snapshot.error.has_value())
			{
				ImGui::TextColored(ImVec4{0.95f, 0.4f, 0.4f, 1.0f}, "Last error: %s", snapshot.error->c_str());
			}

			ImGui::SeparatorText("Your Side");
			ImGui::Text(
				"Deck %zu  |  Extra %zu  |  Hand %zu  |  GY %zu  |  Banished %zu",
				view->self_side.deck_count,
				view->self_side.extra_deck.size(),
				view->self_side.hand.size(),
				view->self_side.graveyard.size(),
				view->self_side.banished.size()
			);
			render_cards(*view, "Hand", view->self_side.hand, false);
			render_cards(*view, "Monsters", view->self_side.monsters, true);
			render_cards(*view, "Spell / Trap", view->self_side.spell_traps, true);
			if (view->self_side.field_spell.valid())
			{
				ImGui::Text("Field Spell: %s", card_text(*view, view->self_side.field_spell).c_str());
			}

			ImGui::SeparatorText("Opponent");
			ImGui::Text(
				"Deck %zu  |  Extra %zu  |  Hand %zu  |  GY %zu  |  Banished %zu",
				view->opponent_side.deck_count,
				view->opponent_side.extra_deck_count,
				view->opponent_side.hand_count,
				view->opponent_side.graveyard.size(),
				view->opponent_side.banished.size()
			);
			render_cards(*view, "Monsters", view->opponent_side.monsters, true);
			render_cards(*view, "Spell / Trap", view->opponent_side.spell_traps, true);

			if (snapshot.cards.has_value())
			{
				render_card_selection(*snapshot.cards);
				return;
			}
			if (snapshot.locations.has_value())
			{
				render_location_selection(*snapshot.locations);
				return;
			}
			if (snapshot.triggers.has_value())
			{
				render_trigger_selection(*view, *snapshot.triggers);
				return;
			}

			render_actions(*view, snapshot);
		}

	public:
		PlayerWindow(ImGuiController& controller, std::string title) noexcept
			: controller_{controller},
			  title_{std::move(title)} {}

		PlayerWindow(const PlayerWindow&) = delete;
		auto operator=(const PlayerWindow&) -> PlayerWindow& = delete;
		PlayerWindow(PlayerWindow&&) = delete;
		auto operator=(PlayerWindow&&) -> PlayerWindow& = delete;

		~PlayerWindow()
		{
			shutdown();
		}

		[[nodiscard]] auto initialize(const int x, const int y) -> bool
		{
			window_ = glfwCreateWindow(760, 900, title_.c_str(), nullptr, nullptr);
			if (window_ == nullptr)
			{
				return false;
			}

			glfwMakeContextCurrent(window_);
			glfwSwapInterval(1);
			glfwSetWindowPos(window_, x, y);

			context_ = ImGui::CreateContext();
			if (context_ == nullptr)
			{
				return false;
			}

			ImGui::SetCurrentContext(context_);
			ImGuiIO& io = ImGui::GetIO();
			io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
			// 两个 ImGui context 共用 imgui.ini 会互相覆盖.
			io.IniFilename = nullptr;

			ImGui::StyleColorsDark();
			ImGuiStyle& style = ImGui::GetStyle();
			style.WindowRounding = 8.0f;
			style.FrameRounding = 4.0f;
			style.ScrollbarRounding = 4.0f;

			float x_scale{1.0f};
			float y_scale{1.0f};
			glfwGetWindowContentScale(window_, &x_scale, &y_scale);
			if (y_scale > 1.0f)
			{
				io.FontGlobalScale = y_scale;
				style.ScaleAllSizes(y_scale);
			}

			if (!ImGui_ImplGlfw_InitForOpenGL(window_, true))
			{
				return false;
			}
			if (!ImGui_ImplOpenGL3_Init("#version 330 core"))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] auto should_close() const noexcept -> bool
		{
			return window_ == nullptr || glfwWindowShouldClose(window_) != 0;
		}

		auto render(const SessionRunner& runner) -> void
		{
			if (window_ == nullptr || context_ == nullptr)
			{
				return;
			}

			glfwMakeContextCurrent(window_);
			ImGui::SetCurrentContext(context_);

			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();

			const auto snapshot = controller_.snapshot();
			const auto status = snapshot.waiting
				                    ? std::string{"your action is required"}
				                    : runner.status();

			const ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(viewport->WorkPos);
			ImGui::SetNextWindowSize(viewport->WorkSize);

			constexpr ImGuiWindowFlags flags =
					ImGuiWindowFlags_NoMove |
					ImGuiWindowFlags_NoResize |
					ImGuiWindowFlags_NoCollapse;

			if (ImGui::Begin(title_.c_str(), nullptr, flags))
			{
				ImGui::TextColored(ImVec4{0.6f, 0.8f, 1.0f, 1.0f}, "%s", status.c_str());
				ImGui::SameLine();
				ImGui::TextDisabled("| controller %s", std::string{controller_.name()}.c_str());

				ImGui::Separator();
				render_view(snapshot);
			}
			ImGui::End();

			ImGui::Render();

			int width{};
			int height{};
			glfwGetFramebufferSize(window_, &width, &height);
			if (width <= 0 || height <= 0)
			{
				return;
			}

			glViewport(0, 0, width, height);
			glClearColor(0.055f, 0.060f, 0.075f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
			glfwSwapBuffers(window_);
		}

		auto shutdown() noexcept -> void
		{
			if (shutdown_)
			{
				return;
			}
			shutdown_ = true;

			if (context_ != nullptr)
			{
				ImGui::SetCurrentContext(context_);
				ImGui_ImplOpenGL3_Shutdown();
				ImGui_ImplGlfw_Shutdown();
				ImGui::DestroyContext(context_);
				context_ = nullptr;
			}

			if (window_ != nullptr)
			{
				glfwDestroyWindow(window_);
				window_ = nullptr;
			}
		}
	};

	auto default_config_path(const char* executable) -> std::filesystem::path
	{
		if (executable != nullptr)
		{
			const auto beside_executable = std::filesystem::absolute(executable).parent_path() / "configs" / "local_duel.json";
			if (std::filesystem::exists(beside_executable))
			{
				return beside_executable;
			}
		}

		return "configs/local_duel.json";
	}

	auto parse_arguments(const int argc, char** argv, std::filesystem::path& config_path, int& smoke_frames) -> bool
	{
		for (int i = 1; i < argc; ++i)
		{
			const std::string_view argument = argv[i] != nullptr ? argv[i] : "";
			if (argument == "--smoke")
			{
				smoke_frames = 120;
			}
			else if (argument.starts_with("--smoke="))
			{
				std::size_t frames{};
				const auto text = argument.substr(std::string_view{"--smoke="}.size());
				const auto result = std::from_chars(text.data(), text.data() + text.size(), frames);
				if (result.ec != std::errc{} || frames == 0)
				{
					return false;
				}
				smoke_frames = static_cast<int>(std::min<std::size_t>(frames, 100000));
			}
			else if (argument == "--help" || argument == "-h")
			{
				return false;
			}
			else if (!argument.empty())
			{
				config_path = std::filesystem::path{argument};
			}
		}

		return true;
	}

	auto glfw_error(const int code, const char* description) -> void
	{
		std::println(stderr, "GLFW error {}: {}", code, description != nullptr ? description : "unknown");
	}
}

auto main(const int argc, char** argv) noexcept -> int
{
	std::filesystem::path config_path = default_config_path(argc > 0 ? argv[0] : nullptr);
	int smoke_frames = -1;
	if (!parse_arguments(argc, argv, config_path, smoke_frames))
	{
		std::println("usage: cg-app-imgui [config.json] [--smoke[=frames]]");
		return argc > 1 ? 0 : 1;
	}

	const auto config = server::MatchConfig::load(config_path);
	if (!config.has_value())
	{
		std::println(stderr, "config load failed: {}", config.error());
		return 1;
	}

	auto first = std::make_unique<ImGuiController>("FIRST", domain::Player::FIRST);
	auto second = std::make_unique<ImGuiController>("SECOND", domain::Player::SECOND);
	auto* first_controller = first.get();
	auto* second_controller = second.get();

	driver::LocalDuelSession session{std::move(first), std::move(second)};
	if (!session.load_scripts(config->cards_path))
	{
		std::println(stderr, "load scripts failed");
		return 1;
	}
	if (!session.initialize(config->duel))
	{
		std::println(stderr, "initialize duel failed");
		return 1;
	}

	glfwSetErrorCallback(glfw_error);
	if (glfwInit() == GLFW_FALSE)
	{
		std::println(stderr, "glfwInit failed");
		return 1;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, 4);

	PlayerWindow first_window{*first_controller, "cg - FIRST"};
	PlayerWindow second_window{*second_controller, "cg - SECOND"};

	if (!first_window.initialize(40, 40) || !second_window.initialize(840, 40))
	{
		std::println(stderr, "window initialization failed");
		first_window.shutdown();
		second_window.shutdown();
		glfwTerminate();
		return 1;
	}

	SessionRunner runner{session, {first_controller, second_controller}};
	runner.start();

	int frames{};
	while (!first_window.should_close() && !second_window.should_close())
	{
		if (smoke_frames >= 0 && frames >= smoke_frames)
		{
			break;
		}

		glfwPollEvents();

		// 冒烟模式自动过优先权，验证 UI 提交 -> controller 唤醒 -> session 推进的完整链路。
		if (smoke_frames >= 0)
		{
			for (auto* controller: {first_controller, second_controller})
			{
				const auto snapshot = controller->snapshot();
				if (snapshot.waiting && !snapshot.action_queued)
				{
					(void)controller->submit(driver::LocalAction{driver::PassAction{}});
				}
			}
		}

		first_window.render(runner);
		second_window.render(runner);
		++frames;
	}

	const auto final_status = runner.status();
	runner.stop();
	first_window.shutdown();
	second_window.shutdown();
	glfwTerminate();

	std::println(
		"{} | first actions={} second actions={}",
		final_status,
		first_controller->submitted_actions(),
		second_controller->submitted_actions()
	);
	return 0;
}
