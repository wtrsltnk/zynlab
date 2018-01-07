cmake_minimum_required(VERSION 2.8)

find_package(Qt5 COMPONENTS Quick Core)

add_executable(app.qml WIN32
    main.cpp
    ui/main.qml
    )

target_link_libraries(app.qml Qt5::Quick Qt5::Core)
