cmake_minimum_required(VERSION 3.20)
project(SecurityGuardManager VERSION 1.0.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
find_package(Qt6 REQUIRED COMPONENTS Widgets Sql PrintSupport)
find_package(Qt6 QUIET COMPONENTS Charts)
if(Qt6Charts_FOUND)
    message(STATUS "Qt6 Charts found")
else()
    message(STATUS "Qt6 Charts not found - building without charts (OK)")
endif()

set(SOURCES
    src/main.cpp src/database/DatabaseManager.cpp
    src/ui/LoginDialog.cpp src/ui/MainWindow.cpp src/ui/DashboardWidget.cpp
    src/ui/GuardWidget.cpp src/ui/GuardDialog.cpp
    src/ui/ClientWidget.cpp src/ui/ClientDialog.cpp
    src/ui/SiteWidget.cpp src/ui/SiteDialog.cpp
    src/ui/AttendanceWidget.cpp
    src/ui/DutyWidget.cpp src/ui/DutyDialog.cpp
    src/ui/LeaveWidget.cpp src/ui/LeaveDialog.cpp
    src/ui/SalaryWidget.cpp
    src/ui/UniformWidget.cpp src/ui/UniformDialog.cpp
    src/ui/EquipmentWidget.cpp src/ui/EquipmentDialog.cpp
    src/ui/VisitorWidget.cpp src/ui/VisitorDialog.cpp
    src/ui/VehicleWidget.cpp src/ui/VehicleDialog.cpp
    src/ui/IncidentWidget.cpp src/ui/IncidentDialog.cpp
    src/ui/TrainingWidget.cpp src/ui/TrainingDialog.cpp
    src/ui/DocumentWidget.cpp src/ui/DocumentDialog.cpp
    src/ui/StyleManager.cpp
)

set(HEADERS
    src/database/DatabaseManager.h
    src/ui/LoginDialog.h src/ui/MainWindow.h src/ui/DashboardWidget.h
    src/ui/GuardWidget.h src/ui/GuardDialog.h
    src/ui/ClientWidget.h src/ui/ClientDialog.h
    src/ui/SiteWidget.h src/ui/SiteDialog.h
    src/ui/AttendanceWidget.h
    src/ui/DutyWidget.h src/ui/DutyDialog.h
    src/ui/LeaveWidget.h src/ui/LeaveDialog.h
    src/ui/SalaryWidget.h
    src/ui/UniformWidget.h src/ui/UniformDialog.h
    src/ui/EquipmentWidget.h src/ui/EquipmentDialog.h
    src/ui/VisitorWidget.h src/ui/VisitorDialog.h
    src/ui/VehicleWidget.h src/ui/VehicleDialog.h
    src/ui/IncidentWidget.h src/ui/IncidentDialog.h
    src/ui/TrainingWidget.h src/ui/TrainingDialog.h
    src/ui/DocumentWidget.h src/ui/DocumentDialog.h
    src/ui/StyleManager.h
)

set(RESOURCES src/resources/resources.qrc)
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS} ${RESOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Widgets Qt6::Sql Qt6::PrintSupport)
if(Qt6Charts_FOUND)
    target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Charts)
    target_compile_definitions(${PROJECT_NAME} PRIVATE HAS_QT_CHARTS)
endif()
if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE $<CONFIG:Release>)
endif()
