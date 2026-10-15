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

#ifndef FRAME_CEC_OUTPUT_CONFIGURATION_H
#define FRAME_CEC_OUTPUT_CONFIGURATION_H

#include "cec_manager.h"

#include <miral/output_configuration.h>

#include <memory>
#include <span>

class CecOutputConfiguration final : public miral::OutputConfiguration::Strategy
{
public:
    explicit CecOutputConfiguration(std::shared_ptr<CecManager> manager);

    void apply_configuration(
        std::span<mir::graphics::UserDisplayConfigurationOutput> outputs) override;
    void confirm_configuration(
        std::span<mir::graphics::DisplayConfigurationOutput const> outputs) override;

private:
    std::shared_ptr<CecManager> const manager;
};

#endif // FRAME_CEC_OUTPUT_CONFIGURATION_H
