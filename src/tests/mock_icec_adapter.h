#ifndef FRAME_TESTS_MOCK_ICEC_ADAPTER_H
#define FRAME_TESTS_MOCK_ICEC_ADAPTER_H

#include <gmock/gmock.h>
#include <libcec/cec.h>

class MockICECAdapter : public CEC::ICECAdapter
{
public:
    MOCK_METHOD(bool, PowerOnDevices, (CEC::cec_logical_address), (override));
    MOCK_METHOD(bool, SetActiveSource, (CEC::cec_device_type), (override));
    MOCK_METHOD(bool, StandbyDevices, (CEC::cec_logical_address), (override));
    MOCK_METHOD(bool, Open, (const char*, uint32_t), (override));
    MOCK_METHOD(std::int8_t, DetectAdapters, (CEC::cec_adapter_descriptor*, std::uint8_t, const char*, bool), (override));
    MOCK_METHOD(void, Close, (), (override));
    MOCK_METHOD(bool, GetCurrentConfiguration, (CEC::libcec_configuration*), (override));
    MOCK_METHOD(void, InitVideoStandalone, (), (override));

    bool PingAdapter() override { return false; }
    bool StartBootloader() override { return false; }
    bool Transmit(const CEC::cec_command&) override { return false; }
    bool SetLogicalAddress(CEC::cec_logical_address) override { return false; }
    bool SetPhysicalAddress(uint16_t) override { return false; }
    bool SetDeckControlMode(CEC::cec_deck_control_mode, bool) override { return false; }
    bool SetDeckInfo(CEC::cec_deck_info, bool) override { return false; }
    bool SetInactiveView() override { return false; }
    bool SetMenuState(CEC::cec_menu_state, bool) override { return false; }
    bool SetOSDString(CEC::cec_logical_address, CEC::cec_display_control, const char*) override { return false; }
    bool SwitchMonitoring(bool) override { return false; }
    CEC::cec_version GetDeviceCecVersion(CEC::cec_logical_address) override { return {}; }
    std::string GetDeviceMenuLanguage(CEC::cec_logical_address) override { return {}; }
    uint32_t GetDeviceVendorId(CEC::cec_logical_address) override { return 0; }
    CEC::cec_power_status GetDevicePowerStatus(CEC::cec_logical_address) override { return {}; }
    bool PollDevice(CEC::cec_logical_address) override { return false; }
    CEC::cec_logical_addresses GetActiveDevices() override { return {}; }
    bool IsActiveDevice(CEC::cec_logical_address) override { return false; }
    bool IsActiveDeviceType(CEC::cec_device_type) override { return false; }
    uint8_t VolumeUp(bool) override { return 0; }
    uint8_t VolumeDown(bool) override { return 0; }
#if CEC_LIB_VERSION_MAJOR >= 5
    uint8_t MuteAudio() override { return 0; }
#endif
    bool SendKeypress(CEC::cec_logical_address, CEC::cec_user_control_code, bool) override { return false; }
    bool SendKeyRelease(CEC::cec_logical_address, bool) override { return false; }
    std::string GetDeviceOSDName(CEC::cec_logical_address) override { return {}; }
    CEC::cec_logical_address GetActiveSource() override { return {}; }
    bool IsActiveSource(CEC::cec_logical_address) override { return false; }
    bool SetStreamPath(CEC::cec_logical_address) override { return false; }
    bool SetStreamPath(uint16_t) override { return false; }
    CEC::cec_logical_addresses GetLogicalAddresses() override { return {}; }
    bool SetConfiguration(const CEC::libcec_configuration*) override { return false; }
#if CEC_LIB_VERSION_MAJOR >= 5
    bool CanSaveConfiguration() override { return false; }
#else
    bool CanPersistConfiguration() override { return false; }
    bool PersistConfiguration(CEC::libcec_configuration*) override { return false; }
#endif
    void RescanActiveDevices() override {}
    bool IsLibCECActiveSource() override { return false; }
    bool GetDeviceInformation(const char*, CEC::libcec_configuration*, uint32_t) override { return false; }
#if CEC_LIB_VERSION_MAJOR >= 5
    bool SetCallbacks(CEC::ICECCallbacks*, void*) override { return false; }
    bool DisableCallbacks() override { return false; }
#else
    bool EnableCallbacks(void*, CEC::ICECCallbacks*) override { return false; }
#endif
    bool SetHDMIPort(CEC::cec_logical_address, uint8_t) override { return false; }
    uint16_t GetDevicePhysicalAddress(CEC::cec_logical_address) override { return 0; }
    const char* GetLibInfo() override { return ""; }
    uint16_t GetAdapterVendorId() const override { return 0; }
    uint16_t GetAdapterProductId() const override { return 0; }
    const char* ToString(CEC::cec_menu_state) override { return ""; }
    const char* ToString(CEC::cec_version) override { return ""; }
    const char* ToString(CEC::cec_power_status) override { return ""; }
    const char* ToString(CEC::cec_logical_address) override { return ""; }
    const char* ToString(CEC::cec_deck_control_mode) override { return ""; }
    const char* ToString(CEC::cec_deck_info) override { return ""; }
    const char* ToString(CEC::cec_opcode) override { return ""; }
    const char* ToString(CEC::cec_system_audio_status) override { return ""; }
    const char* ToString(CEC::cec_audio_status) override { return ""; }
    const char* ToString(CEC::cec_device_type) override { return ""; }
    const char* ToString(CEC::cec_user_control_code) override { return ""; }
    const char* ToString(CEC::cec_adapter_type) override { return ""; }
    std::string VersionToString(uint32_t) override { return {}; }
    void PrintVersion(uint32_t, char*, size_t) override {}
    const char* VendorIdToString(uint32_t) override { return ""; }
    uint8_t AudioToggleMute() override { return 0; }
    uint8_t AudioMute() override { return 0; }
    uint8_t AudioUnmute() override { return 0; }
    uint8_t AudioStatus() override { return 0; }
    CEC::cec_command CommandFromString(const char*) override { return {}; }
    bool AudioEnable(bool) override { return false; }
    uint8_t SystemAudioModeStatus() override { return 0; }
#if CEC_LIB_VERSION_MAJOR >= 5
    bool GetStats(CEC::cec_adapter_stats*) override { return false; }
#endif
#if CEC_LIB_VERSION_MAJOR > 8 || (CEC_LIB_VERSION_MAJOR == 8 && CEC_LIB_VERSION_MINOR >= 1)
    bool SendPlay(CEC::cec_logical_address, CEC::cec_play_mode) override { return false; }
#endif
};

#endif // FRAME_TESTS_MOCK_ICEC_ADAPTER_H
