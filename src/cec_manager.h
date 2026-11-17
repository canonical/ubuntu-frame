/*
 * Copyright © Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef FRAME_CEC_MANAGER_H
#define FRAME_CEC_MANAGER_H

#include "cec_adapter.h"
#include "cec_output.h"

#include <mir/graphics/display_configuration.h>
#include <mir/synchronised.h>

#include <map>
#include <memory>
#include <span>
#include <vector>

class CecManager
{
public:
    explicit CecManager(std::unique_ptr<CecAdapterFactory> adapter_factory);
    ~CecManager();

    CecManager(CecManager const&) = delete;
    auto operator=(CecManager const&) -> CecManager& = delete;

    void start();
    void configuration_confirmed(std::span<mir::graphics::DisplayConfigurationOutput const> outputs);
    void shutdown();

private:
    struct State
    {
        std::vector<std::unique_ptr<CecAdapter>> available_adapters;
        std::map<mir::graphics::DisplayConfigurationOutputId, std::unique_ptr<CecOutput>> outputs;
        bool started{false};
        bool stopping{false};
    };

    static auto is_hdmi(mir::graphics::DisplayConfigurationOutputType type) -> bool;
    void refresh_outputs(std::span<mir::graphics::DisplayConfigurationOutput const> outputs, State& state);
    void complete_unambiguous_matches(std::span<mir::graphics::DisplayConfigurationOutput const> outputs, State& state);

    std::unique_ptr<CecAdapterFactory> const adapter_factory;
    mir::Synchronised<State> state{State{}};
};

#endif // FRAME_CEC_MANAGER_H
