// wxWidgets "Hello World" Program

#include <wx/wx.h>
#include <string>

class MyApp : public wxApp
{
public:
    virtual bool OnInit();
};
class MyFrame : public wxFrame
{
public:
    MyFrame();
private:
    void OnStart(wxCommandEvent& event);
    void OnExit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
    void OnSelect(wxCommandEvent& event);
};
enum
{
    ID_Start = 1
};
wxIMPLEMENT_APP(MyApp);

wxRadioButton *rb,*rb2;
wxButton* b;

bool MyApp::OnInit()
{
    MyFrame* frame = new MyFrame();

    rb = new wxRadioButton(frame, 0, "Webcam 1",wxPoint(10,0),wxDefaultSize, wxRB_GROUP);
    rb2 = new wxRadioButton(frame, 1, "Webcam 2", wxPoint(10,20));
    b = new wxButton(frame, wxID_ANY, "Start", wxPoint(30, 90));
    frame->Show(true);

    return true;
}
MyFrame::MyFrame()
    : wxFrame(NULL, wxID_ANY, "Camshift cameras")
{
    wxMenu* menuFile = new wxMenu;
    wxBoxSizer Sizer(wxVERTICAL);
    menuFile->Append(ID_Start, "&Start...\tCtrl-H",
        "Start Camshift with the slected camera");
    menuFile->AppendSeparator();
    menuFile->Append(wxID_EXIT);
    wxMenu* menuHelp = new wxMenu;
    menuHelp->Append(wxID_ABOUT);
    wxMenuBar* menuBar = new wxMenuBar;
    menuBar->Append(menuFile, "&File");
    menuBar->Append(menuHelp, "&Help");
    SetMenuBar(menuBar);
    CreateStatusBar();
    SetStatusText("Welcome to Camshift!");
    Sizer.Add(this);
    Bind(wxEVT_MENU, &MyFrame::OnAbout, this, wxID_ABOUT);
    Bind(wxEVT_MENU, &MyFrame::OnExit, this, wxID_EXIT);
    Bind(wxEVT_MENU, &MyFrame::OnStart, this, ID_Start);
    Bind(wxEVT_RADIOBUTTON, &MyFrame::OnSelect, this);
    Bind(wxEVT_BUTTON, &MyFrame::OnStart, this);
}
void MyFrame::OnExit(wxCommandEvent& event)
{
    Close(true);
}
void MyFrame::OnAbout(wxCommandEvent& event)
{
    wxMessageBox("Camshift by Jaga using OpenCV, gStreamer and wxWidgets",
        "About Camshift", wxOK | wxICON_INFORMATION);
}


void MyFrame::OnStart(wxCommandEvent& event)
{
    wxLogMessage("Program is running!");
}

void MyFrame::OnSelect(wxCommandEvent& event)
{
    int r = event.GetId();
    std::string str = std::to_string(r);
    wxLogMessage(wxFormatString(str));
}