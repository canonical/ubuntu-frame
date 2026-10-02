#include "../libcec_adapter.h"
#include "mock_icec_adapter.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <memory>

using namespace testing;

namespace
{
void destroy_mock(CEC::ICECAdapter* adapter)
{
    delete adapter;
}

auto open_mock_adapter(
    std::unique_ptr<MockICECAdapter> connection,
    std::optional<std::uint16_t> physical_address = 0x3400)
    -> std::unique_ptr<LibCecAdapter>
{
    auto* const mock = connection.get();
    EXPECT_CALL(*mock, InitVideoStandalone()).Times(AnyNumber());
    EXPECT_CALL(*mock, Open(StrEq("cec-test-port"), _)).WillOnce(Return(true));
    EXPECT_CALL(*mock, GetCurrentConfiguration(_)).WillOnce([physical_address](CEC::libcec_configuration* config)
    {
        config->iPhysicalAddress = physical_address.value_or(CEC_INVALID_PHYSICAL_ADDRESS);
        return true;
    });

    LibCecConnection owned{connection.release(), &destroy_mock};
    return LibCecAdapter::open("cec-test-port", std::move(owned));
}
}

TEST(LibCecAdapter, exposes_port_and_physical_address)
{
    auto connection = std::make_unique<StrictMock<MockICECAdapter>>();
    auto* const mock = connection.get();
    EXPECT_CALL(*mock, Close()).Times(1);
    auto adapter = open_mock_adapter(std::move(connection));

    ASSERT_THAT(adapter, NotNull());
    EXPECT_EQ(adapter->port(), "cec-test-port");
    EXPECT_EQ(adapter->physical_address(), 0x3400);
}

TEST(LibCecAdapter, sends_tv_power_commands_using_expected_roles)
{
    auto connection = std::make_unique<StrictMock<MockICECAdapter>>();
    auto* const mock = connection.get();
    EXPECT_CALL(*mock, PowerOnDevices(CEC::CECDEVICE_TV)).WillOnce(Return(true));
    EXPECT_CALL(*mock, SetActiveSource(CEC::CEC_DEVICE_TYPE_PLAYBACK_DEVICE)).WillOnce(Return(true));
    EXPECT_CALL(*mock, StandbyDevices(CEC::CECDEVICE_TV)).WillOnce(Return(true));
    EXPECT_CALL(*mock, Close()).Times(1);

    auto adapter = open_mock_adapter(std::move(connection));

    ASSERT_THAT(adapter, NotNull());
    EXPECT_TRUE(adapter->power_on_tv());
    EXPECT_TRUE(adapter->make_active_source());
    EXPECT_TRUE(adapter->standby_tv());
    adapter->close();
    adapter->close();
}

TEST(LibCecAdapter, propagates_command_failures)
{
    auto connection = std::make_unique<NiceMock<MockICECAdapter>>();
    auto* const mock = connection.get();
    EXPECT_CALL(*mock, PowerOnDevices(CEC::CECDEVICE_TV)).WillOnce(Return(false));
    EXPECT_CALL(*mock, SetActiveSource(CEC::CEC_DEVICE_TYPE_PLAYBACK_DEVICE)).WillOnce(Return(false));
    EXPECT_CALL(*mock, StandbyDevices(CEC::CECDEVICE_TV)).WillOnce(Return(false));
    EXPECT_CALL(*mock, Close()).Times(1);

    auto adapter = open_mock_adapter(std::move(connection), std::nullopt);

    ASSERT_THAT(adapter, NotNull());
    EXPECT_FALSE(adapter->power_on_tv());
    EXPECT_FALSE(adapter->make_active_source());
    EXPECT_FALSE(adapter->standby_tv());
    adapter->close();
}

TEST(LibCecAdapter, does_not_query_configuration_when_open_fails)
{
    auto connection = std::make_unique<StrictMock<MockICECAdapter>>();
    auto* const mock = connection.get();
    EXPECT_CALL(*mock, InitVideoStandalone()).Times(AnyNumber());
    EXPECT_CALL(*mock, Open(StrEq("cec-test-port"), _)).WillOnce(Return(false));
    EXPECT_CALL(*mock, GetCurrentConfiguration(_)).Times(0);

    LibCecConnection owned{connection.release(), &destroy_mock};
    EXPECT_THAT(LibCecAdapter::open("cec-test-port", std::move(owned)), IsNull());
}

TEST(LibCecAdapter, null_connection_returns_empty)
{
    LibCecConnection empty{nullptr, &destroy_mock};
    EXPECT_THAT(LibCecAdapter::open("cec-test-port", std::move(empty)), IsNull());
}

TEST(LibCecAdapter, configuration_query_failure_leaves_address_unknown)
{
    auto connection = std::make_unique<StrictMock<MockICECAdapter>>();
    auto* const mock = connection.get();
    EXPECT_CALL(*mock, InitVideoStandalone()).Times(AnyNumber());
    EXPECT_CALL(*mock, Open(StrEq("cec-test-port"), _)).WillOnce(Return(true));
    EXPECT_CALL(*mock, GetCurrentConfiguration(_)).WillOnce(Return(false));
    EXPECT_CALL(*mock, Close()).Times(1);

    LibCecConnection owned{connection.release(), &destroy_mock};
    auto adapter = LibCecAdapter::open("cec-test-port", std::move(owned));

    ASSERT_THAT(adapter, NotNull());
    EXPECT_EQ(adapter->physical_address(), std::nullopt);
}
