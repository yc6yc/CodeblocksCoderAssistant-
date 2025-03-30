#include <sdk.h>
#include <cbplugin.h>
#include <manager.h>       // 用于获取 Code::Blocks 主窗口
#include <wx/app.h>
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/textctrl.h>
#include <wx/button.h>
#include <wx/msgdlg.h>
#include <wx/file.h>
#include <wx/accel.h>
#include <wx/scrolwin.h>    // 用于 wxScrolledWindow
#include <wx/stattext.h>
#include "model.h"
#include "ClaudModel.h"

// 删除全局变量 mark

// 定义快捷键命令 ID
const int ID_SHOW_DIALOG = wxNewId();
const int ID_SHOW_LOG    = wxNewId();  // 显示 log 文件的快捷键ID

// 模式选择对话框：显示 Local 和 Cloud 两个按钮
class ModeSelectDialog : public wxDialog {
public:
    // 构造时传入对 mark 的引用
    ModeSelectDialog(wxWindow* parent, bool &markRef)
        : wxDialog(parent, wxID_ANY, wxT("Select Mode"), wxDefaultPosition, wxSize(300, 150),
                   wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxMINIMIZE_BOX | wxMAXIMIZE_BOX),
          m_mark(markRef)
    {
        wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
        wxStaticText* text = new wxStaticText(this, wxID_ANY, wxT("Select Mode:"));
        sizer->Add(text, 0, wxALL | wxALIGN_CENTER_HORIZONTAL, 10);

        wxBoxSizer* btnSizer = new wxBoxSizer(wxHORIZONTAL);
        wxButton* localBtn = new wxButton(this, wxID_ANY, wxT("Local"));
        wxButton* cloudBtn = new wxButton(this, wxID_ANY, wxT("Cloud"));
        btnSizer->Add(localBtn, 0, wxALL, 5);
        btnSizer->Add(cloudBtn, 0, wxALL, 5);

        sizer->Add(btnSizer, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5);
        SetSizer(sizer);
        sizer->Fit(this);

        // 按下 Local 时将 mark 置为 false；按下 Cloud 时 mark 保持 true
        localBtn->Bind(wxEVT_BUTTON, &ModeSelectDialog::OnLocal, this);
        cloudBtn->Bind(wxEVT_BUTTON, &ModeSelectDialog::OnCloud, this);
    }
private:
    bool &m_mark;

    void OnLocal(wxCommandEvent& event) {
        m_mark = false;  // 设置为 Local 模式
        EndModal(wxID_OK);
    }

    void OnCloud(wxCommandEvent& event) {
        m_mark = true;   // 保持 Cloud 模式
        EndModal(wxID_OK);
    }
};


class ModelConfigDialog : public wxDialog
{
public:
    ModelConfigDialog(wxWindow* parent, bool mode)
        : wxDialog(parent, wxID_ANY, wxT("modelinformation"), wxDefaultPosition, wxSize(400, 300),
                   wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxMINIMIZE_BOX | wxMAXIMIZE_BOX),
          m_mode(mode)
    {
        wxBoxSizer* vSizer = new wxBoxSizer(wxVERTICAL);

        // API 输入框
        wxStaticText* apiLabel = new wxStaticText(this, wxID_ANY, wxT("API:"));
        m_apiCtrl = new wxTextCtrl(this, wxID_ANY, wxT(""), wxDefaultPosition, wxSize(300, -1));
        vSizer->Add(apiLabel, 0, wxALL, 5);
        vSizer->Add(m_apiCtrl, 0, wxALL | wxEXPAND, 5);

        // 模型名称输入框
        wxStaticText* modelLabel = new wxStaticText(this, wxID_ANY, wxT("modelname:"));
        m_modelNameCtrl = new wxTextCtrl(this, wxID_ANY, wxT(""), wxDefaultPosition, wxSize(300, -1));
        vSizer->Add(modelLabel, 0, wxALL, 5);
        vSizer->Add(m_modelNameCtrl, 0, wxALL | wxEXPAND, 5);

        // 云模式下，添加 API 密钥输入框
        if (m_mode) {
            wxStaticText* apiKeyLabel = new wxStaticText(this, wxID_ANY, wxT("APIKEY:"));
            m_apiKeyCtrl = new wxTextCtrl(this, wxID_ANY, wxT(""), wxDefaultPosition, wxSize(300, -1));
            vSizer->Add(apiKeyLabel, 0, wxALL, 5);
            vSizer->Add(m_apiKeyCtrl, 0, wxALL | wxEXPAND, 5);
        }

        // 保存和取消按钮
        wxBoxSizer* hSizer = new wxBoxSizer(wxHORIZONTAL);
        wxButton* saveButton = new wxButton(this, wxID_OK, wxT("save"));
        wxButton* cancelButton = new wxButton(this, wxID_CANCEL, wxT("cancel"));
        hSizer->Add(saveButton, 0, wxALL, 5);
        hSizer->Add(cancelButton, 0, wxALL, 5);

        vSizer->Add(hSizer, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 10);

        SetSizerAndFit(vSizer);

        // 加载之前的设置
        LoadSettings();
    }

    // 保存当前的设置到文件
 void SaveSettings()
{
    // 根据模式选择文件路径
    wxString filePath = m_mode ? wxT("C:\\CodeBlocks_file\\claude.txt") : wxT("C:\\CodeBlocks_file\\local.txt");

    wxFile file;
    if (file.Open(filePath, wxFile::write))
    {
        // 生成保存内容的字符串，不使用 wxT()
        wxString content = wxString::Format("API=%s\nmodelname=%s\n", m_apiCtrl->GetValue(), m_modelNameCtrl->GetValue());

        if (m_mode) {
            content += wxString::Format("APIKEY=%s\n", m_apiKeyCtrl->GetValue());
        }

        // 使用 wxConvLibc 进行编码转换，确保保存为系统的默认字符集（GBK）
        file.Write(content.mb_str(wxConvLibc));  // 使用系统默认字符集编码（GBK）
        file.Close();
    }
    else
    {
        wxLogError("can’t open file: %s", filePath);
    }
}



    // 从文件加载配置
    void LoadSettings()
    {
        wxString filePath = m_mode ? wxT("C:\\CodeBlocks_file\\claude.txt") : wxT("C:\\CodeBlocks_file\\local.txt");
        wxFile file;
        if (file.Exists(filePath) && file.Open(filePath))
        {
            wxString content;
            file.ReadAll(&content);
            file.Close();

            wxArrayString lines = wxSplit(content, '\n');
            if (lines.Count() >= 2)
            {
                m_apiCtrl->SetValue(lines[0].AfterFirst('='));
                m_modelNameCtrl->SetValue(lines[1].AfterFirst('='));
            }
            if (m_mode && lines.Count() >= 3) {
                m_apiKeyCtrl->SetValue(lines[2].AfterFirst('='));
            }
        }
    }

private:
    bool m_mode; // true为云模式，false为本地模式
    wxTextCtrl* m_apiCtrl;
    wxTextCtrl* m_modelNameCtrl;
    wxTextCtrl* m_apiKeyCtrl;
};

// 写入界面对话框：新增构造参数 mode，保存选择的模式
class MyPluginDialog : public wxDialog
{
public:
    MyPluginDialog(wxWindow* parent, bool mode)
        : wxDialog(parent, wxID_ANY, wxT("File Read/Write Plugin"), wxDefaultPosition, wxSize(600, 400),
                   wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxMINIMIZE_BOX | wxMAXIMIZE_BOX),
          m_mode(mode)
    {
        // 创建滚动窗口
        wxScrolledWindow* scrolledWindow = new wxScrolledWindow(this, wxID_ANY, wxDefaultPosition,
                                                                wxDefaultSize, wxHSCROLL | wxVSCROLL);
        scrolledWindow->SetScrollRate(5, 5);

        // 垂直 sizer 用于排列文本输入框和按钮区域
        wxBoxSizer* vSizer = new wxBoxSizer(wxVERTICAL);

        // 多行文本输入框，占据大部分空间
        inputText = new wxTextCtrl(scrolledWindow, wxID_ANY, wxT("Please enter content..."),
                                   wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE);
        vSizer->Add(inputText, 1, wxALL | wxEXPAND, 5);

        // 水平 sizer 用于放置按钮，添加伸缩项使按钮在右下角
        wxBoxSizer* hSizer = new wxBoxSizer(wxHORIZONTAL);
        hSizer->AddStretchSpacer(1); // 填充左侧区域
        wxButton* writeButton = new wxButton(scrolledWindow, wxID_ANY, wxT("Write File"));
        hSizer->Add(writeButton, 0, wxALL, 5);
        vSizer->Add(hSizer, 0, wxEXPAND | wxALL, 5);

        scrolledWindow->SetSizer(vSizer);
        vSizer->Fit(scrolledWindow);
        scrolledWindow->Layout();

        // 外层 sizer，将滚动窗口加入对话框
        wxBoxSizer* outerSizer = new wxBoxSizer(wxVERTICAL);
        outerSizer->Add(scrolledWindow, 1, wxEXPAND);
        SetSizer(outerSizer);
        Layout();

        // 绑定写入按钮事件
        writeButton->Bind(wxEVT_BUTTON, &MyPluginDialog::OnWrite, this);
    }
private:
    wxTextCtrl* inputText;
    bool m_mode; // true 表示 Cloud，false 表示 Local
    wxString writeFilePath = wxT("C:\\CodeBlocks_file\\chat_input.txt");
    wxString readFilePath  = wxT("C:\\CodeBlocks_file\\chat_log.txt");

  void OnWrite(wxCommandEvent& event)
{
    // 读取配置文件内容
    wxString api, modelName, apiKey;
    wxString filePath = m_mode ? wxT("C:\\CodeBlocks_file\\claude.txt") : wxT("C:\\CodeBlocks_file\\local.txt");

    wxFile file;
    if (file.Exists(filePath) && file.Open(filePath))
    {
        wxString content;
        file.ReadAll(&content);
        file.Close();

        wxArrayString lines = wxSplit(content, '\n');
        if (lines.Count() >= 2) {
            api = lines[0].AfterFirst('=');  // 获取 API
            modelName = lines[1].AfterFirst('=');  // 获取模型名称
        }

        // 如果是云模式，还需要读取 API Key
        if (m_mode && lines.Count() >= 3) {
            apiKey = lines[2].AfterFirst('=');  // 获取 API Key
        }
    }
    else
    {
        wxLogError("Cannot open config file: %s", filePath);
        return;
    }

    // 将 wxString 转换为 std::string
    std::string apiStr = api.ToStdString();
    std::string modelNameStr = modelName.ToStdString();
    std::string apiKeyStr = apiKey.ToStdString();  // 云模式时需要

    wxLogMessage("Using mode: %s", m_mode ? "Cloud" : "Local");

    try {
        if (m_mode) {
            // Cloud Model
            wxLogMessage("Invoking Cloud Model");

            // 使用从文件中读取的配置创建模型对象，并按正确的顺序传递参数
            ClaudModel model(apiStr, apiKeyStr, modelNameStr);  // 先是 API, 然后 APIKEY, 最后是 Modelname
            model.single_chat();  // 调用模型交互
        } else {
            // Local Model
            wxLogMessage("Invoking Local Model");

            // 使用从文件中读取的配置创建本地模型对象
            Model model(apiStr, modelNameStr);  // 从文件中读取的配置传递给构造函数
            model.single_chat();  // 调用模型交互
        }
    } catch (const std::exception& e) {
        wxLogError("Error during model interaction: %s", e.what());
        return;
    }

    // 调用读取日志
    OpenReadDialog();
}




    void OpenReadDialog()
    {
        wxDialog* readDialog = new wxDialog(this, wxID_ANY, wxT("Read File Content"), wxDefaultPosition,
                                              wxSize(600, 400),
                                              wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxMINIMIZE_BOX | wxMAXIMIZE_BOX);
        wxScrolledWindow* scrolledWindow = new wxScrolledWindow(readDialog, wxID_ANY, wxDefaultPosition,
                                                                wxDefaultSize, wxHSCROLL | wxVSCROLL);
        scrolledWindow->SetScrollRate(5, 5);

        wxBoxSizer* vSizer = new wxBoxSizer(wxVERTICAL);
        wxTextCtrl* outputText = new wxTextCtrl(scrolledWindow, wxID_ANY, wxT(""),
                                                wxDefaultPosition, wxDefaultSize,
                                                wxTE_MULTILINE | wxTE_READONLY);
        vSizer->Add(outputText, 1, wxALL | wxEXPAND, 5);

        wxBoxSizer* hSizer = new wxBoxSizer(wxHORIZONTAL);
        hSizer->AddStretchSpacer(1);
        wxButton* readButton = new wxButton(scrolledWindow, wxID_ANY, wxT("Read File"));
        hSizer->Add(readButton, 0, wxALL, 5);
        vSizer->Add(hSizer, 0, wxEXPAND | wxALL, 5);

        scrolledWindow->SetSizer(vSizer);
        vSizer->Fit(scrolledWindow);
        scrolledWindow->Layout();

        wxBoxSizer* outerSizer = new wxBoxSizer(wxVERTICAL);
        outerSizer->Add(scrolledWindow, 1, wxEXPAND);
        readDialog->SetSizer(outerSizer);
        readDialog->Layout();

        wxString content;
        wxFile readFile;
        if (readFile.Exists(readFilePath) && readFile.Open(readFilePath))
        {
            readFile.ReadAll(&content);
            readFile.Close();
            outputText->SetValue(content);
        }
        else
        {
            outputText->SetValue(wxT("Cannot open file for reading!"));
        }

        readDialog->ShowModal();
        readDialog->Destroy();
    }
};

// 插件类：在 Code::Blocks 主窗口中注册快捷键显示界面
class MyPlugin : public cbPlugin
{
public:
    MyPlugin() : cbPlugin(), dialog(nullptr), mark(true) { }
    virtual ~MyPlugin() { if (dialog) delete dialog; }

    virtual void OnAttach()
    {
        wxFrame* mainFrame = Manager::Get()->GetAppFrame();

        // 定义两个加速器：CTRL+M 显示写入界面，CTRL+K 显示 log 文件内容
        wxAcceleratorEntry entries[2];
        entries[0].Set(wxACCEL_CTRL, (int)'M', ID_SHOW_DIALOG);
        entries[1].Set(wxACCEL_CTRL, (int)'K', ID_SHOW_LOG);
        wxAcceleratorTable accel(2, entries);
        mainFrame->SetAcceleratorTable(accel);

        mainFrame->Bind(wxEVT_MENU,
                        wxCommandEventHandler(MyPlugin::OnShowDialog),
                        this,
                        ID_SHOW_DIALOG);
        mainFrame->Bind(wxEVT_MENU,
                        wxCommandEventHandler(MyPlugin::OnShowLog),
                        this,
                        ID_SHOW_LOG);
    }

    virtual void OnRelease()
    {
        if (dialog)
        {
            dialog->Destroy();
            dialog = nullptr;
        }
        wxFrame* mainFrame = Manager::Get()->GetAppFrame();
        mainFrame->Unbind(wxEVT_MENU,
                          wxCommandEventHandler(MyPlugin::OnShowDialog),
                          this,
                          ID_SHOW_DIALOG);
        mainFrame->Unbind(wxEVT_MENU,
                          wxCommandEventHandler(MyPlugin::OnShowLog),
                          this,
                          ID_SHOW_LOG);
    }

 void OnShowDialog(wxCommandEvent& event)
{
    bool mark = true; // 默认是云模式
    ModeSelectDialog modeDlg(Manager::Get()->GetAppFrame(), mark);

    // 显示模式选择对话框
    if (modeDlg.ShowModal() == wxID_OK)
    {
        wxLogMessage("Mode selected: %s", mark ? "Cloud" : "Local");

        // 显示模型配置信息对话框
        ModelConfigDialog configDlg(Manager::Get()->GetAppFrame(), mark);
        if (configDlg.ShowModal() == wxID_OK)
        {
            // 配置信息保存后，继续显示主界面
            configDlg.SaveSettings(); // 保存配置文件

            // 创建并显示主对话框（与模型对话的主界面）
            if (dialog)
            {
                dialog->Destroy(); // 销毁之前的对话框（如果存在）
                dialog = nullptr;
            }

            // 创建新的主对话框并显示
            dialog = new MyPluginDialog(Manager::Get()->GetAppFrame(), mark);
            dialog->Show();
            dialog->Raise(); // 确保主对话框显示在最前面
        }
    }
}

    void OnShowLog(wxCommandEvent& event)
    {
        wxDialog* logDialog = new wxDialog(Manager::Get()->GetAppFrame(), wxID_ANY, wxT("Log File Content"),
                                             wxDefaultPosition, wxSize(600, 400),
                                             wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxMINIMIZE_BOX | wxMAXIMIZE_BOX);
        wxScrolledWindow* scrolledWindow = new wxScrolledWindow(logDialog, wxID_ANY, wxDefaultPosition,
                                                                wxDefaultSize, wxHSCROLL | wxVSCROLL);
        scrolledWindow->SetScrollRate(5, 5);

        wxBoxSizer* vSizer = new wxBoxSizer(wxVERTICAL);
        wxTextCtrl* logText = new wxTextCtrl(scrolledWindow, wxID_ANY, wxT(""),
                                             wxDefaultPosition, wxDefaultSize,
                                             wxTE_MULTILINE | wxTE_READONLY);
        vSizer->Add(logText, 1, wxALL | wxEXPAND, 5);

        scrolledWindow->SetSizer(vSizer);
        vSizer->Fit(scrolledWindow);
        scrolledWindow->Layout();

        wxBoxSizer* outerSizer = new wxBoxSizer(wxVERTICAL);
        outerSizer->Add(scrolledWindow, 1, wxEXPAND);
        logDialog->SetSizer(outerSizer);
        logDialog->Layout();

        wxString logFilePath = wxT("C:\\CodeBlocks_file\\log.txt");
        wxFile file;
        wxString content;
        if (file.Exists(logFilePath) && file.Open(logFilePath))
        {
            file.ReadAll(&content);
            file.Close();
            logText->SetValue(content);
        }
        else
        {
            logText->SetValue(wxT("Cannot open log file for reading!"));
        }

        logDialog->ShowModal();
        logDialog->Destroy();
    }

private:
    MyPluginDialog* dialog;
    bool mark; // true 表示 Cloud，false 表示 Local
};

cbPlugin* CreatePlugIn()
{
    return new MyPlugin();
}
PluginRegistrant<MyPlugin> reg("MyPlugin");
