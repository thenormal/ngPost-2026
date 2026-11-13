# unit test of the move of the posted files (no GUI): built and run by tests/run_tests.py
QT       += core
QT       -= gui
CONFIG   += console c++11
CONFIG   -= app_bundle
TARGET    = posted_mover_test
TEMPLATE  = app
INCLUDEPATH += ../../src/utils
SOURCES  += posted_mover_test.cpp ../../src/utils/PostedMover.cpp
HEADERS  += ../../src/utils/PostedMover.h
