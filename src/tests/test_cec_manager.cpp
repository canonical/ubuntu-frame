#include "../cec_manager.h"
#include "mock_cec_adapter.h"

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
class CecManagerTest : public Test
{
protected:
    CecManagerTest()
    {
        ON_CALL(mock_factory, discover()).WillByDefault([this] { return std::move(adapters); });
        EXPECT_CALL(mock_factory, discover()).Times(1);
    }

    auto add_adapter() -> MockCecAdapter&
    {
        auto adapter = std::make_unique<StrictMock<MockCecAdapter>>();
        auto& mock = *adapter;
        EXPECT_CALL(mock, physical_address()).Times(AnyNumber());
        EXPECT_CALL(mock, close()).Times(1);
        adapters.push_back(std::move(adapter));
        return mock;
    }

    void expect_on_state()
    {
        EXPECT_CALL(mock_adapter, power_on_tv()).Times(1);
        EXPECT_CALL(mock_adapter, make_active_source()).WillOnce([this]
        {
            on_applied.set_value();
            return true;
        });
        EXPECT_CALL(mock_adapter, standby_tv()).Times(1);
    }

    std::promise<void> on_applied;
    std::future<void> on_applied_future = on_applied.get_future();
    std::vector<std::unique_ptr<CecAdapter>> adapters;
    MockCecAdapter& mock_adapter = add_adapter();
    std::unique_ptr<MockCecAdapterFactory> factory = std::make_unique<StrictMock<MockCecAdapterFactory>>();
    MockCecAdapterFactory& mock_factory = *factory;
    std::unique_ptr<CecManager> manager = std::make_unique<CecManager>(std::move(factory));
    mg::DisplayConfigurationOutput display = cec_display_output();
};
}

TEST_F(CecManagerTest, matches_physical_address_and_applies_on_state)
{
    expect_on_state();

    manager->start();
    manager->configuration_confirmed(std::span{&display, 1});
    ASSERT_EQ(on_applied_future.wait_for(2s), std::future_status::ready);
    manager->shutdown();
}

TEST_F(CecManagerTest, reconciles_configuration_confirmed_during_discovery)
{
    expect_on_state();
    ON_CALL(mock_factory, discover())
        .WillByDefault(
            [&]
            {
                manager->configuration_confirmed(std::span{&display, 1});
                return std::move(adapters);
            });

    manager->start();
    ASSERT_EQ(on_applied_future.wait_for(2s), std::future_status::ready);
    manager->shutdown();
}

TEST_F(CecManagerTest, retries_unmatched_output_when_display_info_becomes_available)
{
    expect_on_state();
    display.display_info.physical_address = std::nullopt;

    manager->start();
    manager->configuration_confirmed(std::span{&display, 1});

    display.display_info.physical_address = 0x3400;
    manager->configuration_confirmed(std::span{&display, 1});
    ASSERT_EQ(on_applied_future.wait_for(2s), std::future_status::ready);
    manager->shutdown();
}

TEST_F(CecManagerTest, maps_output_hotplugged_with_a_new_id)
{
    expect_on_state();
    display.connected = false;

    manager->start();
    manager->configuration_confirmed(std::span{&display, 1});

    auto hotplugged = cec_display_output(2);
    manager->configuration_confirmed(std::span{&hotplugged, 1});
    ASSERT_EQ(on_applied_future.wait_for(2s), std::future_status::ready);
    manager->shutdown();
}

TEST_F(CecManagerTest, confirmed_power_off_sends_standby)
{
    EXPECT_CALL(mock_adapter, power_on_tv()).Times(AtMost(1));
    EXPECT_CALL(mock_adapter, make_active_source()).Times(AtMost(1));
    EXPECT_CALL(mock_adapter, standby_tv()).Times(AtLeast(1));

    manager->start();
    manager->configuration_confirmed(std::span{&display, 1});

    display.power_mode = mir_power_mode_off;
    manager->configuration_confirmed(std::span{&display, 1});
    manager->shutdown();
}

TEST_F(CecManagerTest, does_not_guess_when_output_physical_addresses_are_ambiguous)
{
    add_adapter();
    std::vector<mg::DisplayConfigurationOutput> displays{display, cec_display_output(2)};

    manager->start();
    manager->configuration_confirmed(displays);
    manager->shutdown();
}

TEST_F(CecManagerTest, replays_pre_start_configuration_discovers_once_and_ignores_configuration_after_shutdown)
{
    expect_on_state();

    manager->configuration_confirmed(std::span{&display, 1});
    manager->start();
    manager->start();
    ASSERT_EQ(on_applied_future.wait_for(2s), std::future_status::ready);
    manager->shutdown();

    manager->configuration_confirmed(std::span{&display, 1});
}

TEST(CecManager, does_not_discover_or_send_commands_without_start)
{
    auto factory = std::make_unique<StrictMock<MockCecAdapterFactory>>();
    EXPECT_CALL(*factory, discover()).Times(0);
    CecManager manager{std::move(factory)};
    auto display = cec_display_output();

    manager.configuration_confirmed(std::span{&display, 1});
    manager.shutdown();
}

TEST_F(CecManagerTest, ignores_disconnected_and_non_hdmi_outputs)
{
    EXPECT_CALL(mock_adapter, physical_address()).WillOnce(Return(std::nullopt));
    display.connected = false;
    auto displayport = cec_display_output(2);
    displayport.type = mg::DisplayConfigurationOutputType::displayport;
    std::vector<mg::DisplayConfigurationOutput> displays{display, displayport};

    manager->start();
    manager->configuration_confirmed(displays);
    manager->shutdown();
}

TEST_F(CecManagerTest, ignores_invalid_adapter_and_display_physical_addresses)
{
    ON_CALL(mock_adapter, physical_address()).WillByDefault(Return(0xFFFF));
    display.display_info.physical_address = 0xFFFF;

    manager->start();
    manager->configuration_confirmed(std::span{&display, 1});
    manager->shutdown();
}
