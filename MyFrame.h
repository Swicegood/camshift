#pragma once
#ifndef MYFRAME_H
#define MYFRAME_H

#include <wx/wx.h>

class MyFrame : public wxFrame {
public:
    MyFrame();
    void OnStart(wxCommandEvent& event);
    void OnSelect(wxCommandEvent& event);

private:
    void OnExit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
    void OnSelectRes(wxCommandEvent& event);

    int r_ID = 0;
    int sel, resh, resv;
    wxString reso;
    std::string resh_str, resv_str;
    wxChoice* cs;
    wxChoice* m_choice;
    wxBoxSizer* m_mainSizer;
};

enum {
    ID_Start = 1
};

#endif // MYFRAME_H