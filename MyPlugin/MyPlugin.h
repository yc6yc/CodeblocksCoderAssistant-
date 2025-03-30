#ifndef MYPLUGIN_H
#define MYPLUGIN_H

#include <cbplugin.h>
#include <wx/wx.h>

class MyPlugin : public cbPlugin {
public:
    MyPlugin();
    ~MyPlugin();

    void OnAttach() override;
    void OnRelease() override;
    void Execute() override;

private:
    void OnSave(wxCommandEvent& event);
    void OnRead(wxCommandEvent& event);

    wxFrame* m_frame;
    wxTextCtrl* m_textInput;
    wxStaticText* m_textDisplay;

    wxDECLARE_EVENT_TABLE();
};

#endif
