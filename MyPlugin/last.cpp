#include "last.h"
#include "model.h"
#include "ClaudModel.h"
// ���������ͷ�ļ��������ļ���������־��¼����Ϣ���
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

// ����������ȥ���ַ�����β�հ��ַ�
std::string Trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

// ������UTF8 �� GBK ת������
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

// ����˵�����ID�����ڱ�ʶ��ͬ�Ŀ�ݼ�����
const int ID_EXPORT_FULL = wxNewId();
const int ID_EXPORT_BEFORE_CURSOR = wxNewId();
const int ID_CTRL_H = wxNewId(); // Ctrl+H
const int ID_CTRL_N = wxNewId(); // Ctrl+N���ڶ�������Ĺ��ܣ�

// ���¼�������ݼ��¼�����Ӧ�Ĵ�������
wxBEGIN_EVENT_TABLE(CodeExportPlugin, cbPlugin)
    EVT_MENU(ID_EXPORT_FULL, CodeExportPlugin::OnShortcutFull)
    EVT_MENU(ID_EXPORT_BEFORE_CURSOR, CodeExportPlugin::OnShortcutBeforeCursor)
    EVT_MENU(ID_CTRL_H, CodeExportPlugin::OnShortPasteCtrlH)
    EVT_MENU(ID_CTRL_N, CodeExportPlugin::OnShortcutCtrlN)
wxEND_EVENT_TABLE()

// ���캯������ʼ��״̬����Ϊ false
CodeExportPlugin::CodeExportPlugin()
    : m_toggleState_1(false)
    , m_toggleState_2(false)
{
}

// ��������
CodeExportPlugin::~CodeExportPlugin()
{
}

// ����������ӵ� Code::Blocks ʱ���ô˺���
void CodeExportPlugin::OnAttach()
{
    // �����������еĿ�ݼ���Alt+Space��Ctrl+H �� Ctrl+N��
    wxAcceleratorEntry entries[3];
    entries[0].Set(wxACCEL_ALT, WXK_SPACE, ID_EXPORT_BEFORE_CURSOR);
    entries[1].Set(wxACCEL_CTRL, (int)'H', ID_CTRL_H);
    entries[2].Set(wxACCEL_CTRL, (int)'N', ID_CTRL_N);
    wxAcceleratorTable accel(3, entries);
    Manager::Get()->GetAppWindow()->SetAcceleratorTable(accel);

    // �������ڵ��ַ������¼������ڲ��� Shift+S�������ִ�Сд��
    Manager::Get()->GetAppWindow()->Bind(wxEVT_CHAR_HOOK, &CodeExportPlugin::OnCharHook, this);

    wxLogMessage("CodeExportPlugin attached. Shift+S (for full export), Alt+Space (for export before cursor), Ctrl+H (for paste), and Ctrl+N (for diff paste) are active.");
}

// �������� CodeExportPlugin ������� OnCharHook ������ʵ�֣����ڲ��� Shift+S�������ִ�Сд��
void CodeExportPlugin::OnCharHook(wxKeyEvent& event)
{
    // ����Ƿ��� Shift �������Ұ���Ϊ 'S' �� 's'
    if (event.ShiftDown() && (event.GetKeyCode() == 'S' || event.GetKeyCode() == 's'))
    {
        wxLogMessage("Shift+S detected via char hook.");
        wxCommandEvent dummyEvent;
        OnShortcutFull(dummyEvent);
        // ���ٴ����¼�����ֹ�ظ�����
        return;
    }
    event.Skip(); // �������������������¼�
}

// ������ Shift+S ��ݼ�ʱ���ô˺��������������ļ���
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

// ������ Alt+�ո��ݼ�ʱ���ô˺������������֮ǰ�Ĵ��룩
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

// ������ Ctrl+H ��ݼ�ʱ���ô˺������滻�����༭�����ݣ�
void CodeExportPlugin::OnShortPasteCtrlH(wxCommandEvent& event)
{
    wxLogMessage("Ctrl+H paste triggered.");

    // ��ȡ��ǰ��༭��
    EditorBase* editor = Manager::Get()->GetEditorManager()->GetActiveEditor();
    if (!editor)
    {
        wxLogError("No active editor found for Ctrl+H paste.");
        wxMessageBox("No active editor found.", "Error", wxOK | wxICON_ERROR);
        return;
    }

    // ���༭��ת��Ϊ cbEditor ����
    cbEditor* cbEditorInstance = dynamic_cast<cbEditor*>(editor);
    if (!cbEditorInstance)
    {
        wxLogError("Failed to cast EditorBase to cbEditor for Ctrl+H paste.");
        wxMessageBox("Failed to cast EditorBase to cbEditor.", "Error", wxOK | wxICON_ERROR);
        return;
    }

    // ��յ�ǰ�ļ����ݲ��滻Ϊ���ļ�����ȡ��������
    InsertNewCodeAtCursor(static_cast<void*>(cbEditorInstance));
}

// ������������ Ctrl+N ��ݼ�ʱ���ô˺���������ճ������������첿�֣�
void CodeExportPlugin::OnShortcutCtrlN(wxCommandEvent& event)
{
    wxLogMessage("Ctrl+N detected, processing file differences.");

    // ��ȡ single_chat_log.txt �ļ�����
    std::ifstream singleFile("C:\\CodeBlocks_file\\single_chat_log.txt");
    if (singleFile)
    {
        std::stringstream ss;
        ss << singleFile.rdbuf();
        std::string singleContent = ss.str();
        singleFile.close();

        // ����������ȡ��һ�������
        std::string extractedContent = ExtractFirstCodeBlock(singleContent);

        // ���� '#' �ַ���������������
        size_t hashPos = extractedContent.find('#');
        if (hashPos != std::string::npos)
        {
            extractedContent = extractedContent.substr(hashPos);
        }

        // ����ȡ�����ݽ��� trim ����
        extractedContent = Trim(extractedContent);

        wxLogMessage("Extracted content after processing: [%s]", extractedContent.c_str());

        // ��ȡ cursor_input_log.txt �ļ�����
        std::ifstream cursorFile("C:\\CodeBlocks_file\\cursor_input_log.txt");
        if (cursorFile)
        {
            std::stringstream ssCursor;
            ssCursor << cursorFile.rdbuf();
            std::string cursorContent = ssCursor.str();
            cursorFile.close();

            // �� cursorContent ���� trim ����
            cursorContent = Trim(cursorContent);
            wxLogMessage("Cursor content: [%s]", cursorContent.c_str());

            // ��� extractedContent �� cursorContent Ϊǰ׺����ֻ������ͬ�Ĳ���
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

        // ��ת��������ݲ��뵽��ǰ�༭���Ĺ��λ��
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

// ���������ļ��Ĵ��뵽�ļ������ܲ��䣩
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

// �������֮ǰ�Ĵ��뵽�ļ������ܲ��䣩
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

// �޸ĺ�� ExtractFirstCodeBlock ������
// �������в��ҵ�һ���ɶԵ����������� ``` ����ȡ��֮����ı�
std::string CodeExportPlugin::ExtractFirstCodeBlock(const std::string &content)
{
    const std::string delim = "```";
    size_t start = content.find(delim);
    if (start == std::string::npos)
        return content; // δ�ҵ��򷵻���������

    start += delim.length(); // ������һ�� ```
    size_t end = content.find(delim, start);
    if (end == std::string::npos)
        return content.substr(start);

    return content.substr(start, end - start);
}

// �޸ĺ�� InsertNewCodeAtCursor ������
// ��ȡ�ļ����ӵ�һ�����������ȡ���ݣ��ٲ��� '#' ���Ӹ�λ�ÿ�ʼ�滻�༭���е������ı���
// ���� UTF8 �� GBK ��ת����ȷ�������ı�Ϊ GBK ���롣
void InsertNewCodeAtCursor(void* eb)
{
    std::ifstream file("C:\\CodeBlocks_file\\single_chat_log.txt");
    if (file)
    {
        std::stringstream ss;
        ss << file.rdbuf();
        std::string fileContent = ss.str();
        file.close();

        // ��ȡ��һ��������� ``` �� ``` ֮�������
        std::string extractedContent = CodeExportPlugin::ExtractFirstCodeBlock(fileContent);
        // ���� '#' �ַ���������������
        size_t hashPos = extractedContent.find('#');
        if (hashPos != std::string::npos)
        {
            extractedContent = extractedContent.substr(hashPos);
        }

        // �� UTF8 ��ʽ���ı�ת��Ϊ GBK ��ʽ
        std::string convertedContent = UTF8ToGBK(extractedContent);

        cbEditor* editor = static_cast<cbEditor*>(eb);
        if (editor)
        {
            cbStyledTextCtrl* control = editor->GetControl();
            if (control)
            {
                // ʹ�� SetText() �滻�����༭������Ϊת����� GBK �ı�
                control->SetText(convertedContent);
            }
        }
    }
    else
    {
        wxLogDebug("Failed to open file: C:\\CodeBlocks_file\\single_chat_log.txt");
    }
}

// ���ע�ᣬȷ�� Code::Blocks �ܹ�ʶ�𲢼��ش˲��
PluginRegistrant<CodeExportPlugin> reg("MyPlugin");
