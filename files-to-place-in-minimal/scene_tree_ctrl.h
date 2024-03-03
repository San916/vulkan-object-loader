#pragma once
#ifndef SCENE_TREE_CTRL_H
#define SCENE_TREE_CTRL_H

using namespace std;
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

class SceneTreeItemData : public wxTreeItemData
{
public:
    SceneTreeItemData(const wxString& gameObjName, int id) : gameObjName(gameObjName), id(id), expanded(false) { }
    wxString const& GetDesc() const { return gameObjName; }

public:
    int id;
    wxString gameObjName;
    bool expanded;
};

class SceneTreeCtrl : public wxTreeCtrl
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

    SceneTreeCtrl();

    SceneTreeCtrl(wxWindow* parent, const wxWindowID id,
        const wxPoint& pos, const wxSize& size,
        long style, string rootName);

    virtual ~SceneTreeCtrl() {}

    void addObjItem(string objItemName, int id);

protected:

private:

public:
    string rootNameOnly; 
    string rootParentDirPathOnLeftPanel;

    string curDirPathOnLeftPanel;

    wxTreeItemId rootId;

    int selectedObject;
    int prevSelectedObject;

    void OnSelChanging(wxTreeEvent& event);

    // NB: due to an ugly wxMSW hack you _must_ use wxDECLARE_DYNAMIC_CLASS();
    //     if you want your overloaded OnCompareItems() to be called.
    //     OTOH, if you don't want it you may omit the next line - this will
    //     make default (alphabetical) sorting much faster under wxMSW.
    wxDECLARE_DYNAMIC_CLASS(SceneTreeCtrl);
    wxDECLARE_EVENT_TABLE();
};

#endif
