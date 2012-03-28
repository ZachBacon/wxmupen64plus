#ifndef DEBUGGER_DV_TREELIST_H
#define DEBUGGER_DV_TREELIST_H

#include <wx/dataview.h>
#include <unordered_map>
// Generic code for implementing custom list control with collapsible groups

struct dvtlModelItem
{
    void *val;
    dvtlModelItem *parent;
    bool isGroup;
};

extern template class std::unordered_map<void *, dvtlModelItem *>;


class dvtlGroup
{
    public:
        dvtlGroup(const wxString &name);
        ~dvtlGroup();

        void *const *GetChildren(int *amt);
        int AddChild(void *child);
        void RemoveChild(void *child);

        wxString name;

    private:
        void **children;
        uint16_t child_amount;
        uint16_t child_array_size;
};

class DataViewTreeListModel : public wxDataViewModel
{
    public:
        DataViewTreeListModel(int cols);
        virtual ~DataViewTreeListModel();
        bool IsContainer(const wxDataViewItem &item) const;
        wxDataViewItem GetParent(const wxDataViewItem &item_) const;
        unsigned int GetChildren(const wxDataViewItem &item_, wxDataViewItemArray &children) const;
        unsigned int GetColumnCount() const;

        void AddItem(void *value, dvtlModelItem *parent = 0);
        void RemoveItem(void *value);
        dvtlModelItem *FindItem(void *value);

    private:
        int cols;

        // I'm not sure about using a hash table here..
        // Another possibility would be having dvtlModelItem to be an inheritable class,
        // which all values must inherit
        // Oh well, simpler api is nice
        std::unordered_map<void *, dvtlModelItem *> value_lookup;
};

#endif // DEBUGGER_DV_TREELIST_H

