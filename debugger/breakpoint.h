#ifndef BREAKPOINT_H
#define BREAKPOINT_H
#include <stdint.h>
#include <wx/string.h>
#include <unordered_map>
#include <unordered_set>

// These correspond to core's dpg_breakpoint.h
#define BREAK_TYPE_EXECUTE 0x20
#define BREAK_TYPE_READ 0x8
#define BREAK_TYPE_WRITE 0x10
#define BREAK_TYPE_ALL (BREAK_TYPE_EXECUTE | BREAK_TYPE_READ | BREAK_TYPE_WRITE)

class Breakpoint;
extern template class std::unordered_map<uint32_t, Breakpoint *>;

enum BreakValidity
{
    BREAK_VALID = 0,
    BREAK_INVALID_TYPE,
    BREAK_INVALID_ADDRESS,
    BREAK_INVALID_NAME,
    BREAK_INVALID_LENGTH
};

class Breakpoint
{
    public:
        Breakpoint();
        Breakpoint(const wxString &name, uint32_t address, int length, char type);
        bool Update(const wxString &name, uint32_t address, int length, char type);
        ~Breakpoint();
        bool Add();
        void Remove();
        void Enable();
        void Disable();

        BreakValidity IsValid() const;
        bool IsEnabled() const { return enabled; }
        char GetType() const { return type; }
        const wxString &GetName() const { return name; }
        uint32_t GetAddress() const { return address; }
        int GetLength() const { return length; }

        static void SetChanged(bool val) { changed = val; }
        static bool IsChanged() { return changed; }

        static Breakpoint *Find(uint32_t address);
        static const std::unordered_set<Breakpoint *> *GetAllBreakpoints() { return breakset; }

    private:
        wxString name;
        int id;
        uint32_t address;
        int length;
        char type;
        bool active;
        bool enabled;

        // I don't think hash table is the best option for data ranges far aways each other, but it shall be enough for now
        static std::unordered_map<uint32_t, Breakpoint *> *breakmap;
        static std::unordered_set<Breakpoint *> *breakset;
        static bool changed;
};


#endif // BREAKPOINT_H

