QT += core gui widgets network xml
CONFIG += c++14
TEMPLATE = app
TARGET = AIAssit
INCLUDEPATH += $$PWD

SOURCES += \
    $$files($$PWD/*.cpp, true) \
    ChatList.cpp \
    MessageManager.cpp \
    OllamaClient.cpp \
    ShortcutManager.cpp \
    ShortcutEdit.cpp

HEADERS += \
    $$files($$PWD/*.h, true) \
    ChatList.h \
    ClipboardTools.h \
    FunctionCallRouter.h \
    VisionTools.h \
    GitTools.h \
    DataFormatTools.h \
    DateTimeTools.h \
    DifyClient.h \
    FileSystemTools.h \
    LLMParams.h \
    MessageManager.h \
    OllamaClient.h \
    PromptLibrary.h \
    PromptLibraryDialog.h \
    SyntaxHighlighter.h \
    SystemInfoTools.h \
    TextProcessingTools.h \
    UtilityTools.h \
    ShortcutManager.h \
    ShortcutEdit.h

FORMS += \
    $$files($$PWD/*.ui, true)

RESOURCES += \
    $$files($$PWD/*.qrc, true) \
    AIIcon.qrc

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

# Copy FunctionCall.json to output directory
# Note: FunctionCall.json should be manually copied to debug/AIAssit/ and release/AIAssit/ directories
# Or it will be created at runtime if it doesn't exist

INCLUDEPATH += $$PWD
