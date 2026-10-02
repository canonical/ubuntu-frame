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
}

TEST(LibCecAdapterFactory, discovers_opens_and_returns_each_adapter)
{
    auto discovery = std::make_unique<StrictMock<MockICECAdapter>>();
    auto first = std::make_unique<StrictMock<MockICECAdapter>>();
    auto second = std::make_unique<StrictMock<MockICECAdapter>>();
    auto* const discovery_ptr = discovery.get();
    auto* const first_ptr = first.get();
    auto* const second_ptr = second.get();

    EXPECT_CALL(*discovery_ptr, InitVideoStandalone()).Times(AnyNumber());
    EXPECT_CALL(*discovery_ptr, DetectAdapters(_, 16, IsNull(), true))
        .WillOnce(Invoke([](CEC::cec_adapter_descriptor* descriptors, std::uint8_t, const char*, bool)
        {
            std::snprintf(descriptors[0].strComName, sizeof(descriptors[0].strComName), "cec-a");
            std::snprintf(descriptors[1].strComName, sizeof(descriptors[1].strComName), "cec-b");
            return 2;
        }));

    EXPECT_CALL(*first_ptr, InitVideoStandalone()).Times(AnyNumber());
    EXPECT_CALL(*first_ptr, Open(StrEq("cec-a"), _)).WillOnce(Return(true));
    EXPECT_CALL(*first_ptr, GetCurrentConfiguration(_)).WillOnce(Invoke([](CEC::libcec_configuration* config)
    {
        config->iPhysicalAddress = 0x3400;
        return true;
    }));
    EXPECT_CALL(*first_ptr, Close()).Times(1);

    EXPECT_CALL(*second_ptr, InitVideoStandalone()).Times(AnyNumber());
    EXPECT_CALL(*second_ptr, Open(StrEq("cec-b"), _)).WillOnce(Return(true));
    EXPECT_CALL(*second_ptr, GetCurrentConfiguration(_)).WillOnce(Invoke([](CEC::libcec_configuration* config)
    {
        config->iPhysicalAddress = 0x1200;
        return true;
    }));
    EXPECT_CALL(*second_ptr, Close()).Times(1);

    StrictMock<MockInitializer> initializer;
    EXPECT_CALL(initializer, initialize(_))
        .WillOnce(Invoke([&](CEC::libcec_configuration* config)
        {
            EXPECT_EQ(config->bActivateSource, 0);
            EXPECT_EQ(config->deviceTypes.types[0], CEC::CEC_DEVICE_TYPE_PLAYBACK_DEVICE);
            return discovery.release();
        }))
        .WillOnce(Return(first.release()))
        .WillOnce(Return(second.release()));

    LibCecAdapterFactory factory{
        [&](CEC::libcec_configuration* config) { return initializer.initialize(config); },
        &destroy_mock};

    auto adapters = factory.discover();

    ASSERT_THAT(adapters, SizeIs(2));
    EXPECT_EQ(adapters[0]->port(), "cec-a");
    EXPECT_EQ(adapters[0]->physical_address(), 0x3400);
    EXPECT_EQ(adapters[1]->port(), "cec-b");
    EXPECT_EQ(adapters[1]->physical_address(), 0x1200);

    adapters.clear();
}

TEST(LibCecAdapterFactory, returns_empty_when_libcec_initialization_fails)
{
    StrictMock<MockInitializer> initializer;
    EXPECT_CALL(initializer, initialize(_)).WillOnce(Return(nullptr));
    LibCecAdapterFactory factory{
        [&](CEC::libcec_configuration* config) { return initializer.initialize(config); },
        &destroy_mock};

    EXPECT_THAT(factory.discover(), IsEmpty());
}

TEST(LibCecAdapterFactory, returns_empty_when_adapter_detection_fails)
{
    auto discovery = std::make_unique<StrictMock<MockICECAdapter>>();
    auto* const discovery_ptr = discovery.get();
    EXPECT_CALL(*discovery_ptr, InitVideoStandalone()).Times(AnyNumber());
    EXPECT_CALL(*discovery_ptr, DetectAdapters(_, 16, IsNull(), true)).WillOnce(Return(-1));

    StrictMock<MockInitializer> initializer;
    EXPECT_CALL(initializer, initialize(_)).WillOnce(Return(discovery.release()));
    LibCecAdapterFactory factory{
        [&](CEC::libcec_configuration* config) { return initializer.initialize(config); },
        &destroy_mock};

    EXPECT_THAT(factory.discover(), IsEmpty());
}

TEST(LibCecAdapterFactory, skips_an_adapter_that_fails_to_open)
{
    auto discovery = std::make_unique<StrictMock<MockICECAdapter>>();
    auto failed_adapter = std::make_unique<StrictMock<MockICECAdapter>>();
    auto* const discovery_ptr = discovery.get();
    auto* const failed_adapter_ptr = failed_adapter.get();

    EXPECT_CALL(*discovery_ptr, InitVideoStandalone()).Times(AnyNumber());
    EXPECT_CALL(*discovery_ptr, DetectAdapters(_, 16, IsNull(), true))
        .WillOnce(Invoke([](CEC::cec_adapter_descriptor* descriptors, std::uint8_t, const char*, bool)
        {
            std::snprintf(descriptors[0].strComName, sizeof(descriptors[0].strComName), "cec-fail");
            return 1;
        }));
    EXPECT_CALL(*failed_adapter_ptr, InitVideoStandalone()).Times(AnyNumber());
    EXPECT_CALL(*failed_adapter_ptr, Open(StrEq("cec-fail"), _)).WillOnce(Return(false));
    EXPECT_CALL(*failed_adapter_ptr, Close()).Times(0);

    StrictMock<MockInitializer> initializer;
    EXPECT_CALL(initializer, initialize(_))
        .WillOnce(Return(discovery.release()))
        .WillOnce(Return(failed_adapter.release()));
    LibCecAdapterFactory factory{
        [&](CEC::libcec_configuration* config) { return initializer.initialize(config); },
        &destroy_mock};

    EXPECT_THAT(factory.discover(), IsEmpty());
}

TEST(LibCecAdapterFactory, skips_adapter_when_session_initialization_fails)
{
    auto discovery = std::make_unique<StrictMock<MockICECAdapter>>();
    auto* const discovery_ptr = discovery.get();
    EXPECT_CALL(*discovery_ptr, InitVideoStandalone()).Times(AnyNumber());
    EXPECT_CALL(*discovery_ptr, DetectAdapters(_, 16, IsNull(), true))
        .WillOnce(Invoke([](CEC::cec_adapter_descriptor* descriptors, std::uint8_t, const char*, bool)
        {
            std::snprintf(descriptors[0].strComName, sizeof(descriptors[0].strComName), "cec-unavailable");
            return 1;
        }));

    StrictMock<MockInitializer> initializer;
    EXPECT_CALL(initializer, initialize(_))
        .WillOnce(Return(discovery.release()))
        .WillOnce(Return(nullptr));
    LibCecAdapterFactory factory{
        [&](CEC::libcec_configuration* config) { return initializer.initialize(config); },
        &destroy_mock};

    EXPECT_THAT(factory.discover(), IsEmpty());
}
