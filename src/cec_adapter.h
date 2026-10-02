#ifndef FRAME_CEC_ADAPTER_H
#define FRAME_CEC_ADAPTER_H

#include <cstdint>
#include <memory>
#include <optional>
#include <string_view>
#include <vector>

class CecAdapter
{
public:
    virtual ~CecAdapter() = default;

    virtual auto port() const -> std::string_view = 0;
    virtual auto physical_address() const -> std::optional<std::uint16_t> = 0;
    virtual auto power_on_tv() -> bool = 0;
    virtual auto make_active_source() -> bool = 0;
    virtual auto standby_tv() -> bool = 0;
    virtual void close() = 0;
};

class CecAdapterFactory
{
public:
    virtual ~CecAdapterFactory() = default;
    virtual auto discover() -> std::vector<std::unique_ptr<CecAdapter>> = 0;
};

class LibCecAdapterFactory final : public CecAdapterFactory
{
public:
    auto discover() -> std::vector<std::unique_ptr<CecAdapter>> override;
};

#endif // FRAME_CEC_ADAPTER_H
