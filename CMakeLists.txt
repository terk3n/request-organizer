cmake_minimum_required(VERSION 3.16)
project(RequestOrganizer VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Sql)

set(SOURCES
    src/main.cpp
    src/MainWindow.cpp
    src/OrganizerModel.cpp
    src/OrganizerItem.cpp
    src/DatabaseManager.cpp
    src/HttpSyntaxHighlighter.cpp
)

set(HEADERS
    src/MainWindow.h
    src/OrganizerModel.h
    src/OrganizerItem.h
    src/DatabaseManager.h
    src/HttpSyntaxHighlighter.h
)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

target_link_libraries(${PROJECT_NAME}
    Qt6::Core
    Qt6::Widgets
    Qt6::Sql
)

install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
)
