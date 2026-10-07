#include "../cec_output.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <chrono>
#include <future>
#include <memory>

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
}

TEST(CecOutput, applies_latest_power_request_after_in_flight_command)
{
    std::promise<void> entered;
    std::promise<void> release;
    std::promise<void> off_applied;
    auto release_future = release.get_future();
    auto off_applied_future = off_applied.get_future();
    auto adapter = std::make_unique<StrictMock<MockCecAdapter>>();
    auto* const adapter_ptr = adapter.get();

    {
        InSequence sequence;
        EXPECT_CALL(*adapter_ptr, power_on_tv())
            .WillOnce(
                [&]
                {
                    entered.set_value();
                    release_future.wait();
                    return true;
                });
        EXPECT_CALL(*adapter_ptr, make_active_source()).WillOnce(Return(true));
        EXPECT_CALL(*adapter_ptr, standby_tv())
            .WillOnce(
                [&]
                {
                    off_applied.set_value();
                    return true;
                });
        EXPECT_CALL(*adapter_ptr, standby_tv()).WillOnce(Return(true));
        EXPECT_CALL(*adapter_ptr, close()).Times(1);
    }

    CecOutput output{mg::DisplayConfigurationOutputId{1}, std::move(adapter)};

    output.request_power(true);
    ASSERT_EQ(entered.get_future().wait_for(2s), std::future_status::ready);
    output.request_power(false);
    release.set_value();
    ASSERT_EQ(off_applied_future.wait_for(2s), std::future_status::ready);

    output.shutdown();
    output.shutdown();
}

TEST(CecOutput, powers_on_and_announces_active_source)
{
    std::promise<void> active_source_sent;
    auto active_source_sent_future = active_source_sent.get_future();
    auto adapter = std::make_unique<StrictMock<MockCecAdapter>>();
    auto* const adapter_ptr = adapter.get();

    {
        InSequence sequence;
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
    }

    CecOutput output{mg::DisplayConfigurationOutputId{1}, std::move(adapter)};
    output.request_power(true);

    ASSERT_EQ(active_source_sent_future.wait_for(2s), std::future_status::ready);
    output.shutdown();
}

TEST(CecOutput, resends_duplicate_power_request_and_stands_by_on_shutdown)
{
    std::promise<void> first_standby_sent;
    std::promise<void> repeated_standby_sent;
    auto first_standby_sent_future = first_standby_sent.get_future();
    auto repeated_standby_sent_future = repeated_standby_sent.get_future();
    auto adapter = std::make_unique<StrictMock<MockCecAdapter>>();
    auto* const adapter_ptr = adapter.get();

    {
        InSequence sequence;
        EXPECT_CALL(*adapter_ptr, standby_tv())
            .WillOnce(
                [&]
                {
                    first_standby_sent.set_value();
                    return true;
                });
        EXPECT_CALL(*adapter_ptr, standby_tv())
            .WillOnce(
                [&]
                {
                    repeated_standby_sent.set_value();
                    return true;
                });
        EXPECT_CALL(*adapter_ptr, standby_tv()).WillOnce(Return(true));
        EXPECT_CALL(*adapter_ptr, close()).Times(1);
    }

    CecOutput output{mg::DisplayConfigurationOutputId{1}, std::move(adapter)};
    output.request_power(false);
    ASSERT_EQ(first_standby_sent_future.wait_for(2s), std::future_status::ready);

    output.request_power(false);
    ASSERT_EQ(repeated_standby_sent_future.wait_for(2s), std::future_status::ready);
    output.shutdown();
}

TEST(CecOutput, exposes_output_and_adapter_identity)
{
    auto adapter = std::make_unique<StrictMock<MockCecAdapter>>();
    auto* const adapter_ptr = adapter.get();
    EXPECT_CALL(*adapter_ptr, port()).WillOnce(Return("cec-test-port"));
    EXPECT_CALL(*adapter_ptr, standby_tv()).WillOnce(Return(true));
    EXPECT_CALL(*adapter_ptr, close()).Times(1);

    CecOutput output{mg::DisplayConfigurationOutputId{7}, std::move(adapter)};
    EXPECT_EQ(output.output_id(), mg::DisplayConfigurationOutputId{7});
    EXPECT_EQ(output.adapter_port(), "cec-test-port");
    output.shutdown();
}

TEST(CecOutput, ignores_power_requests_after_shutdown)
{
    auto adapter = std::make_unique<StrictMock<MockCecAdapter>>();
    auto* const adapter_ptr = adapter.get();
    EXPECT_CALL(*adapter_ptr, standby_tv()).WillOnce(Return(true));
    EXPECT_CALL(*adapter_ptr, close()).Times(1);

    CecOutput output{mg::DisplayConfigurationOutputId{7}, std::move(adapter)};
    output.shutdown();
    output.request_power(true);
    output.request_power(false);
}

TEST(CecOutput, resends_duplicate_on_request)
{
    std::promise<void> active_source_sent;
    std::promise<void> repeated_active_source_sent;
    auto active_source_sent_future = active_source_sent.get_future();
    auto repeated_active_source_sent_future = repeated_active_source_sent.get_future();
    auto adapter = std::make_unique<StrictMock<MockCecAdapter>>();
    auto* const adapter_ptr = adapter.get();

    {
        InSequence sequence;
        EXPECT_CALL(*adapter_ptr, power_on_tv()).WillOnce(Return(true));
        EXPECT_CALL(*adapter_ptr, make_active_source())
            .WillOnce(
                [&]
                {
                    active_source_sent.set_value();
                    return true;
                });
        EXPECT_CALL(*adapter_ptr, power_on_tv()).WillOnce(Return(true));
        EXPECT_CALL(*adapter_ptr, make_active_source())
            .WillOnce(
                [&]
                {
                    repeated_active_source_sent.set_value();
                    return true;
                });
        EXPECT_CALL(*adapter_ptr, standby_tv()).WillOnce(Return(true));
        EXPECT_CALL(*adapter_ptr, close()).Times(1);
    }

    CecOutput output{mg::DisplayConfigurationOutputId{7}, std::move(adapter)};
    output.request_power(true);
    ASSERT_EQ(active_source_sent_future.wait_for(2s), std::future_status::ready);
    output.request_power(true);
    ASSERT_EQ(repeated_active_source_sent_future.wait_for(2s), std::future_status::ready);
    output.shutdown();
}
