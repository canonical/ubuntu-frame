#include "../cec_output_configuration.h"
#include "mock_cec_adapter.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <future>
#include <memory>
#include <vector>

using namespace testing;
namespace mg = mir::graphics;

namespace
{
class CecOutputConfigurationTest : public Test
{
protected:
    auto create_manager() -> std::shared_ptr<CecManager>
    {
        EXPECT_CALL(mock_adapter, physical_address()).Times(AnyNumber());
        EXPECT_CALL(mock_adapter, standby_tv()).Times(1);
        EXPECT_CALL(mock_adapter, close()).Times(1);

        std::vector<std::unique_ptr<CecAdapter>> adapters;
        adapters.push_back(std::move(adapter));
        auto factory = std::make_unique<StrictMock<MockCecAdapterFactory>>();
        EXPECT_CALL(*factory, discover()).WillOnce(Return(ByMove(std::move(adapters))));
        return std::make_shared<CecManager>(std::move(factory));
    }

    void expect_active_source(std::promise<void>& sent)
    {
        EXPECT_CALL(mock_adapter, power_on_tv()).Times(1);
        EXPECT_CALL(mock_adapter, make_active_source()).WillOnce([&sent]
        {
            sent.set_value();
            return true;
        });
    }

    std::unique_ptr<MockCecAdapter> adapter = std::make_unique<StrictMock<MockCecAdapter>>();
    MockCecAdapter& mock_adapter = *adapter;
    CecOutputConfiguration strategy;
    mg::DisplayConfigurationOutput output = cec_display_output(3);
};
}

TEST_F(CecOutputConfigurationTest, apply_does_not_modify_outputs)
{
    mg::UserDisplayConfigurationOutput user_output{output};

    strategy.apply_configuration(std::span{&user_output, 1});

    EXPECT_TRUE(output.used);
    EXPECT_EQ(output.power_mode, mir_power_mode_on);
}

TEST_F(CecOutputConfigurationTest, replays_latest_confirmation_when_manager_is_attached)
{
    std::promise<void> active_source_sent;
    auto active_source_sent_future = active_source_sent.get_future();
    auto manager = create_manager();
    expect_active_source(active_source_sent);

    output.power_mode = mir_power_mode_off;
    strategy.confirm_configuration(std::span<mg::DisplayConfigurationOutput const>{&output, 1});
    output.power_mode = mir_power_mode_on;
    strategy.confirm_configuration(std::span<mg::DisplayConfigurationOutput const>{&output, 1});
    strategy.set_manager(manager);
    manager->start();

    ASSERT_EQ(active_source_sent_future.wait_for(std::chrono::seconds{2}), std::future_status::ready);
    std::promise<void> forwarded_active_source_sent;
    auto forwarded_active_source_sent_future = forwarded_active_source_sent.get_future();
    expect_active_source(forwarded_active_source_sent);
    strategy.confirm_configuration(std::span<mg::DisplayConfigurationOutput const>{&output, 1});
    ASSERT_EQ(forwarded_active_source_sent_future.wait_for(std::chrono::seconds{2}), std::future_status::ready);
    manager->shutdown();
}
