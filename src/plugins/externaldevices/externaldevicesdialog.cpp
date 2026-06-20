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

#include "externaldevicesdialog.h"

#include "externaldevicescontroller.h"

#include <gui/guiconstants.h>
#include <gui/iconloader.h>
#include <utils/stringutils.h>
#include <utils/utils.h>

#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>

using namespace Qt::StringLiterals;

namespace Fooyin::ExternalDevices {
namespace {
QString formatDeviceSize(const quint64 size)
{
    if(size == 0) {
        return {};
    }

    return Utils::formatFileSize(static_cast<qint64>(size));
}

QString deviceDescription(const ExternalDevice& device)
{
    QStringList parts;

    if(device.isMounted) {
        parts.append(device.mountPath);
    }
    else {
        parts.append(ExternalDevicesDialog::tr("Not mounted"));
    }

    if(const QString sizeText = formatDeviceSize(device.size); !sizeText.isEmpty()) {
        parts.append(sizeText);
    }

    if(device.isReadOnly) {
        parts.append(ExternalDevicesDialog::tr("Read-only"));
    }

    return parts.join(u" • "_s);
}
} // namespace

ExternalDevicesDialog::ExternalDevicesDialog(ExternalDevicesController* controller, QWidget* parent)
    : QDialog{parent}
    , m_controller{controller}
    , m_deviceList{new QListWidget(this)}
    , m_emptyLabel{new QLabel(this)}
{
    setWindowTitle(tr("External Devices"));
    setWindowIcon(Gui::iconFromTheme(Fooyin::Constants::Icons::ExternalDevice));

    m_deviceList->setSelectionMode(QAbstractItemView::SingleSelection);
    m_deviceList->setAlternatingRowColors(true);

    m_emptyLabel->setWordWrap(true);
    m_emptyLabel->setAlignment(Qt::AlignCenter);
    m_emptyLabel->setText(tr("No external devices detected."));

    auto* refreshButton = new QPushButton(tr("Refresh"), this);
    QObject::connect(refreshButton, &QPushButton::clicked, m_controller, &ExternalDevicesController::refreshDevices);

    auto* buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch();
    buttonLayout->addWidget(refreshButton);

    auto* layout = new QVBoxLayout(this);
    layout->addWidget(m_deviceList, 1);
    layout->addWidget(m_emptyLabel);
    layout->addLayout(buttonLayout);

    populateDevices();
    updateEmptyState();

    QObject::connect(m_controller, &ExternalDevicesController::devicesChanged, this, [this]() {
        populateDevices();
        updateEmptyState();
    });

    resize(640, 420);
}

QSize ExternalDevicesDialog::sizeHint() const
{
    return {640, 420};
}

void ExternalDevicesDialog::populateDevices()
{
    m_deviceList->clear();

    for(const ExternalDevice& device : m_controller->devices()) {
        auto* item = new QListWidgetItem(device.label, m_deviceList);
        item->setToolTip(device.id);
        item->setData(Qt::UserRole, device.id);
        item->setData(Qt::UserRole + 1, device.mountPath);
        item->setStatusTip(deviceDescription(device));
    }
}

void ExternalDevicesDialog::updateEmptyState()
{
    const bool hasDevices = m_deviceList->count() > 0;
    m_deviceList->setVisible(hasDevices);
    m_emptyLabel->setVisible(!hasDevices);
}
} // namespace Fooyin::ExternalDevices

#include "moc_externaldevicesdialog.cpp"
