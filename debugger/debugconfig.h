#ifndef DEBUG_CONFIG_H
#define DEBUG_CONFIG_H

#include <stdio.h>

/** These are desgined for quick  read/write for small non-unique sections,
    and the sections will be invalid once class is deleted. **/

#define DCONF_MAX_VAL 8

struct DebugConfigSection
{
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

        void WriteSection(DebugConfigSection *sect);

    private:
        FILE *file;
};

#endif // DEBUG_CONFIG_H

