/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   wxMupen64Plus frontend                                                *
 *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
 *   Copyright (C) 2010 Marianne Gagnon                                    *
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

#include "gamespanel.h"
#include "mupen64plusplus/MupenAPIpp.h"

#include "sdlkeypicker.h"
#include <wx/sizer.h>
#include <wx/filepicker.h>
#include <wx/listctrl.h>
#include <wx/dir.h>
#include <wx/stdpaths.h>
#include <wx/bmpbuttn.h>
#include <wx/msgdlg.h>
#include <wx/log.h>
#include <wx/msgqueue.h>
#include <wx/progdlg.h>

#include <stdexcept>
#include <map>

// ---------------------------------------------- WORKER THREAD ----------------------------------------------
// Getting ROM info like internal name, etc. is a slow operation, so it's performed in
// a worker thread

const int ROM_INFO_READ_ID = 100000;

struct ScheduledOpen
{
    wxString m_file;
    long m_table_id;
};

struct CompletedOpen
{
    Mupen64PlusPlus::RomInfo rominfo;
    wxString rompath;
    int listid;
};

/** List of tasks for the worker thread to perform */
wxMessageQueue<ScheduledOpen> workerThreadData;

/** Queue of tasks completed by the worker thread; used to pass data from working thread to GUI thread */
wxMessageQueue<CompletedOpen> workerThreadResults;

/** The current instance of the thread, always stop a previous thread before creating a new one.
 *  When a thread end, it will automatically set this pointer to NULL
 */
class WorkerThread;
WorkerThread* workerThread = NULL;

int threadCount = 0;

// so that we can wait until a thread is deleted
wxMutex conditionBackend;
wxCondition threadDeleted(conditionBackend);

wxMutex threadDeleteMutex;

/** The worker thread class */
class WorkerThread : public wxThread
{
    Mupen64PlusPlus* m_api;
    GamesPanel* m_parent;
    
public:

    WorkerThread(GamesPanel* parent, Mupen64PlusPlus* api)
    {
        m_api = api;
        m_parent = parent;
        threadCount++;
        
        assert(threadCount <= 1);
    }

    virtual ExitCode Entry()
    {
        ScheduledOpen task;
        Mupen64PlusPlus::RomInfo info;
        
        while (!TestDestroy())
        {
            assert(threadCount <= 1);
            
            wxMessageQueueError result = workerThreadData.Receive(task);
                
            if (result != wxMSGQUEUE_NO_ERROR)
            {
                wxLogWarning("Problem communicating between threads, the ROM list may not load fully");
                break;
            }
        
            // ID -1 is the end of the task queue
            if (task.m_table_id == -1)
            {
                return 0;
            }
            
            // FIXME: accesses to m_api needs to be synchronized with main thread
            try
            {
                m_api->loadRom(task.m_file, false);
            }
            catch (std::runtime_error& e)
            {
                wxLogWarning("Can't get info about %s", (const char*)task.m_file.utf8_str());
                continue;
            }
            
            try
            {
                info = m_api->getRomInfo();
            }
            catch (std::runtime_error& e)
            {
                wxLogWarning("Can't get info about %s", (const char*)task.m_file.utf8_str());
            }
            
            try
            {
                m_api->closeRom(false);
            }
            catch (std::runtime_error& e)
            {
                wxLogWarning("Can't free rom %s", (const char*)task.m_file.utf8_str());
            }
            
            CompletedOpen msg;
            msg.listid = task.m_table_id;
            msg.rompath = task.m_file;
            msg.rominfo = info;
            workerThreadResults.Post(msg);
            
            wxCommandEvent event( wxEVT_COMMAND_TEXT_UPDATED, ROM_INFO_READ_ID );
            m_parent->GetEventHandler()->AddPendingEvent( event );
        }
        return 0;
    }
    
    ~WorkerThread()
    {
        wxMutexLocker mutex(threadDeleteMutex);
        
        workerThread = NULL;
        threadCount--;
        assert(threadCount <= 1);
        threadDeleted.Signal();
    }
    
    void waitDelete()
    {
        {
            wxMutexLocker mutex(threadDeleteMutex);
            wxThread::Delete();
        }
        threadDeleted.Wait();
    }
};

/** Only to be called from the main thrad. Stops the worker thread */
void killThread()
{
    assert(threadCount <= 1);
    
    if (workerThread != NULL)
    {
        workerThread->waitDelete();
        workerThread = NULL;
    }
    
    assert(threadCount <= 1);
}

/** Only to be called from the main thrad. Starts the worker thread */
void spawnThread(GamesPanel* parent, Mupen64PlusPlus* api)
{
    assert(threadCount <= 1);
    
    killThread();
    
    workerThread = new WorkerThread(parent, api);
    workerThread->Create();
    workerThread->Run();
    
    assert(threadCount <= 1);
}

// -----------------------------------------------------------------------------------------------------------
// ----------------------------------------------- GAMES CACHE -----------------------------------------------
// Info like game internal name, CRC, etc. is slow to calculate and should be remembered, hence this small
// cache
std::map<wxString, Mupen64PlusPlus::RomInfo> g_cache;

// -----------------------------------------------------------------------------------------------------------
// ----------------------------------------------- GAMES PANEL -----------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// The main part where the game list is shown

BEGIN_EVENT_TABLE(GamesPanel, wxPanel)
EVT_COMMAND  (ROM_INFO_READ_ID, wxEVT_COMMAND_TEXT_UPDATED, GamesPanel::onRomInfoReady)
END_EVENT_TABLE()
 

GamesPanel::GamesPanel(wxWindow* parent, Mupen64PlusPlus* api, ConfigParam gamesPathParam) :
        wxPanel(parent, wxID_ANY), m_gamesPathParam(gamesPathParam)
{
    m_api = api;
    
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    
    wxString path;
    //if (m_gamesPathParam != NULL)
    {
        try
        {
            path = m_gamesPathParam.getStringValue();
        }
        catch (std::runtime_error& ex)
        {
            wxLogWarning("Failed to read ROMs path from config file : %s", ex.what());
        }
    }
    
    m_dir_picker = new wxDirPickerCtrl(this, wxID_ANY, path);
    sizer->Add(m_dir_picker, 0, wxALL | wxEXPAND, 5);

    m_dir_picker->Connect(m_dir_picker->GetId(), wxEVT_COMMAND_DIRPICKER_CHANGED,
                          wxFileDirPickerEventHandler(GamesPanel::onPathChange), NULL, this);
    
    m_item_list = new wxListCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                                 wxLC_REPORT | wxLC_SINGLE_SEL | wxLC_HRULES);
    sizer->Add(m_item_list, 1, wxALL | wxEXPAND, 5);
    
    populateList();
    
    wxBoxSizer* buttons = new wxBoxSizer(wxHORIZONTAL);
    
#ifdef DATADIR
    wxString datadir = wxString(DATADIR) + wxFileName::GetPathSeparator();
#else
    wxString datadir = wxStandardPaths::Get().GetResourcesDir() + wxFileName::GetPathSeparator();
#endif

    wxBitmap icon_play(datadir + "play.png", wxBITMAP_TYPE_PNG);  
    wxBitmapButton* playBtn = new wxBitmapButton(this, wxID_ANY, icon_play, wxDefaultPosition, wxDefaultSize,
                                                 wxBORDER_NONE);
    buttons->Add(playBtn, 0, wxALL, 5);
    
    sizer->Add(buttons, 0, wxALL, 5);
    
    playBtn->Connect(playBtn->GetId(), wxEVT_COMMAND_BUTTON_CLICKED,
                    wxCommandEventHandler(GamesPanel::onPlay), NULL, this);
    
    SetSizer(sizer);
}

// -----------------------------------------------------------------------------------------------------------

void GamesPanel::populateList()
{
    // stop any previous working thread
    killThread();
    
    wxString path = m_dir_picker->GetPath();
    m_item_list->ClearAll();
        
    if (path.IsEmpty()) return;
    
    wxArrayString columns;              std::vector<int> sizes;
    columns.Add( _("File Name") );      sizes.push_back( 300 );
    columns.Add( _("Internal Name") );  sizes.push_back( 225 );
    columns.Add( _("Good Name") );      sizes.push_back( 225 );
    columns.Add( _("Country") );        sizes.push_back( 75 );
    //columns.Add( _("Size") );        sizes.push_back( 75 );
    
    const int count = columns.Count();
    for (int n=0; n<count; n++)
    {
        wxListItem col;
        col.SetId(n);
        col.SetText( columns[n] );
        col.SetWidth( sizes[n] );
        m_item_list->InsertColumn(n, col);
    }
    
    std::vector<RomInfo> roms = getRomsInDir(path);
    
    int romsNotInCache = 0;
    
    // We will re-fill the queue if needed
    workerThreadData.Clear();
    workerThreadResults.Clear();
    
    const int item_amount = roms.size();
    for (int n=0; n<item_amount; n++)
    {
        RomInfo& curritem = roms[n];
                
        wxListItem item;
        item.SetId(n);
        item.SetText( curritem.m_file_name );
        
        Mupen64PlusPlus::RomInfo info;
        
        long id = m_item_list->InsertItem( item );
        
        if (id != -1)
        {
            std::map<wxString, Mupen64PlusPlus::RomInfo>::iterator elem = g_cache.find(curritem.m_full_path);
            if (elem != g_cache.end())
            {
                // Element found in cache, great
                info = g_cache[curritem.m_full_path];
            }
            else
            {
                // The item is currently not in cache, schedule to retrieve it
                ScheduledOpen task;
                task.m_file = curritem.m_full_path;
                task.m_table_id = id;
                workerThreadData.Post(task);
                romsNotInCache++;
                
                info.country = "...";
                info.goodname = "...";
                info.name = "...";
                
                /*
                // TODO: use this faster technique to speed up getting ROM info
                try
                {
                    info = m_api->getRomInfo(curritem.m_full_path.utf8_str());
                }
                catch (std::runtime_error& ex)
                {
                    fprintf(stderr, "Failed to load rom %s : %s",
                                    (const char*)curritem.m_full_path.utf8_str(),
                                    ex.what());
                }
                printf( "ROM name : '%s'\n", (const char*)info.name.utf8_str());
                printf("ROM country : '%s'\n", (const char*)info.country.utf8_str());
                 */
            }
        
            // set value in first column
            m_item_list->SetItem(id, 0, curritem.m_file_name);
            
            // set value in second column
            m_item_list->SetItem(id, 1, info.name);
            
            // set value in third column
            m_item_list->SetItem(id, 2, info.goodname);
            
            // set value in fourth column
            m_item_list->SetItem(id, 3, info.country);
        }
    } // end for
    
    if (romsNotInCache > 0)
    {
        // Send end of work message
        ScheduledOpen task;
        task.m_table_id = -1;
        workerThreadData.Post(task);
        
        spawnThread(this, m_api);
    }
}

// -----------------------------------------------------------------------------------------------------------

GamesPanel::~GamesPanel()
{
    killThread();
}

// -----------------------------------------------------------------------------------------------------------

void GamesPanel::onRomInfoReady(wxCommandEvent& evt)
{
    CompletedOpen msg;
    wxMessageQueueError result = workerThreadResults.ReceiveTimeout( 10 /* max 10 milliseconds */, msg);
    
    if (result == wxMSGQUEUE_NO_ERROR)
    {
        g_cache[msg.rompath] = msg.rominfo;
        
        const int id = msg.listid;

        // set value in second column
        m_item_list->SetItem(id, 1, msg.rominfo.name);
        
        // set value in third column
        m_item_list->SetItem(id, 2, msg.rominfo.goodname);
        
        // set value in fourth column
        m_item_list->SetItem(id, 3, msg.rominfo.country);
    }
    else
    {
        wxLogWarning("Problem communicating between threads, ROM list may not load fully");
    }
}

// -----------------------------------------------------------------------------------------------------------

std::vector<GamesPanel::RomInfo> GamesPanel::getRomsInDir(wxString dirpath)
{
    std::vector<RomInfo> out;
    
    wxArrayString children;
    // wxDIR_FILES is used to avoid very long processing (e.g. searching the entire disk if path is set to /...)
    // TODO: allow limited recursion into directories?
    const int count = wxDir::GetAllFiles(dirpath, &children, wxEmptyString, wxDIR_FILES);
    
    assert(count == (int)children.Count());
    
    for (int n=0; n<count; n++)
    {
        wxString filename = wxFileName::FileName(children[n]).GetFullName();
        
        // roms typically end with "*.n64|*.z64|*.v64"
        if (filename.EndsWith("64") && !filename.StartsWith("."))
        {
            out.push_back( RomInfo(children[n],
                                   filename) );
        }
    }
    
    return out;
}

// -----------------------------------------------------------------------------------------------------------

void GamesPanel::onPathChange(wxFileDirPickerEvent& event)
{
    killThread();
    
    try
    {
        m_gamesPathParam.setStringValue(m_dir_picker->GetPath().ToStdString());
    }
    catch (std::runtime_error& ex)
    {
        wxLogWarning("Failed to save ROM path to config file : %s", ex.what());
    }

    populateList();
}

// -----------------------------------------------------------------------------------------------------------

void GamesPanel::onPlay(wxCommandEvent& evt)
{
    killThread();
        
    wxString path = m_dir_picker->GetPath();        
    if (path.IsEmpty())
    {
        wxBell();
        return;
    }
    
    wxProgressDialog dialog( _("Loading..."), _("Your game is loading") );
    dialog.Show();
    
    long item = m_item_list->GetNextItem(-1,
                                        wxLIST_NEXT_ALL,
                                        wxLIST_STATE_SELECTED);
    wxString file = path + wxFileName::GetPathSeparator() + m_item_list->GetItemText(item);
    printf("\n==== Running file '%s' ====\n\n", (const char*)file.utf8_str());

    try
    {
        m_api->loadRom(file, true, &dialog);
        dialog.Hide();
        m_api->runEmulation();
    }
    catch (std::runtime_error& ex)
    {
        wxMessageBox(ex.what());
    }
}

// -----------------------------------------------------------------------------------------------------------
