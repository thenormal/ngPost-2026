# unit test of the checks done before a Quick Post (no GUI): built and run by tests/run_tests.py
QT       += core
QT       -= gui
CONFIG   += console c++11
CONFIG   -= app_bundle
TARGET    = post_checker_test
TEMPLATE  = app
INCLUDEPATH += ../../src/utils
SOURCES  += post_checker_test.cpp ../../src/utils/PostChecker.cpp ../../src/utils/NfoWriter.cpp
HEADERS  += ../../src/utils/PostChecker.h ../../src/utils/NfoWriter.h
