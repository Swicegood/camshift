#include "MyApp.h"

int main(int argc, char** argv)
{
    wxApp::SetInstance(new MyApp());
    return wxEntry(argc, argv);
}