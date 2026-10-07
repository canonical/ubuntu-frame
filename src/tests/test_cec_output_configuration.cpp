#include "../cec_output_configuration.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <future>
#include <memory>
#include <vector>

using namespace testing;
namespace mg = mir::graphics;

namespace
{
class MockCecAdapter : public CecAdapter
{
public:
    MOCK_METHOD(std::string_view, port, (), (const, override));
    MOCK_METHOD(std::optional<std::uint16_t>, physical_address, (), (const, override));
    MOCK_METHOD(bool, power_on_tv, (), (override));
    MOCK_METHOD(bool, make_active_source, (), (override));
    MOCK_METHOD(bool, standby_tv, (), (override));
    MOCK_METHOD(void, close, (), (override));
};

class FakeCecAdapterFactory : public CecAdapterFactory
{
public:
    explicit FakeCecAdapterFactory(std::vector<std::unique_ptr<CecAdapter>> adapters) : adapters{std::move(adapters)} {}

    auto discover() -> std::vector<std::unique_ptr<CecAdapter>> override { return std::move(adapters); }

private:
    std::vector<std::unique_ptr<CecAdapter>> adapters;
};
}

TEST(CecOutputConfiguration, apply_does_not_modify_outputs)
{
    CecOutputConfiguration strategy;
    mg::DisplayConfigurationOutput output{};
    output.type = mg::DisplayConfigurationOutputType::hdmia;
    output.connected = true;
    output.used = true;
    output.power_mode = mir_power_mode_on;
    mg::UserDisplayConfigurationOutput user_output{output};

    strategy.apply_configuration(std::span{&user_output, 1});

    EXPECT_TRUE(output.used);
    EXPECT_EQ(output.power_mode, mir_power_mode_on);
}

TEST(CecOutputConfiguration, replays_latest_confirmation_when_manager_is_attached)
{
    std::promise<void> active_source_sent;
    auto active_source_sent_future = active_source_sent.get_future();
    auto adapter = std::make_unique<StrictMock<MockCecAdapter>>();
    auto* const adapter_ptr = adapter.get();
    EXPECT_CALL(*adapter_ptr, physical_address()).WillRepeatedly(Return(0x3400));
    EXPECT_CALL(*adapter_ptr, power_on_tv()).WillOnce(Return(true));
    EXPECT_CALL(*adapter_ptr, make_active_source())
        .WillOnce(
            [&]
            {
                active_source_sent.set_value();
                return true;
            });
    EXPECT_CALL(*adapter_ptr, standby_tv()).WillOnce(Return(true));
    EXPECT_CALL(*adapter_ptr, close()).Times(1);

    std::vector<std::unique_ptr<CecAdapter>> adapters;
    adapters.push_back(std::move(adapter));
    auto manager = std::make_shared<CecManager>(std::make_unique<FakeCecAdapterFactory>(std::move(adapters)));
    CecOutputConfiguration strategy;

    mg::DisplayConfigurationOutput output{};
    output.id = mg::DisplayConfigurationOutputId{3};
    output.type = mg::DisplayConfigurationOutputType::hdmia;
    output.connected = true;
    output.used = true;
    output.power_mode = mir_power_mode_off;
    output.display_info.physical_address = 0x3400;
    strategy.confirm_configuration(std::span<mg::DisplayConfigurationOutput const>{&output, 1});
    output.power_mode = mir_power_mode_on;
    strategy.confirm_configuration(std::span<mg::DisplayConfigurationOutput const>{&output, 1});
    strategy.set_manager(manager);
    manager->start();

    ASSERT_EQ(active_source_sent_future.wait_for(std::chrono::seconds{2}), std::future_status::ready);
    std::promise<void> forwarded_active_source_sent;
    auto forwarded_active_source_sent_future = forwarded_active_source_sent.get_future();
    EXPECT_CALL(*adapter_ptr, power_on_tv()).WillOnce(Return(true));
    EXPECT_CALL(*adapter_ptr, make_active_source())
        .WillOnce(
            [&]
            {
                forwarded_active_source_sent.set_value();
                return true;
            });
    strategy.confirm_configuration(std::span<mg::DisplayConfigurationOutput const>{&output, 1});
    ASSERT_EQ(forwarded_active_source_sent_future.wait_for(std::chrono::seconds{2}), std::future_status::ready);
    manager->shutdown();
}
