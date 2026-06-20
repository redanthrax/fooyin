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

#include "linuxudisksbackend.h"

#include <QDBusArgument>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusObjectPath>
#include <QLoggingCategory>

#include <algorithm>
#include <optional>

using namespace Qt::StringLiterals;

Q_LOGGING_CATEGORY(EXTERNAL_DEVICES_UDISKS, "fy.externaldevices.udisks")

namespace {
constexpr auto UDisksService       = "org.freedesktop.UDisks2";
constexpr auto UDisksPath          = "/org/freedesktop/UDisks2";
constexpr auto ObjectManagerIface  = "org.freedesktop.DBus.ObjectManager";
constexpr auto BlockIface          = "org.freedesktop.UDisks2.Block";
constexpr auto DriveIface          = "org.freedesktop.UDisks2.Drive";
constexpr auto FilesystemIface     = "org.freedesktop.UDisks2.Filesystem";

using ManagedObjects = QMap<QDBusObjectPath, QMap<QString, QVariantMap>>;

[[nodiscard]] QString variantToString(const QVariant& value)
{
    if(value.canConvert<QDBusObjectPath>()) {
        return value.value<QDBusObjectPath>().path();
    }

    if(value.canConvert<QByteArray>()) {
        return QString::fromUtf8(value.toByteArray());
    }

    return value.toString();
}

[[nodiscard]] QString variantMapString(const QVariantMap& map, const QLatin1StringView& key)
{
    return variantToString(map.value(QString{key}));
}

[[nodiscard]] bool variantMapBool(const QVariantMap& map, const QLatin1StringView& key)
{
    return map.value(QString{key}).toBool();
}

[[nodiscard]] quint64 variantMapUInt64(const QVariantMap& map, const QLatin1StringView& key)
{
    return map.value(QString{key}).toULongLong();
}

[[nodiscard]] QString mountPathFromFilesystem(const QVariantMap& filesystem)
{
    const QVariant mountPointsVar = filesystem.value(u"MountPoints"_s);

    QList<QByteArray> mountPoints;
    if(mountPointsVar.canConvert<QDBusArgument>()) {
        mountPointsVar.value<QDBusArgument>() >> mountPoints;
    }
    else if(mountPointsVar.canConvert<QList<QByteArray>>()) {
        mountPoints = mountPointsVar.value<QList<QByteArray>>();
    }
    else {
        const QVariantList list = mountPointsVar.toList();
        mountPoints.reserve(list.size());
        for(const QVariant& item : list) {
            mountPoints.append(item.toByteArray());
        }
    }

    if(mountPoints.isEmpty()) {
        return {};
    }

    return QString::fromUtf8(mountPoints.front());
}

[[nodiscard]] bool isExternalDrive(const QVariantMap& drive)
{
    if(variantMapBool(drive, "Removable"_L1)) {
        return true;
    }

    const QString connectionBus = variantMapString(drive, "ConnectionBus"_L1);
    if(connectionBus == u"usb"_s || connectionBus == u"firewire"_s) {
        return true;
    }

    const QString media = variantMapString(drive, "Media"_L1);
    return media == u"flash"_s || media == u"sd"_s || media == u"floppy"_s;
}

[[nodiscard]] QString driveLabel(const QVariantMap& drive)
{
    const QString vendor = variantMapString(drive, "Vendor"_L1).trimmed();
    const QString model  = variantMapString(drive, "Model"_L1).trimmed();

    if(!vendor.isEmpty() && !model.isEmpty()) {
        return u"%1 %2"_s.arg(vendor, model);
    }

    if(!model.isEmpty()) {
        return model;
    }

    if(!vendor.isEmpty()) {
        return vendor;
    }

    return {};
}

[[nodiscard]] QString blockLabel(const QVariantMap& block, const QVariantMap& drive)
{
    if(const QString idLabel = variantMapString(block, "IdLabel"_L1); !idLabel.isEmpty()) {
        return idLabel;
    }

    if(const QString label = driveLabel(drive); !label.isEmpty()) {
        return label;
    }

    return variantMapString(block, "Device"_L1);
}

[[nodiscard]] bool shouldIncludeBlock(const QVariantMap& block, const QVariantMap& drive)
{
    if(variantMapBool(block, "HintSystem"_L1) || variantMapBool(block, "HintIgnore"_L1)) {
        return false;
    }

    if(drive.isEmpty()) {
        return false;
    }

    if(!isExternalDrive(drive)) {
        return false;
    }

    // Skip whole-disk block devices; list mountable filesystem partitions only.
    return variantMapString(block, "IdUsage"_L1) == u"filesystem"_s;
}

[[nodiscard]] std::optional<ManagedObjects> queryManagedObjects()
{
    QDBusMessage message = QDBusMessage::createMethodCall(QString::fromLatin1(UDisksService),
                                                          QString::fromLatin1(UDisksPath),
                                                          QString::fromLatin1(ObjectManagerIface), u"GetManagedObjects"_s);
    const QDBusMessage reply = QDBusConnection::systemBus().call(message);
    if(reply.type() != QDBusMessage::ReplyMessage || reply.arguments().isEmpty()) {
        qCWarning(EXTERNAL_DEVICES_UDISKS) << "Failed to query UDisks managed objects:" << reply.errorMessage();
        return std::nullopt;
    }

    return qdbus_cast<ManagedObjects>(reply.arguments().constFirst());
}

[[nodiscard]] QList<Fooyin::ExternalDevices::ExternalDevice> enumerateExternalDevices()
{
    const auto managedObjects = queryManagedObjects();
    if(!managedObjects) {
        return {};
    }

    QList<Fooyin::ExternalDevices::ExternalDevice> devices;

    for(auto it = managedObjects->cbegin(); it != managedObjects->cend(); ++it) {
        const QString objectPath = it.key().path();
        const QMap<QString, QVariantMap>& interfaces = it.value();

        const auto blockIt = interfaces.find(QString::fromLatin1(BlockIface));
        if(blockIt == interfaces.end()) {
            continue;
        }

        const QVariantMap& block = blockIt.value();
        const QString drivePath  = variantMapString(block, "Drive"_L1);
        if(drivePath.isEmpty() || drivePath == u"/"_s) {
            continue;
        }

        const auto driveIt = managedObjects->find(QDBusObjectPath{drivePath});
        if(driveIt == managedObjects->cend()) {
            continue;
        }

        const auto driveIfaceIt = driveIt->find(QString::fromLatin1(DriveIface));
        if(driveIfaceIt == driveIt->cend()) {
            continue;
        }

        const QVariantMap& drive = driveIfaceIt.value();
        if(!shouldIncludeBlock(block, drive)) {
            continue;
        }

        Fooyin::ExternalDevices::ExternalDevice device;
        device.id        = objectPath;
        device.label     = blockLabel(block, drive);
        device.size      = variantMapUInt64(block, "Size"_L1);
        device.isReadOnly = variantMapBool(block, "ReadOnly"_L1);

        if(const auto filesystemIt = interfaces.find(QString::fromLatin1(FilesystemIface));
           filesystemIt != interfaces.end()) {
            device.mountPath = mountPathFromFilesystem(filesystemIt.value());
            device.isMounted = !device.mountPath.isEmpty();
            device.isReadOnly = device.isReadOnly || variantMapBool(filesystemIt.value(), "ReadOnly"_L1);
        }

        if(device.label.isEmpty()) {
            device.label = objectPath;
        }

        devices.push_back(device);
    }

    std::ranges::sort(devices, {}, &Fooyin::ExternalDevices::ExternalDevice::label);
    return devices;
}
} // namespace

namespace Fooyin::ExternalDevices {
LinuxUdisksBackend::LinuxUdisksBackend(QObject* parent)
    : ExternalDeviceBackend{parent}
{ }

bool LinuxUdisksBackend::isAvailable() const
{
    if(!QDBusConnection::systemBus().isConnected()) {
        return false;
    }

    QDBusMessage message
        = QDBusMessage::createMethodCall(QString::fromLatin1(UDisksService), QString::fromLatin1(UDisksPath),
                                         QString::fromLatin1(ObjectManagerIface), u"GetManagedObjects"_s);
    const QDBusMessage reply = QDBusConnection::systemBus().call(message);
    return reply.type() == QDBusMessage::ReplyMessage;
}

QList<ExternalDevice> LinuxUdisksBackend::devices() const
{
    return m_devices;
}

void LinuxUdisksBackend::startMonitoring()
{
    if(m_monitoring) {
        return;
    }

    QDBusConnection bus = QDBusConnection::systemBus();
    if(!bus.isConnected()) {
        return;
    }

    const auto connected = bus.connect(QString::fromLatin1(UDisksService), QString::fromLatin1(UDisksPath),
                                     QString::fromLatin1(ObjectManagerIface), u"InterfacesAdded"_s, this,
                                     SLOT(handleDeviceChange()));
    const auto removedConnected = bus.connect(QString::fromLatin1(UDisksService), QString::fromLatin1(UDisksPath),
                                              QString::fromLatin1(ObjectManagerIface), u"InterfacesRemoved"_s, this,
                                              SLOT(handleDeviceChange()));

    if(!connected || !removedConnected) {
        qCWarning(EXTERNAL_DEVICES_UDISKS) << "Failed to subscribe to UDisks device changes";
        return;
    }

    m_monitoring = true;
    refreshDevices();
}

void LinuxUdisksBackend::stopMonitoring()
{
    if(!m_monitoring) {
        return;
    }

    QDBusConnection bus = QDBusConnection::systemBus();
    bus.disconnect(QString::fromLatin1(UDisksService), QString::fromLatin1(UDisksPath),
                   QString::fromLatin1(ObjectManagerIface), u"InterfacesAdded"_s, this, SLOT(handleDeviceChange()));
    bus.disconnect(QString::fromLatin1(UDisksService), QString::fromLatin1(UDisksPath),
                   QString::fromLatin1(ObjectManagerIface), u"InterfacesRemoved"_s, this, SLOT(handleDeviceChange()));

    m_monitoring = false;
}

void LinuxUdisksBackend::refreshDevices()
{
    const QList<ExternalDevice> devices = enumerateExternalDevices();
    if(devices == m_devices) {
        return;
    }

    m_devices = devices;
    Q_EMIT devicesChanged();
}

void LinuxUdisksBackend::handleDeviceChange()
{
    refreshDevices();
}
} // namespace Fooyin::ExternalDevices

#include "moc_linuxudisksbackend.cpp"
