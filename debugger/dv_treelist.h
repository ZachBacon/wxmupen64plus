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

#ifndef DEBUGGER_DV_TREELIST_H
#define DEBUGGER_DV_TREELIST_H

#include <wx/dataview.h>
#include <unordered_map>
// Generic code for implementing custom list control with collapsible groups
// It's somewhat hard to understand, maybe templates would be better
// And groups don't likely work (yet)

class dvtlGroup;

struct dvtlModelItem
{
    void *value;
    dvtlModelItem *parent;
    dvtlGroup *group;
};

extern template class std::unordered_map<void *, dvtlModelItem *>;


class dvtlGroup
{
    public:
        dvtlGroup(const wxString &name);
        ~dvtlGroup();

        dvtlModelItem *const *GetChildren(int *amt) const;
        int AddChild(dvtlModelItem *child);
        void RemoveChild(dvtlModelItem *child);

        void Clear();

        wxString name;

    private:
        dvtlModelItem **children;
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
        void Clear();

    protected:
        dvtlGroup root_group;

    private:
        int cols;

        // I'm not sure about using a hash table here..
        // Another possibility would be having dvtlModelItem to be an inheritable class,
        // which all values must inherit
        // Oh well, simpler api is nice
        std::unordered_map<void *, dvtlModelItem *> value_lookup;
};

#endif // DEBUGGER_DV_TREELIST_H

