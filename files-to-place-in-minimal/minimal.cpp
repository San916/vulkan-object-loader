/////////////////////////////////////////////////////////////////////////////
// Name:        minimal.cpp
// Purpose:     Minimal wxWidgets sample
// Author:      Julian Smart
// Modified by:
// Created:     04/01/98
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////
#define _HAS_STD_BYTE 0

//#pragma pack(1)

// ============================================================================
// declarations
// ============================================================================

// Place the name of the folder you wish to be at the root of the file tree
// e.g "minimal"
#define ROOT_HOME_FOLDER_NAME ""
// Place the name of the directory up to ROOT_HOME_FOLDER_NAME 
// e.g /Users/username/Documents/vulkan_app/wxWidgets-3.2.4/samples/
#define ROOT_DIR_NAME "/Users/"


// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------
//#define WX_FRAME_WIDTH 1024
#define WX_FRAME_WIDTH (1424 + 16) // + 16 and + 82 to account for elements in the wxWidgets window that take up the 1424 x 768 pixels
#define WX_FRAME_HEIGHT (768 + 82)
#define WX_PANEL_WIDTH 1424 // Use this number to actually allocate space in the wxWidgets window
#define WX_PANEL_HEIGHT 768

#define FRAME_UPPER_H (WX_PANEL_HEIGHT/2)

//------------------------------------------------
// File Tree Panel(Top Left Panel)
//------------------------------------------------
#define TREE_PANEL_W (WX_PANEL_WIDTH / 4)
#define TREE_PANEL_H FRAME_UPPER_H

#define TREE_PANEL_X 0
#define TREE_PANEL_Y 0

//------------------------------------------------
// File List Panel(Bottom Left Panel)
//------------------------------------------------
#define FILE_LIST_PANEL_W TREE_PANEL_W
#define FILE_LIST_PANEL_H FRAME_UPPER_H

#define FILE_LIST_PANEL_X 0
#define FILE_LIST_PANEL_Y FRAME_UPPER_H

//------------------------------------------------
// Vulkan Panel (Middle Panel)
//------------------------------------------------
#define VULKAN_PANEL_W (WX_PANEL_WIDTH / 2)
#define VULKAN_PANEL_H WX_PANEL_HEIGHT

#define VULKAN_PANEL_X TREE_PANEL_W
#define VULKAN_PANEL_Y 0

#define VULKAN_PANEL_VULKAN_CANVAS_START_LOCAL_X 0
#define VULKAN_PANEL_VULKAN_CANVAS_START_LOCAL_Y 0

#define VULKAN_PANEL_VULKAN_CANVAS_W VULKAN_PANEL_W
#define VULKAN_PANEL_VULKAN_CANVAS_H VULKAN_PANEL_H

#define NUM_STARTING_OBJECTS 1

//------------------------------------------------
// Button Panel(Upper Right Panel)
//------------------------------------------------
#define BUTTON_LIST_PANEL_W (WX_PANEL_WIDTH / 4)
#define BUTTON_LIST_PANEL_H (WX_PANEL_HEIGHT / 2)

#define BUTTON_LIST_PANEL_X (3 * (WX_PANEL_WIDTH / 4))
#define BUTTON_LIST_PANEL_Y 0

//------------------------------------------------
// Obj Hierarchy Panel(Lower Right Panel)
//------------------------------------------------
#define OBJ_HIERARCHY_PANEL_W (WX_PANEL_WIDTH / 4)
#define OBJ_HIERARCHY_PANEL_H (WX_PANEL_HEIGHT / 2)

#define OBJ_HIERARCHY_PANEL_X (3 * (WX_PANEL_WIDTH / 4))
#define OBJ_HIERARCHY_PANEL_Y BUTTON_LIST_PANEL_H

#define _HAS_STD_BYTE 0

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

//#include "wxVulkanTutorialApp.h"
#include "VulkanWindow.h"
#include "VulkanException.h"

#pragma warning(disable: 28251)

#ifdef _UNICODE
#ifdef _DEBUG
#pragma comment(lib, "wxbase32ud.lib")
#else
#pragma comment(lib, "wxbase32u.lib")
#endif
#else
#ifdef _DEBUG
#pragma comment(lib, "wxbase32d.lib")
#else
#pragma comment(lib, "wxbase32.lib")
#endif
#endif

// for directory (files)
#include <iostream>
#include <iomanip>
#include <string>
#include <dirent.h>
#include <sys/stat.h>
#include <fstream>
#include <sstream>

using namespace std;

#include "my_object.h"

// This is for the obj loader for Vulkan
#define CUBE_OBJ_FILENAME "assets/cube/cube.obj"

#define GLM_ENABLE_EXPERIMENTAL

// ----------------------------------------------------------------------------
// resources
// ----------------------------------------------------------------------------

// the application icon (under Windows it is in resources and even
// though we could still include the XPM here it would be unused)
#ifndef wxHAS_IMAGES_IN_RESOURCES
    #include "../sample.xpm"
#endif

string converter(uint8_t *str);

// ----------------------------------------------------------------------------
// private classes
// ----------------------------------------------------------------------------
wxImage *cellImageFolder = NULL;
wxImage *cellImageFile = NULL;
wxBitmap *cellBitmapFolder = NULL;
wxBitmap *cellBitmapFile = NULL;

class FileGridCellRenderer : public wxGridCellStringRenderer
{
public:
    virtual void Draw(wxGrid& grid, wxGridCellAttr& attr, wxDC& dc, const wxRect& rect, int row, int col, bool isSelected);
};

void FileGridCellRenderer::Draw(wxGrid& grid, wxGridCellAttr& attr, wxDC& dc, const wxRect& rect, int row, int col, bool isSelected)
{
    dc.DrawBitmap(*cellBitmapFile, rect.x+1, rect.y+1);
}

class FolderGridCellRenderer : public wxGridCellStringRenderer
{
public:
    virtual void Draw(wxGrid& grid, wxGridCellAttr& attr, wxDC& dc, const wxRect& rect, int row, int col, bool isSelected);
};

void FolderGridCellRenderer::Draw(wxGrid& grid, wxGridCellAttr& attr, wxDC& dc, const wxRect& rect, int row, int col, bool isSelected)
{
    dc.DrawBitmap(*cellBitmapFolder, rect.x+2, rect.y+2);
}

#include "minimal.h"
#include "my_tree_ctrl.h"
#include "scene_tree_ctrl.h"

//#include "hello_triangle_application.h"

wxDECLARE_APP(MyApp);

// MyFrame extends wxFrame. It is a child class
// wxFrame has the base of a GUI application
// Define a new frame type: this is going to be our main frame
int gridRowAmount = 0;

MyFrame *frame = NULL;

void MyFrame::OnItemExpanding(wxTreeEvent& event) {

}

// ----------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------

// IDs for the controls and the menu commands

const int ID_BUTTON_HIDE_OBJECT = 100;
const int ID_BUTTON_SHOW_OBJECT = 101;

// ----------------------------------------------------------------------------
// event tables and other macros for wxWidgets
// ----------------------------------------------------------------------------

// the event tables connect the wxWidgets events with the functions (event
// handlers) which process them. It can be also done at run-time, but for the
// simple menu events like this the static method is much simpler.
wxBEGIN_EVENT_TABLE(MyFrame, wxFrame)
    EVT_MENU(Minimal_Quit,  MyFrame::OnQuit)
    EVT_MENU(Minimal_About, MyFrame::OnAbout)
    EVT_GRID_CELL_LEFT_CLICK(MyFrame::OnGridLeftClick)
    EVT_GRID_CELL_RIGHT_CLICK(MyFrame::OnGridRightClick)
    EVT_LEFT_DOWN(MyFrame::OnMouseLeftClick)
    EVT_TIMER(VulkanCanvas_Timer, MyFrame::OnVkTimer)
    EVT_BUTTON(ID_BUTTON_HIDE_OBJECT, MyFrame::OnHideLeftClick)
    EVT_BUTTON(ID_BUTTON_SHOW_OBJECT, MyFrame::OnShowLeftClick)
    //EVT_KEY_DOWN(MyFrame::OnKeyPressed)
wxEND_EVENT_TABLE()

// Create a new application object: this macro will allow wxWidgets to create
// the application object during program execution (it's better than using a
// static object for many reasons) and also implements the accessor function
// wxGetApp() which will return the reference of the right type (i.e. MyApp and
// not wxApp)
wxIMPLEMENT_APP(MyApp);

//TREE_EVENT_HANDLER(OnItemExpanding)

// ============================================================================
// implementation
// ============================================================================

// ----------------------------------------------------------------------------
// the application class
// ----------------------------------------------------------------------------

// 'Main program' equivalent: the program execution "starts" here
bool MyApp::OnInit()
{
#ifdef WIN32
    AllocConsole();
    freopen("CONOUT$", "w", stdout);
#endif

    // call the base class initialization method, currently it only parses a
    // few common command-line options but it could be do more in the future
    if ( !wxApp::OnInit() )
        return false;

    // create the main application window
    frame = new MyFrame("Minimal wxWidgets App");
    frame->Show(true);

    return true;
}

MyFrame *thisFrame;

/**
 * Get the size of a file.
 * @return The filesize, or 0 if the file does not exist.
 */
size_t getFileSize(const char* filename) {
    struct stat st;
    if (stat(filename, &st) != 0) {
        return 0;
    }
    return st.st_size;
}

// ----------------------------------------------------------------------------
// main frame
// ----------------------------------------------------------------------------
// the constructor of a child class needs to call the parents constructor
// frame constructor
MyFrame::MyFrame(const wxString& title) : wxFrame(NULL, wxID_ANY, title, wxDefaultPosition, wxSize(WX_FRAME_WIDTH, WX_FRAME_HEIGHT)) {
    thisFrame = this;
    
    //SetSizerAndFit(NULL);
    //Layout();
    
    // set the frame icon
    SetIcon(wxICON(sample));

#if wxUSE_MENUS
    // create a menu bar
    wxMenu *fileMenu = new wxMenu;

    // the "About" item should be in the help menu
    wxMenu *helpMenu = new wxMenu;
    helpMenu->Append(Minimal_About, "&About\tF1", "Show about dialog");

    fileMenu->Append(Minimal_Quit, "E&xit\tAlt-X", "Quit this program");

    // now append the freshly created menu to the menu bar...
    wxMenuBar *menuBar = new wxMenuBar();
    menuBar->Append(fileMenu, "&File");
    menuBar->Append(helpMenu, "&Help");

    // ... and attach this menu bar to the frame
    SetMenuBar(menuBar);
#else // !wxUSE_MENUS
    // If menus are not available add a button to access the about box
    wxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
    wxButton* aboutBtn = new wxButton(this, wxID_ANY, "About...");
    aboutBtn->Bind(wxEVT_BUTTON, &MyFrame::OnAbout, this);
    sizer->Add(aboutBtn, wxSizerFlags().Center());
#endif // wxUSE_MENUS/!wxUSE_MENUS
    
// a statusbar is the application sending the user a message
#if wxUSE_STATUSBAR
    // create a status bar just for fun (by default with 1 pane only)
    CreateStatusBar(2);
    SetStatusText("Welcome to wxWidgets!");
#endif // wxUSE_STATUSBAR
    
// Make label and text field here
// label->setLabel() changes the label

    // vulkan testing
    cout << "MyFrame::MyFrame: VK_ERROR_DEVICE_LOST: " << VK_ERROR_DEVICE_LOST << endl;

    // Initialize the main panel that contains every other panel
    mainPanel = new wxPanel(this, -1,
                            wxPoint(0, 0),
                            wxSize(WX_FRAME_WIDTH, WX_FRAME_HEIGHT));

    //--------------------------------------------------------------------------
    // Upper Left Panel
    //--------------------------------------------------------------------------

    treePanel = new wxPanel(mainPanel, wxID_ANY,
                            wxPoint(TREE_PANEL_X, TREE_PANEL_Y),
                            wxSize(TREE_PANEL_W, TREE_PANEL_H));
    
    tree = new MyTreeCtrl(treePanel, TreeTest_Ctrl,
        wxPoint(0, 0),
        wxSize(TREE_PANEL_W, TREE_PANEL_H), wxTR_DEFAULT_STYLE, ROOT_DIR_NAME, ROOT_HOME_FOLDER_NAME);

    //Expands all the nodes
    //tree->ExpandAll();
    
    //--------------------------------------------------------------------------
    // Lower Left Panel
    //--------------------------------------------------------------------------
    cellImageFolder = new wxImage();
    cellImageFile = new wxImage();
#ifdef WIN32
    if (cellImageFolder->LoadFile(wxT("folder.bmp"))) {
        cellBitmapFolder = new wxBitmap(*cellImageFolder);
    } else {
        wxLogError(_T("folder.bmp didn't load, does it exist?"));
    }
#else
    if (cellImageFolder->LoadFile(wxT("../../folder.bmp"))) {
        cellBitmapFolder = new wxBitmap(*cellImageFolder);
    }
    else {
        wxLogError(_T("../../folder.bmp didn't load, does it exist?"));
    }
#endif

#ifdef WIN32
    if (cellImageFile->LoadFile(wxT("file.bmp"))) {
        cellBitmapFile = new wxBitmap(*cellImageFile);
    } else {
        wxLogError(_T("file.bmp didn't load, does it exist?"));
    }
#else
    if (cellImageFile->LoadFile(wxT("../../file.bmp"))) {
        cellBitmapFile = new wxBitmap(*cellImageFile);
    }
    else {
        wxLogError(_T("../../file.bmp didn't load, does it exist?"));
    }
#endif

    fileListPanel = new wxPanel(mainPanel, wxID_ANY,
                                         wxPoint(FILE_LIST_PANEL_X, FILE_LIST_PANEL_Y),
                                         wxSize(FILE_LIST_PANEL_W, FILE_LIST_PANEL_H));
    
    grid = new wxGrid(fileListPanel, wxID_ANY,
                              wxPoint( 0, 0 ),
                              wxSize(FILE_LIST_PANEL_W, FILE_LIST_PANEL_H));
    // this will create a grid and, by default, an associated grid
    // table for strings
    grid->CreateGrid( 0, 0 );
    
    grid->SetRowLabelSize(0);

    grid->AppendCols(2);

    grid->EnableEditing(false);

    grid->SetColLabelValue(0, _("")); // from wxSmith
    grid->SetColLabelValue(1, _("Name")); // from wxSmith

    grid->SetColSize(0, 20);
    grid->SetColSize(1, FILE_LIST_PANEL_W - 20);

    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir(tree->curDirPathOnLeftPanel.c_str())) != NULL) {
        /* print all the files and directories within directory */
        int i = 0;
        while ((ent = readdir(dir)) != NULL) {
            if (ent->d_name[0] != '.') {
                grid->AppendRows(1);
                gridRowAmount++;
                if (ent->d_type == DT_DIR) {
//                    printf ("%s\n", ent->d_name);
                    grid->SetCellRenderer(i, 0, new FolderGridCellRenderer);
                }
                else {
                    grid->SetCellRenderer(i, 0, new FileGridCellRenderer);
                }

                grid->SetCellValue( i, 1, ent->d_name);

                i++;
            }
            
        }
        closedir (dir);
        
    }
    else {
        /* could not open directory */
        perror ("");
        //return EXIT_FAILURE;
        cout << "tree->curDirPathOnLeftPanel: " << tree->curDirPathOnLeftPanel << endl;
        while (1) {};
        exit(1);
     }

    //--------------------------------------------------------------------------
    // Middle Panel
    //--------------------------------------------------------------------------
    
    contentsPanel = new wxPanel(mainPanel, wxID_ANY,
        wxPoint(VULKAN_PANEL_X, VULKAN_PANEL_Y),
        wxSize(VULKAN_PANEL_W, VULKAN_PANEL_H));

    vulkanCanvas = new VulkanCanvas(contentsPanel, wxID_ANY,
        wxPoint(VULKAN_PANEL_VULKAN_CANVAS_START_LOCAL_X, VULKAN_PANEL_VULKAN_CANVAS_START_LOCAL_Y),
        { VULKAN_PANEL_VULKAN_CANVAS_W, VULKAN_PANEL_VULKAN_CANVAS_H }, 0L, "VulkanCanvas", this);
    vulkanCanvas->Bind(wxEVT_LEFT_UP, &MyFrame::OnMouseLeftClickEnd, this);
    vulkanCanvas->Bind(wxEVT_LEFT_DOWN, &MyFrame::OnMouseLeftClick, this);
    vulkanCanvas->Bind(wxEVT_MOTION, &MyFrame::OnMouseMotion, this);
    vulkanCanvas->Bind(wxEVT_MOUSEWHEEL, &MyFrame::OnMouseWheel, this);
    vulkanCanvas->Bind(wxEVT_RIGHT_DOWN, &MyFrame::OnMouseRightClick, this);
    vulkanCanvas->Bind(wxEVT_RIGHT_UP, &MyFrame::OnMouseRightClickEnd, this);

    static const int INTERVAL = (1000/60); // milliseconds
    m_timer = new wxTimer(this, VulkanCanvas_Timer);
    m_timer->Start(INTERVAL);

    leftMouseOnVulkan = false;
    rightMouseOnVulkan = false;

    //--------------------------------------------------------------------------
    // Lower Right Panel
    //--------------------------------------------------------------------------
    objHierarchyPanel = new wxPanel(mainPanel, wxID_ANY,
        wxPoint(OBJ_HIERARCHY_PANEL_X, OBJ_HIERARCHY_PANEL_Y),
        wxSize(OBJ_HIERARCHY_PANEL_W, OBJ_HIERARCHY_PANEL_H));

    objHierarchy = new SceneTreeCtrl(objHierarchyPanel, Tree_Obj_Hierarchy,
        wxPoint(0, 0),
        wxSize(OBJ_HIERARCHY_PANEL_W, OBJ_HIERARCHY_PANEL_H), wxTR_DEFAULT_STYLE, "Scene");

    vulkanCanvas->Bind(wxEVT_LEFT_DOWN, &MyFrame::OnMouseLeftClick, this);

    //--------------------------------------------------------------------------
    // Upper Right Panel
    //--------------------------------------------------------------------------
    objectInfoPanel = new wxPanel(mainPanel, wxID_ANY,
                              wxPoint(BUTTON_LIST_PANEL_X, BUTTON_LIST_PANEL_Y),
                              wxSize(BUTTON_LIST_PANEL_W, BUTTON_LIST_PANEL_H));

    // wxTextCrtls, wxStaticTexts for the camera UI
    txtLblCurCameraTarget = new wxStaticText(objectInfoPanel, wxID_ANY, "Camera Target:",
        wxPoint(20, 160), wxSize(BUTTON_LIST_PANEL_W - 2 * 20, 20), wxST_NO_AUTORESIZE);

    txtCtrlCameraLookAtX = new wxTextCtrl(objectInfoPanel, -1, wxT("0.0"),
        wxPoint(40, 180),
        wxSize(BUTTON_LIST_PANEL_W - 65, 20));

    txtCtrlCameraLookAtY = new wxTextCtrl(objectInfoPanel, -1, wxT("0.0"),
        wxPoint(40, 200),
        wxSize(BUTTON_LIST_PANEL_W - 65, 20));

    txtCtrlCameraLookAtZ = new wxTextCtrl(objectInfoPanel, -1, wxT("0.0"),
        wxPoint(40, 220),
        wxSize(BUTTON_LIST_PANEL_W - 65, 20));

    txtLblCameraLookAtX = new wxStaticText(objectInfoPanel, wxID_ANY, "X:",
        wxPoint(25, 182), wxSize(10, 18), wxST_NO_AUTORESIZE);
    txtLblCameraLookAtY = new wxStaticText(objectInfoPanel, wxID_ANY, "Y:",
        wxPoint(25, 202), wxSize(10, 18), wxST_NO_AUTORESIZE);
    txtLblCameraLookAtZ = new wxStaticText(objectInfoPanel, wxID_ANY, "Z:",
        wxPoint(25, 222), wxSize(10, 18), wxST_NO_AUTORESIZE);


    txtLblCurCameraPos = new wxStaticText(objectInfoPanel, wxID_ANY, "Camera Position:",
        wxPoint(20, 60), wxSize(BUTTON_LIST_PANEL_W - 2 * 20, 20), wxST_NO_AUTORESIZE);

    txtCtrlCameraPosX = new wxTextCtrl(objectInfoPanel, -1, wxT("0.0"),
        wxPoint(40, 80),
        wxSize(BUTTON_LIST_PANEL_W - 65, 20));
    txtCtrlCameraPosY = new wxTextCtrl(objectInfoPanel, -1, wxT("0.0"),
        wxPoint(40, 100),
        wxSize(BUTTON_LIST_PANEL_W - 65, 20));
    txtCtrlCameraPosZ = new wxTextCtrl(objectInfoPanel, -1, wxT("0.0"),
        wxPoint(40, 120),
        wxSize(BUTTON_LIST_PANEL_W - 65, 20));

    txtLblCameraPosX = new wxStaticText(objectInfoPanel, wxID_ANY, "X:",
        wxPoint(25, 82), wxSize(10, 18), wxST_NO_AUTORESIZE);
    txtLblCameraPosY = new wxStaticText(objectInfoPanel, wxID_ANY, "Y:",
        wxPoint(25, 102), wxSize(10, 18), wxST_NO_AUTORESIZE);
    txtLblCameraPosZ = new wxStaticText(objectInfoPanel, wxID_ANY, "Z:",
        wxPoint(25, 122), wxSize(10, 18), wxST_NO_AUTORESIZE);

    // Update the text controls showing the position of the camera
    txtCtrlCameraPosX->ChangeValue(wxString::Format(wxT("%f"), 10.0f));
    txtCtrlCameraPosY->ChangeValue(wxString::Format(wxT("%f"), 10.0f));
    txtCtrlCameraPosZ->ChangeValue(wxString::Format(wxT("%f"), 10.0f));

    // Update the text controls showing the position of the camera
    txtCtrlCameraLookAtX->ChangeValue(wxString::Format(wxT("%f"), 0.0f));
    txtCtrlCameraLookAtY->ChangeValue(wxString::Format(wxT("%f"), 0.0f));
    txtCtrlCameraLookAtZ->ChangeValue(wxString::Format(wxT("%f"), 0.0f));

    // wxTextCrtls, wxButtons, wxStaticTexts for the current object UI
    txtCtrlPositionX = new wxTextCtrl(objectInfoPanel, -1, wxT("0.0"),
        wxPoint(40, 280),
        wxSize(BUTTON_LIST_PANEL_W - 65, 20));

    txtCtrlPositionY = new wxTextCtrl(objectInfoPanel, -1, wxT("0.0"),
        wxPoint(40, 300),
        wxSize(BUTTON_LIST_PANEL_W - 65, 20));

    txtCtrlPositionZ = new wxTextCtrl(objectInfoPanel, -1, wxT("0.0"),
        wxPoint(40, 320),
        wxSize(BUTTON_LIST_PANEL_W - 65, 20));

    txtLblPositionX = new wxStaticText(objectInfoPanel, wxID_ANY, "X:",
        wxPoint(25, 282), wxSize(10, 18), wxST_NO_AUTORESIZE);
    txtLblPositionY = new wxStaticText(objectInfoPanel, wxID_ANY, "Y:",
        wxPoint(25, 302), wxSize(10, 18), wxST_NO_AUTORESIZE);
    txtLblPositionZ = new wxStaticText(objectInfoPanel, wxID_ANY, "Z:",
        wxPoint(25, 322), wxSize(10, 18), wxST_NO_AUTORESIZE);

    txtLblCurObject = new wxStaticText(objectInfoPanel, wxID_ANY, "Object:",
        wxPoint(20, 260), wxSize(BUTTON_LIST_PANEL_W - 2 * 20, 20), wxST_NO_AUTORESIZE);

    //hideObjectCheckBox = new wxCheckBox(objectInfoPanel, wxID_ANY, "Hide",
    //    wxPoint(25, 340), wxSize(60, 20), wxST_NO_AUTORESIZE);
    hideObjectButton = new wxButton(objectInfoPanel, ID_BUTTON_HIDE_OBJECT, "Hide",
        wxPoint(25, 340), wxSize((BUTTON_LIST_PANEL_W - 50) / 2, 20));
    showObjectButton = new wxButton(objectInfoPanel, ID_BUTTON_SHOW_OBJECT, "Show",
        wxPoint(25 + (BUTTON_LIST_PANEL_W - 50) / 2, 340), wxSize((BUTTON_LIST_PANEL_W - 50) / 2, 20));
}

MyFrame::~MyFrame() {
    delete cellImageFolder;
    delete cellImageFile;
    delete cellBitmapFolder;
    delete cellBitmapFile;
}

// event handlers

void MyFrame::OnQuit(wxCommandEvent& WXUNUSED(event))
{
    // true is to force the frame to close
    Close(true);
}

void MyFrame::OnAbout(wxCommandEvent& WXUNUSED(event))
{
    wxMessageBox(wxString::Format
                 (
                    "Welcome to %s!\n"
                    "\n"
                    "This is the minimal wxWidgets sample\n"
                    "running under %s.",
                    wxVERSION_STRING,
                    wxGetOsDescription()
                 ),
                 "About wxWidgets minimal sample",
                 wxOK | wxICON_INFORMATION,
                 this);
}

// Certain constants used in the functions below
#define ID_LOAD_OBJECT 2001
#define MAX_LEN_PATH 4096
#define MAX_LEN_NAME 256
#define PANNING_SPEED_MULTIPLIER 10
#define ROTATING_SPEED_X (2 * M_PI / VULKAN_PANEL_VULKAN_CANVAS_W) // a movement from left to right = 2*PI = 360 deg
#define ROTATING_SPEED_Y (M_PI / VULKAN_PANEL_VULKAN_CANVAS_H)  // a movement from top to bottom = PI = 180 deg
#define ZOOM_SPEED_MULTIPLIER 2

// Handle when clicking on the popup after right clicking on a grid cell (from OnGridRightClick())
void MyFrame::OnPopupClick(wxCommandEvent& ev) {
    void* data = static_cast<wxMenu*>(ev.GetEventObject())->GetClientData();
    wxString* cellNamePtr = (wxString*)data;
    std::cout << "MyFrame::OnPopupClick(): cellName: " << *cellNamePtr << std::endl;

    ifstream inputToEncode;
    fstream outputEncoded;

    std::cout << "MyFrame::OnPopupClick(): curDirPathOnLeftPanel + cellName: " << tree->curDirPathOnLeftPanel + "/" + *cellNamePtr << std::endl;

    DIR* dir;
    struct dirent* ent;

    dir = opendir(tree->curDirPathOnLeftPanel.c_str());

    string nameOfFolder;
    bool isFile = true;
    if (ev.GetId() == ID_LOAD_OBJECT) {
        wxString wxStringFullpath = tree->curDirPathOnLeftPanel + "/" + *cellNamePtr;
        string fullpath = wxStringFullpath.ToStdString();

        vulkanCanvas->ShowObject(fullpath, true, true);

        vulkanCanvas->m_objects.at(vulkanCanvas->m_objects.size() - 1)->offset3D = glm::vec3(0.0f, 0.0f, 0.0f);
    }
}

// Handle when clicking on a file in the grid
void MyFrame::OnGridLeftClick(wxGridEvent& ev) {
    fullpath = tree->curDirPathOnLeftPanel;
    fullpath += "/";
    fullpath += grid->GetCellValue(ev.GetRow(), ev.GetCol());

    DIR* dir;
    if ((dir = opendir(fullpath.c_str())) != NULL) {
        // when the fullPath is for a dir then dont open
        closedir(dir);
    }
    else if (fullpath.substr(fullpath.size() - 4) == ".obj" || fullpath.substr(fullpath.size() - 8) == ".md5mesh") { // it is an object/md5 file so show it as an object
        vulkanCanvas->ShowObject(fullpath, true, true);

        vulkanCanvas->m_objects.at(vulkanCanvas->m_objects.size() - 1)->offset3D = glm::vec3(0.0f, 0.0f, 0.0f);
    }

    ev.Skip();
}

// Handle right clicking in a grid
void MyFrame::OnGridRightClick(wxGridEvent& ev) {
    wxString cellName = grid->GetCellValue(ev.GetRow(), ev.GetCol());
    void *data = &cellName;
    
    wxMenu mnu;
    mnu.SetClientData( data ); // only accepts void pointers

    // Create the option to load an object
    mnu.Append(ID_LOAD_OBJECT, "Load Object");

    string name = cellName.ToStdString();

    std::size_t foundObj = name.find(".obj");
    std::size_t foundMD5 = name.find(".md5mesh");

    // If the file in the cell is an accepted file, allow the "Load Object" option to be selected in the popup
    if (name.length() >= 4 && foundObj == name.length() - 4 || name.length() >= 8 && foundMD5 == name.length() - 8) {
        mnu.Enable(ID_LOAD_OBJECT, true);
    }
    else {
        mnu.Enable(ID_LOAD_OBJECT, false);
    }

    mnu.Connect(wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MyFrame::OnPopupClick), NULL, this);
    PopupMenu(&mnu);
}

// When the user stops left clicking, stop the panning motion of the camera
void MyFrame::OnMouseLeftClickEnd(wxMouseEvent& ev) {
    printf("MyFrame::OnMouseLeftClickEnd()\n");

    leftMouseOnVulkan = false;
}

// WHen the user stops right clicking, stop the rotational motion of hte camera
void MyFrame::OnMouseRightClickEnd(wxMouseEvent& ev) {
    printf("MyFrame::OnMouseRightClickEnd()\n");

    wxPoint mouse_position = ev.GetPosition();

    rightMouseOnVulkan = false;
}

// Handle mouse motion. Relevant for when the mouse is either left or right clicking
void MyFrame::OnMouseMotion(wxMouseEvent& ev) {
    wxPoint mouse_position = ev.GetPosition();

    // If the left mouse button is down, pan the camera
    if (leftMouseOnVulkan) {
        // Followed guide here: https://antongerdelan.net/opengl/raycasting.html

        // Change the coordinates in the viewport to normalized device coordinates
        float x = (2.0f * mouse_position.x) / VULKAN_PANEL_VULKAN_CANVAS_W - 1.0f;
        float y = 1.0f - (2.0f * mouse_position.y) / VULKAN_PANEL_VULKAN_CANVAS_H;
        float z = 1.0f;

        // Create inverse matrices for transformations we need to make
        glm::mat4 inverseProjectionMatrix = glm::inverse(vulkanCanvas->m_proj);
        glm::mat4 inverseViewMatrix = glm::inverse(vulkanCanvas->v_proj);

        // Create homogenous clip vector
        glm::vec3 ray_nds = glm::vec3(x, y, z);
        glm::vec4 ray_clip = glm::vec4(ray_nds.x, ray_nds.y, -1.0, 1.0);

        // Transform clip into 4d eye space coordinates
        glm::vec4 ray_eye = inverseProjectionMatrix * ray_clip;
        ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1.0, 0.0);

        // Finally, transform the eye space coordinates into world coordinates by...
        glm::vec3 targetPoint = glm::vec3(vulkanCanvas->viewingX, vulkanCanvas->viewingY, vulkanCanvas->viewingZ);
        glm::vec3 camera_pos;

        camera_pos.x = vulkanCanvas->cameraXBeforeDrag;
        camera_pos.y = vulkanCanvas->cameraYBeforeDrag;
        camera_pos.z = vulkanCanvas->cameraZBeforeDrag;

        glm::vec3 upVector = glm::vec3(0.0f, 1.0f, 0.0f);

        // Applying the inverse of the transformation that takes world to eye coordinates
        glm::vec4 ray_wor_in_vec4 = glm::inverse(glm::lookAt(camera_pos, targetPoint, upVector)) * ray_eye;
        glm::vec3 ray_wor = glm::vec3(ray_wor_in_vec4.x, ray_wor_in_vec4.y, ray_wor_in_vec4.z);
        ray_wor = glm::normalize(ray_wor);

        //cout << "x: " << ray_wor.x - vulkanCanvas->mouseDragRay.x;
        //cout << " y: " << ray_wor.y - vulkanCanvas->mouseDragRay.y;
        //cout << " z: " << ray_wor.z - vulkanCanvas->mouseDragRay.z << endl;

        // vulkanCanvas->mouseDragRay - ray_wor will give the direction that the camera
        //     (and also where the camera is looking, as this describes a panning movement of the camera) has to move
        vulkanCanvas->cameraX = vulkanCanvas->cameraXBeforeDrag + PANNING_SPEED_MULTIPLIER * (vulkanCanvas->mouseDragRay.x - ray_wor.x);
        vulkanCanvas->cameraY = vulkanCanvas->cameraYBeforeDrag + PANNING_SPEED_MULTIPLIER * (vulkanCanvas->mouseDragRay.y - ray_wor.y);
        vulkanCanvas->cameraZ = vulkanCanvas->cameraZBeforeDrag + PANNING_SPEED_MULTIPLIER * (vulkanCanvas->mouseDragRay.z - ray_wor.z);

        vulkanCanvas->viewingX = vulkanCanvas->viewingXBeforeDrag + PANNING_SPEED_MULTIPLIER * (vulkanCanvas->mouseDragRay.x - ray_wor.x);
        vulkanCanvas->viewingY = vulkanCanvas->viewingYBeforeDrag + PANNING_SPEED_MULTIPLIER * (vulkanCanvas->mouseDragRay.y - ray_wor.y);
        vulkanCanvas->viewingZ = vulkanCanvas->viewingZBeforeDrag + PANNING_SPEED_MULTIPLIER * (vulkanCanvas->mouseDragRay.z - ray_wor.z);

        //cout << "cameraX: " << vulkanCanvas->cameraX;
        //cout << " cameraY: " << vulkanCanvas->cameraY;
        //cout << " cameraZ: " << vulkanCanvas->cameraZ << endl;

        //cout << "viewingX: " << vulkanCanvas->viewingX;
        //cout << " viewingY: " << vulkanCanvas->viewingY;
        //cout << " viewingZ: " << vulkanCanvas->viewingZ << endl;

        vulkanCanvas->cameraDirection.x = vulkanCanvas->cameraX - vulkanCanvas->viewingX;
        vulkanCanvas->cameraDirection.y = vulkanCanvas->cameraY - vulkanCanvas->viewingY;
        vulkanCanvas->cameraDirection.z = vulkanCanvas->cameraZ - vulkanCanvas->viewingZ;

        vulkanCanvas->cameraDirection = glm::normalize(vulkanCanvas->cameraDirection);

        // Update the text controls showing the position the camera is looking at
        txtCtrlCameraLookAtX->ChangeValue(wxString::Format(wxT("%f"), vulkanCanvas->viewingX));
        txtCtrlCameraLookAtY->ChangeValue(wxString::Format(wxT("%f"), vulkanCanvas->viewingY));
        txtCtrlCameraLookAtZ->ChangeValue(wxString::Format(wxT("%f"), vulkanCanvas->viewingZ));

        // Update the text controls showing the position of the camera
        txtCtrlCameraPosX->ChangeValue(wxString::Format(wxT("%f"), vulkanCanvas->cameraX));
        txtCtrlCameraPosY->ChangeValue(wxString::Format(wxT("%f"), vulkanCanvas->cameraY));
        txtCtrlCameraPosZ->ChangeValue(wxString::Format(wxT("%f"), vulkanCanvas->cameraZ));
    }
    else if (rightMouseOnVulkan) { // If the right mouse button is down, rotate the camera
        // Made following this guide here:
        // https://asliceofrendering.com/camera/2019/11/30/ArcballCamera/

        glm::vec4 position = glm::vec4(vulkanCanvas->cameraX, vulkanCanvas->cameraY, vulkanCanvas->cameraZ, 1);
        glm::vec4 pivot = glm::vec4(vulkanCanvas->viewingX, vulkanCanvas->viewingY, vulkanCanvas->viewingZ, 1);

        float deltaAngleX = ROTATING_SPEED_X;
        float deltaAngleY = ROTATING_SPEED_Y;
        float xAngle = (startPosForRightMouseDrag.x - mouse_position.x) * deltaAngleX;
        float yAngle = (startPosForRightMouseDrag.y - mouse_position.y) * deltaAngleY;

        //cout << "xAngle: " << xAngle << endl;
        //cout << "yAngle: " << yAngle << endl;

        // This is to ensure that we dont align our camera direction with the up direction of our rotation pivot
        //     which would result in some glitchyness
        float cosAngle = glm::dot(vulkanCanvas->getViewDir(), glm::vec3(0.0f, 1.0f, 0.0f));
        if (cosAngle > 0.999f && yAngle > 0 || cosAngle < -0.999f && yAngle < 0) {
            yAngle = 0;
        }
        //cout << "cosAngle: " << cosAngle << endl;

        glm::mat4x4 rotationMatrixX(1.0f);
        rotationMatrixX = glm::rotate(rotationMatrixX, xAngle, vulkanCanvas->upVector);
        position = (rotationMatrixX * (position - pivot)) + pivot;

        glm::mat4x4 rotationMatrixY(1.0f);
        rotationMatrixY = glm::rotate(rotationMatrixY, yAngle, vulkanCanvas->getRightVector());
        glm::vec3 finalPosition = (rotationMatrixY * (position - pivot)) + pivot;

        vulkanCanvas->SetCameraView(finalPosition, glm::vec3(vulkanCanvas->viewingX, vulkanCanvas->viewingY, vulkanCanvas->viewingZ), vulkanCanvas->upVector);

        startPosForRightMouseDrag.x = mouse_position.x;
        startPosForRightMouseDrag.y = mouse_position.y;

        // Fix the direction that the camera is pointing, such that the scrolling to zoom in/out works properly
        vulkanCanvas->cameraDirection.x = vulkanCanvas->cameraX - vulkanCanvas->viewingX;
        vulkanCanvas->cameraDirection.y = vulkanCanvas->cameraY - vulkanCanvas->viewingY;
        vulkanCanvas->cameraDirection.z = vulkanCanvas->cameraZ - vulkanCanvas->viewingZ;

        vulkanCanvas->cameraDirection = glm::normalize(vulkanCanvas->cameraDirection);

        // Update the text controls showing the position of the camera
        txtCtrlCameraPosX->ChangeValue(wxString::Format(wxT("%f"), vulkanCanvas->cameraX));
        txtCtrlCameraPosY->ChangeValue(wxString::Format(wxT("%f"), vulkanCanvas->cameraY));
        txtCtrlCameraPosZ->ChangeValue(wxString::Format(wxT("%f"), vulkanCanvas->cameraZ));
    }
}

// Used to start camera panning
void MyFrame::OnMouseLeftClick(wxMouseEvent& ev) {
    printf("MyFrame::OnMouseLeftClick()\n");

    wxPoint mouse_position = ev.GetPosition();

    startPosForMouseDrag.x = mouse_position.x;
    startPosForMouseDrag.y = mouse_position.y;

    cout << "MyFrame::OnMouseLeftClick(): MOUSE POSITION X: " << mouse_position.x << endl;
    cout << "MyFrame::OnMouseLeftClick(): MOUSE POSITION Y: " << mouse_position.y << endl;

    // If the mouse is inside the vulkan panel, initiate camera panning
    // Also check if the right mouse button is pressed
    if (mouse_position.x >= 0 &&
        mouse_position.x < VULKAN_PANEL_W &&
        mouse_position.y >= 0 &&
        mouse_position.y < VULKAN_PANEL_H &&
        !rightMouseOnVulkan) {

        leftMouseOnVulkan = true;

        vulkanCanvas->cameraXBeforeDrag = vulkanCanvas->cameraX;
        vulkanCanvas->cameraYBeforeDrag = vulkanCanvas->cameraY;
        vulkanCanvas->cameraZBeforeDrag = vulkanCanvas->cameraZ;

        vulkanCanvas->viewingXBeforeDrag = vulkanCanvas->viewingX;
        vulkanCanvas->viewingYBeforeDrag = vulkanCanvas->viewingY;
        vulkanCanvas->viewingZBeforeDrag = vulkanCanvas->viewingZ;

        vulkanCanvas->cameraDirection.x = vulkanCanvas->cameraX - vulkanCanvas->viewingX;
        vulkanCanvas->cameraDirection.y = vulkanCanvas->cameraY - vulkanCanvas->viewingY;
        vulkanCanvas->cameraDirection.z = vulkanCanvas->cameraZ - vulkanCanvas->viewingZ;

        vulkanCanvas->cameraDirection = glm::normalize(vulkanCanvas->cameraDirection);

        // Change the coordinates in the viewport to normalized device coordinates
        float x = (2.0f * mouse_position.x) / VULKAN_PANEL_VULKAN_CANVAS_W - 1.0f;
        float y = 1.0f - (2.0f * mouse_position.y) / VULKAN_PANEL_VULKAN_CANVAS_H;
        float z = 1.0f;

        // Create inverse matrices for transformations we need to make
        glm::mat4 inverseProjectionMatrix = glm::inverse(vulkanCanvas->m_proj);
        glm::mat4 inverseViewMatrix = glm::inverse(vulkanCanvas->v_proj);

        // Create homogenous clip vector
        glm::vec3 ray_nds = glm::vec3(x, y, z);
        glm::vec4 ray_clip = glm::vec4(ray_nds.x, ray_nds.y, -1.0, 1.0);

        // Transform clip into 4d eye space coordinates
        glm::vec4 ray_eye = inverseProjectionMatrix * ray_clip;
        ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1.0, 0.0);

        // Transform the eye space coordinates into world coordinates and store them into vulkanCanvas->mouseDragRay
        //     vulkanCanvas->mouseDragRay will be used when dragging with the mouse
        glm::vec3 targetPoint = glm::vec3(vulkanCanvas->viewingX, vulkanCanvas->viewingY, vulkanCanvas->viewingZ);
        glm::vec3 camera_pos;

        camera_pos.x = vulkanCanvas->cameraX;
        camera_pos.y = vulkanCanvas->cameraY;
        camera_pos.z = vulkanCanvas->cameraZ;

        glm::vec3 upVector = glm::vec3(0.0f, 1.0f, 0.0f);

        glm::vec4 ray_wor_in_vec4 = glm::inverse(glm::lookAt(camera_pos, targetPoint, upVector)) * ray_eye;
        glm::vec3 ray_wor = glm::vec3(ray_wor_in_vec4.x, ray_wor_in_vec4.y, ray_wor_in_vec4.z);
        ray_wor = glm::normalize(ray_wor);

        vulkanCanvas->mouseDragRay = ray_wor;
    }
}

// User to start camera rotation
void MyFrame::OnMouseRightClick(wxMouseEvent& ev) {
    printf("MyFrame::OnMouseRightClick()\n");

    wxPoint mouse_position = ev.GetPosition();

    startPosForRightMouseDrag.x = mouse_position.x;
    startPosForRightMouseDrag.y = mouse_position.y;

    cout << "MyFrame::OnMouseRightClick(): MOUSE POSITION X: " << mouse_position.x << endl;
    cout << "MyFrame::OnMouseRightClick(): MOUSE POSITION Y: " << mouse_position.y << endl;

    // If the mouse is inside the vulkan panel, initiate camera rotation
    // Also check if the left mouse button is pressed
    if (mouse_position.x >= 0 &&
        mouse_position.x < VULKAN_PANEL_W &&
        mouse_position.y >= 0 &&
        mouse_position.y < VULKAN_PANEL_H &&
        !leftMouseOnVulkan) {

        rightMouseOnVulkan = true;
    }
}

// Used for zooming in/out
void MyFrame::OnMouseWheel(wxMouseEvent& ev) {
    int direction = ev.GetWheelRotation();

    // Make sure that the camera is not currently rotating or panning
    if (!rightMouseOnVulkan && !leftMouseOnVulkan) {
        //cout << direction << endl;
        if (direction > 0) {
            direction = 1;
        }
        else {
            direction = -1;
        }

        vulkanCanvas->cameraX -= ZOOM_SPEED_MULTIPLIER * direction * vulkanCanvas->cameraDirection.x;
        vulkanCanvas->cameraY -= ZOOM_SPEED_MULTIPLIER * direction * vulkanCanvas->cameraDirection.y;
        vulkanCanvas->cameraZ -= ZOOM_SPEED_MULTIPLIER * direction * vulkanCanvas->cameraDirection.z;

        //cout << "cameraDirectionX: " << vulkanCanvas->cameraDirection.x;
        //cout << " cameraDirectionY: " << vulkanCanvas->cameraDirection.y;
        //cout << " cameraDirectionZ: " << vulkanCanvas->cameraDirection.z << endl;

        //cout << "cameraX: " << vulkanCanvas->cameraX;
        //cout << " cameraY: " << vulkanCanvas->cameraY;
        //cout << " cameraZ: " << vulkanCanvas->cameraZ << endl;

        //cout << "viewingX: " << vulkanCanvas->viewingX;
        //cout << " viewingY: " << vulkanCanvas->viewingY;
        //cout << " viewingZ: " << vulkanCanvas->viewingZ << endl;

        vulkanCanvas->cameraDirection.x = vulkanCanvas->cameraX - vulkanCanvas->viewingX;
        vulkanCanvas->cameraDirection.y = vulkanCanvas->cameraY - vulkanCanvas->viewingY;
        vulkanCanvas->cameraDirection.z = vulkanCanvas->cameraZ - vulkanCanvas->viewingZ;

        vulkanCanvas->cameraDirection = glm::normalize(vulkanCanvas->cameraDirection);

        // Update the text controls showing the position of the camera
        txtCtrlCameraPosX->ChangeValue(wxString::Format(wxT("%f"), vulkanCanvas->cameraX));
        txtCtrlCameraPosY->ChangeValue(wxString::Format(wxT("%f"), vulkanCanvas->cameraY));
        txtCtrlCameraPosZ->ChangeValue(wxString::Format(wxT("%f"), vulkanCanvas->cameraZ));
    }
}

// For pressing hide button in the UI
void MyFrame::OnHideLeftClick(wxCommandEvent& event) {
    if (objHierarchy->prevSelectedObject != -1) {
        vulkanCanvas->m_objects.at(NUM_STARTING_OBJECTS + objHierarchy->selectedObject)->drawObject = false;
    }
}

// For pressing show button in the UI
void MyFrame::OnShowLeftClick(wxCommandEvent& event) {
    if (objHierarchy->prevSelectedObject != -1) {
        vulkanCanvas->m_objects.at(NUM_STARTING_OBJECTS + objHierarchy->selectedObject)->drawObject = true;
    }
}

bool MyFrame::makeLowerLeftPanel(wxString fullPath) {
    //std::cout << "makeLowerLeftPanel: fullPath = |" << fullPath.c_str() << "|" << std::endl;

    if (grid != NULL) {
        delete grid;
        grid = NULL;
    }
    
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir(fullPath.c_str())) == NULL) {
        return false;
    }

    grid = new wxGrid(fileListPanel, wxID_ANY,
                      wxPoint( 0, 0 ),
                      wxSize(FILE_LIST_PANEL_W, FILE_LIST_PANEL_H));
    // this will create a grid and, by default, an associated grid
    // table for strings
    grid->CreateGrid( 0, 0 );
    
    grid->SetRowLabelSize(0);
    
    grid->AppendCols(2);
    
    grid->EnableEditing(false);
    
    grid->SetColLabelValue(0, _("")); // from wxSmith
    grid->SetColLabelValue(1, _("Name")); // from wxSmith
    
    grid->SetColSize(0, 20);
    grid->SetColSize(1, FILE_LIST_PANEL_W - 20);
    
    /* print all the files and directories within directory */
    int i = 0;
    while ((ent = readdir(dir)) != NULL) {
        if (ent->d_name[0] != '.') {
            grid->AppendRows(1);
            gridRowAmount++;
            if (ent->d_type == DT_DIR) {
                //                    printf ("%s\n", ent->d_name);
                grid->SetCellRenderer(i, 0, new FolderGridCellRenderer);
            }
            else {
                grid->SetCellRenderer(i, 0, new FileGridCellRenderer);
            }
            grid->SetCellValue( i, 1, ent->d_name);
         
            i++;
        }
    }
    closedir (dir);
    return true;
}

void MyFrame::getAllInfoFromFolder(vector<string> &names, vector<bool> &isFolder, string path, int *totalChunks, int *totalFiles, int *totalFolders, long *totalSize) {
    DIR *dir;
    struct dirent *ent;
    dir = opendir(path.c_str());
    while ((ent = readdir(dir)) != NULL) {
        if (ent->d_name[0] != '.') {
            (*totalChunks)++;
            string curItemFullPath = path + "/";
            curItemFullPath += ent->d_name;
            names.push_back(curItemFullPath);
            cout << "MyFrame::getAllInfoFromFolder curItemFullPath:" << curItemFullPath.c_str() << endl;
            if (ent->d_type != DT_DIR) {
                (*totalFiles)++;
                isFolder.push_back(false);
                (*totalSize) += getFileSize(curItemFullPath.c_str());
                cout << "MyFrame::getAllInfoFromFolder getFileSize(curItemFullPath.c_str()):" << getFileSize(curItemFullPath.c_str()) << endl;
            }
            else {
                (*totalFolders)++;
                isFolder.push_back(true);
                getAllInfoFromFolder(names, isFolder, curItemFullPath, totalChunks, totalFiles, totalFolders, totalSize);
            }
        }
    }
}

// Acts as an update function
void MyFrame::OnVkTimer(wxTimerEvent& event) {
    //----------------------------------------------------------------------------------------------------
    // Check if there is a new selected item in the object hierarchy, then change the text controls in the 
    //     upper right panel to reflect the information on the new object that has been selected
    //----------------------------------------------------------------------------------------------------
    wxString objPosXInWxStr = txtCtrlPositionX->GetValue();
    wxString objPosYInWxStr = txtCtrlPositionY->GetValue();
    wxString objPosZInWxStr = txtCtrlPositionZ->GetValue();

    double objPosX = 0.0;
    double objPosY = 0.0;
    double objPosZ = 0.0;

    objPosXInWxStr.ToDouble(&objPosX);
    objPosYInWxStr.ToDouble(&objPosY);
    objPosZInWxStr.ToDouble(&objPosZ);

    //cout << "MyFrame::OnVkTimer(): tempX = " << tempX << endl;
    //cout << "MyFrame::OnVkTimer(): tempY = " << tempY << endl;
    //cout << "MyFrame::OnVkTimer(): tempZ = " << tempZ << endl;

    // If the selected object is in the object hierarchy, and is not the 'Scene' root object
    if (vulkanCanvas->m_objects.size() - NUM_STARTING_OBJECTS > 0 && objHierarchy->selectedObject != -1){
        if (objHierarchy->selectedObject != objHierarchy->prevSelectedObject) { // If there has been a new selected object, update the text controls to contain the new object's position
            txtCtrlPositionX->ChangeValue(wxString::Format(wxT("%f"), (vulkanCanvas->m_objects.at(NUM_STARTING_OBJECTS + objHierarchy->selectedObject)->offset3D.x)));
            txtCtrlPositionY->ChangeValue(wxString::Format(wxT("%f"), (vulkanCanvas->m_objects.at(NUM_STARTING_OBJECTS + objHierarchy->selectedObject)->offset3D.y)));
            txtCtrlPositionZ->ChangeValue(wxString::Format(wxT("%f"), (vulkanCanvas->m_objects.at(NUM_STARTING_OBJECTS + objHierarchy->selectedObject)->offset3D.z)));

            objHierarchy->prevSelectedObject = objHierarchy->selectedObject;
        }
        else { // Otherwise, update the current selected object's position to whatever is inside the text controls
            vulkanCanvas->m_objects.at(NUM_STARTING_OBJECTS + objHierarchy->selectedObject)->offset3D = glm::vec3(
                (float)objPosX,
                (float)objPosY,
                (float)objPosZ);
        }
        txtLblCurObject->SetLabel("Object: " + vulkanCanvas->m_objects.at(NUM_STARTING_OBJECTS + objHierarchy->selectedObject)->sceneTreeName);
    }
    else if (objHierarchy->selectedObject == -1) {
        txtLblCurObject->SetLabel("Object: ");
    }

    // Update the point the camera is looking at using the input in the text controls
    wxString cameraLookAtXInWxStr = txtCtrlCameraLookAtX->GetValue();
    wxString cameraLookAtYInWxStr = txtCtrlCameraLookAtY->GetValue();
    wxString cameraLookAtZInWxStr = txtCtrlCameraLookAtZ->GetValue();

    double cameraLookAtX = 0.0;
    double cameraLookAtY = 0.0;
    double cameraLookAtZ = 0.0;

    cameraLookAtXInWxStr.ToDouble(&cameraLookAtX);
    cameraLookAtYInWxStr.ToDouble(&cameraLookAtY);
    cameraLookAtZInWxStr.ToDouble(&cameraLookAtZ);

    vulkanCanvas->viewingX = (float)cameraLookAtX;
    vulkanCanvas->viewingY = (float)cameraLookAtY;
    vulkanCanvas->viewingZ = (float)cameraLookAtZ;

    // Update the camera's position using the input in the text controls
    wxString cameraPosXInWxStr = txtCtrlCameraPosX->GetValue();
    wxString cameraPosYInWxStr = txtCtrlCameraPosY->GetValue();
    wxString cameraPosZInWxStr = txtCtrlCameraPosZ->GetValue();

    double cameraPosX = 0.0;
    double cameraPosY = 0.0;
    double cameraPosZ = 0.0;

    cameraPosXInWxStr.ToDouble(&cameraPosX);
    cameraPosYInWxStr.ToDouble(&cameraPosY);
    cameraPosZInWxStr.ToDouble(&cameraPosZ);

    vulkanCanvas->cameraX = (float)cameraPosX;
    vulkanCanvas->cameraY = (float)cameraPosY;
    vulkanCanvas->cameraZ = (float)cameraPosZ;

    vulkanCanvas->DrawFrame();
}
