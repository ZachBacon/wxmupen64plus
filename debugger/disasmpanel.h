#ifndef DISASMPANEL_H
#define DISASMPANEL_H

#include "debugpanel.h"


class DisasmPanel : public DebugPanel
{
    public:
        DisasmPanel(DebuggerFrame *parent, int id = -1);
        virtual ~DisasmPanel();

        void Update(bool vi);
};

#endif // DISASMPANEL_H

