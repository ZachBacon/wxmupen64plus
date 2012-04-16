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

    while (pos != eof)
    {
        char *start = pos;
        while (pos != eof && *pos != '\n')
            pos++;
        while (start != pos && *start == ' ')
            start++;
        if (start != pos)
        {
            if (*start++ != '[')
                return false;
            char *end = pos - 1;
            while (end != start && *end == ' ')
                end--;
            if (end[0] != ']')
                return false;
            end[0] = 0;
            out->name = start;
            pos++;
            break;
        }
        pos++;
    }
    if (pos == eof)
        return false;

    out->num_values = 0;
    while (pos != eof && out->num_values != DCONF_MAX_VAL)
    {
        char *start = pos;
        while (pos != eof && *pos != '\n')
            pos++;
        while (start != pos && *start == ' ')
            start++;
        if (start != pos)
        {
            if (*start == '[')
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
                eq[0] = 0;
                end[1] = 0;
                out->keys[out->num_values] = start;
                out->values[out->num_values] = eq + 1;
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

void DebugConfigOut::WriteSection(DebugConfigSection *sect)
{
    fprintf(file, "[%s]\n", sect->name);
    for (int i = 0; i < sect->num_values; i++)
        fprintf(file, "%s=%s\n", sect->keys[i], sect->values[i]);
    fputc('\n', file);
}
