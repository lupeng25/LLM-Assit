QT += core gui widgets network
CONFIG += c++14
TEMPLATE = app
TARGET = AIAssit
INCLUDEPATH += $$PWD

SOURCES += \
    $$files($$PWD/*.cpp, true) \
    ChatList.cpp \
    MessageManager.cpp \
    OllamaClient.cpp

HEADERS += \
    $$files($$PWD/*.h, true) \
    ChatList.h \
    DifyClient.h \
    LLMParams.h \
    MessageManager.h \
    OllamaClient.h \
    PromptLibrary.h \
    PromptLibraryDialog.h \
    SyntaxHighlighter.h

FORMS += \
    $$files($$PWD/*.ui, true)

RESOURCES += \
    $$files($$PWD/*.qrc, true)

# cmark integration
CMARK_INC = $$PWD/include/cmark
CMARK_LIB_DIR_MSVS = $$PWD/Lib
INCLUDEPATH += $$CMARK_INC
win32:msvc {
    CMARK_LIB_FILE = $$CMARK_LIB_DIR_MSVS/cmark.lib
    exists($$CMARK_LIB_FILE) {
        LIBS += $$CMARK_LIB_FILE
    }
}

# output directories
CONFIG(debug, debug|release) {
    DESTDIR = $$PWD/debug
} else {
    DESTDIR = $$PWD/release
}

INCLUDEPATH += $$PWD
