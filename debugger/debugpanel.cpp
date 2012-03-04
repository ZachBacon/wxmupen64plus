#include "debugpanel.h"
#include "debuggerframe.h"

DebugPanel::DebugPanel(DebuggerFrame *parent_, int id) : wxPanel(parent_, id)
{
    parent = parent_;
}

DebugPanel::~DebugPanel()
{
    parent->RemovePanel(this);
}

void DebugPanel::Rename(wxCommandEvent &evt)
{

}
