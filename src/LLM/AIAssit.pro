QT += core gui widgets network xml
CONFIG += c++14
TEMPLATE = app
TARGET = AIAssit
INCLUDEPATH += $$PWD

SOURCES += \
    AIParamWidget.cpp \
    AppConfigRepository.cpp \
    ChatInputWidget.cpp \
    ChatList.cpp \
    ChatSessionService.cpp \
    ChatShowWidget.cpp \
    ClipboardTools.cpp \
    DataFormatTools.cpp \
    DateTimeTools.cpp \
    DifyClient.cpp \
    FileSystemTools.cpp \
    Frm_AIAssit.cpp \
    FunctionCallRouter.cpp \
    GitLogReader.cpp \
    GitTools.cpp \
    LLMChatFrame.cpp \
    LLMClientManager.cpp \
    LLMFunctionCall.cpp \
    LLMParams.cpp \
    main.cpp \
    MessageManager.cpp \
    ModelSelectorWidget.cpp \
    OllamaClient.cpp \
    Open_WebUIClient.cpp \
    PromptLibrary.cpp \
    PromptLibraryDialog.cpp \
    ShortcutEdit.cpp \
    ShortcutManager.cpp \
    SyntaxHighlighter.cpp \
    SystemInfoTools.cpp \
    TextProcessingTools.cpp \
    UtilityTools.cpp \
    VisionTools.cpp

HEADERS += \
    AIParamWidget.h \
    AppConfigRepository.h \
    ChatInputWidget.h \
    ChatList.h \
    ChatSessionService.h \
    ChatSessionTypes.h \
    ChatShowWidget.h \
    ClipboardTools.h \
    CommonTypes.h \
    DataFormatTools.h \
    DateTimeTools.h \
    DifyClient.h \
    FileSystemTools.h \
    Frm_AIAssit.h \
    FunctionCallRouter.h \
    GitLogReader.h \
    GitTools.h \
    LLMChatFrame.h \
    LLMClientManager.h \
    LLMFunctionCall.h \
    LLMParams.h \
    MessageManager.h \
    ModelSelectorWidget.h \
    OllamaClient.h \
    Open_WebUIClient.h \
    PromptLibrary.h \
    PromptLibraryDialog.h \
    ShortcutEdit.h \
    ShortcutManager.h \
    SyntaxHighlighter.h \
    SystemInfoTools.h \
    TextProcessingTools.h \
    UtilityTools.h \
    VisionTools.h

FORMS += \
    Frm_AIAssit.ui

RESOURCES += \
    AIIcon.qrc

# cmark integration
# 项目根目录（.pro 文件在 src/LLM/ 目录下，需要回到项目根目录）
PROJECT_ROOT = $$_PRO_FILE_PWD_/../..
CMARK_INC = $$PROJECT_ROOT/include/cmark
CMARK_LIB_DIR_MSVS = $$PROJECT_ROOT/Lib
INCLUDEPATH += $$CMARK_INC
win32:msvc {
    CMARK_LIB_FILE = $$CMARK_LIB_DIR_MSVS/cmark.lib
    exists($$CMARK_LIB_FILE) {
        LIBS += $$CMARK_LIB_FILE
    }
}

# output directories
CONFIG(debug, debug|release) {
    DESTDIR = $$PROJECT_ROOT/bin/debug
} else {
    DESTDIR = $$PROJECT_ROOT/bin/release
}

# Copy FunctionCall.json to output directory
# Note: FunctionCall.json should be manually copied to debug/AIAssit/ and release/AIAssit/ directories
# Or it will be created at runtime if it doesn't exist

INCLUDEPATH += $$PWD
