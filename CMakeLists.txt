cmake_minimum_required(VERSION 3.22)

option(MV_UNITY_BUILD "Combine target source files into batches for faster compilation" OFF)

# -----------------------------------------------------------------------------
# Scatterplot Plugin
# -----------------------------------------------------------------------------
set(PROJECT "ScatterplotPlugin")

PROJECT(${PROJECT}
        DESCRIPTION "A scatterplot viewer plugin for ManiVault"
        LANGUAGES CXX
)

# -----------------------------------------------------------------------------
# CMake Options
# -----------------------------------------------------------------------------
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOMOC ON)

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /DWIN32 /EHsc /MP /permissive- /Zc:__cplusplus")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /MD")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")
endif()

# -----------------------------------------------------------------------------
# Dependencies
# -----------------------------------------------------------------------------
find_package(Qt6 COMPONENTS Widgets WebEngineWidgets OpenGL OpenGLWidgets REQUIRED)

find_package(ManiVault COMPONENTS Core PointData ClusterData ColorData ImageData CONFIG QUIET)

# -----------------------------------------------------------------------------
# Source files
# -----------------------------------------------------------------------------
set(PLUGIN
    src/ScatterplotPlugin.h
    src/ScatterplotPlugin.cpp
)

set(UI
    src/ScatterplotWidget.h
    src/ScatterplotWidget.cpp
)

set(Actions
    src/ColoringAction.h
    src/ColoringAction.cpp
    src/DensityPlotAction.h
    src/DensityPlotAction.cpp
    src/ClusteringAction.h
    src/ClusteringAction.cpp
    src/MiscellaneousAction.h
    src/MiscellaneousAction.cpp
    src/PlotAction.h
    src/PlotAction.cpp
    src/PointPlotAction.h
    src/PointPlotAction.cpp
    src/PositionAction.h
    src/PositionAction.cpp
    src/RenderModeAction.h
    src/RenderModeAction.cpp
    src/ScalarAction.h
    src/ScalarAction.cpp
    src/ScalarSourceAction.h
    src/ScalarSourceAction.cpp
    src/SelectionAction.h
    src/SelectionAction.cpp
    src/SettingsAction.h
    src/SettingsAction.cpp
    src/SubsetAction.h
    src/SubsetAction.cpp
    src/ExportAction.h
    src/ExportAction.cpp
    src/DatasetsAction.h
    src/DatasetsAction.cpp
)

set(Models
    src/ColorSourceModel.h
    src/ColorSourceModel.cpp
    src/ScalarSourceModel.h
    src/ScalarSourceModel.cpp
)

set(AUX
    PluginInfo.json
)

set(SOURCES ${PLUGIN} ${UI} ${Actions} ${Models})

source_group(Plugin FILES ${PLUGIN})
source_group(UI FILES ${UI})
source_group(Actions FILES ${Actions})
source_group(Models FILES ${Models})
source_group(Aux FILES ${AUX})

# -----------------------------------------------------------------------------
# CMake Target
# -----------------------------------------------------------------------------
add_library(${PROJECT} SHARED ${SOURCES} ${SHADERS} ${AUX})

# -----------------------------------------------------------------------------
# Target include directories
# -----------------------------------------------------------------------------
target_include_directories(${PROJECT} PRIVATE "${ManiVault_INCLUDE_DIR}")

# -----------------------------------------------------------------------------
# Target properties
# -----------------------------------------------------------------------------
target_compile_features(${PROJECT} PRIVATE cxx_std_20)

if(MV_UNITY_BUILD)
    set_target_properties(${PROJECT} PROPERTIES UNITY_BUILD ON)
endif()

# -----------------------------------------------------------------------------
# Target library linking
# -----------------------------------------------------------------------------
target_link_libraries(${PROJECT} PRIVATE Qt6::Widgets)
target_link_libraries(${PROJECT} PRIVATE Qt6::WebEngineWidgets)
target_link_libraries(${PROJECT} PRIVATE Qt6::OpenGL)
target_link_libraries(${PROJECT} PRIVATE Qt6::OpenGLWidgets)

target_link_libraries(${PROJECT} PRIVATE ManiVault::Core)
target_link_libraries(${PROJECT} PRIVATE ManiVault::PointData)
target_link_libraries(${PROJECT} PRIVATE ManiVault::ClusterData)
target_link_libraries(${PROJECT} PRIVATE ManiVault::ImageData)
target_link_libraries(${PROJECT} PRIVATE ManiVault::ColorData)

# -----------------------------------------------------------------------------
# Target installation
# -----------------------------------------------------------------------------
install(TARGETS ${PROJECT}
    RUNTIME DESTINATION Plugins COMPONENT PLUGINS # Windows .dll
    LIBRARY DESTINATION Plugins COMPONENT PLUGINS # Linux/Mac .so
)

add_custom_command(TARGET ${PROJECT} POST_BUILD
    COMMAND "${CMAKE_COMMAND}"
        --install ${CMAKE_CURRENT_BINARY_DIR}
        --config $<CONFIGURATION>
        --prefix ${ManiVault_INSTALL_DIR}/$<CONFIGURATION>
)

# Read config file
set(PLUGIN_CONFIG_FILE "${CMAKE_CURRENT_SOURCE_DIR}/PluginInfo.json")

if(NOT EXISTS "${PLUGIN_CONFIG_FILE}")
    message(FATAL_ERROR "JSON file not found: ${PLUGIN_CONFIG_FILE}")
endif()

file(READ ${PLUGIN_CONFIG_FILE} PLUGIN_INFO_JSON)

# Check if config file contains certain entries
set(HAS_PLUGIN_TYPE 0)

# Get the number of top-level keys
string(JSON TOP_LEVEL_COUNT LENGTH "${PLUGIN_INFO_JSON}")

math(EXPR LAST_INDEX "${TOP_LEVEL_COUNT} - 1")
foreach(I RANGE 0 ${LAST_INDEX})
    string(JSON KEY_NAME MEMBER "${PLUGIN_INFO_JSON}" ${I})
    if("${KEY_NAME}" STREQUAL "type")
        set(HAS_PLUGIN_TYPE 1)
    endif()
endforeach()

# Extract plugin version
string(JSON PLUGIN_VERSION GET "${PLUGIN_INFO_JSON}" version plugin)
message(STATUS "PLUGIN_VERSION: ${PLUGIN_VERSION}")
set_target_properties(${PROJECT} PROPERTIES
    OUTPUT_NAME  "${PROJECT}_p${PLUGIN_VERSION}_c${ManiVault_VERSION}"
)

# Extract plugin type
if(HAS_PLUGIN_TYPE)
    string(JSON PLUGIN_TYPE GET "${PLUGIN_INFO_JSON}" type)
    set_target_properties(${PROJECT} PROPERTIES
        FOLDER "${PLUGIN_TYPE}Plugins"
    )
endif()

# -----------------------------------------------------------------------------
# Miscellaneous
# -----------------------------------------------------------------------------
# Automatically set the debug environment (command + working directory) for MSVC
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set_property(TARGET ${PROJECT} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY $<IF:$<CONFIG:DEBUG>,${ManiVault_INSTALL_DIR}/Debug,$<IF:$<CONFIG:RELWITHDEBINFO>,${ManiVault_INSTALL_DIR}/RelWithDebInfo,${ManiVault_INSTALL_DIR}/Release>>)
    set_property(TARGET ${PROJECT} PROPERTY VS_DEBUGGER_COMMAND $<IF:$<CONFIG:DEBUG>,"${ManiVault_INSTALL_DIR}/Debug/ManiVault Studio.exe",$<IF:$<CONFIG:RELWITHDEBINFO>,"${ManiVault_INSTALL_DIR}/RelWithDebInfo/ManiVault Studio.exe","${ManiVault_INSTALL_DIR}/Release/ManiVault Studio.exe">>)
endif()
