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

// ������Ҫ��ͷ�ļ�
#include <sdk.h>
#include <configurationpanel.h>
#include <cbplugin.h>
#include <wx/menu.h>
#include <wx/event.h>
#include <string>

// CodeExportPlugin ����࣬���ڵ������뵽�ļ���
// ֧�ֵ��������ļ��Լ����֮ǰ�Ĵ��룬ͬʱ����״̬�л����ܡ�
class CodeExportPlugin : public cbPlugin
{
public:
    // ���캯������ʼ��������ڲ�״̬
    CodeExportPlugin();
    // ����������������Դ����ǰ��������������
    virtual ~CodeExportPlugin();

    // ����������ӵ� Code::Blocks ʱ����
    virtual void OnAttach();
    // ��������ͷ�ʱ���ã����ﱣ�ֿ�ʵ��
    virtual void OnRelease() {}
    // �� CodeExportPlugin ��� public ������ӣ�
    void OnShortcutCtrlN(wxCommandEvent& event);
    static std::string ExtractFirstCodeBlock(const std::string &content);
    // ��ȡ��ǰ״̬�Ľӿڣ�����������ͨ���˷������״̬�仯
    bool GetToggleState_1() const { return m_toggleState_1; }
    bool GetToggleState_2() const { return m_toggleState_2; }

private:
    // ��ݼ��¼��������������¿�ݼ�ʱ�������Щ����
    void OnShortcutFull(wxCommandEvent& event);         // ���� Shift+S ��ݼ�
    void OnShortcutBeforeCursor(wxCommandEvent& event);   // ���� Alt+�ո� ��ݼ�

    // ����ĵ�����������
    void SaveFullCodeToFile();           // ���������ļ��Ĵ���
    void SaveCodeBeforeCursorToFile();   // �������֮ǰ�Ĵ���

    // ״̬�����������л�����ʼֵΪ false
    bool m_toggleState_1;
    bool m_toggleState_2;

    // ���������ڲ��� Shift+S ���ַ������¼�������
    void OnCharHook(wxKeyEvent& event);
    //bool ClearFileContent(const std::string& filePath);
    void OnShortPasteCtrlH(wxCommandEvent& event);

    wxDECLARE_EVENT_TABLE()
};

#endif // CODEEXPORTPLUGIN_H
