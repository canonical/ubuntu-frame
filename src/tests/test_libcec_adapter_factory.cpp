#include "../libcec_adapter.h"
#include "mock_icec_adapter.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cstdio>
#include <memory>
#include <vector>

using namespace testing;

namespace
{
class MockInitializer
{
public:
    MOCK_METHOD(CEC::ICECAdapter*, initialize, (CEC::libcec_configuration*));
};

void destroy_mock(CEC::ICECAdapter* adapter)
{
    delete adapter;
}

class LibCecAdapterFactoryTest : public Test
{
protected:
    auto expect_discovery(std::vector<std::string> ports = {"cec-a"}) -> MockICECAdapter&
    {
        auto connection = std::make_unique<StrictMock<MockICECAdapter>>();
        auto& mock = *connection;
        EXPECT_CALL(mock, InitVideoStandalone()).Times(AnyNumber());
        ON_CALL(mock, DetectAdapters(_, _, _, _)).WillByDefault(
            [ports](CEC::cec_adapter_descriptor* descriptors, std::uint8_t, const char*, bool)
            {
                for (std::size_t index = 0; index < ports.size(); ++index)
                    std::snprintf(
                        descriptors[index].strComName,
                        sizeof(descriptors[index].strComName),
                        "%s",
                        ports[index].c_str());
                return static_cast<std::int8_t>(ports.size());
            });
        EXPECT_CALL(mock, DetectAdapters(_, 16, IsNull(), true)).Times(1);
        EXPECT_CALL(initializer, initialize(_))
            .InSequence(initialization)
            .WillOnce([connection = std::move(connection)](CEC::libcec_configuration* config) mutable
            {
                EXPECT_EQ(config->bActivateSource, 0);
                EXPECT_EQ(config->deviceTypes.types[0], CEC::CEC_DEVICE_TYPE_PLAYBACK_DEVICE);
                return connection.release();
            });
        return mock;
    }

    auto expect_adapter(std::string port = "cec-a", std::uint16_t address = 0x3400, bool opens = true)
        -> MockICECAdapter&
    {
        auto connection = std::make_unique<StrictMock<MockICECAdapter>>();
        auto& mock = *connection;
        EXPECT_CALL(mock, InitVideoStandalone()).Times(AnyNumber());
        EXPECT_CALL(mock, Open(StrEq(port), _)).WillOnce(Return(opens));
        EXPECT_CALL(mock, GetCurrentConfiguration(_)).Times(opens ? 1 : 0);
        ON_CALL(mock, GetCurrentConfiguration(_)).WillByDefault([address](CEC::libcec_configuration* config)
        {
            config->iPhysicalAddress = address;
            return true;
        });
        EXPECT_CALL(mock, Close()).Times(opens ? 1 : 0);
        EXPECT_CALL(initializer, initialize(_))
            .InSequence(initialization)
            .WillOnce([connection = std::move(connection)](CEC::libcec_configuration*) mutable
            {
                return connection.release();
            });
        return mock;
    }

    StrictMock<MockInitializer> initializer;
    Sequence initialization;
    LibCecAdapterFactory factory{
        [this](CEC::libcec_configuration* config) { return initializer.initialize(config); },
        &destroy_mock};
};
}

TEST_F(LibCecAdapterFactoryTest, discovers_opens_and_returns_each_adapter)
{
    expect_discovery({"cec-a", "cec-b"});
    expect_adapter();
    expect_adapter("cec-b", 0x1200);

    auto adapters = factory.discover();

    ASSERT_THAT(adapters, SizeIs(2));
    EXPECT_EQ(adapters[0]->port(), "cec-a");
    EXPECT_EQ(adapters[0]->physical_address(), 0x3400);
    EXPECT_EQ(adapters[1]->port(), "cec-b");
    EXPECT_EQ(adapters[1]->physical_address(), 0x1200);

    adapters.clear();
}

TEST_F(LibCecAdapterFactoryTest, returns_empty_when_libcec_initialization_fails)
{
    EXPECT_CALL(initializer, initialize(_)).WillOnce(Return(nullptr));

    EXPECT_THAT(factory.discover(), IsEmpty());
}

TEST_F(LibCecAdapterFactoryTest, returns_empty_when_adapter_detection_fails)
{
    auto& discovery = expect_discovery();
    ON_CALL(discovery, DetectAdapters(_, _, _, _)).WillByDefault(Return(-1));

    EXPECT_THAT(factory.discover(), IsEmpty());
}

TEST_F(LibCecAdapterFactoryTest, skips_an_adapter_that_fails_to_open)
{
    expect_discovery({"cec-fail"});
    expect_adapter("cec-fail", 0x3400, false);

    EXPECT_THAT(factory.discover(), IsEmpty());
}

TEST_F(LibCecAdapterFactoryTest, skips_adapter_when_session_initialization_fails)
{
    expect_discovery({"cec-unavailable"});
    EXPECT_CALL(initializer, initialize(_)).InSequence(initialization).WillOnce(Return(nullptr));

    EXPECT_THAT(factory.discover(), IsEmpty());
}
