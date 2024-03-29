#ifndef MINIMAL_H
#define MINIMAL_H

//#define DBG_MSG_OBJ_FILE_TURN_ON

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"
#include "wx/grid.h"
#include "wx/treectrl.h"
//#include "wx_pch.h"
#include "wx/msgdlg.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

// for all others, include the necessary headers (this file is usually all you
// need because it includes almost all "standard" wxWidgets headers)
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "VulkanWindow.h"
#include "VulkanException.h"

class MyTreeCtrl;
class SceneTreeCtrl;

struct ObjectInfo;

// Define a new application type, each program should derive a class from wxApp
class MyApp : public wxApp
{
public:
    // override base class virtuals
    // ----------------------------

    // this one is called on application startup and is a good place for the app
    // initialization (doing it here and not in the ctor allows to have an error
    // return: if OnInit() returns false, the application terminates)
    virtual bool OnInit() wxOVERRIDE;

    void SetShowImages(bool show) { m_showImages = show; }
    bool ShowImages() const { return m_showImages; }

    void SetShowStates(bool show) { m_showStates = show; }
    bool ShowStates() const { return m_showStates; }

    void SetShowButtons(bool show) { m_showButtons = show; }
    bool ShowButtons() const { return m_showButtons; }

private:
    bool m_showImages, m_showStates, m_showButtons;
};

enum
{
    // menu items
    Minimal_Quit = wxID_EXIT,

    // it is important for the id corresponding to the "About" command to have
    // this standard value as otherwise it won't be handled properly under Mac
    // (where it is special and put into the "Apple" menu)
    Minimal_About = wxID_ABOUT,
    TreeTest_Ctrl = 1000,
    Tree_Obj_Hierarchy = 1001,
    VulkanCanvas_Timer
};

class MyFrame : public wxFrame {
private:
    //--------------------------------------------------------------------------
    // Member variables
    //--------------------------------------------------------------------------
    wxPanel* mainPanel;
    string fullpath;

    //--------------------------------------------------------------------------
    // Top Left Panel
    //--------------------------------------------------------------------------
    wxPanel* treePanel;
    MyTreeCtrl* tree;
    MyTreeCtrl* tree1;

    //--------------------------------------------------------------------------
    // Bottom Left Panel
    //--------------------------------------------------------------------------
    wxPanel* fileListPanel;

    wxGrid* grid;

    //--------------------------------------------------------------------------
    // Middle Panel
    //--------------------------------------------------------------------------
    wxPanel* contentsPanel;
    VulkanWindow* vkWindow;
    VulkanCanvas* vulkanCanvas;

    int indexOfObjBeingDragged;

    glm::vec2 startPosForMouseDrag;
    glm::vec2 startPosForRightMouseDrag;

    boolean leftMouseOnVulkan;
    boolean rightMouseOnVulkan;

    //--------------------------------------------------------------------------
    // Upper Right Panel
    //--------------------------------------------------------------------------
    wxPanel* objectInfoPanel;

    wxTextCtrl* txtCtrlPositionX;
    wxTextCtrl* txtCtrlPositionY;
    wxTextCtrl* txtCtrlPositionZ;
    wxStaticText* txtLblPositionX;
    wxStaticText* txtLblPositionY;
    wxStaticText* txtLblPositionZ;

    wxTextCtrl* txtCtrlCameraPosX;
    wxTextCtrl* txtCtrlCameraPosY;
    wxTextCtrl* txtCtrlCameraPosZ;
    wxStaticText* txtLblCameraPosX;
    wxStaticText* txtLblCameraPosY;
    wxStaticText* txtLblCameraPosZ;

    wxTextCtrl* txtCtrlCameraLookAtX;
    wxTextCtrl* txtCtrlCameraLookAtY;
    wxTextCtrl* txtCtrlCameraLookAtZ;
    wxStaticText* txtLblCameraLookAtX;
    wxStaticText* txtLblCameraLookAtY;
    wxStaticText* txtLblCameraLookAtZ;

    wxStaticText* txtLblCurObject;
    wxStaticText* txtLblCurCameraTarget;
    wxStaticText* txtLblCurCameraPos;

    wxCheckBox* hideObjectCheckBox;
    wxButton* hideObjectButton;
    wxButton* showObjectButton;

    //--------------------------------------------------------------------------
    // Lower Right Panel
    //--------------------------------------------------------------------------
    wxPanel* objHierarchyPanel;

    //--------------------------------------------------------------------------
    // Vulkan Canvas
    //--------------------------------------------------------------------------
    wxTimer* m_timer;

public:
    SceneTreeCtrl* objHierarchy;

    // constructor(s)
    MyFrame(const wxString& title); // constructor
    virtual ~MyFrame();

    // event handlers (these functions should _not_ be virtual)
    void OnQuit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);

    bool makeLowerLeftPanel(wxString fullPath);

    void OnGridLeftClick(wxGridEvent& ev);
    void OnGridRightClick(wxGridEvent& ev);
    void OnMouseLeftClick(wxMouseEvent& ev);
    void OnMouseRightClick(wxMouseEvent& ev);
    void OnMouseLeftClickEnd(wxMouseEvent& ev);
    void OnMouseRightClickEnd(wxMouseEvent& ev);
    void OnMouseMotion(wxMouseEvent& ev);
    void OnPopupClick(wxCommandEvent& ev);
    void OnMouseWheel(wxMouseEvent& ev);
    void OnHideLeftClick(wxCommandEvent& event);
    void OnShowLeftClick(wxCommandEvent& event);

    void OnItemExpanding(wxTreeEvent& event);

    void getAllInfoFromFolder(vector<string>& names, vector<bool>& isFolder, string path, int* totalChunks, int* totalFiles, int* totalFolders, long* totalSize);

    void OnVkTimer(wxTimerEvent& event);


private:
    // any class wishing to process wxWidgets events must use this macro
    wxDECLARE_EVENT_TABLE();
};

#endif
