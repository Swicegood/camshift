#include "MyFrame.h"
#include "CameraApp.h"

MyFrame::MyFrame() : wxFrame(NULL, wxID_ANY, "Camshift Cameras", wxDefaultPosition, wxSize(400, 300)) {
    // Menu setup
    wxMenu* menuFile = new wxMenu;
    menuFile->Append(ID_Start, "&Start...\tCtrl-H", "Start Camshift with the selected camera");
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

    // Create main sizer
    m_mainSizer = new wxBoxSizer(wxVERTICAL);

    // Create resolution choices
    wxArrayString resolutions;
    resolutions.Add("640x360");
    resolutions.Add("800x448");
    resolutions.Add("1280x720");

    // Create and add resolution selector
    wxStaticText* resolutionLabel = new wxStaticText(this, wxID_ANY, "Select Resolution:");
    m_mainSizer->Add(resolutionLabel, 0, wxALL, 5);

    m_choice = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, resolutions);
    m_mainSizer->Add(m_choice, 0, wxALL | wxEXPAND, 5);

    // Create and add start button
    wxButton* startButton = new wxButton(this, wxID_ANY, "Start");
    m_mainSizer->Add(startButton, 0, wxALL | wxEXPAND, 5);

    // Set the sizer for the frame
    SetSizer(m_mainSizer);
    Layout();

    // Event bindings
    Bind(wxEVT_MENU, &MyFrame::OnAbout, this, wxID_ABOUT);
    Bind(wxEVT_MENU, &MyFrame::OnExit, this, wxID_EXIT);
    Bind(wxEVT_MENU, &MyFrame::OnStart, this, ID_Start);
    Bind(wxEVT_RADIOBUTTON, &MyFrame::OnSelect, this);
    startButton->Bind(wxEVT_BUTTON, &MyFrame::OnStart, this);
    m_choice->Bind(wxEVT_CHOICE, &MyFrame::OnSelectRes, this);
}

void MyFrame::OnExit(wxCommandEvent& event) {
    Close(true);
}

void MyFrame::OnAbout(wxCommandEvent& event) {
    wxMessageBox("Camshift by Jaga using OpenCV, GStreamer, and wxWidgets",
        "About Camshift", wxOK | wxICON_INFORMATION);
}

void MyFrame::OnSelect(wxCommandEvent& event) {
    r_ID = event.GetId();
}

void MyFrame::OnSelectRes(wxCommandEvent& event) {
    if (m_choice) {
        sel = m_choice->GetSelection();
        reso = m_choice->GetString(sel);

        // Parse resolution
        std::string stlstring = std::string(reso.mb_str());
        std::string delim = "x";
        resh_str = stlstring.substr(0, stlstring.find(delim));
        resv_str = stlstring.substr(resh_str.length() + 1);

        resh = std::stoi(resh_str);
        resv = std::stoi(resv_str);
    }
}

void MyFrame::OnStart(wxCommandEvent& event) {
    if (runCameraApp(r_ID, resh, resv) == 0) {
        Close(true);
    }
}