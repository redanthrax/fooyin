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

#include "externaldevicebackend.h"

namespace Fooyin::ExternalDevices {
class LinuxUdisksBackend : public ExternalDeviceBackend
{
    Q_OBJECT

public:
    explicit LinuxUdisksBackend(QObject* parent = nullptr);

    [[nodiscard]] bool isAvailable() const override;
    [[nodiscard]] QList<ExternalDevice> devices() const override;

    void startMonitoring() override;
    void stopMonitoring() override;
    void refreshDevices() override;

private Q_SLOTS:
    void handleDeviceChange();

private:
    QList<ExternalDevice> m_devices;
    bool m_monitoring{false};
};
} // namespace Fooyin::ExternalDevices
