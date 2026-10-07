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
        ON_CALL(*this, power_on_tv()).WillByDefault(testing::Return(true));
        ON_CALL(*this, make_active_source()).WillByDefault(testing::Return(true));
        ON_CALL(*this, standby_tv()).WillByDefault(testing::Return(true));
    }

    MOCK_METHOD(std::string_view, port, (), (const, override));
    MOCK_METHOD(std::optional<std::uint16_t>, physical_address, (), (const, override));
    MOCK_METHOD(bool, power_on_tv, (), (override));
    MOCK_METHOD(bool, make_active_source, (), (override));
    MOCK_METHOD(bool, standby_tv, (), (override));
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
