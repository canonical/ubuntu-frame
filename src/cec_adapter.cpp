#include "libcec_adapter.h"

#include <libcec/cec.h>
#include <mir/log.h>

#include <cstdio>
#include <mutex>
#include <optional>
#include <string>

namespace
{
constexpr char const* log_component = "frame-cec";
std::once_flag libcec_video_initialization;

void init_video_once(CEC::ICECAdapter& connection)
{
    std::call_once(libcec_video_initialization, [&]
    {
        mir::log(mir::logging::Severity::debug, log_component, "Initializing libcec host services");
        connection.InitVideoStandalone();
    });
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
        mir::log(mir::logging::Severity::warning, log_component,
                 "Unable to initialize libcec for adapter %s", port.c_str());
        return {};
    }

    LibCecConnection connection{raw_connection, destroy};
    mir::log(mir::logging::Severity::debug, log_component,
             "Opening libcec adapter %s", port.c_str());
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
        mir::log(mir::logging::Severity::warning, log_component,
                 "Unable to open libcec adapter %s", port.c_str());
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

    auto adapter = std::unique_ptr<LibCecAdapter>{
        new LibCecAdapter{std::move(port), std::move(connection), physical_address}};
    mir::log(mir::logging::Severity::informational, log_component,
             "Opened CEC adapter %s at physical address %s",
             adapter->port().data(),
             physical_address ? std::format("{:04x}", *physical_address).c_str() : "unknown");
    return adapter;
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
    mir::log(mir::logging::Severity::debug, log_component,
             "Sending TV power-on on adapter %s", port_name.c_str());
    bool const sent = connection && connection->PowerOnDevices(CEC::CECDEVICE_TV);
    if (!sent)
        mir::log(mir::logging::Severity::warning, log_component,
                 "CEC TV power-on command failed on adapter %s", port_name.c_str());
    return sent;
}

auto LibCecAdapter::make_active_source() -> bool
{
    mir::log(mir::logging::Severity::debug, log_component,
             "Announcing active source on adapter %s", port_name.c_str());
    bool const sent = connection && connection->SetActiveSource(CEC::CEC_DEVICE_TYPE_PLAYBACK_DEVICE);
    if (!sent)
        mir::log(mir::logging::Severity::warning, log_component,
                 "CEC active-source command failed on adapter %s", port_name.c_str());
    return sent;
}

auto LibCecAdapter::standby_tv() -> bool
{
    mir::log(mir::logging::Severity::debug, log_component,
             "Sending TV standby on adapter %s", port_name.c_str());
    bool const sent = connection && connection->StandbyDevices(CEC::CECDEVICE_TV);
    if (!sent)
        mir::log(mir::logging::Severity::warning, log_component,
                 "CEC TV standby command failed on adapter %s", port_name.c_str());
    return sent;
}

void LibCecAdapter::close()
{
    if (connection)
    {
        mir::log(mir::logging::Severity::debug, log_component,
                 "Closing CEC adapter %s", port_name.c_str());
        connection->Close();
        connection.reset();
    }
}

auto LibCecAdapterFactory::discover() -> std::vector<std::unique_ptr<CecAdapter>>
{
    std::vector<std::unique_ptr<CecAdapter>> adapters;
    mir::log(mir::logging::Severity::debug, log_component, "Discovering CEC adapters");
    CEC::libcec_configuration config;
    config.Clear();
    std::snprintf(config.strDeviceName, sizeof(config.strDeviceName), "Ubuntu Frame");
    config.clientVersion = CEC::LIBCEC_VERSION_CURRENT;
    config.bActivateSource = 0;
    config.deviceTypes.Add(CEC::CEC_DEVICE_TYPE_PLAYBACK_DEVICE);
    auto* const discovery = initialize(&config);
    if (!discovery)
    {
        mir::log(mir::logging::Severity::warning, log_component,
                 "Unable to initialize libcec adapter discovery");
        return adapters;
    }

    init_video_once(*discovery);
    constexpr std::uint8_t max_adapters = 16;
    CEC::cec_adapter_descriptor descriptors[max_adapters]{};
    auto const count = discovery->DetectAdapters(descriptors, max_adapters, nullptr, true);
    destroy(discovery);

    if (count < 0)
    {
        mir::log(mir::logging::Severity::warning, log_component,
                 "libcec adapter discovery failed");
        return adapters;
    }

    mir::log(mir::logging::Severity::informational, log_component,
             "Discovered %d CEC adapter(s)", count);

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
