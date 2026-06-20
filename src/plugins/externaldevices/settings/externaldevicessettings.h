/*
 * Fooyin
 * Copyright © 2026, Luke Taylor <luket@pm.me>
 *
 * Fooyin is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Fooyin is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Fooyin.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#pragma once

#include <utils/settings/settingsentry.h>

#include <QObject>

namespace Fooyin {
class SettingsManager;

namespace Settings::ExternalDevices {
Q_NAMESPACE
enum ExternalDevicesSettings : uint32_t
{
    Enabled = 1 | Type::Bool,
};
Q_ENUM_NS(ExternalDevicesSettings)
} // namespace Settings::ExternalDevices

namespace ExternalDevices {
class ExternalDevicesSettings
{
public:
    explicit ExternalDevicesSettings(SettingsManager* settingsManager);

private:
    SettingsManager* m_settings;
};
} // namespace ExternalDevices
} // namespace Fooyin
