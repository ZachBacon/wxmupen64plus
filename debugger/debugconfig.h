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

#ifndef DEBUG_CONFIG_H
#define DEBUG_CONFIG_H

#include <stdio.h>
#include <wx/string.h>

/** These are desgined for quick  read/write for small non-unique sections,
    and the sections will be invalid once class is deleted. **/

// If needed, constant array can just be replaced with std::vector or something
// However, this implementation has only one memory allocation per file (and nobody cares)
#define DCONF_MAX_VAL 8

class DebugConfigSection
{
    public:
        DebugConfigSection() {}
        const char *GetValue(const char *key, const char *def);

        const char *name;
        const char *keys[DCONF_MAX_VAL];
        const char *values[DCONF_MAX_VAL];
        int num_values;
};

class DebugConfigIn
{
    public:
        DebugConfigIn(const char *filename);
        ~DebugConfigIn();

        bool GetNextSection(DebugConfigSection *out);
        bool IsOk() { return file != 0; }
    private:
        FILE *file;
        char *buf;
        char *pos;
        char *eof;
};

class DebugConfigOut
{
    public:
        DebugConfigOut(const char *filename);
        ~DebugConfigOut();

        void WriteSection(DebugConfigSection &sect);
        void WriteComment(const char *comment);

    private:
        FILE *file;
        wxString filename;
};

#endif // DEBUG_CONFIG_H

