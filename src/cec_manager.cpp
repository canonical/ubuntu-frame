#include "cec_manager.h"

#include <algorithm>
#include <map>
#include <mir/log.h>
#include <optional>
#include <utility>

namespace mg = mir::graphics;

namespace
{
constexpr char const* log_component = "frame-cec";
constexpr std::uint16_t invalid_physical_address = 0xFFFF;
}

CecManager::CecManager(std::unique_ptr<CecAdapterFactory> adapter_factory) : adapter_factory{std::move(adapter_factory)}
{}

CecManager::~CecManager() { shutdown(); }

void CecManager::start()
{
    {
        auto locked = state.lock();
        if (locked->started || locked->stopping)
            return;
        locked->started = true;
    }

    auto discovered = adapter_factory->discover();
    auto locked = state.lock();
    if (locked->stopping)
    {
        locked.drop();
        for (auto& adapter : discovered)
            adapter->close();
        return;
    }
    mir::log(
        mir::logging::Severity::informational,
        log_component,
        "CEC manager discovered %zu adapter(s)",
        discovered.size());
    locked->available_adapters = std::move(discovered);
    if (locked->has_confirmed_configuration)
        reconcile_configuration(locked->confirmed_outputs, *locked);
}

void CecManager::configuration_confirmed(std::span<mg::DisplayConfigurationOutput const> outputs)
{
    mir::log(
        mir::logging::Severity::debug,
        log_component,
        "Received confirmed display configuration with %zu output(s)",
        outputs.size());
    auto locked = state.lock();
    if (locked->stopping)
        return;

    locked->confirmed_outputs.assign(outputs.begin(), outputs.end());
    locked->has_confirmed_configuration = true;
    if (locked->started)
        reconcile_configuration(outputs, *locked);
}

void CecManager::reconcile_configuration(std::span<mg::DisplayConfigurationOutput const> outputs, State& state)
{
    refresh_outputs(outputs, state);
    complete_unambiguous_matches(outputs, state);

    for (auto const& output : outputs)
    {
        auto const it = state.outputs.find(output.id);
        if (it != state.outputs.end() && it->second)
        {
            it->second->request_power(output.connected && output.used && output.power_mode == mir_power_mode_on);
        }
    }
}

void CecManager::shutdown()
{
    std::map<mg::DisplayConfigurationOutputId, std::unique_ptr<CecOutput>> outputs;
    std::vector<std::unique_ptr<CecAdapter>> adapters;
    {
        auto locked = state.lock();
        if (locked->stopping)
            return;
        mir::log(
            mir::logging::Severity::informational,
            log_component,
            "Stopping CEC manager with %zu mapped output(s)",
            locked->outputs.size());
        locked->stopping = true;
        outputs = std::move(locked->outputs);
        adapters = std::move(locked->available_adapters);
    }

    for (auto& [id, output] : outputs)
        if (output)
            output->shutdown();
    for (auto& adapter : adapters)
        adapter->close();
}

auto CecManager::is_hdmi(mg::DisplayConfigurationOutputType type) -> bool
{
    return type == mg::DisplayConfigurationOutputType::hdmia || type == mg::DisplayConfigurationOutputType::hdmib;
}

void CecManager::refresh_outputs(std::span<mg::DisplayConfigurationOutput const> outputs, State& state)
{
    for (auto const& output : outputs)
        if (output.connected && is_hdmi(output.type))
            state.outputs.try_emplace(output.id, nullptr);
}

void CecManager::complete_unambiguous_matches(std::span<mg::DisplayConfigurationOutput const> outputs, State& state)
{
    std::map<std::uint16_t, std::vector<mg::DisplayConfigurationOutputId>> output_candidates;
    std::map<std::uint16_t, std::vector<std::size_t>> adapter_candidates;

    for (auto const& output : outputs)
    {
        auto const mapping = state.outputs.find(output.id);
        if (mapping == state.outputs.end() || mapping->second || !output.connected || !is_hdmi(output.type))
            continue;
        if (output.display_info.physical_address && *output.display_info.physical_address != invalid_physical_address)
        {
            output_candidates[*output.display_info.physical_address].push_back(output.id);
        }
    }

    for (std::size_t i = 0; i < state.available_adapters.size(); ++i)
    {
        auto const address = state.available_adapters[i]->physical_address();
        if (address && *address != invalid_physical_address)
            adapter_candidates[*address].push_back(i);
    }

    std::vector<bool> adapter_consumed(state.available_adapters.size(), false);
    for (auto const& [address, output_ids] : output_candidates)
    {
        auto const adapters = adapter_candidates.find(address);
        if (output_ids.size() != 1 || adapters == adapter_candidates.end() || adapters->second.size() != 1)
            continue;

        auto const adapter_index = adapters->second.front();
        if (adapter_consumed[adapter_index])
            continue;

        auto& mapping = state.outputs.at(output_ids.front());
        mir::log(
            mir::logging::Severity::informational,
            log_component,
            "Mapped Mir output %d to CEC adapter at physical address %04x",
            output_ids.front().as_value(),
            address);
        mapping = std::make_unique<CecOutput>(output_ids.front(), std::move(state.available_adapters[adapter_index]));
        adapter_consumed[adapter_index] = true;
    }

    auto adapter = state.available_adapters.begin();
    std::size_t index = 0;
    while (adapter != state.available_adapters.end())
    {
        if (adapter_consumed[index])
            adapter = state.available_adapters.erase(adapter);
        else
            ++adapter;
        ++index;
    }
}
