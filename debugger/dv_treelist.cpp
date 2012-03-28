#include "dv_treelist.h"

template class std::unordered_map<void *, dvtlModelItem *>;

dvtlGroup::dvtlGroup(const wxString &name_)
{
    name = name_;
    child_amount = 0;
    child_array_size = 8;
    children = (void **)malloc(child_array_size * sizeof(void **));
}

dvtlGroup::~dvtlGroup()
{
    free(children);
}

void *const *dvtlGroup::GetChildren(int *amt)
{
    *amt = child_amount;
    return children;
}

int dvtlGroup::AddChild(void *child)
{
    if (child_amount >= child_array_size)
    {
        child_array_size += 8;
        children = (void **)realloc(children, child_array_size * sizeof(void **));
    }
    children[child_array_size] = child;
    return child_array_size++;
}

void dvtlGroup::RemoveChild(void *child)
{
    for (int i = 0; i < child_amount; i++)
    {
        if (child == children[i])
        {
            memcpy(children + i, children + i + 1, child_amount - i - 1);
            child_amount--;
            return;
        }
    }
}

// ----------------------------------------------------------------------

DataViewTreeListModel::DataViewTreeListModel(int cols_)
{
    cols = cols_;
}

DataViewTreeListModel::~DataViewTreeListModel()
{
}

bool DataViewTreeListModel::IsContainer(const wxDataViewItem &item_) const
{
    dvtlModelItem *item = (dvtlModelItem *)item_.GetID();
    if (item && item->isGroup) // This may have null item, or is it a bug?
        return true;
    return false;
}

wxDataViewItem DataViewTreeListModel::GetParent(const wxDataViewItem &item_) const
{
    dvtlModelItem *item = (dvtlModelItem *)item_.GetID();
    return wxDataViewItem((dvtlGroup *)item->parent);
}

unsigned int DataViewTreeListModel::GetChildren(const wxDataViewItem &item_, wxDataViewItemArray &children) const
{
    dvtlModelItem *item = (dvtlModelItem *)item_.GetID();
    int count = 0;
    if (item->isGroup)
    {
        dvtlGroup *group = (dvtlGroup *)(item->val);
        void *const *raw_children = group->GetChildren(&count);
        for (int i = 0; i < count; i++)
        {
            wxDataViewItem new_item(raw_children[i]);
            children.Add(new_item);
        }
    }
    return count;
}

unsigned int DataViewTreeListModel::GetColumnCount() const
{
    return cols;
}

void DataViewTreeListModel::AddItem(void *value, dvtlModelItem *parent)
{
    dvtlModelItem *item = new dvtlModelItem;
    item->isGroup = false;
    item->val = value;
    item->parent = parent;
    value_lookup[value] = item;
    wxDataViewItem item_(item);
    ItemAdded(wxDataViewItem(parent), item_);
}

dvtlModelItem *DataViewTreeListModel::FindItem(void *value)
{
    auto it = value_lookup.find(value);
    if (it == value_lookup.end())
        return 0 ;

    return it->second;
}

void DataViewTreeListModel::RemoveItem(void *value)
{
    auto it = value_lookup.find(value);
    if (it == value_lookup.end())
        return;

    dvtlModelItem *item = it->second;

    wxDataViewItem item_(item);
    ItemDeleted(wxDataViewItem(item->parent), item_);

    value_lookup.erase(it);
}

