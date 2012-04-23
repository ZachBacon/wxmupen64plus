#include "memorysearch.h"

#include <wx/sizer.h>
#include <wx/dataview.h>
#include <wx/list.h>
#include <wx/radiobut.h>
#include <wx/choice.h>
#include <wx/stattext.h>
#include <wx/menu.h>
#include <wx/textctrl.h>
#include <wx/button.h>
#include <wx/statline.h>
#include <wx/checkbox.h>

#include "../mupen64plusplus/MupenAPI.h"
#include "debugconfig.h"

enum
{
    menu_filter = 2,
    menu_clear,
    menu_undo,
    menu_redo,
    //radio_any_id,
    radio_old_id,
    radio_custom_id
};

MemChunk::MemChunk(void *data_, uint32_t len, uint32_t address) : start_address(address), length(len)
{
    if (!(start_address & 0x3)) // already aligned
    {
        if (len & 0x3)
            len = (len + 4) & ~0x3;
        realdata = (uint8_t *)malloc(len);
        memcpy(realdata, data_, len);
        data = realdata;
    }
    else
    {
        realdata = (uint8_t *)malloc((length + 4) & ~0x3);
        memcpy(realdata, (void *)((uint32_t)data_ & ~0x3), (length + 4) & ~0x3);
        data = realdata + (start_address & 0x3);
    }
}

MemChunk::MemChunk(MemChunk &&other)
{
    data = other.data;
    realdata = other.realdata;
    start_address = other.start_address;
    length = other.length;
    other.realdata = 0;
}

MemChunk &MemChunk::operator=(MemChunk &&other)
{
    data = other.data;
    realdata = other.realdata;
    start_address = other.start_address;
    length = other.length;
    other.realdata = 0;
    return *this;
}

MemChunk::~MemChunk()
{
    if (realdata)
        free(realdata);
}

MemSearchResult::MemSearchResult(MemSearchResult &&other)
{
    chunks = move(other.chunks);
    mem_usage = other.mem_usage;
}

MemSearchResult &MemSearchResult::operator=(MemSearchResult &&other)
{
    chunks = move(other.chunks);
    mem_usage = other.mem_usage;
    return *this;
}

void MemSearchResult::AddChunk(MemChunk &chunk)
{
    mem_usage += chunk.length;
    chunks.push_back(move(chunk));
}

MemSearch::MemSearch()
{
    undo_pos = -1;
    undo_memusage = 0;
}

void MemSearch::Clear()
{
    undo_list.clear();
    undo_pos = -1;
    undo_memusage = 0;
}

void MemSearch::Undo()
{
    if (!CanUndo())
        return;
    --undo_pos;
}

void MemSearch::Redo()
{
    if (!CanRedo())
        return;
    ++undo_pos;
}

void MemSearch::NewSearch(uint32_t beg, uint32_t end)
{
    MemSearchResult base;
    uint8_t *rdram = (uint8_t *)GetMemoryPointer(M64P_DBG_PTR_RDRAM);
    MemChunk everything(rdram + beg, end - beg + 1, 0);
    base.AddChunk(everything);
    undo_list.push_back(move(base));
    undo_memusage += (end - beg + 1);
    undo_pos++;
}

// Allocated memory is always aligned by at least 4, right?
// If not, this will bug a loooot..

template<typename type_>
type_ magicswap(uint8_t *pointer);
//{
//    return (_type)*pointer;
//}

template<>
int8_t magicswap<int8_t>(uint8_t *pointer)
{
    switch ((uint32_t)pointer & 3)
    {
        default:
        case 0:
            return *(int8_t *)(pointer + 3);
        case 1:
            return *(int8_t *)(pointer + 1);
        case 2:
            return *(int8_t *)(pointer - 1);
        case 3:
            return *(int8_t *)(pointer - 3);
    }
}

template<>
uint8_t magicswap<uint8_t>(uint8_t *pointer)
{
    switch ((uint32_t)pointer & 3)
    {
        default:
        case 0:
            return *(pointer + 3);
        case 1:
            return *(pointer + 1);
        case 2:
            return *(pointer - 1);
        case 3:
            return *(pointer - 3);
    }
}

template<>
int16_t magicswap<int16_t>(uint8_t *pointer)
{
    if ((uint32_t)pointer & 2)
        return *(int16_t *)(pointer - 2);
    else
        return *(int16_t *)(pointer + 2);
}

template<>
uint16_t magicswap<uint16_t>(uint8_t *pointer)
{
    if ((uint32_t)pointer & 2)
        return *(uint16_t *)(pointer);
    else
        return *(uint16_t *)(pointer + 2);
}

template<>
int32_t magicswap<int32_t>(uint8_t *pointer)
{
    return *(int32_t *)pointer;
}

template<>
uint32_t magicswap<uint32_t>(uint8_t *pointer)
{
    return *(uint32_t *)pointer;
}

template<>
float magicswap<float>(uint8_t *pointer)
{
    return *(float *)pointer;
}

template<>
int64_t magicswap<int64_t>(uint8_t *pointer)
{
    return (int64_t)(((uint64_t)(*(uint32_t *)pointer) << 32) | (*(uint32_t *)(pointer + 4)));
}

template<>
uint64_t magicswap<uint64_t>(uint8_t *pointer)
{
    return ((uint64_t)(*(uint32_t *)pointer) << 32) | (*(uint32_t *)(pointer + 4));
}

template<>
double magicswap<double>(uint8_t *pointer)
{
    return (double)(((uint64_t)(*(uint32_t *)pointer) << 32) | (*(uint32_t *)(pointer + 4))); // TODO: does this even work?
}

template<typename type, class compare>
void MemSearch::Filter(compare op, bool initial_filter)
{
    undo_list.resize(undo_pos + 1);

    vector<MemChunk> *chunks = undo_list[undo_pos].GetChunks();

    MemSearchResult new_result;
    uint8_t *rdram = (uint8_t *)GetMemoryPointer(M64P_DBG_PTR_RDRAM); // rdram is actually uint32_t * with weird endianess tricks <.<

    for (uint32_t i = 0; i < chunks->size(); i++)
    {
        MemChunk &chunk = (*chunks)[i];
        uint32_t new_chunk_beg = 0;
        uint32_t j;
        for (j = 0; j + sizeof(type) <= chunk.length; j += sizeof(type))
        {
            if (op(magicswap<type>(rdram + j + chunk.start_address), magicswap<type>(chunk.data + j)))
            {
                if (!new_chunk_beg)
                {
                    new_chunk_beg = (chunk.start_address + j) + 1; // +1 as it could be 0 otherwise
                }
            }
            else
            {
                if (new_chunk_beg)
                {
                    MemChunk new_chunk((rdram + new_chunk_beg - 1), j - (new_chunk_beg - 1) + chunk.start_address, new_chunk_beg - 1);
                    new_result.AddChunk(new_chunk);
                    new_chunk_beg = 0;
                }
            }
        }
        if (new_chunk_beg)
        {
            MemChunk new_chunk((rdram + new_chunk_beg - 1), j - (new_chunk_beg - 1) + chunk.start_address, new_chunk_beg - 1);
            new_result.AddChunk(new_chunk);
        }
    }

    int new_memusage = new_result.MemUsage();
    if (initial_filter)
    {
        undo_list[0] = move(new_result);
    }
    else
    {
        undo_list.push_back(move(new_result));
        undo_pos++;
    }
    while (undo_memusage && undo_memusage + new_memusage > (1 << 22)) // 4 megabytes, could be choosable by user?
    {
        undo_memusage -= undo_list[0].MemUsage();
        undo_list.erase(undo_list.begin());
        undo_pos--;
    }

    undo_memusage += new_memusage;
}

static const wxString valuetype_choices[] = // NOTE: These must correspond to ValueType enum in memorysearch.h
{
    "8-bit signed", "16-bit signed", "32-bit signed", "64-bit signed",
    "8-bit unsigned", "16-bit unsigned", "32-bit unsigned", "64-bit unsigned",
    "32-bit float", "64-bit float"
};

static const wxString cmp_choices[] = // And these correspond to the enum below
{
    "==", "!=", ">", "<", ">=", "<="
};

enum
{
    cmp_eq = 0, cmp_ne, cmp_gt, cmp_lt, cmp_ge, cmp_le
};

MemSearchPanel::MemSearchPanel(DebuggerFrame *parent, int id, int type, DebugConfigSection &config) : DebugPanel(parent, id, type)
{
    first_filter = true;


    wxBoxSizer *sizer = new wxBoxSizer(wxHORIZONTAL), *filtersizer = new wxBoxSizer(wxVERTICAL);

    list = new wxDataViewListCtrl(this, -1);
    list->AppendTextColumn(_("Address"), wxDATAVIEW_CELL_INERT, -1, wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_SORTABLE);
    list->AppendTextColumn(_("Value"), wxDATAVIEW_CELL_INERT, -1, wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_SORTABLE);

    wxPanel *filterpanel = new wxPanel(this, -1);

    wxStaticText *text_searchfor = new wxStaticText(filterpanel, -1, _("Search for.."));
    choice_valuetype = new wxChoice(filterpanel, -1, wxDefaultPosition, wxDefaultSize, sizeof(valuetype_choices) / sizeof(wxString), valuetype_choices);
    wxStaticLine *line = new wxStaticLine(filterpanel, -1);

    text_newvalue = new wxStaticText(filterpanel, -1, _("New value.."));
    choice_cmp = new wxChoice(filterpanel, -1, wxDefaultPosition, wxDefaultSize, sizeof(cmp_choices) / sizeof(wxString), cmp_choices);
    radio_old = new wxRadioButton(filterpanel, radio_old_id, _("Anything"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
    radio_custom = new wxRadioButton(filterpanel, radio_custom_id, _("Custom value"));
    custom_value = new wxTextCtrl(filterpanel, -1);
    wxStaticLine *line2 = new wxStaticLine(filterpanel, -1);
    btn_filter = new wxButton(filterpanel, -1, _("Search"));

    choice_valuetype->Select(atoi(config.GetValue("ValueType", "0")));
    choice_cmp->Select(atoi(config.GetValue("Compare", "0")));
    previous_choice = 0;
    radio_custom->SetValue(true);
    current_radio = radio_custom_id;

    filtersizer->Add(text_searchfor, 0, wxEXPAND);
    filtersizer->Add(choice_valuetype, 0, wxEXPAND | wxTOP | wxBOTTOM, 2);
    filtersizer->Add(line, 0, wxEXPAND | wxALL, 2);
    filtersizer->Add(text_newvalue, 0, wxEXPAND);
    filtersizer->Add(choice_cmp, 0, wxEXPAND | wxTOP | wxBOTTOM, 2);
    filtersizer->Add(radio_old, 0, wxEXPAND | wxTOP | wxBOTTOM, 2);
    filtersizer->Add(radio_custom, 0, wxEXPAND | wxTOP | wxBOTTOM, 2);
    filtersizer->Add(custom_value, 0, wxEXPAND | wxTOP | wxBOTTOM, 2);
    filtersizer->Add(line2, 0, wxEXPAND | wxALL, 2);
    filtersizer->Add(btn_filter, 0, wxEXPAND | wxTOP | wxBOTTOM, 2);


    sizer->Add(list, 1, wxEXPAND);
    sizer->Add(filterpanel, 0, wxEXPAND | wxALL, 3);
    filterpanel->SetSizer(filtersizer);
    SetSizer(sizer);

    list->Bind(wxEVT_CONTEXT_MENU, &MemSearchPanel::RClickMenu, this);
    list->Bind(wxEVT_COMMAND_DATAVIEW_ITEM_CONTEXT_MENU, &MemSearchPanel::ItemRClick, this);
    radio_old->Bind(wxEVT_COMMAND_RADIOBUTTON_SELECTED, &MemSearchPanel::RadioEvent, this);
    radio_custom->Bind(wxEVT_COMMAND_RADIOBUTTON_SELECTED, &MemSearchPanel::RadioEvent, this);
    btn_filter->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &MemSearchPanel::FilterEvent, this);
}

MemSearchPanel::~MemSearchPanel()
{
}

void MemSearchPanel::SaveConfig(DebugConfigOut &config, DebugConfigSection &section)
{
    section.num_values = 2;
    char cmp_buf[8], type_buf[8];
    sprintf(cmp_buf, "%d", choice_cmp->GetSelection());
    section.keys[0] = "Compare";
    section.values[0] = cmp_buf;
    sprintf(type_buf, "%d", choice_valuetype->GetSelection());
    section.keys[1] = "ValueType";
    section.values[1] = type_buf;
    config.WriteSection(section);
}

template<typename type>
type atot(const char *str)
{
    return strtoul(str, 0, 16);
}

template<>
int8_t atot<int8_t>(const char *str)
{
    return atoi(str);
}

template<>
int16_t atot<int16_t>(const char *str)
{
    return atoi(str);
}

template<>
int32_t atot<int32_t>(const char *str)
{
    return atoi(str);
}

template<>
int64_t atot<int64_t>(const char *str)
{
    return strtoll(str, 0, 10);
}

template<>
uint64_t atot<uint64_t>(const char *str)
{
    return strtoull(str, 0, 16);
}

template<>
float atot<float>(const char *str)
{
    return atof(str);
}

template<>
double atot<double>(const char *str)
{
    return atof(str);
}

template<typename type, const char *formatstr>
wxString ValueFormat(uint8_t *data)
{
    return wxString::Format(formatstr, magicswap<type>(data));
}

wxString I64Format(uint8_t *data)
{
    return wxString::Format("%lld", magicswap<int64_t>(data));
}

wxString U64Format(uint8_t *data)
{
    uint64_t val = magicswap<uint64_t>(data);
    return wxString::Format("08X%08X", val >> 32, val && ~0);
}

char deci[] = "%d", h8[] = "%02X", h16[] = "%04X", h32[] = "%08X", fl[] = "%f"; // sigh

template<typename type_, class formatfunc>
void MemSearchPanel::Filter(formatfunc valueformat)
{
    if (radio_custom->GetValue())
    {
        type_ value = atot<type_>(custom_value->GetValue());
        switch (choice_cmp->GetSelection())
        {
            case cmp_eq:
                search.Filter<type_>([&value](type_ a, type_ b) { return a == value; }, first_filter);
            break;
            case cmp_ne:
                search.Filter<type_>([&value](type_ a, type_ b) { return a != value; }, first_filter);
            break;
            case cmp_gt:
                search.Filter<type_>([&value](type_ a, type_ b) { return a > value; }, first_filter);
            break;
            case cmp_lt:
                search.Filter<type_>([&value](type_ a, type_ b) { return a < value; }, first_filter);
            break;
            case cmp_ge:
                search.Filter<type_>([&value](type_ a, type_ b) { return a >= value; }, first_filter);
            break;
            case cmp_le:
                search.Filter<type_>([&value](type_ a, type_ b) { return a <= value; }, first_filter);
            break;
        }
    }
    else
    {
        switch (choice_cmp->GetSelection())
        {
            case cmp_eq:
                search.Filter<type_>([](type_ a, type_ b) { return a == b; }, first_filter);
            break;
            case cmp_ne:
                search.Filter<type_>([](type_ a, type_ b) { return a != b; }, first_filter);
            break;
            case cmp_gt:
                search.Filter<type_>([](type_ a, type_ b) { return a > b; }, first_filter);
            break;
            case cmp_lt:
                search.Filter<type_>([](type_ a, type_ b) { return a < b; }, first_filter);
            break;
            case cmp_ge:
                search.Filter<type_>([](type_ a, type_ b) { return a >= b; }, first_filter);
            break;
            case cmp_le:
                search.Filter<type_>([](type_ a, type_ b) { return a <= b; }, first_filter);
            break;
        }
    }

    UpdateList(sizeof(type_), valueformat);
}

template<class formatfunc>
void MemSearchPanel::UpdateList(uint32_t value_size, formatfunc valueformat)
{
    if (first_filter)
        first_filter = false;
    else
        list->DeleteAllItems();

    MemSearchResult *result = search.GetResult();
    vector<MemChunk> *chunks = result->GetChunks();
    uint32_t memusage = result->MemUsage();
    if (memusage > value_size * 1000)
    {
        wxVector<wxVariant> value;
        value.push_back(wxString::Format("(%d results)", memusage / value_size));
        value.push_back(wxEmptyString);
        list->AppendItem(value);
    }
    else
    {
        for (uint32_t i = 0; i < chunks->size(); i++)
        {
            MemChunk &chunk = (*chunks)[i];
            for (uint32_t j = 0; j + value_size <= chunk.length; j += value_size)
            {
                wxVector<wxVariant> values;
                values.push_back(wxString::Format("+%06X", chunk.start_address + j));
                values.push_back(valueformat(chunk.data + j));
                list->AppendItem(values);
            }
        }
    }
}

void MemSearchPanel::FilterEvent(wxCommandEvent &evt)
{
    if (first_filter)
    {
        if (current_radio == radio_old_id) // "Anything"
        {
            choice_cmp->Select(previous_choice);
            choice_cmp->Enable(true);
        }

        search.NewSearch(0, 0x7fffff);

        choice_valuetype->Enable(false);
        radio_old->SetLabel(_("Old value"));
    }
    if (!first_filter || current_radio != radio_old_id) // Anything else than anything
    {
        switch (choice_valuetype->GetSelection())
        {
            case int8:
                Filter<int8_t>(ValueFormat<int8_t, deci>);
            break;
            case uint8:
                Filter<uint8_t>(ValueFormat<uint8_t, h8>);
            break;
            case int16:
                Filter<int16_t>(ValueFormat<int16_t, deci>);
            break;
            case uint16:
                Filter<uint16_t>(ValueFormat<uint16_t, h16>);
            break;
            case int32:
                Filter<int32_t>(ValueFormat<int32_t, deci>);
            break;
            case uint32:
                Filter<uint32_t>(ValueFormat<uint32_t, h32>);
            break;
            case float32:
                Filter<float>(ValueFormat<float, fl>);
            break;
            case int64:
                Filter<int64_t>(I64Format);
            break;
            case uint64:
                Filter<uint64_t>(U64Format);
            break;
            case float64:
                Filter<double>(ValueFormat<double, fl>);
            break;
        }
    }
    else
    {
        wxVector<wxVariant> value;
        value.push_back("(Everything)");
        value.push_back(wxEmptyString);
        list->AppendItem(value);
        first_filter = false;
        return;
    }
}

void MemSearchPanel::RadioEvent(wxCommandEvent &evt)
{
    switch (current_radio)
    {
        case radio_old_id:
            if (first_filter)
            {
                choice_cmp->Select(previous_choice);
                choice_cmp->Enable(true);
            }
        break;
        case radio_custom_id:
            custom_value->Enable(false);
        break;
    }
    current_radio = evt.GetId();
    switch (current_radio)
    {
        case radio_old_id:
            if (first_filter)
            {
                previous_choice = choice_cmp->GetSelection();
                choice_cmp->Select(0);
                choice_cmp->Enable(false);
            }
        break;
        case radio_custom_id:
            custom_value->Enable(true);
        break;
    }
}


void MemSearchPanel::ItemRClick(wxDataViewEvent &evt)
{
    wxMenu menu;
    wxMenuItem *clear, *undo, *redo;
    clear = new wxMenuItem(&menu, menu_clear, _("Reset"));
    undo = new wxMenuItem(&menu, menu_undo, _("Undo"));
    redo = new wxMenuItem(&menu, menu_redo, _("Redo"));

    // Remove result maybe?
    // Somewhat difficult to implement with current code though..

    menu.Append(undo);
    menu.Append(redo);
    menu.AppendSeparator();
    menu.Append(clear);

    if (!search.CanUndo())
        undo->Enable(false);
    if (!search.CanRedo())
        redo->Enable(false);
    if (first_filter)
        clear->Enable(false);

    menu.Bind(wxEVT_COMMAND_MENU_SELECTED, &MemSearchPanel::RClickEvent, this);
    PopupMenu(&menu);
}

void MemSearchPanel::RClickMenu(wxContextMenuEvent &evt)
{
    wxMenu menu;
    wxMenuItem *clear, *undo, *redo;
    clear = new wxMenuItem(&menu, menu_clear, _("Reset"));
    undo = new wxMenuItem(&menu, menu_undo, _("Undo"));
    redo = new wxMenuItem(&menu, menu_redo, _("Redo"));

    menu.Append(undo);
    menu.Append(redo);
    menu.AppendSeparator();
    menu.Append(clear);

    if (!search.CanUndo())
        undo->Enable(false);
    if (!search.CanRedo())
        redo->Enable(false);
    if (first_filter)
        clear->Enable(false);

    menu.Bind(wxEVT_COMMAND_MENU_SELECTED, &MemSearchPanel::RClickEvent, this);
    PopupMenu(&menu);
}

void MemSearchPanel::RClickEvent(wxCommandEvent &evt)
{
    int id = evt.GetId();
    switch (id)
    {
        case menu_clear:
            search.Clear();
            list->DeleteAllItems();
            choice_valuetype->Enable();
            radio_old->SetLabel(_("Anything"));
            first_filter = true;
        break;
        case menu_undo:
        case menu_redo:
            if (id == menu_undo)
                search.Undo();
            else
                search.Redo();

            switch (choice_valuetype->GetSelection())
            {
                case int8:
                    UpdateList(1, ValueFormat<int8_t, deci>);
                break;
                case uint8:
                    UpdateList(1, ValueFormat<uint8_t, h8>);
                break;
                case int16:
                    UpdateList(2, ValueFormat<int16_t, deci>);
                break;
                case uint16:
                    UpdateList(2, ValueFormat<uint16_t, h16>);
                break;
                case int32:
                    UpdateList(4, ValueFormat<int32_t, deci>);
                break;
                case uint32:
                    UpdateList(4, ValueFormat<uint32_t, h32>);
                break;
                case float32:
                    UpdateList(4, ValueFormat<float, fl>);
                break;
                case int64:
                    UpdateList(4, I64Format);
                break;
                case uint64:
                    UpdateList(4, U64Format);
                break;
                case float64:
                    UpdateList(4, ValueFormat<double, fl>);
                break;
            }
        break;
        default:
        break;
    }
}




