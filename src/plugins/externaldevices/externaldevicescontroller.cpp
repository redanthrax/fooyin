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

#include "externaldevicescontroller.h"

#include "backends/externaldevicebackend.h"
#include "backends/stubexternaldevicebackend.h"
#include "settings/externaldevicessettings.h"

#if defined(Q_OS_LINUX)
#include "backends/linuxudisksbackend.h"
#endif

#include <utils/settings/settingsmanager.h>

namespace Fooyin::ExternalDevices {
namespace {
std::unique_ptr<ExternalDeviceBackend> createExternalDeviceBackend(QObject* parent)
{
#if defined(Q_OS_LINUX)
    auto udisksBackend = std::make_unique<LinuxUdisksBackend>(parent);
    if(udisksBackend->isAvailable()) {
        return udisksBackend;
    }
#endif

    return std::make_unique<StubExternalDeviceBackend>(parent);
}
} // namespace

ExternalDevicesController::ExternalDevicesController(SettingsManager* settings, QObject* parent)
    : QObject{parent}
    , m_settings{settings}
    , m_backend{createExternalDeviceBackend(this)}
{
    QObject::connect(m_backend.get(), &ExternalDeviceBackend::devicesChanged, this, &ExternalDevicesController::devicesChanged);

    if(m_settings->value<Settings::ExternalDevices::Enabled>()) {
        m_backend->startMonitoring();
    }
}

ExternalDevicesController::~ExternalDevicesController() = default;

bool ExternalDevicesController::isMonitoring() const
{
    return m_backend != nullptr;
}

QList<ExternalDevice> ExternalDevicesController::devices() const
{
    return m_backend ? m_backend->devices() : QList<ExternalDevice>{};
}

void ExternalDevicesController::refreshDevices()
{
    if(m_backend) {
        m_backend->refreshDevices();
    }
}
} // namespace Fooyin::ExternalDevices

#include "moc_externaldevicescontroller.cpp"
