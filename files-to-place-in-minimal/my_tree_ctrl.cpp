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
#include "VulkanCanvas.h"
#include "VulkanCanvas.h"

wxDECLARE_APP(MyApp);

#include "my_tree_ctrl.h"

static const int NUM_CHILDREN_PER_LEVEL = 5;
static const int NUM_LEVELS = 2;

#if USE_GENERIC_TREECTRL
wxBEGIN_EVENT_TABLE(MyTreeCtrl, wxGenericTreeCtrl)
#else
wxBEGIN_EVENT_TABLE(MyTreeCtrl, wxTreeCtrl)
#endif
EVT_TREE_ITEM_EXPANDED(TreeTest_Ctrl, MyTreeCtrl::OnItemExpanded)
EVT_TREE_ITEM_EXPANDING(TreeTest_Ctrl, MyTreeCtrl::OnItemExpanding)

EVT_TREE_SEL_CHANGED(TreeTest_Ctrl, MyTreeCtrl::OnSelChanged)
EVT_TREE_SEL_CHANGING(TreeTest_Ctrl, MyTreeCtrl::OnSelChanging)

wxEND_EVENT_TABLE()

// MyTreeCtrl implementation
#if USE_GENERIC_TREECTRL
wxIMPLEMENT_DYNAMIC_CLASS(MyTreeCtrl, wxGenericTreeCtrl);
#else
wxIMPLEMENT_DYNAMIC_CLASS(MyTreeCtrl, wxTreeCtrl);
#endif

extern MyFrame* frame;

MyTreeCtrl::MyTreeCtrl(wxWindow* parent, const wxWindowID id,
    const wxPoint& pos, const wxSize& size,
    long style, string rootParentDirPathOnLeftPanel, string rootNameOnly, VulkanCanvas* vulkanCanvas)
    : wxTreeCtrl(parent, id, pos, size, style)
{
    this->vulkanCanvas = vulkanCanvas;
    this->rootNameOnly = rootNameOnly;
    this->rootParentDirPathOnLeftPanel = rootParentDirPathOnLeftPanel;

    curDirPathOnLeftPanel = rootParentDirPathOnLeftPanel;
    curDirPathOnLeftPanel += rootNameOnly;

    wxString newItemText(rootNameOnly);
    wxString newItemFullPath(curDirPathOnLeftPanel);
    MyTreeItemData* newItemData = new MyTreeItemData(newItemFullPath);
    rootId = AddRoot(newItemText, -1, -1, newItemData);

    //recShowAllDir(curDirPathOnLeftPanel, rootId, 0, "");
    OpenDirsFromPath(curDirPathOnLeftPanel, rootId, 1);
    Expand(rootId); // trigger OnItemExpanding() with rootId
}

void MyTreeCtrl::OpenDirsFromPath(string curDirPath, wxTreeItemId curRootID, int depth) {
    MyTreeItemData* itemData = (MyTreeItemData*)GetItemData(curRootID);
    cout << "MyTreeCtrl::OpenDirsFromPath(): itemData: |" << (int)itemData << "|" << endl;
    cout << "MyTreeCtrl::OpenDirsFromPath(): itemData->fullPath: |" << itemData->fullPath.ToStdString() << "|" << endl;

    if (depth == 1 && itemData->grandChildrenLoaded) {
        cout << "MyTreeCtrl::OpenDirsFromPath(): if (itemData->grandChildrenLoaded) {" << endl;
        return;
    }

    wxTreeItemId parentId = this->GetItemParent(curRootID);
    if (parentId.IsOk()) {
        MyTreeItemData* parentItemData = (MyTreeItemData*)GetItemData(this->GetItemParent(curRootID));
        if (parentItemData != NULL) {
            parentItemData->grandChildrenLoaded = true;
        }
    }

    if (itemData->childrenLoaded) {
        wxTreeItemIdValue cookie;
        wxTreeItemId childId = this->GetFirstChild(curRootID, cookie);

        while (childId.IsOk())
        {

            //m_tiiAllTreeViewItems.push_back(child);
            //string sCurrentPath = GetPathOfTreeViewItem(child);
            //wxMessageBox(sCurrentPath);

            //GetAllTreeViewItems(this, child);
            MyTreeItemData* childItemData = (MyTreeItemData*)GetItemData(childId);
            OpenDirsFromPath(childItemData->fullPath.ToStdString(), childId, depth - 1);

            childId = this->GetNextChild(childId, cookie);
        }
        return;
    }

    itemData->childrenLoaded = true;
    /*if (depth) {
        itemData->grandChildrenLoaded = true;
    }*/
    DIR* dir;
    struct dirent* ent;

    dir = opendir(curDirPath.c_str());
    if (dir != NULL) { // dir is not NULL for some reason // fixed
        while ((ent = readdir(dir)) != NULL) {
            if (ent->d_name[0] != '.') {
                //wxTreeItemId curChildID = AppendItem(curRootID, ent->d_name);
                wxString newItemText(ent->d_name);
                wxString newItemFullPath(curDirPath + "/" + ent->d_name);
                MyTreeItemData* newItemData = new MyTreeItemData(newItemFullPath);
                wxTreeItemId curChildID = AppendItem(curRootID, newItemText, -1, -1, newItemData);
                if (ent->d_type == DT_DIR && depth > 0) {
                    //    wxTreeItemId child2Id = tree->AppendItem(rootId, "Node 2");
                    //    tree->AppendItem(child2Id, "Child of node 2");
                    //    tree->AppendItem(rootId, "Node 3");
//                    printf("%sdir: %s\n", tabs.c_str(), ent->d_name);
                    string subDirFullPath = curDirPath;
                    subDirFullPath += "/";
                    subDirFullPath += ent->d_name;

                    OpenDirsFromPath(subDirFullPath, curChildID, depth - 1);
                }
            }
        }
        closedir(dir);
    }
    else {
        //        exit(1);
    }
}

void MyTreeCtrl::recShowAllDir(string curDirPath, wxTreeItemId curRootID, int depth, string tabs) {
    //    std::cout << "MyTreeCtrl::recShowAllDir(" << curDirPath << ")" << std::endl;
    DIR* dir;
    struct dirent* ent;
    //    std::cout << "MyTreeCtrl::recShowAllDir(): curDirPath.c_str(): " <<curDirPath.c_str() << std::endl;

    dir = opendir(curDirPath.c_str());
    if (dir != NULL) {
        int i = 0;
        while ((ent = readdir(dir)) != NULL) {
            if (ent->d_name[0] != '.') {
                wxTreeItemId curChildID = AppendItem(curRootID, ent->d_name);
                if (ent->d_type == DT_DIR) {
                    string subDirFullPath = curDirPath;
                    subDirFullPath += "/";
                    subDirFullPath += ent->d_name;
                    recShowAllDir(subDirFullPath, curChildID, depth + 1, tabs + "    ");
                }
            }
        }
        closedir(dir);
    }
}

void MyTreeCtrl::OnItemExpanded(wxTreeEvent& event) {
    //cout << "MyTreeCtrl::OnItemExpanded(): hello" << endl;
}

void MyTreeCtrl::OnItemExpanding(wxTreeEvent& event) {
    cout << "MyTreeCtrl::OnItemExpanding(): hello" << endl;

    // wxPoint mouse_position = ev.GetPosition();
    /*
        Since expanding is a different event than mouse down, there is no GetPosition()
        Instead of using GetPosition and then getting the label, I can use GetLabel()
        GetLabel() returns wxString so i turn wxPoint mouse_position into wxString label
    */
    wxString label = event.GetLabel();
    cout << "MyTreeCtrl::OnItemExpanding(): label: |" << label.ToStdString() << "|" << endl;
    wxTreeItemId itemId = (wxTreeItemId)event.GetItem();
    cout << "MyTreeCtrl::OnItemExpanding(): itemId: |" << itemId << "|" << endl;

    MyTreeItemData* itemData = (MyTreeItemData*)GetItemData(itemId);
    cout << "MyTreeCtrl::OnItemExpanding(): itemData: |" << (int)itemData << "|" << endl;
    cout << "MyTreeCtrl::OnItemExpanding(): itemData->fullPath: |" << itemData->fullPath.ToStdString() << "|" << endl;

    if (itemData->childrenLoaded && itemData->grandChildrenLoaded) {
        cout << "MyTreeCtrl::OpenDirsFromPath(): if (itemData->grandChildrenLoaded) {" << endl;
        return;
    }
    else {
        OpenDirsFromPath(itemData->fullPath.ToStdString(), itemId, 1);
    }
    
    cout << "MyTreeCtrl::OnItemExpanding(): curDirPathOnLeftPanel" << curDirPathOnLeftPanel << endl;
}

void MyTreeCtrl::OnSelChanged(wxTreeEvent& event) {
    cout << "MyTreeCtrl::OnSelChanged(): hello" << endl;
}

void MyTreeCtrl::OnSelChanging(wxTreeEvent& event) {
    cout << "MyTreeCtrl::OnSelChanging(): hello" << endl;

    wxString label = event.GetLabel();
    wxTreeItemId itemId = (wxTreeItemId)event.GetItem();

    MyTreeItemData* itemData = (MyTreeItemData*)GetItemData(itemId);

    DIR* dir;
    if ((dir = opendir(itemData->fullPath.c_str())) != NULL) {
        // when the fullPath is for a dir then dont open
        closedir(dir);
    }
    else if (itemData->fullPath.substr(itemData->fullPath.size() - 4) == ".obj" || itemData->fullPath.substr(itemData->fullPath.size() - 8) == ".md5mesh") { // it is an object/md5 file so show it as an object
        if (vulkanCanvas->m_objects.size() > 0) {
            vulkanCanvas->ShowObject(itemData->fullPath.ToStdString(), true, true);
        }
    }

    //ev.Skip();
}
