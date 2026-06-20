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

#include "externaldevicescontroller.h"
#include "settings/externaldevicessettings.h"

#include <core/plugins/coreplugin.h>
#include <core/plugins/plugin.h>
#include <gui/plugins/guiplugin.h>

namespace Fooyin::ExternalDevices {
class ExternalDevicesButton;

class ExternalDevicesPlugin : public QObject,
                              public Plugin,
                              public CorePlugin,
                              public GuiPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.foyin.fooyin.plugin/1.0" FILE "externaldevices.json")
    Q_INTERFACES(Fooyin::Plugin Fooyin::CorePlugin Fooyin::GuiPlugin)

public:
    void initialise(const CorePluginContext& context) override;
    void initialise(const GuiPluginContext& context) override;

private:
    void showExternalDevicesPanel();

    ActionManager* m_actionManager{nullptr};
    SettingsManager* m_settings{nullptr};

    std::unique_ptr<ExternalDevicesSettings> m_externalDevicesSettings;
    std::unique_ptr<ExternalDevicesController> m_controller;
    ExternalDevicesButton* m_menuButton{nullptr};
};
} // namespace Fooyin::ExternalDevices
