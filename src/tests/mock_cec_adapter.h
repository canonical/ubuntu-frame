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

#ifndef FRAME_TESTS_MOCK_CEC_ADAPTER_H
#define FRAME_TESTS_MOCK_CEC_ADAPTER_H

#include "../cec_adapter.h"

#include <gmock/gmock.h>
#include <mir/graphics/display_configuration.h>

class MockCecAdapter : public CecAdapter
{
public:
    MockCecAdapter()
    {
        ON_CALL(*this, port()).WillByDefault(testing::Return("cec-test-port"));
        ON_CALL(*this, physical_address()).WillByDefault(testing::Return(0x3400));
        ON_CALL(*this, send_power_on_tv()).WillByDefault(testing::Return(true));
        ON_CALL(*this, send_make_active_source()).WillByDefault(testing::Return(true));
        ON_CALL(*this, send_standby_tv()).WillByDefault(testing::Return(true));
    }

    MOCK_METHOD(std::string_view, port, (), (const, override));
    MOCK_METHOD(std::optional<std::uint16_t>, physical_address, (), (const, override));
    MOCK_METHOD(bool, send_power_on_tv, (), (override));
    MOCK_METHOD(bool, send_make_active_source, (), (override));
    MOCK_METHOD(bool, send_standby_tv, (), (override));
    MOCK_METHOD(void, close, (), (override));
};

class MockCecAdapterFactory : public CecAdapterFactory
{
public:
    using AdapterList = std::vector<std::unique_ptr<CecAdapter>>;
    MOCK_METHOD(AdapterList, discover, (), (override));
};

inline auto cec_display_output(int id = 1, std::optional<std::uint16_t> address = 0x3400)
    -> mir::graphics::DisplayConfigurationOutput
{
    mir::graphics::DisplayConfigurationOutput value{};
    value.id = mir::graphics::DisplayConfigurationOutputId{id};
    value.type = mir::graphics::DisplayConfigurationOutputType::hdmia;
    value.connected = true;
    value.used = true;
    value.power_mode = mir_power_mode_on;
    value.display_info.physical_address = address;
    return value;
}

#endif
