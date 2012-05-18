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

#include "debugconfig.h"
#include "../mupen64plusplus/osal_preproc.h"

const char *DebugConfigSection::GetValue(const char *key, const char *def)
{
    for (int i = 0; i < num_values; i++)
    {
        if (osal_insensitive_strcmp(key, keys[i]) == 0)
            return values[i];
    }
    return def;
}

DebugConfigIn::DebugConfigIn(const char *filename)
{
    file = fopen(filename, "r");
    if (file)
    {
        fseek(file, 0, SEEK_END);
        int length = ftell(file);
        fseek(file, 0, SEEK_SET);
        pos = buf = new char[length + 1];
        buf[length] = 0;
        eof = buf + fread(buf, 1, length, file);
    }
    else
        buf = 0;
}

DebugConfigIn::~DebugConfigIn()
{
    delete[] buf;
    if (file)
        fclose(file);
}

bool DebugConfigIn::GetNextSection(DebugConfigSection *out)
{
    if (pos == eof)
        return false;

    // Fun
    while (pos != eof)
    {
        char *start = pos;
        while (pos != eof && *pos != '\n')                          // separate the current line
            pos++;
        while (start != pos && (*start == ' ' || *start == '\x09')) // ignore leading spaces and tabs
            start++;
        if (start != pos && *start != '#')                          // ignore empty lines and comments
        {
            if (*start++ != '[')                                    // didn't start with section, assume corrupted
                return false;
            char *end = pos - 1;
            while (end != start && *end == ' ')
                end--;
            if (end[0] != ']')
                return false;
            end[0] = 0;
            out->name = start;
            pos++;
            break;                                                  // section title found, go on
        }
        pos++;
    }
    if (pos == eof)
        return false;

    out->num_values = 0;
    while (pos != eof && out->num_values != DCONF_MAX_VAL)
    {
        char *start = pos;
        while (pos != eof && *pos != '\n')                          // separate the current line
            pos++;
        while (start != pos && (*start == ' ' || *start == '\x09')) // ignore leading spaces and tabs
            start++;
        if (start != pos && *start != '#')                          // ignore empty lines and comments
        {
            if (*start == '[')                                      // new section, so the current one ends here
            {
                pos = start;
                return true;
            }
            char *end = pos - 1;
            while (end != start && *end == ' ')
                end--;
            char *eq = start;
            while (eq != end && *eq != '=')
                eq++;
            if (*eq == '=')
            {
                eq[0] = 0;                                          // as long as the first character isn't #, keys
                end[1] = 0;                                         // can contain anything but = or newline
                out->keys[out->num_values] = start;
                out->values[out->num_values] = eq + 1;              // values can contain anything expect newlines
                out->num_values++;
            }
        }
        pos++;
    }
    return true;
}


DebugConfigOut::DebugConfigOut(const char *filename_) : filename(filename_)
{
    file = fopen(filename + ".tmp", "w");
}

DebugConfigOut::~DebugConfigOut()
{
    if (file)
    {
        fclose(file);
        remove(filename);
        rename(filename + ".tmp", filename);
    }
}

void DebugConfigOut::WriteSection(DebugConfigSection &sect)
{
    fprintf(file, "[%s]\n", sect.name);
    for (int i = 0; i < sect.num_values; i++)
        fprintf(file, "%s=%s\n", sect.keys[i], sect.values[i]);
    fputc('\n', file);
}

void DebugConfigOut::WriteComment(const char *comment)
{
    fprintf(file, "# %s\n", comment);
}
