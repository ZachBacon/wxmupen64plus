#include "debugpanel.h"
#include "debuggerframe.h"
#include "debugconsole.h"
#include "debugconfig.h"

DebugPanel::DebugPanel(DebuggerFrame *parent_, int id, int type_) : wxPanel(parent_, id), type(type_)
{
    parent = parent_;
    output = 0;
}

DebugPanel::~DebugPanel()
{
}

void DebugPanel::Print(const wxString &msg)
{
    if (output)
        output->Print(msg);
    else
        parent->Print(msg);
}

void DebugPanel::SaveConfig(DebugConfigOut &config, DebugConfigSection &section)
{
    section.num_values = 0;
    config.WriteSection(section);
}
