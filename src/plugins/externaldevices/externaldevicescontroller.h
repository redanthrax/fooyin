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

#include "externaldevice.h"

#include <QList>
#include <QObject>

#include <memory>

namespace Fooyin {
class SettingsManager;

namespace ExternalDevices {
class ExternalDeviceBackend;

class ExternalDevicesController : public QObject
{
    Q_OBJECT

public:
    explicit ExternalDevicesController(SettingsManager* settings, QObject* parent = nullptr);
    ~ExternalDevicesController() override;

    [[nodiscard]] bool isMonitoring() const;
    [[nodiscard]] QList<ExternalDevice> devices() const;

    void refreshDevices();

Q_SIGNALS:
    void devicesChanged();

private:
    SettingsManager* m_settings;
    std::unique_ptr<ExternalDeviceBackend> m_backend;
};
} // namespace ExternalDevices
} // namespace Fooyin
