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

#include "externaldevicesplugin.h"

#include "externaldevicesbutton.h"
#include "externaldevicesconstants.h"
#include "externaldevicesdialog.h"

#include <gui/guiconstants.h>
#include <gui/iconloader.h>
#include <gui/windowcontroller.h>
#include <utils/actions/actionmanager.h>
#include <utils/actions/command.h>
#include <utils/settings/settingsmanager.h>
#include <utils/utils.h>

#include <QAction>
#include <QMainWindow>
#include <QMenuBar>

namespace Fooyin::ExternalDevices {
void ExternalDevicesPlugin::initialise(const CorePluginContext& context)
{
    m_settings = context.settingsManager;

    m_externalDevicesSettings = std::make_unique<ExternalDevicesSettings>(m_settings);
    m_controller              = std::make_unique<ExternalDevicesController>(m_settings);
}

void ExternalDevicesPlugin::initialise(const GuiPluginContext& context)
{
    m_actionManager = context.actionManager;

    auto* showPanelAction = new QAction(tr("External Devices"), this);
    Gui::setThemeIcon(showPanelAction, Fooyin::Constants::Icons::ExternalDevice);
    showPanelAction->setToolTip(tr("External Devices"));

    auto* showPanelCmd = m_actionManager->registerAction(showPanelAction, Constants::Actions::ShowPanel);
    showPanelCmd->setCategories({tr("External Devices")});

    QObject::connect(showPanelAction, &QAction::triggered, this, &ExternalDevicesPlugin::showExternalDevicesPanel);

    if(QMenuBar* menuBar = context.windowController->mainWindow()->menuBar()) {
        m_menuButton = new ExternalDevicesButton(m_actionManager, m_settings, menuBar);
        menuBar->setCornerWidget(m_menuButton, Qt::TopRightCorner);
    }
}

void ExternalDevicesPlugin::showExternalDevicesPanel()
{
    m_controller->refreshDevices();

    auto* dialog = new ExternalDevicesDialog(m_controller.get(), Utils::getMainWindow());
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();
}
} // namespace Fooyin::ExternalDevices

#include "moc_externaldevicesplugin.cpp"
