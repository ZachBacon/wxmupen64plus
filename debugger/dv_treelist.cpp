#include "dv_treelist.h"

template class std::unordered_map<void *, dvtlModelItem *>;

dvtlGroup::dvtlGroup(const wxString &name_)
{
    name = name_;
    child_amount = 0;
    child_array_size = 8;
    children = (dvtlModelItem **)malloc(child_array_size * sizeof(dvtlModelItem **));
}

dvtlGroup::~dvtlGroup()
{
    free(children);
}

dvtlModelItem *const *dvtlGroup::GetChildren(int *amt) const
{
    *amt = child_amount;
    return children;
}

int dvtlGroup::AddChild(dvtlModelItem *child)
{
    if (child_amount >= child_array_size)
    {
        child_array_size += 8;
        children = (dvtlModelItem **)realloc(children, child_array_size * sizeof(dvtlModelItem **));
    }
    children[child_amount] = child;
    return child_amount++;
}

void dvtlGroup::RemoveChild(dvtlModelItem *child)
{
    for (int i = 0; i < child_amount; i++)
    {
        if (child == children[i])
        {
            memmove(children + i, children + i + 1, (child_amount - i - 1) * sizeof(dvtlModelItem **));
            child_amount--;
            return;
        }
    }
}

void dvtlGroup::Clear()
{
    child_amount = 0;
}

// ----------------------------------------------------------------------

DataViewTreeListModel::DataViewTreeListModel(int cols_) : root_group("")
{
    cols = cols_;
}

DataViewTreeListModel::~DataViewTreeListModel()
{
    Clear();
}

bool DataViewTreeListModel::IsContainer(const wxDataViewItem &item_) const
{
    dvtlModelItem *item = (dvtlModelItem *)item_.GetID();
    if (item && item->group)
        return true;
    return false;
}

wxDataViewItem DataViewTreeListModel::GetParent(const wxDataViewItem &item_) const
{
    dvtlModelItem *item = (dvtlModelItem *)item_.GetID();
    return wxDataViewItem(item->parent);
}

unsigned int DataViewTreeListModel::GetChildren(const wxDataViewItem &item_, wxDataViewItemArray &children) const
{
    dvtlModelItem *item = (dvtlModelItem *)item_.GetID();

    int count = 0;
    const dvtlGroup *group;

    if (!item)
        group = &root_group;
    else
        group = item->group;

    if (group)
    {
        dvtlModelItem *const *raw_children = group->GetChildren(&count);
        for (int i = 0; i < count; i++)
        {
            children.Add(wxDataViewItem(raw_children[i]));
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
    item->group = 0;
    item->value = value;
    item->parent = parent;
    if (!parent)
        root_group.AddChild(item);

    value_lookup[value] = item;
    wxDataViewItem item_(item);
    ItemAdded(wxDataViewItem(parent), item_);
}

dvtlModelItem *DataViewTreeListModel::FindItem(void *value)
{
    auto it = value_lookup.find(value);
    if (it == value_lookup.end())
        return 0;

    return it->second;
}

void DataViewTreeListModel::RemoveItem(void *value)
{
    auto it = value_lookup.find(value);
    if (it == value_lookup.end())
        return;

    dvtlModelItem *item = it->second;
    if (!item->parent)
        root_group.RemoveChild(item);

    wxDataViewItem item_(item);
    ItemDeleted(wxDataViewItem(item->parent), item_);
    delete item;

    value_lookup.erase(it);
    if (value_lookup.empty()) // Without this weird display lock would happen on windows
        Cleared();
}

void DataViewTreeListModel::Clear()
{
    for (auto it = value_lookup.begin(); it != value_lookup.end(); ++it)
    {
        dvtlModelItem *item = it->second;
        delete item->group;
        delete item;
    }
    root_group.Clear();
    value_lookup.clear();
    Cleared();
}

