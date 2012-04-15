#ifndef DEBUGGER_MEMORYSEARCH_H
#define DEBUGGER_MEMORYSEARCH_H

#include <stdint.h>
#include <vector>
#include <list>

#include "debugpanel.h"

using namespace std;

class wxDataViewListCtrl;
class wxRadioButton;
class wxChoice;
class wxStaticText;
class wxTextCtrl;
class wxButton;
class wxCheckBox;

class MemChunk
{
    public:
        MemChunk() {}
        MemChunk(MemChunk &&other);
        MemChunk &operator=(MemChunk &&other);
        MemChunk(void *data, uint32_t length, uint32_t address);
        ~MemChunk();

        template<typename T> T operator[](uint32_t pos);

        uint32_t start_address;
        uint32_t length;
        uint8_t *data;

    private:
        uint8_t *realdata;
};

class MemSearchResult
{
    public:
        MemSearchResult() { mem_usage = 0; }
        MemSearchResult(MemSearchResult &&other);
        MemSearchResult &operator=(MemSearchResult &&other);
        ~MemSearchResult() {}

        vector<MemChunk> *GetChunks() { return &chunks; }
        void AddChunk(MemChunk &chunk);
        uint32_t MemUsage() { return mem_usage; }

    private:
        vector<MemChunk> chunks;
        uint32_t mem_usage;
};

enum SearchType // NOTE: These must correspond to valuetype_choices array in memorysearch.cpp
{
    int8 = 0, int16, int32, int64,
    uint8, uint16, uint32, uint64,
    float32, float64
};

class MemSearch
{
    public:
        MemSearch();

        void Clear();

        template<typename type, class compare> void Filter(compare, bool initial_filter);
        void NewSearch(uint32_t beg, uint32_t end);

        MemSearchResult *GetResult() { return &undo_list[undo_pos]; }

        void Undo();
        void Redo();
        bool CanUndo() { return undo_pos > 0; }
        bool CanRedo() { return (uint32_t)(undo_pos + 1) != undo_list.size(); }

    private:
        vector<MemSearchResult> undo_list;
        int undo_pos;

        uint32_t undo_memusage;
};

class MemSearchPanel : public DebugPanel
{
    public:
        MemSearchPanel(DebuggerFrame *parent, int id = -1);
        ~MemSearchPanel();

        void RClickMenu(wxContextMenuEvent &evt);
        void RClickEvent(wxCommandEvent &evt);
        void RadioEvent(wxCommandEvent &evt);
        void FilterEvent(wxCommandEvent &evt);

    private:
        template<typename type, class formatfunc> void Filter(formatfunc);
        template<class formatfunc> void UpdateList(uint32_t value_size, formatfunc);

        SearchType type;
        MemSearch search;

        wxDataViewListCtrl *list;

        wxChoice *choice_valuetype;

        wxStaticText *text_newvalue;
        wxChoice *choice_cmp;
        wxRadioButton *radio_old;
        wxRadioButton *radio_custom;
        wxTextCtrl *custom_value;
        wxButton *btn_filter;

        short current_radio;
        short previous_choice;
        bool first_filter;
};



#endif // DEBUGGER_MEMORYSEARCH_H
