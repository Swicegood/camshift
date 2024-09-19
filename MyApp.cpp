#include "MyApp.h"
#include "DeviceEnumerator.h"
#include "Utilities.h"

wxIMPLEMENT_APP(MyApp);

bool MyApp::OnInit() {
    MyFrame* frame = new MyFrame();

    DeviceEnumerator de;
    std::map<int, Device> devices = de.getVideoDevicesMap();

    for (const auto& device : devices) {
        DebugOutputW(L"Device ID from DeviceEnumerator: " + toWString(device.first));
    }

    wxPanel* devicePanel = new wxPanel(frame, wxID_ANY);
    wxBoxSizer* deviceSizer = new wxBoxSizer(wxVERTICAL);

    wxStaticText* deviceLabel = new wxStaticText(devicePanel, wxID_ANY, "Select Camera:");
    deviceSizer->Add(deviceLabel, 0, wxALL, 5);

    wxRadioButton* r;
    for (const auto& device : devices) {
        std::string label = "Webcam " + std::to_string(device.first);
        r = new wxRadioButton(devicePanel, device.first, label);
        deviceSizer->Add(r, 0, wxALL, 5);
        r->Bind(wxEVT_RADIOBUTTON, &MyFrame::OnSelect, frame);
    }

    devicePanel->SetSizer(deviceSizer);
    frame->GetSizer()->Insert(0, devicePanel, 0, wxALL | wxEXPAND, 5);

    frame->Layout();
    frame->Show(true);
    return true;
}