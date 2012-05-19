/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   wxMupen64Plus debugger                                                *
 *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
 *   Copyright (C) 2012 Markus Heikkinen                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef BREAKPOINTPANEL_H
#define BREAKPOINTPANEL_H

#include "debugpanel.h"
class wxDataViewCtrl;
class wxDataViewColumn;
class BreakDataModel;
class DebuggerFrame;
class wxDataViewEvent;
class Breakpoint;


class BreakpointPanel : public DebugPanel
{
    public:
        BreakpointPanel(DebuggerFrame *parent, int id, int type, DebugConfigSection &config);
        virtual ~BreakpointPanel();

        void SaveConfig(DebugConfigOut &config, DebugConfigSection &section);
        void Update(bool vi);
        void BreakpointUpdate(Breakpoint *bpt, BreakUpdateCause reason, bool last_update);

        void RClickMenu(wxCommandEvent &evt);
        void RClickItem(wxDataViewEvent &evt);
        void RClickEvent(wxCommandEvent &evt);
        void Deselect(wxMouseEvent &evt);
        void KeyDown(wxKeyEvent &evt);
        void EditEvent(wxDataViewEvent &evt);

        void AddBreakpoint(Breakpoint *bpt);

        void Reset();

    private:
        void CreateList();
        bool FilterTest(Breakpoint *bpt);
        bool HasValueChanged(int item, const uint64_t &new_value);
        void GenerateFilterMenu(wxMenu *menu);
        int filter;
        int current_item;

        void AddDialog();
        void EditDialog();
        void DeleteSelected();
        void EnableSelected(bool enable);

        wxDataViewCtrl *list;
        wxDataViewColumn *name_column;
        wxDataViewColumn *address_column;
        wxDataViewColumn *value_column;
        wxDataViewColumn *enabled_column;
        // If someday there will be a way tofilter what values wxDataViewCtrl actually shows,
        // this could be shared between all instances of BreakpointPanel
        BreakDataModel *data_model;
};

#endif // BREAKPOINTPANEL_H
