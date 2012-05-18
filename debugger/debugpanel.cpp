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
