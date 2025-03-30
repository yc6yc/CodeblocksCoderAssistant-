/***************************************************************
 * Name:      001
 * Purpose:   Code::Blocks plugin
 * Author:     ()
 * Created:   2025-03-10
 * Copyright:
 * License:   GPL
 **************************************************************/

#ifndef CODEEXPORTPLUGIN_H
#define CODEEXPORTPLUGIN_H

// 包含必要的头文件
#include <sdk.h>
#include <configurationpanel.h>
#include <cbplugin.h>
#include <wx/menu.h>
#include <wx/event.h>
#include <string>

// CodeExportPlugin 插件类，用于导出代码到文件。
// 支持导出整个文件以及光标之前的代码，同时具有状态切换功能。
class CodeExportPlugin : public cbPlugin
{
public:
    // 构造函数：初始化插件及内部状态
    CodeExportPlugin();
    // 析构函数：清理资源（当前无特殊清理需求）
    virtual ~CodeExportPlugin();

    // 当插件被附加到 Code::Blocks 时调用
    virtual void OnAttach();
    // 当插件被释放时调用，这里保持空实现
    virtual void OnRelease() {}
    // 在 CodeExportPlugin 类的 public 部分添加：
    void OnShortcutCtrlN(wxCommandEvent& event);
    static std::string ExtractFirstCodeBlock(const std::string &content);
    // 获取当前状态的接口，其他函数可通过此方法检测状态变化
    bool GetToggleState_1() const { return m_toggleState_1; }
    bool GetToggleState_2() const { return m_toggleState_2; }

private:
    // 快捷键事件处理函数，当按下快捷键时会调用这些函数
    void OnShortcutFull(wxCommandEvent& event);         // 处理 Shift+S 快捷键
    void OnShortcutBeforeCursor(wxCommandEvent& event);   // 处理 Alt+空格 快捷键

    // 具体的导出操作函数
    void SaveFullCodeToFile();           // 导出整个文件的代码
    void SaveCodeBeforeCursorToFile();   // 导出光标之前的代码

    // 状态变量，用于切换，初始值为 false
    bool m_toggleState_1;
    bool m_toggleState_2;

    // 新增：用于捕获 Shift+S 的字符钩子事件处理函数
    void OnCharHook(wxKeyEvent& event);
    //bool ClearFileContent(const std::string& filePath);
    void OnShortPasteCtrlH(wxCommandEvent& event);

    wxDECLARE_EVENT_TABLE()
};

#endif // CODEEXPORTPLUGIN_H
