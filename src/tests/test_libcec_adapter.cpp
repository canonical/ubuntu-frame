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

class LibCecAdapterTest : public Test
{
protected:
    LibCecAdapterTest()
    {
        EXPECT_CALL(mock, InitVideoStandalone()).Times(AnyNumber());
        ON_CALL(mock, Open(_, _)).WillByDefault(Return(true));
        ON_CALL(mock, GetCurrentConfiguration(_)).WillByDefault([this](CEC::libcec_configuration* config)
        {
            config->iPhysicalAddress = physical_address.value_or(CEC_INVALID_PHYSICAL_ADDRESS);
            return true;
        });
    }

    auto open_adapter(bool succeeds = true) -> std::unique_ptr<LibCecAdapter>
    {
        EXPECT_CALL(mock, Open(StrEq("cec-test-port"), _)).Times(1);
        EXPECT_CALL(mock, GetCurrentConfiguration(_)).Times(succeeds ? 1 : 0);
        EXPECT_CALL(mock, Close()).Times(succeeds ? 1 : 0);
        LibCecConnection owned{connection.release(), &destroy_mock};
        return LibCecAdapter::open("cec-test-port", std::move(owned));
    }

    std::unique_ptr<MockICECAdapter> connection = std::make_unique<StrictMock<MockICECAdapter>>();
    MockICECAdapter& mock = *connection;
    std::optional<std::uint16_t> physical_address = 0x3400;
};
}

TEST_F(LibCecAdapterTest, exposes_port_and_physical_address)
{
    auto adapter = open_adapter();

    ASSERT_THAT(adapter, NotNull());
    EXPECT_EQ(adapter->port(), "cec-test-port");
    EXPECT_EQ(adapter->physical_address(), 0x3400);
}

TEST_F(LibCecAdapterTest, sends_tv_power_commands_using_expected_roles)
{
    EXPECT_CALL(mock, PowerOnDevices(CEC::CECDEVICE_TV)).WillOnce(Return(true));
    EXPECT_CALL(mock, SetActiveSource(CEC::CEC_DEVICE_TYPE_PLAYBACK_DEVICE)).WillOnce(Return(true));
    EXPECT_CALL(mock, StandbyDevices(CEC::CECDEVICE_TV)).WillOnce(Return(true));

    auto adapter = open_adapter();

    ASSERT_THAT(adapter, NotNull());
    EXPECT_TRUE(adapter->power_on_tv());
    EXPECT_TRUE(adapter->make_active_source());
    EXPECT_TRUE(adapter->standby_tv());
    adapter->close();
    adapter->close();
}

TEST_F(LibCecAdapterTest, propagates_command_failures)
{
    EXPECT_CALL(mock, PowerOnDevices(CEC::CECDEVICE_TV)).WillOnce(Return(false));
    EXPECT_CALL(mock, SetActiveSource(CEC::CEC_DEVICE_TYPE_PLAYBACK_DEVICE)).WillOnce(Return(false));
    EXPECT_CALL(mock, StandbyDevices(CEC::CECDEVICE_TV)).WillOnce(Return(false));

    physical_address = std::nullopt;
    auto adapter = open_adapter();

    ASSERT_THAT(adapter, NotNull());
    EXPECT_FALSE(adapter->power_on_tv());
    EXPECT_FALSE(adapter->make_active_source());
    EXPECT_FALSE(adapter->standby_tv());
    adapter->close();
}

TEST_F(LibCecAdapterTest, does_not_query_configuration_when_open_fails)
{
    ON_CALL(mock, Open(_, _)).WillByDefault(Return(false));

    EXPECT_THAT(open_adapter(false), IsNull());
}

TEST(LibCecAdapter, null_connection_returns_empty)
{
    LibCecConnection empty{nullptr, &destroy_mock};
    EXPECT_THAT(LibCecAdapter::open("cec-test-port", std::move(empty)), IsNull());
}

TEST_F(LibCecAdapterTest, configuration_query_failure_leaves_address_unknown)
{
    ON_CALL(mock, GetCurrentConfiguration(_)).WillByDefault(Return(false));

    auto adapter = open_adapter();

    ASSERT_THAT(adapter, NotNull());
    EXPECT_EQ(adapter->physical_address(), std::nullopt);
}
