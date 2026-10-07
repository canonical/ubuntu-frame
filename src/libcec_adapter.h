#ifndef FRAME_LIBCEC_ADAPTER_H
#define FRAME_LIBCEC_ADAPTER_H

#include "cec_adapter.h"

#include <libcec/cec.h>

#include <functional>
#include <optional>
#include <string>
#include <vector>

using LibCecConnection = std::unique_ptr<CEC::ICECAdapter, void (*)(CEC::ICECAdapter*)>;

class LibCecAdapter final : public CecAdapter
{
public:
    static auto open(std::string port, LibCecConnection connection) -> std::unique_ptr<LibCecAdapter>;
    ~LibCecAdapter() override;

    auto port() const -> std::string_view override;
    auto physical_address() const -> std::optional<std::uint16_t> override;
    auto power_on_tv() -> bool override;
    auto make_active_source() -> bool override;
    auto standby_tv() -> bool override;
    void close() override;

private:
    LibCecAdapter(std::string port, LibCecConnection connection, std::optional<std::uint16_t> physical_address);

    std::string const port_name;
    LibCecConnection connection;
    std::optional<std::uint16_t> const address;
};

class LibCecAdapterFactory final : public CecAdapterFactory
{
public:
    using Initialize = std::function<CEC::ICECAdapter*(CEC::libcec_configuration*)>;
    using Destroy = void (*)(CEC::ICECAdapter*);

    explicit LibCecAdapterFactory(Initialize initialize = {}, Destroy destroy = &CECDestroy);

    auto discover() -> std::vector<std::unique_ptr<CecAdapter>> override;

private:
    Initialize const initialize;
    Destroy const destroy;
};

#endif // FRAME_LIBCEC_ADAPTER_H
