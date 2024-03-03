#ifndef MY_TREE_CTRL
#define MY_TREE_CTRL

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#include "wx/log.h"
#endif

#include "wx/colordlg.h"
#include "wx/numdlg.h"

#include "wx/artprov.h"
#include "wx/image.h"
#include "wx/imaglist.h"
#include "wx/treectrl.h"
#include "wx/math.h"
#include "wx/renderer.h"
#include "wx/wupdlock.h"

#ifdef __WIN32__
    // this is not supported by native control
#define NO_VARIABLE_HEIGHT
#endif

#define USE_GENERIC_TREECTRL 0

#if USE_GENERIC_TREECTRL
#include "wx/generic/treectlg.h"
#ifndef wxTreeCtrl
#define wxTreeCtrl wxGenericTreeCtrl
#define sm_classwxTreeCtrl sm_classwxGenericTreeCtrl
#endif
#endif

class MyTreeItemData : public wxTreeItemData
{
public:
    MyTreeItemData(const wxString& fullPath) : fullPath(fullPath), expanded(false), childrenLoaded(false), grandChildrenLoaded(false) { }

    void ShowInfo(wxTreeCtrl* tree);
    wxString const& GetDesc() const { return fullPath; }

    //private:
public:
    wxString fullPath;
    bool expanded;
    bool childrenLoaded;
    bool grandChildrenLoaded;
};

class MyTreeCtrl : public wxTreeCtrl
{
public:
    enum
    {
        TreeCtrlIcon_File,
        TreeCtrlIcon_FileSelected,
        TreeCtrlIcon_Folder,
        TreeCtrlIcon_FolderSelected,
        TreeCtrlIcon_FolderOpened
    };

    MyTreeCtrl() { }

    MyTreeCtrl(wxWindow* parent, const wxWindowID id,
        const wxPoint& pos, const wxSize& size,
        long style, string rootParentDirPathOnLeftPanel, string rootNameOnly);

    virtual ~MyTreeCtrl() {}

    void OnItemExpanded(wxTreeEvent& event);
    void OnItemExpanding(wxTreeEvent& event);
   
    void OnSelChanged(wxTreeEvent& event);
    void OnSelChanging(wxTreeEvent& event);

    void refreshLowerLeftPanel(wxTreeItemId itemId, bool fromSelectItem);

public:
    string rootNameOnly;
    string rootParentDirPathOnLeftPanel;

    string curDirPathOnLeftPanel;

    wxTreeItemId rootId;

    void OpenDirsFromPath(string curDirPath, wxTreeItemId curRootID, int depth);
    void recShowAllDir(string curDirPath, wxTreeItemId curRootID, int depth, string tabs);

    // NB: due to an ugly wxMSW hack you _must_ use wxDECLARE_DYNAMIC_CLASS();
    //     if you want your overloaded OnCompareItems() to be called.
    //     OTOH, if you don't want it you may omit the next line - this will
    //     make default (alphabetical) sorting much faster under wxMSW.
    wxDECLARE_DYNAMIC_CLASS(MyTreeCtrl);
    wxDECLARE_EVENT_TABLE();
};

#endif
