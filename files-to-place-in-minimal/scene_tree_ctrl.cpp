#define _HAS_STD_BYTE 0

#include <iostream>
#include <string>
#include <dirent.h>
#include <sys/stat.h>

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

#include "wx/app.h"

#ifdef __WIN32__
    // this is not supported by native control
#define NO_VARIABLE_HEIGHT
#endif

#include "minimal.h"

wxDECLARE_APP(MyApp);

#include "scene_tree_ctrl.h"

static const int NUM_CHILDREN_PER_LEVEL = 5;
static const int NUM_LEVELS = 2;

#if USE_GENERIC_TREECTRL
wxBEGIN_EVENT_TABLE(SceneTreeCtrl, wxGenericTreeCtrl)
#else
wxBEGIN_EVENT_TABLE(SceneTreeCtrl, wxTreeCtrl)
#endif

EVT_TREE_SEL_CHANGING(Tree_Obj_Hierarchy, SceneTreeCtrl::OnSelChanging)

wxEND_EVENT_TABLE()

// SceneTreeCtrl implementation
#if USE_GENERIC_TREECTRL
wxIMPLEMENT_DYNAMIC_CLASS(SceneTreeCtrl, wxGenericTreeCtrl);
#else
wxIMPLEMENT_DYNAMIC_CLASS(SceneTreeCtrl, wxTreeCtrl);
#endif

extern MyFrame* frame;

SceneTreeCtrl::SceneTreeCtrl() {
}

SceneTreeCtrl::SceneTreeCtrl(wxWindow* parent, const wxWindowID id,
    const wxPoint& pos, const wxSize& size,
    long style, string rootName)
    : wxTreeCtrl(parent, id, pos, size, style)
{
    if (id == 1001) {
        wxString newItemText(rootName);

        SceneTreeItemData* itemData = new SceneTreeItemData(newItemText, -1);
        rootId = AddRoot(newItemText, -1, -1, itemData);

        Expand(rootId); // trigger OnItemExpanding() with rootId
    }

    selectedObject = -1;
    prevSelectedObject = -1;
}

void SceneTreeCtrl::addObjItem(string objItemName, int id) {
    cout << "SceneTreeCtrl::addObjItem(): objItemName = " << objItemName << endl;
    wxString newItemFullPath(objItemName);
    SceneTreeItemData* newItemData = new SceneTreeItemData(newItemFullPath, id);
    wxTreeItemId curChildID = AppendItem(rootId, newItemFullPath, -1, -1, newItemData);
}

void SceneTreeCtrl::OnSelChanging(wxTreeEvent& event) {
    wxString label = event.GetLabel();
    wxTreeItemId itemId = (wxTreeItemId)event.GetItem();

    SceneTreeItemData* itemData = (SceneTreeItemData*)GetItemData(itemId);

    int objID = itemData->id;

    prevSelectedObject = selectedObject;
    selectedObject = objID;

    //cout << "SceneTreeCtrl::OnSelChanging(): selectedObject: " << selectedObject << endl;
}
