#include "debugpanel.h"
#include "debuggerframe.h"
#include "debugconsole.h"

DebugPanel::DebugPanel(DebuggerFrame *parent_, int id) : wxPanel(parent_, id)
{
    parent = parent_;
    output = 0;
}

DebugPanel::~DebugPanel()
{
}

void DebugPanel::Rename(wxCommandEvent &evt)
{

}

void DebugPanel::Print(wxString &msg)
{
    if (output)
        output->Print(msg);
    else
        parent->Print(msg);
}
