cmake_minimum_required(VERSION 3.5)
project(AssociationRules VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

set(SOURCES
    main.cpp mainwindow.cpp mainwindow.h mainwindow.ui
    tabs/Grid/grid.cpp tabs/Grid/grid.h
    tabs/FrequentItemset/frequentitemset.cpp tabs/FrequentItemset/frequentitemset.h
    tabs/Distance/distance.cpp tabs/Distance/distance.h
    tabs/RareItemset/rareitemset.cpp tabs/RareItemset/rareitemset.h
)