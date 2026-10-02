#include "libcec_adapter.h"

#include <libcec/cec.h>
#include <mir/log.h>

#include <cstdio>
#include <mutex>
#include <optional>
#include <string>

namespace
{
std::once_flag libcec_video_initialization;

void init_video_once(CEC::ICECAdapter& connection)
{
    std::call_once(libcec_video_initialization, [&] { connection.InitVideoStandalone(); });
}

auto open_adapter(
    std::string port,
    LibCecAdapterFactory::Initialize initialize,
    LibCecAdapterFactory::Destroy destroy) -> std::unique_ptr<CecAdapter>
{
    CEC::libcec_configuration config;
    config.Clear();
    std::snprintf(config.strDeviceName, sizeof(config.strDeviceName), "Ubuntu Frame");
    config.clientVersion = CEC::LIBCEC_VERSION_CURRENT;
    config.bActivateSource = 0;
    config.deviceTypes.Add(CEC::CEC_DEVICE_TYPE_PLAYBACK_DEVICE);
    auto* const raw_connection = initialize(&config);
    if (!raw_connection)
    {
        mir::log_warning("Unable to initialize libcec for adapter %s", port.c_str());
        return {};
    }

    LibCecConnection connection{raw_connection, destroy};
    return LibCecAdapter::open(std::move(port), std::move(connection));
}
}

auto LibCecAdapter::open(std::string port, LibCecConnection connection)
    -> std::unique_ptr<LibCecAdapter>
{
    if (!connection)
        return {};

    init_video_once(*connection);
    if (!connection->Open(port.c_str()))
    {
        mir::log_warning("Unable to open libcec adapter %s", port.c_str());
        return {};
    }

    CEC::libcec_configuration current;
    current.Clear();
    std::optional<std::uint16_t> physical_address;
    if (connection->GetCurrentConfiguration(&current) &&
        current.iPhysicalAddress != CEC_INVALID_PHYSICAL_ADDRESS)
    {
        physical_address = current.iPhysicalAddress;
    }

    return std::unique_ptr<LibCecAdapter>{
        new LibCecAdapter{std::move(port), std::move(connection), physical_address}};
}

LibCecAdapter::LibCecAdapter(
    std::string port,
    LibCecConnection connection,
    std::optional<std::uint16_t> physical_address)
    : port_name{std::move(port)}, connection{std::move(connection)}, address{physical_address}
{
}

LibCecAdapter::~LibCecAdapter()
{
    close();
}

auto LibCecAdapter::port() const -> std::string_view
{
    return port_name;
}

auto LibCecAdapter::physical_address() const -> std::optional<std::uint16_t>
{
    return address;
}

auto LibCecAdapter::power_on_tv() -> bool
{
    bool const sent = connection && connection->PowerOnDevices(CEC::CECDEVICE_TV);
    if (!sent)
        mir::log_warning("CEC TV power-on command failed on adapter %s", port_name.c_str());
    return sent;
}

auto LibCecAdapter::make_active_source() -> bool
{
    bool const sent = connection && connection->SetActiveSource(CEC::CEC_DEVICE_TYPE_PLAYBACK_DEVICE);
    if (!sent)
        mir::log_warning("CEC active-source command failed on adapter %s", port_name.c_str());
    return sent;
}

auto LibCecAdapter::standby_tv() -> bool
{
    bool const sent = connection && connection->StandbyDevices(CEC::CECDEVICE_TV);
    if (!sent)
        mir::log_warning("CEC TV standby command failed on adapter %s", port_name.c_str());
    return sent;
}

void LibCecAdapter::close()
{
    if (connection)
    {
        connection->Close();
        connection.reset();
    }
}

auto LibCecAdapterFactory::discover() -> std::vector<std::unique_ptr<CecAdapter>>
{
    std::vector<std::unique_ptr<CecAdapter>> adapters;
    CEC::libcec_configuration config;
    config.Clear();
    std::snprintf(config.strDeviceName, sizeof(config.strDeviceName), "Ubuntu Frame");
    config.clientVersion = CEC::LIBCEC_VERSION_CURRENT;
    config.bActivateSource = 0;
    config.deviceTypes.Add(CEC::CEC_DEVICE_TYPE_PLAYBACK_DEVICE);
    auto* const discovery = initialize(&config);
    if (!discovery)
    {
        mir::log_warning("Unable to initialize libcec adapter discovery");
        return adapters;
    }

    init_video_once(*discovery);
    constexpr std::uint8_t max_adapters = 16;
    CEC::cec_adapter_descriptor descriptors[max_adapters]{};
    auto const count = discovery->DetectAdapters(descriptors, max_adapters, nullptr, true);
    destroy(discovery);

    if (count < 0)
    {
        mir::log_warning("libcec adapter discovery failed");
        return adapters;
    }

    for (std::int8_t i = 0; i < count; ++i)
    {
        if (auto adapter = open_adapter(descriptors[i].strComName, initialize, destroy))
            adapters.push_back(std::move(adapter));
    }
    return adapters;
}

LibCecAdapterFactory::LibCecAdapterFactory(Initialize initialize, Destroy destroy)
        : initialize{initialize ? std::move(initialize) : Initialize{&CECInitialise}},
            destroy{destroy}
{
}
