#pragma once
#ifndef MYAPP_H
#define MYAPP_H

#include <wx/wx.h>
#include "MyFrame.h"

class MyApp : public wxApp {
public:
    virtual bool OnInit();
};

#endif // MYAPP_H