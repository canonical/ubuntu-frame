/*
 * Copyright © Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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
    auto send_power_on_tv() -> bool override;
    auto send_make_active_source() -> bool override;
    auto send_standby_tv() -> bool override;
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
