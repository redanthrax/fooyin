find_package(
    Qt6
    COMPONENTS DBus
    QUIET
)

set(EXTERNAL_DEVICES_SOURCES
    backends/externaldevicebackend.h
    backends/stubexternaldevicebackend.cpp
    backends/stubexternaldevicebackend.h
    externaldevice.h
    externaldevicesbutton.cpp
    externaldevicesbutton.h
    externaldevicesconstants.h
    externaldevicescontroller.cpp
    externaldevicescontroller.h
    externaldevicesdialog.cpp
    externaldevicesdialog.h
    externaldevicesplugin.cpp
    externaldevicesplugin.h
    settings/externaldevicessettings.cpp
    settings/externaldevicessettings.h
)

set(EXTERNAL_DEVICES_DEPENDS Fooyin::Core Fooyin::Gui)

if(Qt6DBus_FOUND)
    list(APPEND EXTERNAL_DEVICES_SOURCES backends/linuxudisksbackend.cpp backends/linuxudisksbackend.h)
    list(APPEND EXTERNAL_DEVICES_DEPENDS Qt6::DBus)
else()
    message(STATUS "Qt6::DBus not found; externaldevices plugin will use stub device backend only.")
endif()

create_fooyin_plugin_internal(
    externaldevices
    DEPENDS ${EXTERNAL_DEVICES_DEPENDS}
    SOURCES ${EXTERNAL_DEVICES_SOURCES}
)
