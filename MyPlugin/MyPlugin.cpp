#include <sdk.h>
#include <cbplugin.h>
#include <manager.h>       // ���ڻ�ȡ Code::Blocks ������
#include <wx/app.h>
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/textctrl.h>
#include <wx/button.h>
#include <wx/msgdlg.h>
#include <wx/file.h>
#include <wx/accel.h>
#include <wx/scrolwin.h>    // ���� wxScrolledWindow
#include <wx/stattext.h>
#include "model.h"
#include "ClaudModel.h"

// ɾ��ȫ�ֱ��� mark

// �����ݼ����� ID
const int ID_SHOW_DIALOG = wxNewId();
const int ID_SHOW_LOG    = wxNewId();  // ��ʾ log �ļ��Ŀ�ݼ�ID

// ģʽѡ��Ի�����ʾ Local �� Cloud ������ť
class ModeSelectDialog : public wxDialog {
public:
    // ����ʱ����� mark ������
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

        // ���� Local ʱ�� mark ��Ϊ false������ Cloud ʱ mark ���� true
        localBtn->Bind(wxEVT_BUTTON, &ModeSelectDialog::OnLocal, this);
        cloudBtn->Bind(wxEVT_BUTTON, &ModeSelectDialog::OnCloud, this);
    }
private:
    bool &m_mark;

    void OnLocal(wxCommandEvent& event) {
        m_mark = false;  // ����Ϊ Local ģʽ
        EndModal(wxID_OK);
    }

    void OnCloud(wxCommandEvent& event) {
        m_mark = true;   // ���� Cloud ģʽ
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

        // API �����
        wxStaticText* apiLabel = new wxStaticText(this, wxID_ANY, wxT("API:"));
        m_apiCtrl = new wxTextCtrl(this, wxID_ANY, wxT(""), wxDefaultPosition, wxSize(300, -1));
        vSizer->Add(apiLabel, 0, wxALL, 5);
        vSizer->Add(m_apiCtrl, 0, wxALL | wxEXPAND, 5);

        // ģ�����������
        wxStaticText* modelLabel = new wxStaticText(this, wxID_ANY, wxT("modelname:"));
        m_modelNameCtrl = new wxTextCtrl(this, wxID_ANY, wxT(""), wxDefaultPosition, wxSize(300, -1));
        vSizer->Add(modelLabel, 0, wxALL, 5);
        vSizer->Add(m_modelNameCtrl, 0, wxALL | wxEXPAND, 5);

        // ��ģʽ�£���� API ��Կ�����
        if (m_mode) {
            wxStaticText* apiKeyLabel = new wxStaticText(this, wxID_ANY, wxT("APIKEY:"));
            m_apiKeyCtrl = new wxTextCtrl(this, wxID_ANY, wxT(""), wxDefaultPosition, wxSize(300, -1));
            vSizer->Add(apiKeyLabel, 0, wxALL, 5);
            vSizer->Add(m_apiKeyCtrl, 0, wxALL | wxEXPAND, 5);
        }

        // �����ȡ����ť
        wxBoxSizer* hSizer = new wxBoxSizer(wxHORIZONTAL);
        wxButton* saveButton = new wxButton(this, wxID_OK, wxT("save"));
        wxButton* cancelButton = new wxButton(this, wxID_CANCEL, wxT("cancel"));
        hSizer->Add(saveButton, 0, wxALL, 5);
        hSizer->Add(cancelButton, 0, wxALL, 5);

        vSizer->Add(hSizer, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 10);

        SetSizerAndFit(vSizer);

        // ����֮ǰ������
        LoadSettings();
    }

    // ���浱ǰ�����õ��ļ�
 void SaveSettings()
{
    // ����ģʽѡ���ļ�·��
    wxString filePath = m_mode ? wxT("C:\\CodeBlocks_file\\claude.txt") : wxT("C:\\CodeBlocks_file\\local.txt");

    wxFile file;
    if (file.Open(filePath, wxFile::write))
    {
        // ���ɱ������ݵ��ַ�������ʹ�� wxT()
        wxString content = wxString::Format("API=%s\nmodelname=%s\n", m_apiCtrl->GetValue(), m_modelNameCtrl->GetValue());

        if (m_mode) {
            content += wxString::Format("APIKEY=%s\n", m_apiKeyCtrl->GetValue());
        }

        // ʹ�� wxConvLibc ���б���ת����ȷ������Ϊϵͳ��Ĭ���ַ�����GBK��
        file.Write(content.mb_str(wxConvLibc));  // ʹ��ϵͳĬ���ַ������루GBK��
        file.Close();
    }
    else
    {
        wxLogError("can��t open file: %s", filePath);
    }
}



    // ���ļ���������
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
    bool m_mode; // trueΪ��ģʽ��falseΪ����ģʽ
    wxTextCtrl* m_apiCtrl;
    wxTextCtrl* m_modelNameCtrl;
    wxTextCtrl* m_apiKeyCtrl;
};

// д�����Ի�������������� mode������ѡ���ģʽ
class MyPluginDialog : public wxDialog
{
public:
    MyPluginDialog(wxWindow* parent, bool mode)
        : wxDialog(parent, wxID_ANY, wxT("File Read/Write Plugin"), wxDefaultPosition, wxSize(600, 400),
                   wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxMINIMIZE_BOX | wxMAXIMIZE_BOX),
          m_mode(mode)
    {
        // ������������
        wxScrolledWindow* scrolledWindow = new wxScrolledWindow(this, wxID_ANY, wxDefaultPosition,
                                                                wxDefaultSize, wxHSCROLL | wxVSCROLL);
        scrolledWindow->SetScrollRate(5, 5);

        // ��ֱ sizer ���������ı������Ͱ�ť����
        wxBoxSizer* vSizer = new wxBoxSizer(wxVERTICAL);

        // �����ı������ռ�ݴ󲿷ֿռ�
        inputText = new wxTextCtrl(scrolledWindow, wxID_ANY, wxT("Please enter content..."),
                                   wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE);
        vSizer->Add(inputText, 1, wxALL | wxEXPAND, 5);

        // ˮƽ sizer ���ڷ��ð�ť�����������ʹ��ť�����½�
        wxBoxSizer* hSizer = new wxBoxSizer(wxHORIZONTAL);
        hSizer->AddStretchSpacer(1); // ����������
        wxButton* writeButton = new wxButton(scrolledWindow, wxID_ANY, wxT("Write File"));
        hSizer->Add(writeButton, 0, wxALL, 5);
        vSizer->Add(hSizer, 0, wxEXPAND | wxALL, 5);

        scrolledWindow->SetSizer(vSizer);
        vSizer->Fit(scrolledWindow);
        scrolledWindow->Layout();

        // ��� sizer�����������ڼ���Ի���
        wxBoxSizer* outerSizer = new wxBoxSizer(wxVERTICAL);
        outerSizer->Add(scrolledWindow, 1, wxEXPAND);
        SetSizer(outerSizer);
        Layout();

        // ��д�밴ť�¼�
        writeButton->Bind(wxEVT_BUTTON, &MyPluginDialog::OnWrite, this);
    }
private:
    wxTextCtrl* inputText;
    bool m_mode; // true ��ʾ Cloud��false ��ʾ Local
    wxString writeFilePath = wxT("C:\\CodeBlocks_file\\chat_input.txt");
    wxString readFilePath  = wxT("C:\\CodeBlocks_file\\chat_log.txt");

  void OnWrite(wxCommandEvent& event)
{
    // ��ȡ�����ļ�����
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
            api = lines[0].AfterFirst('=');  // ��ȡ API
            modelName = lines[1].AfterFirst('=');  // ��ȡģ������
        }

        // �������ģʽ������Ҫ��ȡ API Key
        if (m_mode && lines.Count() >= 3) {
            apiKey = lines[2].AfterFirst('=');  // ��ȡ API Key
        }
    }
    else
    {
        wxLogError("Cannot open config file: %s", filePath);
        return;
    }

    // �� wxString ת��Ϊ std::string
    std::string apiStr = api.ToStdString();
    std::string modelNameStr = modelName.ToStdString();
    std::string apiKeyStr = apiKey.ToStdString();  // ��ģʽʱ��Ҫ

    wxLogMessage("Using mode: %s", m_mode ? "Cloud" : "Local");

    try {
        if (m_mode) {
            // Cloud Model
            wxLogMessage("Invoking Cloud Model");

            // ʹ�ô��ļ��ж�ȡ�����ô���ģ�Ͷ��󣬲�����ȷ��˳�򴫵ݲ���
            ClaudModel model(apiStr, apiKeyStr, modelNameStr);  // ���� API, Ȼ�� APIKEY, ����� Modelname
            model.single_chat();  // ����ģ�ͽ���
        } else {
            // Local Model
            wxLogMessage("Invoking Local Model");

            // ʹ�ô��ļ��ж�ȡ�����ô�������ģ�Ͷ���
            Model model(apiStr, modelNameStr);  // ���ļ��ж�ȡ�����ô��ݸ����캯��
            model.single_chat();  // ����ģ�ͽ���
        }
    } catch (const std::exception& e) {
        wxLogError("Error during model interaction: %s", e.what());
        return;
    }

    // ���ö�ȡ��־
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

// ����ࣺ�� Code::Blocks ��������ע���ݼ���ʾ����
class MyPlugin : public cbPlugin
{
public:
    MyPlugin() : cbPlugin(), dialog(nullptr), mark(true) { }
    virtual ~MyPlugin() { if (dialog) delete dialog; }

    virtual void OnAttach()
    {
        wxFrame* mainFrame = Manager::Get()->GetAppFrame();

        // ����������������CTRL+M ��ʾд����棬CTRL+K ��ʾ log �ļ�����
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
    bool mark = true; // Ĭ������ģʽ
    ModeSelectDialog modeDlg(Manager::Get()->GetAppFrame(), mark);

    // ��ʾģʽѡ��Ի���
    if (modeDlg.ShowModal() == wxID_OK)
    {
        wxLogMessage("Mode selected: %s", mark ? "Cloud" : "Local");

        // ��ʾģ��������Ϣ�Ի���
        ModelConfigDialog configDlg(Manager::Get()->GetAppFrame(), mark);
        if (configDlg.ShowModal() == wxID_OK)
        {
            // ������Ϣ����󣬼�����ʾ������
            configDlg.SaveSettings(); // ���������ļ�

            // ��������ʾ���Ի�����ģ�ͶԻ��������棩
            if (dialog)
            {
                dialog->Destroy(); // ����֮ǰ�ĶԻ���������ڣ�
                dialog = nullptr;
            }

            // �����µ����Ի�����ʾ
            dialog = new MyPluginDialog(Manager::Get()->GetAppFrame(), mark);
            dialog->Show();
            dialog->Raise(); // ȷ�����Ի�����ʾ����ǰ��
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
    bool mark; // true ��ʾ Cloud��false ��ʾ Local
};

cbPlugin* CreatePlugIn()
{
    return new MyPlugin();
}
PluginRegistrant<MyPlugin> reg("MyPlugin");
