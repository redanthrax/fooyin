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

#include "externaldevicesbutton.h"

#include "externaldevicesconstants.h"

#include <gui/guisettings.h>
#include <gui/iconloader.h>
#include <gui/widgets/toolbutton.h>
#include <utils/actions/actionmanager.h>
#include <utils/actions/command.h>
#include <utils/settings/settingsmanager.h>

#include <QAction>
#include <QHBoxLayout>

namespace Fooyin::ExternalDevices {
ExternalDevicesButton::ExternalDevicesButton(ActionManager* actionManager, SettingsManager* settings, QWidget* parent)
    : QWidget{parent}
    , m_settings{settings}
    , m_button{new ToolButton(settings, this)}
{
    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_button);

    m_button->setAutoRaise(true);

    if(auto* showPanelCmd = actionManager->command(Constants::Actions::ShowPanel)) {
        m_button->setDefaultAction(showPanelCmd->action());
        m_settings->subscribe<Settings::Gui::IconTheme>(showPanelCmd->action(), [action = showPanelCmd->action()]() {
            Gui::refreshThemeIcon(action);
        });
    }
}
} // namespace Fooyin::ExternalDevices

#include "moc_externaldevicesbutton.cpp"
