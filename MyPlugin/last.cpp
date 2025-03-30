#include "last.h"
#include "model.h"
#include "ClaudModel.h"
// 包含额外的头文件，用于文件操作、日志记录、消息框等
#include <wx/file.h>
#include <wx/filename.h>
#include <wx/msgdlg.h>
#include <editormanager.h>
#include <editorbase.h>
#include <cbeditor.h>
#include <projectmanager.h>
#include <cbproject.h>
#include <cbstyledtextctrl.h>
#include <wx/log.h>
#include <wx/dir.h>
#include <wx/filefn.h>
#include <fstream>
#include <sstream>
#include <string>
#include <codecvt>
#include <locale>
#include <cstdlib>
#include <thread>
#include <chrono>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#endif
void InsertNewCodeAtCursor(void* eb);
bool mark1 = true;

// 辅助函数：去除字符串首尾空白字符
std::string Trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

// 新增：UTF8 到 GBK 转换函数
std::string UTF8ToGBK(const std::string& utf8) {
    int wlen = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, NULL, 0);
    if (wlen <= 0) return "";
    std::vector<wchar_t> wbuffer(wlen);
    MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, wbuffer.data(), wlen);
    int gbklen = WideCharToMultiByte(CP_ACP, 0, wbuffer.data(), -1, NULL, 0, NULL, NULL);
    if (gbklen <= 0) return "";
    std::vector<char> gbkbuffer(gbklen);
    WideCharToMultiByte(CP_ACP, 0, wbuffer.data(), -1, gbkbuffer.data(), gbklen, NULL, NULL);
    return std::string(gbkbuffer.data());
}

// 定义菜单命令ID，用于标识不同的快捷键命令
const int ID_EXPORT_FULL = wxNewId();
const int ID_EXPORT_BEFORE_CURSOR = wxNewId();
const int ID_CTRL_H = wxNewId(); // Ctrl+H
const int ID_CTRL_N = wxNewId(); // Ctrl+N（第二个插件的功能）

// 绑定事件表，将快捷键事件与相应的处理函数绑定
wxBEGIN_EVENT_TABLE(CodeExportPlugin, cbPlugin)
    EVT_MENU(ID_EXPORT_FULL, CodeExportPlugin::OnShortcutFull)
    EVT_MENU(ID_EXPORT_BEFORE_CURSOR, CodeExportPlugin::OnShortcutBeforeCursor)
    EVT_MENU(ID_CTRL_H, CodeExportPlugin::OnShortPasteCtrlH)
    EVT_MENU(ID_CTRL_N, CodeExportPlugin::OnShortcutCtrlN)
wxEND_EVENT_TABLE()

// 构造函数，初始化状态变量为 false
CodeExportPlugin::CodeExportPlugin()
    : m_toggleState_1(false)
    , m_toggleState_2(false)
{
}

// 析构函数
CodeExportPlugin::~CodeExportPlugin()
{
}

// 当插件被附加到 Code::Blocks 时调用此函数
void CodeExportPlugin::OnAttach()
{
    // 保留其它已有的快捷键（Alt+Space、Ctrl+H 和 Ctrl+N）
    wxAcceleratorEntry entries[3];
    entries[0].Set(wxACCEL_ALT, WXK_SPACE, ID_EXPORT_BEFORE_CURSOR);
    entries[1].Set(wxACCEL_CTRL, (int)'H', ID_CTRL_H);
    entries[2].Set(wxACCEL_CTRL, (int)'N', ID_CTRL_N);
    wxAcceleratorTable accel(3, entries);
    Manager::Get()->GetAppWindow()->SetAcceleratorTable(accel);

    // 绑定主窗口的字符钩子事件，用于捕获 Shift+S（不区分大小写）
    Manager::Get()->GetAppWindow()->Bind(wxEVT_CHAR_HOOK, &CodeExportPlugin::OnCharHook, this);

    wxLogMessage("CodeExportPlugin attached. Shift+S (for full export), Alt+Space (for export before cursor), Ctrl+H (for paste), and Ctrl+N (for diff paste) are active.");
}

// 新增：在 CodeExportPlugin 类中添加 OnCharHook 方法的实现，用于捕获 Shift+S（不区分大小写）
void CodeExportPlugin::OnCharHook(wxKeyEvent& event)
{
    // 检查是否按下 Shift 键，并且按键为 'S' 或 's'
    if (event.ShiftDown() && (event.GetKeyCode() == 'S' || event.GetKeyCode() == 's'))
    {
        wxLogMessage("Shift+S detected via char hook.");
        wxCommandEvent dummyEvent;
        OnShortcutFull(dummyEvent);
        // 不再传递事件，防止重复处理
        return;
    }
    event.Skip(); // 对其他按键继续传递事件
}

// 当按下 Shift+S 快捷键时调用此函数（导出整个文件）
void CodeExportPlugin::OnShortcutFull(wxCommandEvent& event)
{
    m_toggleState_2 = !m_toggleState_2;
    wxLogMessage("Toggle state changed to: %s", m_toggleState_2 ? "true" : "false");
    wxLogMessage("Shortcut Shift+S triggered for full code export.");
    SaveFullCodeToFile();

    //ClearFileContent("C:\\CodeBlocks_file\\single_chat_log.txt");

    if(mark1){
        ClaudModel model("https://ark.cn-beijing.volces.com/api/v3/chat/completions",
                         "fe614b65-b922-4d15-b2e2-81fc2824df5f",
                         "doubao-1-5-lite-32k-250115");
        model.single_chat();
    } else {
        Model model("http://localhost:11434/api/chat", "deepseek-coder:6.7b");
        model.single_chat();
    }
}

// 当按下 Alt+空格快捷键时调用此函数（导出光标之前的代码）
void CodeExportPlugin::OnShortcutBeforeCursor(wxCommandEvent& event)
{
    m_toggleState_1 = !m_toggleState_1;
    wxLogMessage("Toggle state changed to: %s", m_toggleState_1 ? "true" : "false");
    wxLogMessage("Shortcut Alt+Space triggered for code export before cursor.");
    SaveCodeBeforeCursorToFile();

    //ClearFileContent("C:\\CodeBlocks_file\\single_chat_log.txt");

    if(mark1){
        ClaudModel model("https://ark.cn-beijing.volces.com/api/v3/chat/completions",
                         "fe614b65-b922-4d15-b2e2-81fc2824df5f",
                         "doubao-1-5-lite-32k-250115");
        model.single_chat();
    } else {
        Model model("http://localhost:11434/api/chat", "deepseek-coder:6.7b");
        model.single_chat();
    }
}

// 当按下 Ctrl+H 快捷键时调用此函数（替换整个编辑器内容）
void CodeExportPlugin::OnShortPasteCtrlH(wxCommandEvent& event)
{
    wxLogMessage("Ctrl+H paste triggered.");

    // 获取当前活动编辑器
    EditorBase* editor = Manager::Get()->GetEditorManager()->GetActiveEditor();
    if (!editor)
    {
        wxLogError("No active editor found for Ctrl+H paste.");
        wxMessageBox("No active editor found.", "Error", wxOK | wxICON_ERROR);
        return;
    }

    // 将编辑器转换为 cbEditor 类型
    cbEditor* cbEditorInstance = dynamic_cast<cbEditor*>(editor);
    if (!cbEditorInstance)
    {
        wxLogError("Failed to cast EditorBase to cbEditor for Ctrl+H paste.");
        wxMessageBox("Failed to cast EditorBase to cbEditor.", "Error", wxOK | wxICON_ERROR);
        return;
    }

    // 清空当前文件内容并替换为从文件中提取的新内容
    InsertNewCodeAtCursor(static_cast<void*>(cbEditorInstance));
}

// 新增：当按下 Ctrl+N 快捷键时调用此函数（增量粘贴，仅插入差异部分）
void CodeExportPlugin::OnShortcutCtrlN(wxCommandEvent& event)
{
    wxLogMessage("Ctrl+N detected, processing file differences.");

    // 读取 single_chat_log.txt 文件内容
    std::ifstream singleFile("C:\\CodeBlocks_file\\single_chat_log.txt");
    if (singleFile)
    {
        std::stringstream ss;
        ss << singleFile.rdbuf();
        std::string singleContent = ss.str();
        singleFile.close();

        // 从内容中提取第一个代码块
        std::string extractedContent = ExtractFirstCodeBlock(singleContent);

        // 查找 '#' 字符，保留其后的内容
        size_t hashPos = extractedContent.find('#');
        if (hashPos != std::string::npos)
        {
            extractedContent = extractedContent.substr(hashPos);
        }

        // 对提取的内容进行 trim 处理
        extractedContent = Trim(extractedContent);

        wxLogMessage("Extracted content after processing: [%s]", extractedContent.c_str());

        // 读取 cursor_input_log.txt 文件内容
        std::ifstream cursorFile("C:\\CodeBlocks_file\\cursor_input_log.txt");
        if (cursorFile)
        {
            std::stringstream ssCursor;
            ssCursor << cursorFile.rdbuf();
            std::string cursorContent = ssCursor.str();
            cursorFile.close();

            // 对 cursorContent 进行 trim 处理
            cursorContent = Trim(cursorContent);
            wxLogMessage("Cursor content: [%s]", cursorContent.c_str());

            // 如果 extractedContent 以 cursorContent 为前缀，则只保留不同的部分
            if (!cursorContent.empty() && extractedContent.compare(0, cursorContent.size(), cursorContent) == 0)
            {
                extractedContent = extractedContent.substr(cursorContent.size());
            }
        }
        else
        {
            wxLogMessage("Failed to open file: C:\\CodeBlocks_file\\cursor_input_log.txt");
        }

        wxLogMessage("Content to insert after difference check: [%s]", extractedContent.c_str());

        // 将转换后的内容插入到当前编辑器的光标位置
        std::string convertedContent = UTF8ToGBK(extractedContent);
        EditorBase* editor = Manager::Get()->GetEditorManager()->GetActiveEditor();
        if (editor)
        {
            cbEditor* cbEditorInstance = dynamic_cast<cbEditor*>(editor);
            if (cbEditorInstance)
            {
                cbStyledTextCtrl* control = cbEditorInstance->GetControl();
                if (control)
                {
                    control->InsertText(control->GetCurrentPos(), convertedContent.c_str());
                }
            }
        }
    }
    else
    {
        wxLogMessage("Failed to open file: C:\\CodeBlocks_file\\single_chat_log.txt");
    }
}

// 导出整个文件的代码到文件（功能不变）
void CodeExportPlugin::SaveFullCodeToFile()
{
    EditorBase* editor = Manager::Get()->GetEditorManager()->GetActiveEditor();
    if (!editor)
    {
        wxLogError("No active editor found.");
        wxMessageBox("No active editor found.", "Error", wxOK | wxICON_ERROR);
        return;
    }
    cbEditor* cbEditorInstance = dynamic_cast<cbEditor*>(editor);
    if (!cbEditorInstance)
    {
        wxLogError("Failed to cast EditorBase to cbEditor.");
        wxMessageBox("Failed to cast EditorBase to cbEditor.", "Error", wxOK | wxICON_ERROR);
        return;
    }
    wxString currentFilePath = cbEditorInstance->GetFilename();
    if (currentFilePath.IsEmpty())
    {
        wxLogError("The current file has not been saved.");
        wxMessageBox("The current file has not been saved.", "Error", wxOK | wxICON_ERROR);
        return;
    }
    wxString exportDir = "C:\\CodeBlocks_file";
    wxFileName dir(exportDir);
    if (!dir.DirExists())
    {
        if (!dir.Mkdir(0755, wxPATH_MKDIR_FULL))
        {
            wxLogError("Failed to create directory: %s", exportDir.c_str());
            wxMessageBox("Failed to create directory.", "Error", wxOK | wxICON_ERROR);
            return;
        }
    }
    cbStyledTextCtrl* control = cbEditorInstance->GetControl();
    if (!control)
    {
        wxLogError("Failed to get editor control.");
        wxMessageBox("Failed to get editor control.", "Error", wxOK | wxICON_ERROR);
        return;
    }
    wxString code = control->GetText();
    wxFileName exportFile(exportDir, "cursor_input_log.txt");
    wxString exportFilePath = exportFile.GetFullPath();

    wxFile file(exportFilePath, wxFile::write);
    if (!file.IsOpened())
    {
        wxLogError("Failed to create export file at %s.", exportFilePath.c_str());
        wxMessageBox("Failed to create export file.", "Error", wxOK | wxICON_ERROR);
        return;
    }
    file.Write(code);
    file.Close();

    wxLogMessage("The completed code has been successfully generated");
    wxMessageBox("The completed code has been successfully generated",
                 "Success", wxOK | wxICON_INFORMATION);
}

// 导出光标之前的代码到文件（功能不变）
void CodeExportPlugin::SaveCodeBeforeCursorToFile()
{
    EditorBase* editor = Manager::Get()->GetEditorManager()->GetActiveEditor();
    if (!editor)
    {
        wxLogError("No active editor found.");
        wxMessageBox("No active editor found.", "Error", wxOK | wxICON_ERROR);
        return;
    }
    cbEditor* cbEditorInstance = dynamic_cast<cbEditor*>(editor);
    if (!cbEditorInstance)
    {
        wxLogError("Failed to cast EditorBase to cbEditor.");
        wxMessageBox("Failed to cast EditorBase to cbEditor.", "Error", wxOK | wxICON_ERROR);
        return;
    }
    wxString currentFilePath = cbEditorInstance->GetFilename();
    if (currentFilePath.IsEmpty())
    {
        wxLogError("The current file has not been saved.");
        wxMessageBox("The current file has not been saved.", "Error", wxOK | wxICON_ERROR);
        return;
    }
    wxString exportDir = "C:\\CodeBlocks_file";
    wxFileName dir(exportDir);
    if (!dir.DirExists())
    {
        if (!dir.Mkdir(0755, wxPATH_MKDIR_FULL))
        {
            wxLogError("Failed to create directory: %s", exportDir.c_str());
            wxMessageBox("Failed to create directory.", "Error", wxOK | wxICON_ERROR);
            return;
        }
    }
    cbStyledTextCtrl* control = cbEditorInstance->GetControl();
    if (!control)
    {
        wxLogError("Failed to get editor control.");
        wxMessageBox("Failed to get editor control.", "Error", wxOK | wxICON_ERROR);
        return;
    }
    int currentPos = control->GetCurrentPos();
    wxString codeBeforeCursor = control->GetTextRange(0, currentPos);
    wxLogMessage("Code before cursor (0 to %d):\n%s", currentPos, codeBeforeCursor.c_str());
    wxFileName exportFile(exportDir, "cursor_input_log.txt");
    wxString exportFilePath = exportFile.GetFullPath();

    wxFile file(exportFilePath, wxFile::write);
    if (!file.IsOpened())
    {
        wxLogError("Failed to create export file at %s.", exportFilePath.c_str());
        wxMessageBox("Failed to create export file.", "Error", wxOK | wxICON_ERROR);
        return;
    }
    file.Write(codeBeforeCursor);
    file.Close();

    wxLogMessage("The completed code has been successfully generated");
    wxMessageBox("The completed code has been successfully generated",
                 "Success", wxOK | wxICON_INFORMATION);
}

// 修改后的 ExtractFirstCodeBlock 函数：
// 从内容中查找第一个成对的三个反引号 ``` ，提取其之间的文本
std::string CodeExportPlugin::ExtractFirstCodeBlock(const std::string &content)
{
    const std::string delim = "```";
    size_t start = content.find(delim);
    if (start == std::string::npos)
        return content; // 未找到则返回整个内容

    start += delim.length(); // 跳过第一个 ```
    size_t end = content.find(delim, start);
    if (end == std::string::npos)
        return content.substr(start);

    return content.substr(start, end - start);
}

// 修改后的 InsertNewCodeAtCursor 函数：
// 读取文件，从第一个代码块中提取内容，再查找 '#' 并从该位置开始替换编辑器中的所有文本。
// 新增 UTF8 到 GBK 的转换，确保插入文本为 GBK 编码。
void InsertNewCodeAtCursor(void* eb)
{
    std::ifstream file("C:\\CodeBlocks_file\\single_chat_log.txt");
    if (file)
    {
        std::stringstream ss;
        ss << file.rdbuf();
        std::string fileContent = ss.str();
        file.close();

        // 提取第一个代码块中 ``` 和 ``` 之间的内容
        std::string extractedContent = CodeExportPlugin::ExtractFirstCodeBlock(fileContent);
        // 查找 '#' 字符，保留其后的内容
        size_t hashPos = extractedContent.find('#');
        if (hashPos != std::string::npos)
        {
            extractedContent = extractedContent.substr(hashPos);
        }

        // 将 UTF8 格式的文本转换为 GBK 格式
        std::string convertedContent = UTF8ToGBK(extractedContent);

        cbEditor* editor = static_cast<cbEditor*>(eb);
        if (editor)
        {
            cbStyledTextCtrl* control = editor->GetControl();
            if (control)
            {
                // 使用 SetText() 替换整个编辑器内容为转换后的 GBK 文本
                control->SetText(convertedContent);
            }
        }
    }
    else
    {
        wxLogDebug("Failed to open file: C:\\CodeBlocks_file\\single_chat_log.txt");
    }
}

// 插件注册，确保 Code::Blocks 能够识别并加载此插件
PluginRegistrant<CodeExportPlugin> reg("MyPlugin");
