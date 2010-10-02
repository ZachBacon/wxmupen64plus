#ifndef PARAMETERPANEL_H
#define PARAMETERPANEL_H

#include <wx/sizer.h>
#include <wx/scrolwin.h>
#include <wx/notebook.h>
#include <vector>

#include "mupen64plusplus/MupenAPIpp.h"

class IConfigurationPanel
{
public:
    virtual void commitNewValues() = 0;
    virtual void removeMyselfFrom(wxSizer* parentSizer) = 0;
};

class ParameterPanel : public wxScrolledWindow, public IConfigurationPanel
{
    ConfigSection m_section;
    std::vector<wxWindow*> m_parameter_widgets;
    unsigned long m_magic_number;
    
public:
	ParameterPanel(wxWindow* parent, ConfigSection& section);
	~ParameterPanel();

    virtual void commitNewValues();
    virtual void removeMyselfFrom(wxSizer* parentSizer)
    {
        parentSizer->Detach(this);
        this->Destroy();
    }
};

class ParameterGroupsPanel : public wxNotebook, public IConfigurationPanel
{
    std::vector<ParameterPanel*> m_panels;
    unsigned long m_magic_number;
    
public:

	ParameterGroupsPanel(wxWindow* parent, std::vector<ConfigSection> sections);
    ~ParameterGroupsPanel()
    {
        m_magic_number = 0xDEADBEEF;
    }
    
    virtual void commitNewValues();
    virtual void removeMyselfFrom(wxSizer* parentSizer)
    {
        parentSizer->Detach(this);
        this->Destroy();
    }
};

#endif // PARAMETERPANEL_H
