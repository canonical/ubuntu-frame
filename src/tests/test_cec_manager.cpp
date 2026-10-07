#include "../cec_manager.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <chrono>
#include <condition_variable>
#include <future>
#include <memory>
#include <mutex>
#include <vector>

using namespace testing;
using namespace std::chrono_literals;
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

class MockCecAdapterFactory : public CecAdapterFactory
{
public:
    using AdapterList = std::vector<std::unique_ptr<CecAdapter>>;
    MOCK_METHOD(AdapterList, discover, (), (override));
};

auto output(int id, std::optional<std::uint16_t> address)
    -> mg::DisplayConfigurationOutput
{
    mg::DisplayConfigurationOutput value{};
    value.id = mg::DisplayConfigurationOutputId{id};
    value.type = mg::DisplayConfigurationOutputType::hdmia;
    value.connected = true;
    value.used = true;
    value.power_mode = mir_power_mode_on;
    value.display_info.physical_address = address;
    return value;
}

auto manager_with(std::vector<std::unique_ptr<CecAdapter>> adapters)
    -> std::unique_ptr<CecManager>
{
    auto factory = std::make_unique<StrictMock<MockCecAdapterFactory>>();
    EXPECT_CALL(*factory, discover()).WillOnce(Return(ByMove(std::move(adapters))));
    return std::make_unique<CecManager>(std::move(factory));
}
}

TEST(CecManager, matches_physical_address_and_applies_on_state)
{
    std::promise<void> on_applied;
    auto on_applied_future = on_applied.get_future();
    auto adapter = std::make_unique<StrictMock<MockCecAdapter>>();
    auto* const adapter_ptr = adapter.get();
    EXPECT_CALL(*adapter_ptr, port()).WillRepeatedly(Return("adapter-a"));
    EXPECT_CALL(*adapter_ptr, physical_address()).WillRepeatedly(Return(0x3400));
    EXPECT_CALL(*adapter_ptr, power_on_tv()).WillOnce(Return(true));
    EXPECT_CALL(*adapter_ptr, make_active_source()).WillOnce([&]
    {
        on_applied.set_value();
        return true;
    });
    EXPECT_CALL(*adapter_ptr, standby_tv()).WillOnce(Return(true));
    EXPECT_CALL(*adapter_ptr, close()).Times(1);
    std::vector<std::unique_ptr<CecAdapter>> adapters;
    adapters.push_back(std::move(adapter));
    auto manager = manager_with(std::move(adapters));
    auto display = output(1, 0x3400);

    manager->start();
    manager->configuration_confirmed(std::span{&display, 1});
    ASSERT_EQ(on_applied_future.wait_for(2s), std::future_status::ready);
    manager->shutdown();
}

TEST(CecManager, reconciles_configuration_confirmed_during_discovery)
{
    std::promise<void> on_applied;
    auto on_applied_future = on_applied.get_future();
    auto adapter = std::make_unique<StrictMock<MockCecAdapter>>();
    auto* const adapter_ptr = adapter.get();
    EXPECT_CALL(*adapter_ptr, physical_address()).WillRepeatedly(Return(0x3400));
    EXPECT_CALL(*adapter_ptr, power_on_tv()).WillOnce(Return(true));
    EXPECT_CALL(*adapter_ptr, make_active_source()).WillOnce([&]
    {
        on_applied.set_value();
        return true;
    });
    EXPECT_CALL(*adapter_ptr, standby_tv()).WillOnce(Return(true));
    EXPECT_CALL(*adapter_ptr, close()).Times(1);

    auto factory = std::make_unique<StrictMock<MockCecAdapterFactory>>();
    auto* const factory_ptr = factory.get();
    CecManager manager{std::move(factory)};
    auto display = output(1, 0x3400);
    std::vector<std::unique_ptr<CecAdapter>> adapters;
    adapters.push_back(std::move(adapter));
    EXPECT_CALL(*factory_ptr, discover()).WillOnce([&]
    {
        manager.configuration_confirmed(std::span{&display, 1});
        return std::move(adapters);
    });

    manager.start();
    ASSERT_EQ(on_applied_future.wait_for(2s), std::future_status::ready);
    manager.shutdown();
}

TEST(CecManager, retries_unmatched_output_when_display_info_becomes_available)
{
    std::promise<void> on_applied;
    auto on_applied_future = on_applied.get_future();
    auto adapter = std::make_unique<StrictMock<MockCecAdapter>>();
    auto* const adapter_ptr = adapter.get();
    EXPECT_CALL(*adapter_ptr, physical_address()).WillRepeatedly(Return(0x3400));
    EXPECT_CALL(*adapter_ptr, power_on_tv()).WillOnce(Return(true));
    EXPECT_CALL(*adapter_ptr, make_active_source()).WillOnce([&]
    {
        on_applied.set_value();
        return true;
    });
    EXPECT_CALL(*adapter_ptr, standby_tv()).WillOnce(Return(true));
    EXPECT_CALL(*adapter_ptr, close()).Times(1);
    std::vector<std::unique_ptr<CecAdapter>> adapters;
    adapters.push_back(std::move(adapter));
    auto manager = manager_with(std::move(adapters));
    auto display = output(1, std::nullopt);

    manager->start();
    manager->configuration_confirmed(std::span{&display, 1});

    display.display_info.physical_address = 0x3400;
    manager->configuration_confirmed(std::span{&display, 1});
    ASSERT_EQ(on_applied_future.wait_for(2s), std::future_status::ready);
    manager->shutdown();
}

TEST(CecManager, maps_output_hotplugged_with_a_new_id)
{
    std::promise<void> on_applied;
    auto on_applied_future = on_applied.get_future();
    auto adapter = std::make_unique<StrictMock<MockCecAdapter>>();
    auto* const adapter_ptr = adapter.get();
    EXPECT_CALL(*adapter_ptr, physical_address()).WillRepeatedly(Return(0x3400));
    EXPECT_CALL(*adapter_ptr, power_on_tv()).WillOnce(Return(true));
    EXPECT_CALL(*adapter_ptr, make_active_source()).WillOnce([&]
    {
        on_applied.set_value();
        return true;
    });
    EXPECT_CALL(*adapter_ptr, standby_tv()).WillOnce(Return(true));
    EXPECT_CALL(*adapter_ptr, close()).Times(1);
    std::vector<std::unique_ptr<CecAdapter>> adapters;
    adapters.push_back(std::move(adapter));
    auto manager = manager_with(std::move(adapters));
    auto disconnected = output(1, 0x3400);
    disconnected.connected = false;

    manager->start();
    manager->configuration_confirmed(std::span{&disconnected, 1});

    auto hotplugged = output(2, 0x3400);
    manager->configuration_confirmed(std::span{&hotplugged, 1});
    ASSERT_EQ(on_applied_future.wait_for(2s), std::future_status::ready);
    manager->shutdown();
}

TEST(CecManager, confirmed_power_off_sends_standby)
{
    auto adapter = std::make_unique<StrictMock<MockCecAdapter>>();
    auto* const adapter_ptr = adapter.get();
    EXPECT_CALL(*adapter_ptr, physical_address()).WillRepeatedly(Return(0x3400));
    EXPECT_CALL(*adapter_ptr, power_on_tv()).Times(AtMost(1)).WillRepeatedly(Return(true));
    EXPECT_CALL(*adapter_ptr, make_active_source()).Times(AtMost(1)).WillRepeatedly(Return(true));
    EXPECT_CALL(*adapter_ptr, standby_tv()).Times(AtLeast(1)).WillRepeatedly(Return(true));
    EXPECT_CALL(*adapter_ptr, close()).Times(1);
    std::vector<std::unique_ptr<CecAdapter>> adapters;
    adapters.push_back(std::move(adapter));
    auto manager = manager_with(std::move(adapters));
    auto display = output(1, 0x3400);

    manager->start();
    manager->configuration_confirmed(std::span{&display, 1});

    display.power_mode = mir_power_mode_off;
    manager->configuration_confirmed(std::span{&display, 1});
    manager->shutdown();
}

TEST(CecManager, does_not_guess_when_output_physical_addresses_are_ambiguous)
{
    auto first_adapter = std::make_unique<StrictMock<MockCecAdapter>>();
    auto second_adapter = std::make_unique<StrictMock<MockCecAdapter>>();
    auto* const first_adapter_ptr = first_adapter.get();
    auto* const second_adapter_ptr = second_adapter.get();
    EXPECT_CALL(*first_adapter_ptr, physical_address()).WillRepeatedly(Return(0x3400));
    EXPECT_CALL(*second_adapter_ptr, physical_address()).WillRepeatedly(Return(0x3400));
    // StrictMock makes any CEC power command fail this ambiguous-mapping test.
    EXPECT_CALL(*first_adapter_ptr, close()).Times(1);
    EXPECT_CALL(*second_adapter_ptr, close()).Times(1);
    std::vector<std::unique_ptr<CecAdapter>> adapters;
    adapters.push_back(std::move(first_adapter));
    adapters.push_back(std::move(second_adapter));
    auto manager = manager_with(std::move(adapters));
    std::vector<mg::DisplayConfigurationOutput> displays{
        output(1, 0x3400),
        output(2, 0x3400)};

    manager->start();
    manager->configuration_confirmed(displays);
    manager->shutdown();
}

TEST(CecManager, replays_pre_start_configuration_discovers_once_and_ignores_configuration_after_shutdown)
{
    std::promise<void> on_applied;
    auto on_applied_future = on_applied.get_future();
    auto adapter = std::make_unique<StrictMock<MockCecAdapter>>();
    auto* const adapter_ptr = adapter.get();
    EXPECT_CALL(*adapter_ptr, physical_address()).WillRepeatedly(Return(0x3400));
    EXPECT_CALL(*adapter_ptr, power_on_tv()).WillOnce(Return(true));
    EXPECT_CALL(*adapter_ptr, make_active_source()).WillOnce([&]
    {
        on_applied.set_value();
        return true;
    });
    EXPECT_CALL(*adapter_ptr, standby_tv()).WillOnce(Return(true));
    EXPECT_CALL(*adapter_ptr, close()).Times(1);
    std::vector<std::unique_ptr<CecAdapter>> adapters;
    adapters.push_back(std::move(adapter));
    auto factory = std::make_unique<StrictMock<MockCecAdapterFactory>>();
    auto* const factory_ptr = factory.get();
    EXPECT_CALL(*factory_ptr, discover())
        .Times(1)
        .WillOnce(Return(ByMove(std::move(adapters))));
    CecManager manager{std::move(factory)};
    auto display = output(1, 0x3400);

    manager.configuration_confirmed(std::span{&display, 1});
    manager.start();
    manager.start();
    ASSERT_EQ(on_applied_future.wait_for(2s), std::future_status::ready);
    manager.shutdown();

    manager.configuration_confirmed(std::span{&display, 1});
}

TEST(CecManager, does_not_discover_or_send_commands_without_start)
{
    auto factory = std::make_unique<StrictMock<MockCecAdapterFactory>>();
    EXPECT_CALL(*factory, discover()).Times(0);
    CecManager manager{std::move(factory)};
    auto display = output(1, 0x3400);

    manager.configuration_confirmed(std::span{&display, 1});
    manager.shutdown();
}

TEST(CecManager, ignores_disconnected_and_non_hdmi_outputs)
{
    auto adapter = std::make_unique<StrictMock<MockCecAdapter>>();
    auto* const adapter_ptr = adapter.get();
    EXPECT_CALL(*adapter_ptr, physical_address()).WillOnce(Return(std::nullopt));
    EXPECT_CALL(*adapter_ptr, close()).Times(1);
    std::vector<std::unique_ptr<CecAdapter>> adapters;
    adapters.push_back(std::move(adapter));
    auto manager = manager_with(std::move(adapters));
    auto disconnected = output(1, 0x3400);
    disconnected.connected = false;
    auto displayport = output(2, 0x3400);
    displayport.type = mg::DisplayConfigurationOutputType::displayport;
    std::vector<mg::DisplayConfigurationOutput> displays{disconnected, displayport};

    manager->start();
    manager->configuration_confirmed(displays);
    manager->shutdown();
}

TEST(CecManager, ignores_invalid_adapter_and_display_physical_addresses)
{
    auto adapter = std::make_unique<StrictMock<MockCecAdapter>>();
    auto* const adapter_ptr = adapter.get();
    EXPECT_CALL(*adapter_ptr, physical_address()).WillRepeatedly(Return(0xFFFF));
    EXPECT_CALL(*adapter_ptr, close()).Times(1);
    std::vector<std::unique_ptr<CecAdapter>> adapters;
    adapters.push_back(std::move(adapter));
    auto manager = manager_with(std::move(adapters));
    auto display = output(1, 0xFFFF);

    manager->start();
    manager->configuration_confirmed(std::span{&display, 1});
    manager->shutdown();
}
