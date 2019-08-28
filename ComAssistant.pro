QT += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ComAssistant
TEMPLATE = app
 
CONFIG  += debug_and_release warn_on qt thread
win32-msvc*:CONFIG  += embed_manifest_exe

CONFIG(debug, debug|release) {
    DESTDIR  = ./bin/Debug$$arch
    LIBS += -L./bin/Debug$$arch
    OBJECTS_DIR = ./Debug/obj
    MOC_DIR      = ./Debug/moc
    RCC_DIR 	= ./Debug/rcc
} else {
    DESTDIR  = ./bin/Release$$arch
    LIBS += -L./bin/Release$$arch
    OBJECTS_DIR = ./Release/obj
    MOC_DIR       = ./Release/moc
    RCC_DIR 	= ./Release/rcc
}

HEADERS += src/MainWindow.h \
			src/JobThread.h \
			DevDriver/UartDriver.h \
			src/DlgOptions.h \
			src/DlgASCIITable.h \
			src/DataChecker.h
	
SOURCES += src/main.cpp \
			src/MainWindow.cpp \
			src/JobThread.cpp \
			src/DlgOptions.cpp \
			src/DlgASCIITable.cpp \
			src/DataChecker.cpp

FORMS   += ui/main.ui \
			ui/options.ui \
			ui/ASCII.ui 
	
unix:SOURCES  += DevDriver/UartDriver_unix.cpp
win32:SOURCES  += DevDriver/UartDriver_win32.cpp

RESOURCES += ComAssistant.qrc
RC_ICONS = ui/ComAssist.ico

TRANSLATIONS += translations/ComAssistant_zh-cn.ts translations/ComAssistant_zh-tw.ts translations/ComAssistant_en.ts
