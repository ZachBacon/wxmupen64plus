/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   wxMupen64Plus frontend                                                *
 *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
 *   Copyright (C) 2010 Marianne Gagnon, based on work by Richard Goedeken *
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
 
#ifndef __OWNER_PTR__
#define __OWNER_PTR__

template<typename T>
class OwnerPtr
{
public:
    bool owner;
    T* raw_ptr;
    
    OwnerPtr()
    {
        raw_ptr = NULL;
        owner = true;
    }
    
    OwnerPtr(OwnerPtr<T>& ptr)
    {
        if (owner) delete raw_ptr;
        raw_ptr = ptr;
        ptr.owner = false; // prevent the older instance from deleting the pointer
    }
    
    OwnerPtr(T* obj)
    {
        raw_ptr = obj;
        owner = true;
    }
    
    ~OwnerPtr()
    {
        if (owner) delete raw_ptr;
#ifndef NDEBUG
        raw_ptr = (T*)0xDEADBEEF;
#endif
    }
    
    OwnerPtr& operator=(T* ptr)
    {
        if (owner) delete raw_ptr;
        raw_ptr = ptr;
        return *this;
    }
    OwnerPtr& operator=(OwnerPtr& other)
    {
        if (owner) delete raw_ptr;
        raw_ptr = other.raw_ptr;
        other.owner = false;
        return *this;
    }
    
    operator T*()
    { 
        return raw_ptr; 
    }
    
    operator const T*() const
    {
        return raw_ptr; 
    }
    
    T* operator->() const
    {
        return raw_ptr;
    }
    
};
template<typename T>
class WxOwnerPtr
{
public:
    T* raw_ptr;
    
    WxOwnerPtr()
    {
        raw_ptr = NULL;
    }
    WxOwnerPtr(T* obj)
    {
        raw_ptr = obj;
    }
    
    ~WxOwnerPtr()
    {
        if (raw_ptr != NULL) raw_ptr->Destroy();
    }
    
    WxOwnerPtr& operator=(T* ptr)
    {
        if (raw_ptr != NULL) raw_ptr->Destroy();
        
        raw_ptr = ptr;
        return *this;
    }
    
    operator T*()
    { 
        return raw_ptr; 
    }
    
    T* operator->() const
    {
        return raw_ptr;
    }
    
};

#endif
