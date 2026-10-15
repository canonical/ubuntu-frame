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
    virtual auto send_power_on_tv() -> bool = 0;
    virtual auto send_make_active_source() -> bool = 0;
    virtual auto send_standby_tv() -> bool = 0;
    virtual void close() = 0;
};

class CecAdapterFactory
{
public:
    virtual ~CecAdapterFactory() = default;
    virtual auto discover() -> std::vector<std::unique_ptr<CecAdapter>> = 0;
};

#endif // FRAME_CEC_ADAPTER_H
