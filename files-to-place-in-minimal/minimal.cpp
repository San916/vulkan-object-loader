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
// Obj Hierarchy Panel(Lower Left Panel)
//------------------------------------------------
#define OBJ_HIERARCHY_PANEL_W TREE_PANEL_W
#define OBJ_HIERARCHY_PANEL_H FRAME_UPPER_H

#define OBJ_HIERARCHY_PANEL_X 0
#define OBJ_HIERARCHY_PANEL_Y FRAME_UPPER_H

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
//#define BUTTON_LIST_PANEL_H (WX_PANEL_HEIGHT / 2)
#define BUTTON_LIST_PANEL_H (WX_PANEL_HEIGHT)

#define BUTTON_LIST_PANEL_X (3 * (WX_PANEL_WIDTH / 4))
#define BUTTON_LIST_PANEL_Y 0


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
#include "Vertex.h"
#include <math.h>
#include <cmath>
#include "glm/gtx/closest_point.hpp"
#include "MD5Model.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

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

string converter(uint8_t* str);

#include "minimal.h"
#include "my_tree_ctrl.h"
#include "scene_tree_ctrl.h"

//#include "hello_triangle_application.h"

wxDECLARE_APP(MyApp);

// MyFrame extends wxFrame. It is a child class
// wxFrame has the base of a GUI application
// Define a new frame type: this is going to be our main frame
int gridRowAmount = 0;

MyFrame* frame = NULL;

void MyFrame::OnItemExpanding(wxTreeEvent& event) {

}

// ----------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------

// IDs for the controls and the menu commands

const int ID_BUTTON_HIDE_OBJECT = 100;
const int ID_BUTTON_SHOW_OBJECT = 101;
const int ID_BUTTON_RESET_CAMERA = 102;
const int ID_BUTTON_RESET_OBJECT = 103;
const int ID_BUTTON_UNSELECT_OBJECT = 104;
const int ID_BUTTON_TOGGLE_PHYSICS = 105;

// ----------------------------------------------------------------------------
// event tables and other macros for wxWidgets
// ----------------------------------------------------------------------------

// the event tables connect the wxWidgets events with the functions (event
// handlers) which process them. It can be also done at run-time, but for the
// simple menu events like this the static method is much simpler.
wxBEGIN_EVENT_TABLE(MyFrame, wxFrame)
EVT_MENU(Minimal_Quit, MyFrame::OnQuit)
EVT_MENU(Minimal_About, MyFrame::OnAbout)
EVT_LEFT_DOWN(MyFrame::OnMouseLeftClick)
EVT_TIMER(VulkanCanvas_Timer, MyFrame::OnVkTimer)
EVT_BUTTON(ID_BUTTON_HIDE_OBJECT, MyFrame::OnHideLeftClick)
EVT_BUTTON(ID_BUTTON_SHOW_OBJECT, MyFrame::OnShowLeftClick)
EVT_BUTTON(ID_BUTTON_RESET_CAMERA, MyFrame::OnResetCameraLeftClick)
EVT_BUTTON(ID_BUTTON_RESET_OBJECT, MyFrame::OnResetObjectLeftClick)
EVT_BUTTON(ID_BUTTON_UNSELECT_OBJECT, MyFrame::OnUnselectObjectLeftClick)
EVT_BUTTON(ID_BUTTON_TOGGLE_PHYSICS, MyFrame::TogglePhysics)
EVT_KEY_DOWN(MyFrame::OnKeyPressed)
EVT_KEY_UP(MyFrame::OnKeyReleased)
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
    if (!wxApp::OnInit())
        return false;

    // create the main application window
    frame = new MyFrame("Minimal wxWidgets App");
    frame->Show(true);

    return true;
}

MyFrame* thisFrame;

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
    wxMenu* fileMenu = new wxMenu;

    // the "About" item should be in the help menu
    wxMenu* helpMenu = new wxMenu;
    helpMenu->Append(Minimal_About, "&About\tF1", "Show about dialog");

    fileMenu->Append(Minimal_Quit, "E&xit\tAlt-X", "Quit this program");

    // now append the freshly created menu to the menu bar...
    wxMenuBar* menuBar = new wxMenuBar();
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
    std::cout << "MyFrame::MyFrame: VK_ERROR_DEVICE_LOST: " << VK_ERROR_DEVICE_LOST << endl;

    // Initialize the main panel that contains every other panel
    mainPanel = new wxPanel(this, -1,
        wxPoint(0, 0),
        wxSize(WX_FRAME_WIDTH, WX_FRAME_HEIGHT));

    tickCount = 0;
    // One onVkTimer every x ticks
    tickSpeed = 3;

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

    static const int INTERVAL = (1000 / 60); // milliseconds
    m_timer = new wxTimer(this, VulkanCanvas_Timer);
    m_timer->Start(INTERVAL);

    leftMouseOnVulkan = false;
    rightMouseOnVulkan = false;

    holdingShift = false;
    physicsOn = false;

    // Equation of the plane that acts as the ground
    //     in the form of: (ax  + by  + cz  = d   )
    groundPlane = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
    planeNormal = glm::vec3(0.0f, 1.0f, 0.0f);
    planePoint = glm::vec3(0.0f, 0.0f, 0.0f);

    //--------------------------------------------------------------------------
    // Lower Left Panel
    //--------------------------------------------------------------------------
    objHierarchyPanel = new wxPanel(mainPanel, wxID_ANY,
        wxPoint(OBJ_HIERARCHY_PANEL_X, OBJ_HIERARCHY_PANEL_Y),
        wxSize(OBJ_HIERARCHY_PANEL_W, OBJ_HIERARCHY_PANEL_H));

    objHierarchy = new SceneTreeCtrl(objHierarchyPanel, Tree_Obj_Hierarchy,
        wxPoint(0, 0),
        wxSize(OBJ_HIERARCHY_PANEL_W, OBJ_HIERARCHY_PANEL_H), wxTR_DEFAULT_STYLE, "Scene");

    vulkanCanvas->Bind(wxEVT_LEFT_DOWN, &MyFrame::OnMouseLeftClick, this);

    //--------------------------------------------------------------------------
    // Upper Left Panel
    //--------------------------------------------------------------------------

    treePanel = new wxPanel(mainPanel, wxID_ANY,
        wxPoint(TREE_PANEL_X, TREE_PANEL_Y),
        wxSize(TREE_PANEL_W, TREE_PANEL_H));

    tree = new MyTreeCtrl(treePanel, TreeTest_Ctrl,
        wxPoint(0, 0),
        wxSize(TREE_PANEL_W, TREE_PANEL_H), wxTR_DEFAULT_STYLE, ROOT_DIR_NAME, ROOT_HOME_FOLDER_NAME, vulkanCanvas);

    //Expands all the nodes
    //tree->ExpandAll();

    //--------------------------------------------------------------------------
    // Upper Right Panel
    //--------------------------------------------------------------------------
    objectInfoPanel = new wxPanel(mainPanel, wxID_ANY,
        wxPoint(BUTTON_LIST_PANEL_X, BUTTON_LIST_PANEL_Y),
        wxSize(BUTTON_LIST_PANEL_W, BUTTON_LIST_PANEL_H));

    // wxTextCrtls, wxStaticTexts for the camera UI
    txtLblCurCameraTarget = new wxStaticText(objectInfoPanel, wxID_ANY, "Camera Target:",
        wxPoint(20, 100), wxSize(BUTTON_LIST_PANEL_W - 2 * 20, 20), wxST_NO_AUTORESIZE);

    txtCtrlCameraLookAtX = new wxTextCtrl(objectInfoPanel, -1, wxT("0.0"),
        wxPoint(40, 120),
        wxSize(BUTTON_LIST_PANEL_W - 65, 20));

    txtCtrlCameraLookAtY = new wxTextCtrl(objectInfoPanel, -1, wxT("0.0"),
        wxPoint(40, 140),
        wxSize(BUTTON_LIST_PANEL_W - 65, 20));

    txtCtrlCameraLookAtZ = new wxTextCtrl(objectInfoPanel, -1, wxT("0.0"),
        wxPoint(40, 160),
        wxSize(BUTTON_LIST_PANEL_W - 65, 20));

    txtLblCameraLookAtX = new wxStaticText(objectInfoPanel, wxID_ANY, "X:",
        wxPoint(25, 122), wxSize(10, 18), wxST_NO_AUTORESIZE);
    txtLblCameraLookAtY = new wxStaticText(objectInfoPanel, wxID_ANY, "Y:",
        wxPoint(25, 142), wxSize(10, 18), wxST_NO_AUTORESIZE);
    txtLblCameraLookAtZ = new wxStaticText(objectInfoPanel, wxID_ANY, "Z:",
        wxPoint(25, 162), wxSize(10, 18), wxST_NO_AUTORESIZE);


    txtLblCurCameraPos = new wxStaticText(objectInfoPanel, wxID_ANY, "Camera Position:",
        wxPoint(20, 20), wxSize(BUTTON_LIST_PANEL_W - 2 * 20, 20), wxST_NO_AUTORESIZE);

    txtCtrlCameraPosX = new wxTextCtrl(objectInfoPanel, -1, wxT("0.0"),
        wxPoint(40, 40),
        wxSize(BUTTON_LIST_PANEL_W - 65, 20));
    txtCtrlCameraPosY = new wxTextCtrl(objectInfoPanel, -1, wxT("0.0"),
        wxPoint(40, 60),
        wxSize(BUTTON_LIST_PANEL_W - 65, 20));
    txtCtrlCameraPosZ = new wxTextCtrl(objectInfoPanel, -1, wxT("0.0"),
        wxPoint(40, 80),
        wxSize(BUTTON_LIST_PANEL_W - 65, 20));

    txtLblCameraPosX = new wxStaticText(objectInfoPanel, wxID_ANY, "X:",
        wxPoint(25, 42), wxSize(10, 18), wxST_NO_AUTORESIZE);
    txtLblCameraPosY = new wxStaticText(objectInfoPanel, wxID_ANY, "Y:",
        wxPoint(25, 62), wxSize(10, 18), wxST_NO_AUTORESIZE);
    txtLblCameraPosZ = new wxStaticText(objectInfoPanel, wxID_ANY, "Z:",
        wxPoint(25, 82), wxSize(10, 18), wxST_NO_AUTORESIZE);

    // Update the text controls showing the position of the camera
    txtCtrlCameraPosX->ChangeValue(wxString::Format(wxT("%f"), 10.0f));
    txtCtrlCameraPosY->ChangeValue(wxString::Format(wxT("%f"), 10.0f));
    txtCtrlCameraPosZ->ChangeValue(wxString::Format(wxT("%f"), 10.0f));

    // Update the text controls showing the position of the camera
    txtCtrlCameraLookAtX->ChangeValue(wxString::Format(wxT("%f"), 0.0f));
    txtCtrlCameraLookAtY->ChangeValue(wxString::Format(wxT("%f"), 0.0f));
    txtCtrlCameraLookAtZ->ChangeValue(wxString::Format(wxT("%f"), 0.0f));

    // wxTextCrtls, wxButtons, wxStaticTexts for the current object UI

    // Positions
    txtCtrlPositionX = new wxTextCtrl(objectInfoPanel, -1, wxT("0.0"),
        wxPoint(40, 260),
        wxSize(BUTTON_LIST_PANEL_W - 65, 20));

    txtCtrlPositionY = new wxTextCtrl(objectInfoPanel, -1, wxT("0.0"),
        wxPoint(40, 280),
        wxSize(BUTTON_LIST_PANEL_W - 65, 20));

    txtCtrlPositionZ = new wxTextCtrl(objectInfoPanel, -1, wxT("0.0"),
        wxPoint(40, 300),
        wxSize(BUTTON_LIST_PANEL_W - 65, 20));

    txtLblPositionX = new wxStaticText(objectInfoPanel, wxID_ANY, "X:",
        wxPoint(25, 262), wxSize(10, 18), wxST_NO_AUTORESIZE);
    txtLblPositionY = new wxStaticText(objectInfoPanel, wxID_ANY, "Y:",
        wxPoint(25, 282), wxSize(10, 18), wxST_NO_AUTORESIZE);
    txtLblPositionZ = new wxStaticText(objectInfoPanel, wxID_ANY, "Z:",
        wxPoint(25, 302), wxSize(10, 18), wxST_NO_AUTORESIZE);

    // Rotations
    txtCtrlRotationX = new wxTextCtrl(objectInfoPanel, -1, wxT("0.0"),
        wxPoint(40, 340),
        wxSize(BUTTON_LIST_PANEL_W - 65, 20));

    txtCtrlRotationY = new wxTextCtrl(objectInfoPanel, -1, wxT("0.0"),
        wxPoint(40, 360),
        wxSize(BUTTON_LIST_PANEL_W - 65, 20));

    txtCtrlRotationZ = new wxTextCtrl(objectInfoPanel, -1, wxT("0.0"),
        wxPoint(40, 380),
        wxSize(BUTTON_LIST_PANEL_W - 65, 20));

    txtLblRotationX = new wxStaticText(objectInfoPanel, wxID_ANY, "X:",
        wxPoint(25, 342), wxSize(10, 18), wxST_NO_AUTORESIZE);
    txtLblRotationY = new wxStaticText(objectInfoPanel, wxID_ANY, "Y:",
        wxPoint(25, 362), wxSize(10, 18), wxST_NO_AUTORESIZE);
    txtLblRotationZ = new wxStaticText(objectInfoPanel, wxID_ANY, "Z:",
        wxPoint(25, 382), wxSize(10, 18), wxST_NO_AUTORESIZE);

    txtLblCurObject = new wxStaticText(objectInfoPanel, wxID_ANY, "Object: None selected",
        wxPoint(20, 220), wxSize(BUTTON_LIST_PANEL_W - 2 * 20, 20), wxST_NO_AUTORESIZE);
    txtLblCurObjectPos = new wxStaticText(objectInfoPanel, wxID_ANY, "Position:",
        wxPoint(20, 240), wxSize(BUTTON_LIST_PANEL_W - 2 * 20, 20), wxST_NO_AUTORESIZE);
    txtLblCurObjectRot = new wxStaticText(objectInfoPanel, wxID_ANY, "Rotation:",
        wxPoint(20, 320), wxSize(BUTTON_LIST_PANEL_W - 2 * 20, 20), wxST_NO_AUTORESIZE);


    txtLblVelocity = new wxStaticText(objectInfoPanel, wxID_ANY, "Velocity:",
        wxPoint(20, 400), wxSize((BUTTON_LIST_PANEL_W - 40) / 2, 20), wxST_NO_AUTORESIZE);
    txtLblVelocityX = new wxStaticText(objectInfoPanel, wxID_ANY, "X:",
        wxPoint(25, 422), wxSize(10, 18), wxST_NO_AUTORESIZE);
    txtLblVelocityY = new wxStaticText(objectInfoPanel, wxID_ANY, "Y:",
        wxPoint(25, 442), wxSize(10, 18), wxST_NO_AUTORESIZE);
    txtLblVelocityZ = new wxStaticText(objectInfoPanel, wxID_ANY, "Z:",
        wxPoint(25, 462), wxSize(10, 18), wxST_NO_AUTORESIZE);

    txtCtrlVelocityX = new wxTextCtrl(objectInfoPanel, -1, wxT("0.0"),
        wxPoint(40, 420),
        wxSize((BUTTON_LIST_PANEL_W - 90) / 2, 20));
    txtCtrlVelocityY = new wxTextCtrl(objectInfoPanel, -1, wxT("0.0"),
        wxPoint(40, 440),
        wxSize((BUTTON_LIST_PANEL_W - 90) / 2, 20));
    txtCtrlVelocityZ = new wxTextCtrl(objectInfoPanel, -1, wxT("0.0"),
        wxPoint(40, 460),
        wxSize((BUTTON_LIST_PANEL_W - 90) / 2, 20));


    txtLblAcceleration = new wxStaticText(objectInfoPanel, wxID_ANY, "Angular Velocity:",
        wxPoint(20 + (BUTTON_LIST_PANEL_W - 40) / 2, 400), wxSize((BUTTON_LIST_PANEL_W - 40) / 2, 20), wxST_NO_AUTORESIZE);
    txtLblAngularVelocityX = new wxStaticText(objectInfoPanel, wxID_ANY, "X:",
        wxPoint(25 + (BUTTON_LIST_PANEL_W - 40) / 2, 422), wxSize(10, 18), wxST_NO_AUTORESIZE);
    txtLblAngularVelocityY = new wxStaticText(objectInfoPanel, wxID_ANY, "Y:",
        wxPoint(25 + (BUTTON_LIST_PANEL_W - 40) / 2, 442), wxSize(10, 18), wxST_NO_AUTORESIZE);
    txtLblAngularVelocityZ = new wxStaticText(objectInfoPanel, wxID_ANY, "Z:",
        wxPoint(25 + (BUTTON_LIST_PANEL_W - 40) / 2, 462), wxSize(10, 18), wxST_NO_AUTORESIZE);

    txtCtrlAngularVelocityX = new wxTextCtrl(objectInfoPanel, -1, wxT("0.0"),
        wxPoint(40 + (BUTTON_LIST_PANEL_W - 40) / 2, 420),
        wxSize((BUTTON_LIST_PANEL_W - 90) / 2, 20));
    txtCtrlAngularVelocityY = new wxTextCtrl(objectInfoPanel, -1, wxT("0.0"),
        wxPoint(40 + (BUTTON_LIST_PANEL_W - 40) / 2, 440),
        wxSize((BUTTON_LIST_PANEL_W - 90) / 2, 20));
    txtCtrlAngularVelocityZ = new wxTextCtrl(objectInfoPanel, -1, wxT("0.0"),
        wxPoint(40 + (BUTTON_LIST_PANEL_W - 40) / 2, 460),
        wxSize((BUTTON_LIST_PANEL_W - 90) / 2, 20));

    txtLblGravity = new wxStaticText(objectInfoPanel, wxID_ANY, "Gravity:",
        wxPoint(25, 482), wxSize(100, 18), wxST_NO_AUTORESIZE);
    txtCtrlGravity = new wxTextCtrl(objectInfoPanel, -1, wxT("0.0"),
        wxPoint(40, 500),
        wxSize((BUTTON_LIST_PANEL_W - 90), 20));

    //hideObjectCheckBox = new wxCheckBox(objectInfoPanel, wxID_ANY, "Hide",
    //    wxPoint(25, 340), wxSize(60, 20), wxST_NO_AUTORESIZE);
    hideObjectButton = new wxButton(objectInfoPanel, ID_BUTTON_HIDE_OBJECT, "Hide",
        wxPoint(25, 560), wxSize((BUTTON_LIST_PANEL_W - 50) / 2, 20));
    showObjectButton = new wxButton(objectInfoPanel, ID_BUTTON_SHOW_OBJECT, "Show",
        wxPoint(25 + (BUTTON_LIST_PANEL_W - 50) / 2, 560), wxSize((BUTTON_LIST_PANEL_W - 50) / 2, 20));

    resetCameraButton = new wxButton(objectInfoPanel, ID_BUTTON_RESET_CAMERA, "Reset",
        wxPoint(25, 240), wxSize((BUTTON_LIST_PANEL_W - 50), 20));
    showObjectButton = new wxButton(objectInfoPanel, ID_BUTTON_RESET_OBJECT, "Reset",
        wxPoint(25, 540), wxSize((BUTTON_LIST_PANEL_W - 50), 20));
    unselectObjectButton = new wxButton(objectInfoPanel, ID_BUTTON_UNSELECT_OBJECT, "Unselect Object",
        wxPoint(25, 580), wxSize((BUTTON_LIST_PANEL_W - 50), 20));

    txtLblPhysics = new wxStaticText(objectInfoPanel, wxID_ANY, "Physics: Off",
        wxPoint(20, 620), wxSize(BUTTON_LIST_PANEL_W - 2 * 20, 20), wxST_NO_AUTORESIZE);
    togglePhysicsButton = new wxButton(objectInfoPanel, ID_BUTTON_TOGGLE_PHYSICS, "Turn physics on",
        wxPoint(25, 640), wxSize((BUTTON_LIST_PANEL_W - 50), 20));
}

MyFrame::~MyFrame() {
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
#define PANNING_SPEED_MULTIPLIER 50
#define ROTATING_SPEED_X (2 * M_PI / VULKAN_PANEL_VULKAN_CANVAS_W) // a movement from left to right = 2*PI = 360 deg
#define ROTATING_SPEED_Y (M_PI / VULKAN_PANEL_VULKAN_CANVAS_H)  // a movement from top to bottom = PI = 180 deg
#define ZOOM_SPEED_MULTIPLIER 2

// When the user stops left clicking, stop the panning motion of the camera
void MyFrame::OnMouseLeftClickEnd(wxMouseEvent& ev) {
    //printf("MyFrame::OnMouseLeftClickEnd()\n");

    leftMouseOnVulkan = false;
}

// WHen the user stops right clicking, stop the rotational motion of hte camera
void MyFrame::OnMouseRightClickEnd(wxMouseEvent& ev) {
    //printf("MyFrame::OnMouseRightClickEnd()\n");

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

        camera_pos.x = vulkanCanvas->cameraX;
        camera_pos.y = vulkanCanvas->cameraY;
        camera_pos.z = vulkanCanvas->cameraZ;

        glm::vec3 upVector = glm::vec3(0.0f, 1.0f, 0.0f);

        // Applying the inverse of the transformation that takes world to eye coordinates
        glm::vec4 ray_wor_in_vec4 = glm::inverse(glm::lookAt(camera_pos, targetPoint, upVector)) * ray_eye;
        glm::vec3 ray_wor = glm::vec3(ray_wor_in_vec4.x, ray_wor_in_vec4.y, ray_wor_in_vec4.z);
        ray_wor = glm::normalize(ray_wor);

        // Create a normal vector, which is determined by the direction our camera is pointing
        glm::vec3 normVector = camera_pos - targetPoint;
        normVector = glm::normalize(normVector);

        // Takes the previous mouse pointing ray then subtracts it by the current mouse pointing ray
        glm::vec3 dirToMove = vulkanCanvas->mouseDragRay - ray_wor;

        // Sets the previous mouse pointing ray to the current one since this needs to be updated and wont be used for the rest of this function
        vulkanCanvas->mouseDragRay = ray_wor;

        // Get the projection of dirToMove onto the plane described by the normal vector
        glm::vec3 projVec = dirToMove - dot(dirToMove, normVector) * normVector;
        projVec = glm::normalize(projVec);

        //std::cout << "x: " << projVec.x;
        //std::cout << " y: " << projVec.y;
        //std::cout << " z: " << projVec.z << endl;

        vulkanCanvas->cameraX = vulkanCanvas->cameraX + (projVec.x) / PANNING_SPEED_MULTIPLIER;
        vulkanCanvas->cameraY = vulkanCanvas->cameraY + (projVec.y) / PANNING_SPEED_MULTIPLIER;
        vulkanCanvas->cameraZ = vulkanCanvas->cameraZ + (projVec.z) / PANNING_SPEED_MULTIPLIER;

        vulkanCanvas->viewingX = vulkanCanvas->viewingX + (projVec.x) / PANNING_SPEED_MULTIPLIER;
        vulkanCanvas->viewingY = vulkanCanvas->viewingY + (projVec.y) / PANNING_SPEED_MULTIPLIER;
        vulkanCanvas->viewingZ = vulkanCanvas->viewingZ + (projVec.z) / PANNING_SPEED_MULTIPLIER;

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
    //printf("MyFrame::OnMouseLeftClick()\n");

    wxPoint mouse_position = ev.GetPosition();

    startPosForMouseDrag.x = mouse_position.x;
    startPosForMouseDrag.y = mouse_position.y;

    //cout << "MyFrame::OnMouseLeftClick(): MOUSE POSITION X: " << mouse_position.x << endl;
    //cout << "MyFrame::OnMouseLeftClick(): MOUSE POSITION Y: " << mouse_position.y << endl;

    // Check if the mouse is inside the vulkan panel
    if (mouse_position.x >= 0 &&
        mouse_position.x < VULKAN_PANEL_W &&
        mouse_position.y >= 0 &&
        mouse_position.y < VULKAN_PANEL_H) {

        if (!rightMouseOnVulkan && !holdingShift) {
            // Initiate camera panning
            leftMouseOnVulkan = true;

            initiateCameraDrag(mouse_position);
        }
        else if (!rightMouseOnVulkan && holdingShift) {
            // Initiate object selection
            initiateObjectSelect(mouse_position);
        }
    }
}

// User to start camera rotation
void MyFrame::OnMouseRightClick(wxMouseEvent& ev) {
    printf("MyFrame::OnMouseRightClick()\n");

    wxPoint mouse_position = ev.GetPosition();

    startPosForRightMouseDrag.x = mouse_position.x;
    startPosForRightMouseDrag.y = mouse_position.y;

    std::cout << "MyFrame::OnMouseRightClick(): MOUSE POSITION X: " << mouse_position.x << endl;
    std::cout << "MyFrame::OnMouseRightClick(): MOUSE POSITION Y: " << mouse_position.y << endl;

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

    //cout << "direction: " << direction << endl;

    // Make sure that the camera is not currently rotating or panning
    if (!rightMouseOnVulkan && !leftMouseOnVulkan) {
        if (direction > 0) {
            direction = 1;
        }
        else {
            direction = -1;
        }

        vulkanCanvas->cameraDirection.x = vulkanCanvas->cameraX - vulkanCanvas->viewingX;
        vulkanCanvas->cameraDirection.y = vulkanCanvas->cameraY - vulkanCanvas->viewingY;
        vulkanCanvas->cameraDirection.z = vulkanCanvas->cameraZ - vulkanCanvas->viewingZ;

        vulkanCanvas->cameraDirection = glm::normalize(vulkanCanvas->cameraDirection);

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
    if (objHierarchy->selectedObject != -1) {
        vulkanCanvas->m_objects.at(NUM_STARTING_OBJECTS + objHierarchy->selectedObject)->drawObject = false;
    }
}

// For pressing show button in the UI
void MyFrame::OnShowLeftClick(wxCommandEvent& event) {
    if (objHierarchy->selectedObject != -1) {
        vulkanCanvas->m_objects.at(NUM_STARTING_OBJECTS + objHierarchy->selectedObject)->drawObject = true;
    }
}

// Reset the camera to default settings
void MyFrame::OnResetCameraLeftClick(wxCommandEvent& event) {
    vulkanCanvas->initCamera();
    updateCameraTextCtrls();
}

void MyFrame::OnResetObjectLeftClick(wxCommandEvent& event) {
    if (objHierarchy->selectedObject != -1) {
        vulkanCanvas->initObject(objHierarchy->selectedObject + NUM_STARTING_OBJECTS);
        updateObjTextCtrls();
        vulkanCanvas->m_objects.at(NUM_STARTING_OBJECTS + objHierarchy->selectedObject)->drawObject = true;
    }
}

void MyFrame::OnUnselectObjectLeftClick(wxCommandEvent& event) {
    if (objHierarchy->selectedObject != -1) {
        objHierarchy->selectedObject = -1;
        objHierarchy->prevSelectedObject = -1;
    }
}

void MyFrame::TogglePhysics(wxCommandEvent& event) {
    if (physicsOn) {
        txtLblPhysics->SetLabel("Physics: Off");
        togglePhysicsButton->SetLabel("Turn physics on");
    }
    else {
        txtLblPhysics->SetLabel("Physics: On");
        togglePhysicsButton->SetLabel("Turn physics off");

        // Reset whether or not the object has finished falling
        for (int i = NUM_STARTING_OBJECTS; i < vulkanCanvas->m_objects.size(); i++) {
            vulkanCanvas->m_objects.at(i)->verticesOnGround.clear();
        }
    }
    physicsOn = !physicsOn;
}

void MyFrame::getAllInfoFromFolder(vector<string>& names, vector<bool>& isFolder, string path, int* totalChunks, int* totalFiles, int* totalFolders, long* totalSize) {
    DIR* dir;
    struct dirent* ent;
    dir = opendir(path.c_str());
    while ((ent = readdir(dir)) != NULL) {
        if (ent->d_name[0] != '.') {
            (*totalChunks)++;
            string curItemFullPath = path + "/";
            curItemFullPath += ent->d_name;
            names.push_back(curItemFullPath);
            std::cout << "MyFrame::getAllInfoFromFolder curItemFullPath:" << curItemFullPath.c_str() << endl;
            if (ent->d_type != DT_DIR) {
                (*totalFiles)++;
                isFolder.push_back(false);
                (*totalSize) += getFileSize(curItemFullPath.c_str());
                std::cout << "MyFrame::getAllInfoFromFolder getFileSize(curItemFullPath.c_str()):" << getFileSize(curItemFullPath.c_str()) << endl;
            }
            else {
                (*totalFolders)++;
                isFolder.push_back(true);
                getAllInfoFromFolder(names, isFolder, curItemFullPath, totalChunks, totalFiles, totalFolders, totalSize);
            }
        }
    }
}

// Key codes and corresponding keys here:
#define KEY_CODE_SHIFT 306

void MyFrame::OnKeyPressed(wxKeyEvent& event) {
    //int keyCode = event.GetKeyCode();
    //cout << "MyFrame::OnKeyPressed(): keyCode: " << keyCode << endl;
    //if (keyCode == KEY_CODE_SHIFT) {
    //    holdingShift = true;
    //}
    //cout << event.GetKeyCode() << endl;
}

void MyFrame::OnKeyReleased(wxKeyEvent& event) {
    //int keyCode = event.GetKeyCode();
    //if (keyCode == KEY_CODE_SHIFT) {
    //    holdingShift = false;
    //}
}

void MyFrame::updateObjTextCtrls() {
    GameObject* curObj = vulkanCanvas->m_objects.at(NUM_STARTING_OBJECTS + objHierarchy->selectedObject);

    txtCtrlPositionX->ChangeValue(wxString::Format(wxT("%f"), (curObj->offset3D.x)));
    txtCtrlPositionY->ChangeValue(wxString::Format(wxT("%f"), (curObj->offset3D.y)));
    txtCtrlPositionZ->ChangeValue(wxString::Format(wxT("%f"), (curObj->offset3D.z)));

    txtCtrlRotationX->ChangeValue(wxString::Format(wxT("%f"), (curObj->offset3DRot.x)));
    txtCtrlRotationY->ChangeValue(wxString::Format(wxT("%f"), (curObj->offset3DRot.y)));
    txtCtrlRotationZ->ChangeValue(wxString::Format(wxT("%f"), (curObj->offset3DRot.z)));

    txtCtrlVelocityX->ChangeValue(wxString::Format(wxT("%f"), (curObj->translationVelocity.x)));
    txtCtrlVelocityY->ChangeValue(wxString::Format(wxT("%f"), (curObj->translationVelocity.y)));
    txtCtrlVelocityZ->ChangeValue(wxString::Format(wxT("%f"), (curObj->translationVelocity.z)));

    txtCtrlAngularVelocityX->ChangeValue(wxString::Format(wxT("%f"), (curObj->angularVelocity.x)));
    txtCtrlAngularVelocityY->ChangeValue(wxString::Format(wxT("%f"), (curObj->angularVelocity.y)));
    txtCtrlAngularVelocityZ->ChangeValue(wxString::Format(wxT("%f"), (curObj->angularVelocity.z)));

    txtCtrlGravity->ChangeValue(wxString::Format(wxT("%f"), curObj->acceleration.y));
}

void MyFrame::updateCameraTextCtrls() {
    txtCtrlCameraPosX->ChangeValue(wxString::Format(wxT("%f"), (vulkanCanvas->cameraX)));
    txtCtrlCameraPosY->ChangeValue(wxString::Format(wxT("%f"), (vulkanCanvas->cameraY)));
    txtCtrlCameraPosZ->ChangeValue(wxString::Format(wxT("%f"), (vulkanCanvas->cameraZ)));

    txtCtrlCameraLookAtX->ChangeValue(wxString::Format(wxT("%f"), (vulkanCanvas->viewingX)));
    txtCtrlCameraLookAtY->ChangeValue(wxString::Format(wxT("%f"), (vulkanCanvas->viewingY)));
    txtCtrlCameraLookAtZ->ChangeValue(wxString::Format(wxT("%f"), (vulkanCanvas->viewingZ)));
}

void MyFrame::initiateObjectSelect(wxPoint mouse_position) {
    if (vulkanCanvas->m_objects.size() - NUM_STARTING_OBJECTS == 0) {
        return;
    }
    // Get a normalized vector of where the mouse is currently pointing
    vulkanCanvas->cameraDirection.x = -vulkanCanvas->cameraX + vulkanCanvas->viewingX;
    vulkanCanvas->cameraDirection.y = -vulkanCanvas->cameraY + vulkanCanvas->viewingY;
    vulkanCanvas->cameraDirection.z = -vulkanCanvas->cameraZ + vulkanCanvas->viewingZ;

    glm::vec3 rayVector = returnWorldRay(mouse_position);
    glm::vec3 rayOrigin = glm::vec3(vulkanCanvas->cameraX, vulkanCanvas->cameraY, vulkanCanvas->cameraZ);

    std::vector<float> distances;
    std::vector<int> indices;

    for (int i = NUM_STARTING_OBJECTS; i < vulkanCanvas->m_objects.size(); i++) {
        if (vulkanCanvas->m_objects.at(i)->drawObject) {
            if (vulkanCanvas->m_objects[i]->isObjFile()) {
                MyObject* curObj = (MyObject*)vulkanCanvas->m_objects[i];

                // The vertices in curObj dont have the transformations found in UpdateUniformBuffers() applied to them, so we should manually transform them
                std::vector<glm::vec3> transformedVertices;

                // Create the rotation matrix for the transformed vertices
                glm::mat4 rotMat = glm::toMat4(curObj->quaternion);
                rotMat = rotMat * glm::mat4(1.0f);

                // -------------------------------
                // Go through bounding box only
                // -------------------------------

                for (int j = 0; j < curObj->boundingBox.size(); j++) {
                    // Get a copy of the base vertex with no transformations
                    glm::vec4 tempVector = glm::vec4(curObj->boundingBox.at(j).x, curObj->boundingBox.at(j).y, curObj->boundingBox.at(j).z, 0);

                    // First rotate the object
                    tempVector = rotMat * tempVector;

                    // Then translate
                    tempVector.x += curObj->offset3D.x;
                    tempVector.y += curObj->offset3D.y;
                    tempVector.z += curObj->offset3D.z;

                    transformedVertices.push_back(tempVector);
                }
                for (int j = 0; j < curObj->boundingBoxIndices.size(); j += 3) {
                    // Use indices to grab respective vertices
                    glm::vec3 vertex1 = transformedVertices.at(curObj->boundingBoxIndices[j]);
                    glm::vec3 vertex2 = transformedVertices.at(curObj->boundingBoxIndices[j + 1]);
                    glm::vec3 vertex3 = transformedVertices.at(curObj->boundingBoxIndices[j + 2]);

                    ObjSelectInfo response = rayIntersectTriangle(rayOrigin, rayVector, vertex1, vertex2, vertex3);
                    if (response.success) {
                        distances.push_back(response.distance);
                        indices.push_back(i);
                    }
                }
            }
            if (vulkanCanvas->m_objects[i]->isMD5File()) { // Currently treats the MD5Object as base T-Pose object
                MyMD5Object* curMD5Obj = (MyMD5Object*)vulkanCanvas->m_objects[i];
                MD5Model* md5Model = curMD5Obj->md5Model;
                MD5Model::MeshList meshList = md5Model->m_Meshes;

                // Go through for each body part of the md5 model
                for (int k = 0; k < md5Model->m_iNumMeshes; k++) {
                    MD5Model::Mesh curMesh = meshList.at(k);
                    MD5Model::VertexList vertices = curMesh.m_Verts;
                    MD5Model::TriangleList triangles = curMesh.m_Tris;

                    // The vertices in curObj dont have the transformations found in UpdateUniformBuffers() applied to them, so we should manually transform them
                    std::vector<glm::vec3> transformedVertices;

                    // -------------------------------
                    // Go through each vertex
                    // -------------------------------

                    for (int j = 0; j < vertices.size(); j++) {
                        // Get a copy of the base vertex with no transformations
                        glm::vec4 tempVector = glm::vec4(vertices.at(j).m_Pos.x, vertices.at(j).m_Pos.y, vertices.at(j).m_Pos.z, 0);

                        // First rotate the object
                        glm::mat4 rotMat;
                        rotMat = glm::rotate(glm::mat4(1.0f), glm::radians(curMD5Obj->offset3DRot.x), glm::vec3(1.0f, 0.0f, 0.0f));
                        rotMat = glm::rotate(rotMat, glm::radians(curMD5Obj->offset3DRot.y), glm::vec3(0.0f, 1.0f, 0.0f));
                        rotMat = glm::rotate(rotMat, glm::radians(curMD5Obj->offset3DRot.z), glm::vec3(0.0f, 0.0f, 1.0f));
                        tempVector = rotMat * tempVector;

                        tempVector.x += curMD5Obj->offset3D.x;
                        tempVector.y += curMD5Obj->offset3D.y;
                        tempVector.z += curMD5Obj->offset3D.z;

                        transformedVertices.push_back(tempVector);
                    }

                    // Go through each triangle
                    for (int j = 0; j < triangles.size(); j++) {
                        // Use indices to grab respective vertices
                        glm::vec3 vertex1 = transformedVertices.at(triangles.at(j).m_Indices[0]);
                        glm::vec3 vertex2 = transformedVertices.at(triangles.at(j).m_Indices[1]);
                        glm::vec3 vertex3 = transformedVertices.at(triangles.at(j).m_Indices[2]);

                        ObjSelectInfo response = rayIntersectTriangle(rayOrigin, rayVector, vertex1, vertex2, vertex3);
                        if (response.success) {
                            distances.push_back(response.distance);
                            indices.push_back(i);
                        }
                    }
                }
            }
        }
    }
    if (distances.size() > 0) {
        int closestObjIndex = indices.at(std::distance(std::begin(distances), std::min_element(std::begin(distances), std::end(distances)))) - NUM_STARTING_OBJECTS;
        objHierarchy->selectedObject = closestObjIndex;
    }

}

// Using Möller–Trumbore intersection algorithm described in:
// https://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm
ObjSelectInfo MyFrame::rayIntersectTriangle(glm::vec3 rayOrigin, glm::vec3 rayVector, glm::vec3 vertex1, glm::vec3 vertex2, glm::vec3 vertex3) {
    ObjSelectInfo response;

    constexpr float epsilon = std::numeric_limits<float>::epsilon();

    glm::vec3 edge1 = vertex2 - vertex1;
    glm::vec3 edge2 = vertex3 - vertex1;

    glm::vec3 ray_cross_e2 = glm::cross(rayVector, edge2);

    float det = glm::dot(edge1, ray_cross_e2);

    if (det > -epsilon && det < epsilon) {
        response.success = false;
        return response; // Ray and triangle are parallel
    }

    float inv_det = 1.0 / det;
    glm::vec3 s = rayOrigin - vertex1;
    float u = inv_det * glm::dot(s, ray_cross_e2);

    if (u < 0 || u > 1) {
        response.success = false;
        return response;
    }

    glm::vec3 s_cross_e1 = glm::cross(s, edge1);
    float v = inv_det * glm::dot(rayVector, s_cross_e1);

    if (v < 0 || u + v > 1) {
        response.success = false;
        return response;
    }

    float t = inv_det * glm::dot(edge2, s_cross_e1);

    if (t > epsilon) { // Ray intersection
        response.success = true;

        // Find the closest point to the triangle from the ray origin
        glm::vec3 closestPoint1 = glm::closestPointOnLine(rayOrigin, vertex1, vertex2);
        glm::vec3 closestPoint2 = glm::closestPointOnLine(rayOrigin, vertex1, vertex3);
        glm::vec3 closestPoint3 = glm::closestPointOnLine(rayOrigin, vertex2, vertex3);
        response.distance = sqrt(glm::dot(closestPoint1 - rayOrigin, closestPoint1 - rayOrigin));
        if (response.distance > sqrt(glm::dot(closestPoint2 - rayOrigin, closestPoint2 - rayOrigin))) {
            response.distance = sqrt(glm::dot(closestPoint2 - rayOrigin, closestPoint2 - rayOrigin));
        }
        if (response.distance > sqrt(glm::dot(closestPoint3 - rayOrigin, closestPoint3 - rayOrigin))) {
            response.distance = sqrt(glm::dot(closestPoint3 - rayOrigin, closestPoint3 - rayOrigin));
        }

        return response;
    }
    else { // Line intersection but no ray intersection
        response.success = false;
        return response;
    }
}


void MyFrame::initiateCameraDrag(wxPoint mouse_position) {
    vulkanCanvas->cameraXBeforeDrag = vulkanCanvas->cameraX;
    vulkanCanvas->cameraYBeforeDrag = vulkanCanvas->cameraY;
    vulkanCanvas->cameraZBeforeDrag = vulkanCanvas->cameraZ;

    vulkanCanvas->viewingXBeforeDrag = vulkanCanvas->viewingX;
    vulkanCanvas->viewingYBeforeDrag = vulkanCanvas->viewingY;
    vulkanCanvas->viewingZBeforeDrag = vulkanCanvas->viewingZ;

    vulkanCanvas->cameraDirection.x = -vulkanCanvas->cameraX + vulkanCanvas->viewingX;
    vulkanCanvas->cameraDirection.y = -vulkanCanvas->cameraY + vulkanCanvas->viewingY;
    vulkanCanvas->cameraDirection.z = -vulkanCanvas->cameraZ + vulkanCanvas->viewingZ;

    vulkanCanvas->cameraDirection = glm::normalize(vulkanCanvas->cameraDirection);

    vulkanCanvas->mouseDragRay = returnWorldRay(mouse_position);
}

// Given localized mouse coordinates, return the ray in 3d world space :)
glm::vec3 MyFrame::returnWorldRay(wxPoint mouse_position) {
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

    return ray_wor;
}

// Acts as an update function
void MyFrame::OnVkTimer(wxTimerEvent& event) {
    // Uncomment if you want to use delays

    //tickCount++;
    //if (tickCount % tickSpeed != 0) {
    //    return;
    //}

    //----------------------------------------------------------------------------------------------------
    // Check if there is a new selected item in the object hierarchy, then change the text controls in the 
    //     upper right panel to reflect the information on the new object that has been selected
    //----------------------------------------------------------------------------------------------------

    // Update offset3D for position
    wxString objPosXInWxStr = txtCtrlPositionX->GetValue();
    wxString objPosYInWxStr = txtCtrlPositionY->GetValue();
    wxString objPosZInWxStr = txtCtrlPositionZ->GetValue();

    double objPosX = 0.0;
    double objPosY = 0.0;
    double objPosZ = 0.0;

    objPosXInWxStr.ToDouble(&objPosX);
    objPosYInWxStr.ToDouble(&objPosY);
    objPosZInWxStr.ToDouble(&objPosZ);

    // Update offset3D for rotation
    wxString objRotXInWxStr = txtCtrlRotationX->GetValue();
    wxString objRotYInWxStr = txtCtrlRotationY->GetValue();
    wxString objRotZInWxStr = txtCtrlRotationZ->GetValue();

    double objRotX = 0.0;
    double objRotY = 0.0;
    double objRotZ = 0.0;

    objRotXInWxStr.ToDouble(&objRotX);
    objRotYInWxStr.ToDouble(&objRotY);
    objRotZInWxStr.ToDouble(&objRotZ);

    // Update velocity of object
    wxString objVelXInWxStr = txtCtrlVelocityX->GetValue();
    wxString objVelYInWxStr = txtCtrlVelocityY->GetValue();
    wxString objVelZInWxStr = txtCtrlVelocityZ->GetValue();

    double objVelX = 0.0;
    double objVelY = 0.0;
    double objVelZ = 0.0;

    objVelXInWxStr.ToDouble(&objVelX);
    objVelYInWxStr.ToDouble(&objVelY);
    objVelZInWxStr.ToDouble(&objVelZ);

    // Update acceleration of object
    wxString objAngVelXInWxStr = txtCtrlAngularVelocityX->GetValue();
    wxString objAngVelYInWxStr = txtCtrlAngularVelocityY->GetValue();
    wxString objAngVelZInWxStr = txtCtrlAngularVelocityZ->GetValue();

    double objAngVelX = 0.0;
    double objAngVelY = 0.0;
    double objAngVelZ = 0.0;

    objAngVelXInWxStr.ToDouble(&objAngVelX);
    objAngVelYInWxStr.ToDouble(&objAngVelY);
    objAngVelZInWxStr.ToDouble(&objAngVelZ);

    // Update acceleration of object
    wxString objGravityInWxStr = txtCtrlGravity->GetValue();
    double objGravity = 0.0;
    objGravityInWxStr.ToDouble(&objGravity);

    // If the selected object is in the object hierarchy, and is not the 'Scene' root object
    if (vulkanCanvas->m_objects.size() - NUM_STARTING_OBJECTS > 0 && objHierarchy->selectedObject != -1) {
        if (objHierarchy->selectedObject != objHierarchy->prevSelectedObject) { // If there has been a new selected object, update the text controls to contain the new object's position
            updateObjTextCtrls();

            objHierarchy->prevSelectedObject = objHierarchy->selectedObject;
        }
        else { // Otherwise, update the current selected object's properties to whatever is inside the text controls
            GameObject* curObj = vulkanCanvas->m_objects.at(NUM_STARTING_OBJECTS + objHierarchy->selectedObject);
            curObj->offset3D = glm::vec3(
                (float)objPosX,
                (float)objPosY,
                (float)objPosZ);

            curObj->offset3DRot = glm::vec3(
                (float)objRotX,
                (float)objRotY,
                (float)objRotZ);

            curObj->translationVelocity = glm::vec3(
                (float)objVelX,
                (float)objVelY,
                (float)objVelZ);

            curObj->angularVelocity = glm::vec3(
                (float)objAngVelX,
                (float)objAngVelY,
                (float)objAngVelZ);

            glm::vec3 rot = curObj->offset3DRot;

            glm::quat quaternionX = glm::angleAxis(glm::radians(rot.x), glm::vec3(1.0f, 0.0f, 0.0f));
            glm::quat quaternionY = glm::angleAxis(glm::radians(rot.y), glm::vec3(0.0f, 1.0f, 0.0f));
            glm::quat quaternionZ = glm::angleAxis(glm::radians(rot.z), glm::vec3(0.0f, 0.0f, 1.0f));

            glm::quat combinedQuaternion = quaternionZ * quaternionY * quaternionX;

            curObj->quaternion = combinedQuaternion;

            // Update some hidden object properties that correspond to the shown object properties
            curObj->acceleration = glm::vec3(0.0f, (float)objGravity, 0.0f);
        }
        txtLblCurObject->SetLabel("Object: " + vulkanCanvas->m_objects.at(NUM_STARTING_OBJECTS + objHierarchy->selectedObject)->sceneTreeName);
    }
    else if (objHierarchy->selectedObject == -1) {
        txtLblCurObject->SetLabel("Object: None selected");
    }

    //----------------------------------------------------------------------------------------------------
    // Update camera coordinates and lookat coordinates
    //----------------------------------------------------------------------------------------------------

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

    //----------------------------------------------------------------------------------------------------
    // Use this code to check if shift key is pressed
    // I am too lazy to resolve the issue of EVT_KEY_DOWN not being responsive to key changes
    if (wxGetKeyState((wxKeyCode)KEY_CODE_SHIFT)) {
        holdingShift = true;
    }
    else {
        holdingShift = false;
    }

    // If physics is on, call the physics function:
    if (physicsOn) {
        handlePhysics();
    }
}


// ----------------------------------------------------------------------------------------------------------
// PHYSICS HANDLING
// ----------------------------------------------------------------------------------------------------------

// All of the code that handles physics (besides helper functions) reside in this function
void MyFrame::handlePhysics() {
    for (int i = NUM_STARTING_OBJECTS; i < vulkanCanvas->m_objects.size(); i++) {
        if (vulkanCanvas->m_objects.at(i)->drawObject) {
            if (vulkanCanvas->m_objects[i]->isObjFile()) {
                MyObject* curObj = (MyObject*)vulkanCanvas->m_objects[i];

                // If there is non negligible angular velocity, rotate the object
                // Due to the fact that we rotate the object no matter what, there may be issues with an object clipping into the ground when being pushed down by another object
                //     This is because the correctFallingMotion function only takes into account the translation velocity when finding out how much the object needs to be pushed back upwards
                if (glm::length(curObj->angularVelocity) >= 0.0001f) {
                    // The amount of rotation is determined by the length of the angular velocity
                    float rotationAmount = glm::length(curObj->angularVelocity);

                    // Rotate the object around the axis that the angular velocity is on
                    glm::quat quaternionRotation = glm::angleAxis(rotationAmount, glm::normalize(curObj->angularVelocity));
                    curObj->quaternion = quaternionRotation * curObj->quaternion;
                    curObj->offset3DRot = glm::degrees(glm::eulerAngles(curObj->quaternion));
                }
                else {
                    curObj->angularVelocity *= 0.0f;
                }

                // Update the velocity of the object
                curObj->translationVelocity += curObj->acceleration;

                // Remove any negligible velocities for smoother (yet in some cases inaccurate) looking collisions
                // This is usually useful to prevent a case where two objects are stacked and make tiny collisions that slowly make the objects fall apart 
                if (abs(curObj->translationVelocity.x) < 0.0001f) {
                    curObj->translationVelocity.x = 0.0f;
                }
                if (abs(curObj->translationVelocity.y) < 0.0001f) {
                    curObj->translationVelocity.y = 0.0f;
                }
                if (abs(curObj->translationVelocity.z) < 0.0001f) {
                    curObj->translationVelocity.z = 0.0f;
                }
                if (abs(curObj->angularVelocity.x) < 0.0001f) {
                    curObj->angularVelocity.x = 0.0f;
                }
                if (abs(curObj->angularVelocity.y) < 0.0001f) {
                    curObj->angularVelocity.y = 0.0f;
                }
                if (abs(curObj->angularVelocity.z) < 0.0001f) {
                    curObj->angularVelocity.z = 0.0f;
                }

                // --------------------------------------------------------------
                // Collision dynamics
                // --------------------------------------------------------------

                // ---------------------------------------------------------------------------------
                // Handle collision with da ground :fire: :fire: :fire: :fire: :speaking_head: :speaking_head: :speaking_head:
                // ---------------------------------------------------------------------------------
                std::vector<glm::vec3> transformedBoundingBox = curObj->transformBoundingBox(glm::toMat4(curObj->quaternion));

                // Use velocity perpendicular to the ground plane
                glm::vec3 velPerp = planeNormal * glm::dot(curObj->translationVelocity, planeNormal) / glm::length(planeNormal);
                glm::vec3 velParallel = curObj->translationVelocity - velPerp;
                glm::vec3 minDist;
                bool flag = false; // flag is true if the object ends up inside the ground

                // Get all vertices on ground, and also the minimum distance required to move the object out of the ground
                curObj->verticesOnGround.clear();
                if (glm::length(velPerp) != 0.0f) {
                    for (int j = 0; j < transformedBoundingBox.size(); j++) {
                        float d = glm::dot(planeNormal, planePoint);
                        if ((glm::dot(transformedBoundingBox.at(j) + velPerp, planeNormal) - d) <= 0.0001f) {
                            curObj->verticesOnGround.push_back(j);
                            glm::vec3 tempDist = correctFallingMotion(transformedBoundingBox.at(j), velPerp, planeNormal, planePoint);
                            if (!flag) {
                                flag = true;
                                minDist = tempDist;
                            }
                            else if (glm::length(tempDist) > glm::length(minDist)) {
                                minDist = tempDist;
                            }
                        }
                    }
                }

                if (flag) {
                    minDist += velParallel;
                    curObj->offset3D += minDist;
                }

                if (curObj->verticesOnGround.size() == 0) { // Freefalling
                    curObj->offset3D += curObj->translationVelocity;
                    curObj->freeFall = true;
                }
                else { // Not freefalling
                    // Get an updated bounding box
                    transformedBoundingBox = curObj->transformBoundingBox(glm::toMat4(curObj->quaternion));

                    glm::vec3 pointOfContact;
                    CollisionInfo collisionInfo;
                    collisionInfo.originalNormal1 = -1.0f * planeNormal;
                    if (curObj->verticesOnGround.size() == 2) {
                        pointOfContact = transformedBoundingBox.at(curObj->verticesOnGround.at(0)) + transformedBoundingBox.at(curObj->verticesOnGround.at(1));
                        pointOfContact /= 2.0f;
                    }
                    else if (curObj->verticesOnGround.size() == 4) {
                        pointOfContact = transformedBoundingBox.at(curObj->verticesOnGround.at(0)) + transformedBoundingBox.at(curObj->verticesOnGround.at(1));
                        pointOfContact += transformedBoundingBox.at(curObj->verticesOnGround.at(2)) + transformedBoundingBox.at(curObj->verticesOnGround.at(3));
                        pointOfContact /= 4.0f;
                    }
                    else {
                        pointOfContact = transformedBoundingBox.at(curObj->verticesOnGround.at(0));
                    }
                    if (glm::length(curObj->translationVelocity) >= 0.02f || glm::length(curObj->angularVelocity) >= 0.02f) {
                        handleGroundCollision(curObj, pointOfContact, collisionInfo);
                    }
                    else {
                        curObj->translationVelocity -= curObj->acceleration;
                    }
                }

                // ---------------------------------------------------------------------------------
                // Handle collision with other objects
                // ---------------------------------------------------------------------------------

                // Remove any negligible velocities for smoother (yet in some cases inaccurate) looking collisions
                // This is usually useful to prevent a case where two objects are stacked and make tiny collisions that slowly make the objects fall apart 
                if (abs(curObj->translationVelocity.x) < 0.0001f) {
                    curObj->translationVelocity.x = 0.0f;
                }
                if (abs(curObj->translationVelocity.y) < 0.0001f) {
                    curObj->translationVelocity.y = 0.0f;
                }
                if (abs(curObj->translationVelocity.z) < 0.0001f) {
                    curObj->translationVelocity.z = 0.0f;
                }

                if (abs(curObj->angularVelocity.x) < 0.0001f) {
                    curObj->angularVelocity.x = 0.0f;
                }
                if (abs(curObj->angularVelocity.y) < 0.0001f) {
                    curObj->angularVelocity.y = 0.0f;
                }
                if (abs(curObj->angularVelocity.z) < 0.0001f) {
                    curObj->angularVelocity.z = 0.0f;
                }

                for (int j = NUM_STARTING_OBJECTS; j < vulkanCanvas->m_objects.size(); j++) {
                    if (j != i && vulkanCanvas->m_objects.at(j)->drawObject) {
                        CollisionInfo collisionResult = collisionDetection(vulkanCanvas->m_objects.at(i), vulkanCanvas->m_objects.at(j));
                        if (collisionResult.collision) {
                            vulkanCanvas->m_objects.at(i)->offset3D += collisionResult.distance; // Make sure there's a little gap between both objects
                            handleCollision(vulkanCanvas->m_objects.at(i), vulkanCanvas->m_objects.at(j), collisionResult);
                        }
                        else {
                        }
                    }
                }
                updateObjTextCtrls();
            }
        }
    }
}

// Make it such that a vertex doesnt fall below the plane it is about to go inside, and instead stop right at the point of contact
glm::vec3 MyFrame::correctFallingMotion(glm::vec3 point, glm::vec3 velocity, glm::vec3 normalOfPlane, glm::vec3 pointOnPlane) {
    glm::vec3 vectorToPlane = pointOnPlane - point;
    float lengthOfNormalToPoint = glm::dot(vectorToPlane, normalOfPlane) / glm::length(normalOfPlane);

    float velocityMultiplier = glm::dot(velocity, normalOfPlane) / glm::length(normalOfPlane);
    velocityMultiplier = lengthOfNormalToPoint / velocityMultiplier;

    return (velocity * velocityMultiplier);
}

// Given two objects, use separating axis theorem to determine if two objects collide, and return relevant info
CollisionInfo MyFrame::collisionDetection(GameObject* obj1, GameObject* obj2) {
    // -----------------------------------------------------------------
    // Gathering info
    // -----------------------------------------------------------------
    glm::quat quat1 = obj1->quaternion;
    glm::quat quat2 = obj2->quaternion;

    glm::mat4 rotationMatrix1 = glm::toMat4(quat1) * glm::mat4(1.0f);
    glm::mat4 rotationMatrix2 = glm::toMat4(quat2) * glm::mat4(1.0f);

    // List of vectors to test
    glm::vec3* testAxis = new glm::vec3[15];

    // Get original normals of faces of two bounding boxes
    testAxis[0] = (glm::vec3)(rotationMatrix1 * glm::vec4(1.0f, 0.0f, 0.0f, 0));
    testAxis[1] = (glm::vec3)(rotationMatrix1 * glm::vec4(0.0f, 1.0f, 0.0f, 0));
    testAxis[2] = (glm::vec3)(rotationMatrix1 * glm::vec4(0.0f, 0.0f, 1.0f, 0));
    testAxis[3] = (glm::vec3)(rotationMatrix2 * glm::vec4(1.0f, 0.0f, 0.0f, 0));
    testAxis[4] = (glm::vec3)(rotationMatrix2 * glm::vec4(0.0f, 1.0f, 0.0f, 0));
    testAxis[5] = (glm::vec3)(rotationMatrix2 * glm::vec4(0.0f, 0.0f, 1.0f, 0));

    // Calculate crossed normals
    testAxis[6] = glm::normalize(glm::cross(testAxis[0], testAxis[3]));
    testAxis[7] = glm::normalize(glm::cross(testAxis[0], testAxis[4]));
    testAxis[8] = glm::normalize(glm::cross(testAxis[0], testAxis[5]));
    testAxis[9] = glm::normalize(glm::cross(testAxis[1], testAxis[3]));
    testAxis[10] = glm::normalize(glm::cross(testAxis[1], testAxis[4]));
    testAxis[11] = glm::normalize(glm::cross(testAxis[1], testAxis[5]));
    testAxis[12] = glm::normalize(glm::cross(testAxis[2], testAxis[3]));
    testAxis[13] = glm::normalize(glm::cross(testAxis[2], testAxis[4]));
    testAxis[14] = glm::normalize(glm::cross(testAxis[2], testAxis[5]));

    std::vector<glm::vec3> boundingBox1 = obj1->transformBoundingBox(glm::toMat4(obj1->quaternion));
    std::vector<glm::vec3> boundingBox2 = obj2->transformBoundingBox(glm::toMat4(obj2->quaternion));

    // -----------------------------------------------------------------------
    // Determining if there is a collision and getting collision info if so
    // -----------------------------------------------------------------------
    CollisionInfo result;
    result.collision = true;

    bool flag = false;
    glm::vec3 minimumOverlap = glm::vec3(-1.0f, -1.0f, -1.0f);
    int axisIndex = -1;

    // Index of the vertex, which, when projected onto the line, is closest to the other object's projection
    // Useful for finding the point of contact
    int indexOfClosestPoint1 = -1;
    int indexOfClosestPoint2 = -1;

    for (int i = 0; i < 15; i++) {
        if (glm::length(testAxis[i]) > 0.001f) {
            float min1 = glm::dot(testAxis[i], boundingBox1.at(0));
            float max1 = min1;
            int indexOfMin1 = 0;
            int indexOfMax1 = 0;
            for (int j = 1; j < boundingBox1.size(); j++) {
                float cur = glm::dot(testAxis[i], boundingBox1.at(j));
                if (cur < min1) {
                    min1 = cur;
                    indexOfMin1 = j;
                }
                else if (cur > max1) {
                    max1 = cur;
                    indexOfMax1 = j;
                }
            }
            float min2 = glm::dot(testAxis[i], boundingBox2.at(0));
            float max2 = min2;
            int indexOfMin2 = 0;
            int indexOfMax2 = 0;
            for (int j = 1; j < boundingBox2.size(); j++) {
                float cur = glm::dot(testAxis[i], boundingBox2.at(j));
                if (cur < min2) {
                    min2 = cur;
                    indexOfMin2 = j;
                }
                else if (cur > max2) {
                    max2 = cur;
                    indexOfMax2 = j;
                }
            }
            if ((max2 - min1) > (max1 - min1) + (max2 - min2) || (max1 - min2) > (max1 - min1) + (max2 - min2)) {
                result.collision = false;
                return result;
            }
            if (flag == false) {
                flag = true;
                if (max1 >= min2) {
                    minimumOverlap = -(max1 - min2) * testAxis[i];
                    axisIndex = i;
                    indexOfClosestPoint1 = indexOfMax1;
                    indexOfClosestPoint2 = indexOfMin2;
                }
                if (max2 >= min1) {
                    minimumOverlap = (max2 - min1) * testAxis[i];
                    axisIndex = i;
                    indexOfClosestPoint1 = indexOfMin1;
                    indexOfClosestPoint2 = indexOfMax2;
                }
            }
            else {
                if ((max1 - min2) <= glm::length(minimumOverlap)) {
                    minimumOverlap = -(max1 - min2) * testAxis[i];
                    axisIndex = i;
                    indexOfClosestPoint1 = indexOfMax1;
                    indexOfClosestPoint2 = indexOfMin2;
                }
                if ((max2 - min1) <= glm::length(minimumOverlap)) {
                    minimumOverlap = (max2 - min1) * testAxis[i];
                    axisIndex = i;
                    indexOfClosestPoint1 = indexOfMin1;
                    indexOfClosestPoint2 = indexOfMax2;
                }
            }

            // -----------------------------------------------------------------
            // Printing information
            // -----------------------------------------------------------------

            //std::cout << "-------------------------------------------------------------------" << endl;
            //std::cout << "testAxis[" << i << "]: " << testAxis[i].x << " " << testAxis[i].y << " " << testAxis[i].z << endl;
            //std::cout << "min1: " << min1 << endl;
            //std::cout << "max1: " << max1 << endl;
            //std::cout << "min2: " << min2 << endl;
            //std::cout << "max2: " << max2 << endl;
            //std::cout << "minimumOverlap: " << minimumOverlap.x << " " << minimumOverlap.y << " " << minimumOverlap.z << endl;
            //std::cout << "duplicatesMin1.size(): " << duplicatesMin1.size() << endl;
            //std::cout << "duplicatesMax1.size(): " << duplicatesMax1.size() << endl;
            //std::cout << "duplicatesMin2.size(): " << duplicatesMin2.size() << endl;
            //std::cout << "duplicatesMax2.size(): " << duplicatesMax2.size() << endl;
        }
    }

    result.distance = minimumOverlap;
    result.lineToLine = (axisIndex >= 6);

    // Using the axis index, determine which two normals were originally used to make the axis of least separation
    if (result.lineToLine) {
        int normalIndex1;
        int normalIndex2;

        normalIndex1 = ((int)floor((axisIndex - 6) / 3));
        normalIndex2 = (axisIndex % 3) + 3;

        result.originalNormal1 = testAxis[normalIndex1];
        result.originalNormal2 = testAxis[normalIndex2];
    }
    else {
        result.originalNormal1 = testAxis[axisIndex];
    }

    result.closestIndex1 = indexOfClosestPoint1;
    result.closestIndex2 = indexOfClosestPoint2;
    return result;
}

// Given two objects and info regarding their collision, go through the collision dynamics
// Using formulas from:
// https://www.myphysicslab.com/engine2D/collision-en.html
void MyFrame::handleCollision(GameObject* obj1, GameObject* obj2, CollisionInfo collisionInfo) {
    // --------------------------------------------------------------------------
    // Gathering constants
    // --------------------------------------------------------------------------
    float mass1 = obj1->objectDensity * obj1->boundingBoxVolume;
    float mass2 = obj2->objectDensity * obj2->boundingBoxVolume;

    float momentOfInertia1 = obj1->momentOfInertia;
    float momentOfInertia2 = obj2->momentOfInertia;

    glm::vec3 velocityPreCollision1 = obj1->translationVelocity;
    glm::vec3 velocityPreCollision2 = obj2->translationVelocity;

    glm::vec3 angularVelocityPreCollision1 = obj1->angularVelocity;
    glm::vec3 angularVelocityPreCollision2 = obj2->angularVelocity;

    glm::vec4 centerOfMass1Vec4 = obj1->transformCenterOfMass(glm::toMat4(obj1->quaternion));
    glm::vec4 centerOfMass2Vec4 = obj2->transformCenterOfMass(glm::toMat4(obj2->quaternion));

    glm::vec3 centerOfMass1 = glm::vec3(centerOfMass1Vec4.x, centerOfMass1Vec4.y, centerOfMass1Vec4.z);
    glm::vec3 centerOfMass2 = glm::vec3(centerOfMass2Vec4.x, centerOfMass2Vec4.y, centerOfMass2Vec4.z);

    // --------------------------------------------------------------------------
    // Find point of contact
    // --------------------------------------------------------------------------
    bool pointOfContactFound = false;

    glm::vec3 pointOfContact = findPointOfContact(obj1, obj2, collisionInfo, pointOfContactFound);
    if (!pointOfContactFound) {
        return;
    }

    glm::vec3 distToPointOfContact1 = pointOfContact - centerOfMass1;
    glm::vec3 distToPointOfContact2 = pointOfContact - centerOfMass2;

    // --------------------------------------------------------------------------
    // Can change elasticity in the future
    // --------------------------------------------------------------------------
    float elasticity = 1.0f;

    // --------------------------------------------------------------------------
    // Create collision normal
    // --------------------------------------------------------------------------
    glm::vec3 collisionNormal;
    if (collisionInfo.lineToLine) {
        collisionNormal = glm::cross(collisionInfo.originalNormal1, collisionInfo.originalNormal2);
    }
    else {
        collisionNormal = collisionInfo.originalNormal1;
    }
    collisionNormal = glm::normalize(collisionNormal);

    // Fix the direction of the collision normal if needed (it should always be facing away from the obj1)
    float directionOfCollisionNormal = glm::dot(collisionNormal, -1.0f * distToPointOfContact1);

    if (directionOfCollisionNormal > 0.0f) {
        collisionNormal *= -1.0f;
    }

    // --------------------------------------------------------------------------
    // Work through collision dynamics
    // --------------------------------------------------------------------------
    glm::vec3 velocityOfPointOfContact1 = velocityPreCollision1 + glm::cross(angularVelocityPreCollision1, distToPointOfContact1);
    glm::vec3 velocityOfPointOfContact2 = velocityPreCollision2 + glm::cross(angularVelocityPreCollision2, distToPointOfContact2);

    glm::vec3 relativeVelocityPreCollision = velocityOfPointOfContact1 - velocityOfPointOfContact2;

    float numerator = -(1.0f + elasticity) * glm::dot(relativeVelocityPreCollision, collisionNormal);
    float denominator = (1.0f / mass1) + (1.0f / mass2);
    denominator += glm::dot(glm::cross(distToPointOfContact1, collisionNormal), glm::cross(distToPointOfContact1, collisionNormal)) / momentOfInertia1;
    denominator += glm::dot(glm::cross(distToPointOfContact2, collisionNormal), glm::cross(distToPointOfContact2, collisionNormal)) / momentOfInertia2;

    float impulseParameter = numerator / denominator;

    glm::vec3 velocityPostCollision1 = velocityPreCollision1 + impulseParameter * collisionNormal / mass1;
    glm::vec3 velocityPostCollision2 = velocityPreCollision2 - impulseParameter * collisionNormal / mass2;

    glm::vec3 angularVelocityPostCollision1 = angularVelocityPreCollision1 + glm::cross(distToPointOfContact1, impulseParameter * collisionNormal) / momentOfInertia1;
    glm::vec3 angularVelocityPostCollision2 = angularVelocityPreCollision2 + glm::cross(distToPointOfContact2, impulseParameter * -collisionNormal) / momentOfInertia2;

    obj1->translationVelocity = velocityPostCollision1;
    obj1->angularVelocity = angularVelocityPostCollision1;
    obj2->translationVelocity = velocityPostCollision2;
    obj2->angularVelocity = angularVelocityPostCollision2;

    // Make certain stacking object physics more realistic but lose overall accuracy for other collisions
    //if (abs(obj1->translationVelocity.x) <= 0.01f) {
    //    obj1->translationVelocity.x = 0.0f;
    //}
    //if (abs(obj1->translationVelocity.y) <= 0.01f) {
    //    obj1->translationVelocity.y = 0.0f;
    //}
    //if (abs(obj1->translationVelocity.z) <= 0.01f) {
    //    obj1->translationVelocity.z = 0.0f;
    //}

    //if (abs(obj2->translationVelocity.x) <= 0.01f) {
    //    obj2->translationVelocity.x = 0.0f;
    //}
    //if (abs(obj2->translationVelocity.y) <= 0.01f) {
    //    obj2->translationVelocity.y = 0.0f;
    //}
    //if (abs(obj2->translationVelocity.z) <= 0.01f) {
    //    obj2->translationVelocity.z = 0.0f;
    //}

    // --------------------------------------------------------------------------
    // Printing information
    // --------------------------------------------------------------------------
    cout << "----------------------------------------------------------" << endl;

    cout << "pointOfContact: " << pointOfContact.x << " " << pointOfContact.y << " " << pointOfContact.z << endl;
    cout << "distToPointOfContact1: " << distToPointOfContact1.x << " " << distToPointOfContact1.y << " " << distToPointOfContact1.z << endl;
    cout << "distToPointOfContact2: " << distToPointOfContact2.x << " " << distToPointOfContact2.y << " " << distToPointOfContact2.z << endl;
    cout << "originalNormal1: " << collisionInfo.originalNormal1.x << " " << collisionInfo.originalNormal1.y << " " << collisionInfo.originalNormal1.z << endl;
    cout << "originalNormal2: " << collisionInfo.originalNormal2.x << " " << collisionInfo.originalNormal2.y << " " << collisionInfo.originalNormal2.z << endl;
    cout << "collisionNormal: " << collisionNormal.x << " " << collisionNormal.y << " " << collisionNormal.z << endl;
    cout << "velocityOfPointOfContact1: " << velocityOfPointOfContact1.x << " " << velocityOfPointOfContact1.y << " " << velocityOfPointOfContact1.z << endl;
    cout << "velocityOfPointOfContact2: " << velocityOfPointOfContact2.x << " " << velocityOfPointOfContact2.y << " " << velocityOfPointOfContact2.z << endl;
    cout << "relativeVelocityPreCollision: " << relativeVelocityPreCollision.x << " " << relativeVelocityPreCollision.y << " " << relativeVelocityPreCollision.z << endl;

    cout << "numerator: " << numerator << endl;
    cout << "denominator: " << denominator << endl;

    std::cout << "velocityPreCollision1: " << velocityPreCollision1.x << " " << velocityPreCollision1.y << " " << velocityPreCollision1.z << endl;
    std::cout << "velocityPostCollision1: " << velocityPostCollision1.x << " " << velocityPostCollision1.y << " " << velocityPostCollision1.z << endl;
    std::cout << "velocityPreCollision2: " << velocityPreCollision2.x << " " << velocityPreCollision2.y << " " << velocityPreCollision2.z << endl;
    std::cout << "velocityPostCollision2: " << velocityPostCollision2.x << " " << velocityPostCollision2.y << " " << velocityPostCollision2.z << endl;
    std::cout << "angularVelocityPreCollision1: " << angularVelocityPreCollision1.x << " " << angularVelocityPreCollision1.y << " " << angularVelocityPreCollision1.z << endl;
    std::cout << "angularVelocityPostCollision1: " << angularVelocityPostCollision1.x << " " << angularVelocityPostCollision1.y << " " << angularVelocityPostCollision1.z << endl;
    std::cout << "angularVelocityPreCollision2: " << angularVelocityPreCollision2.x << " " << angularVelocityPreCollision2.y << " " << angularVelocityPreCollision2.z << endl;
    std::cout << "angularVelocityPostCollision2: " << angularVelocityPostCollision2.x << " " << angularVelocityPostCollision2.y << " " << angularVelocityPostCollision2.z << endl;

    return;
}

// Given an object and info regarding its collision to the ground, make necessary changes of the collision
// Using formulas from:
// https://www.myphysicslab.com/engine2D/collision-en.html
void MyFrame::handleGroundCollision(GameObject* obj1, glm::vec3 pointOfContact, CollisionInfo collisionInfo) {
    // --------------------------------------------------------------------------
    // Gathering constants
    // --------------------------------------------------------------------------
    float mass1 = obj1->objectDensity * obj1->boundingBoxVolume;
    float momentOfInertia1 = obj1->momentOfInertia;

    glm::vec3 velocityPreCollision1 = obj1->translationVelocity;
    glm::vec3 angularVelocityPreCollision1 = obj1->angularVelocity;

    glm::vec4 centerOfMass1Vec4 = obj1->transformCenterOfMass(glm::toMat4(obj1->quaternion));
    glm::vec3 centerOfMass1 = glm::vec3(centerOfMass1Vec4.x, centerOfMass1Vec4.y, centerOfMass1Vec4.z);

    glm::vec3 collisionNormal = glm::normalize(collisionInfo.originalNormal1);

    // Can change elasticity in the future
    float elasticity = 1.0f;

    // --------------------------------------------------------------------------
    // Work through collision dynamics
    // --------------------------------------------------------------------------
    glm::vec3 distToPointOfContact1 = pointOfContact - centerOfMass1;
    glm::vec3 velocityOfPointOfContact1 = velocityPreCollision1 + glm::cross(angularVelocityPreCollision1, distToPointOfContact1);
    glm::vec3 relativeVelocityPreCollision = velocityOfPointOfContact1;

    float numerator = -(1.0f + elasticity) * glm::dot(relativeVelocityPreCollision, collisionNormal);
    float denominator = (1.0f / mass1);
    denominator += glm::dot(glm::cross(distToPointOfContact1, collisionNormal), glm::cross(distToPointOfContact1, collisionNormal)) / momentOfInertia1;

    float impulseParameter = numerator / denominator;

    glm::vec3 velocityPostCollision1 = velocityPreCollision1 + impulseParameter * collisionNormal / mass1;
    glm::vec3 angularVelocityPostCollision1 = angularVelocityPreCollision1 + glm::cross(distToPointOfContact1, impulseParameter * collisionNormal) / momentOfInertia1;

    obj1->translationVelocity = velocityPostCollision1;
    obj1->angularVelocity = angularVelocityPostCollision1;


    // --------------------------------------------------------------------------
    // Printing information
    // --------------------------------------------------------------------------

    //cout << "momentOfInertia1: " << momentOfInertia1 << endl;
    //cout << "impulseParameter: " << impulseParameter << endl;
    //cout << "pointOfContact: " << pointOfContact.x << " " << pointOfContact.y << " " << pointOfContact.z << endl;
    //cout << "distToPointOfContact1: " << distToPointOfContact1.x << " " << distToPointOfContact1.y << " " << distToPointOfContact1.z << endl;
    //cout << "originalNormal1: " << collisionInfo.originalNormal1.x << " " << collisionInfo.originalNormal1.y << " " << collisionInfo.originalNormal1.z << endl;
    //cout << "collisionNormal: " << collisionNormal.x << " " << collisionNormal.y << " " << collisionNormal.z << endl;
    //cout << "velocityOfPointOfContact1: " << velocityOfPointOfContact1.x << " " << velocityOfPointOfContact1.y << " " << velocityOfPointOfContact1.z << endl;
    //cout << "relativeVelocityPreCollision: " << relativeVelocityPreCollision.x << " " << relativeVelocityPreCollision.y << " " << relativeVelocityPreCollision.z << endl;
    //std::cout << "velocityPreCollision1: " << velocityPreCollision1.x << " " << velocityPreCollision1.y << " " << velocityPreCollision1.z << endl;
    //std::cout << "velocityPostCollision1: " << velocityPostCollision1.x << " " << velocityPostCollision1.y << " " << velocityPostCollision1.z << endl;
    //std::cout << "angularVelocityPreCollision1: " << angularVelocityPreCollision1.x << " " << angularVelocityPreCollision1.y << " " << angularVelocityPreCollision1.z << endl;
    //std::cout << "angularVelocityPostCollision1: " << angularVelocityPostCollision1.x << " " << angularVelocityPostCollision1.y << " " << angularVelocityPostCollision1.z << endl;

    return;
}

// Given two objects that are colliding, return a reasonable point of contact. success = true if successful, false otherwise
glm::vec3 MyFrame::findPointOfContact(GameObject* obj1, GameObject* obj2, CollisionInfo collisionInfo, bool& success) {
    // Value to be returned; default is the zero vector
    glm::vec3 pointOfContact = glm::vec3(0.0f, 0.0f, 0.0f);

    // collisionInfo.distance is the vector that the objects need to move to not be inside each other
    // This also encodes the axis of least separation, which is needed for normalizedAxis
    glm::vec3 normalizedAxis = glm::normalize(collisionInfo.distance);

    if (glm::length(normalizedAxis) < 0.00001f) { // Incase that the objects have zero separation
        success = false;
        return pointOfContact;
    }

    // Get the transformed bounding boxes
    std::vector<glm::vec3> boundingBox1 = obj1->transformBoundingBox(glm::toMat4(obj1->quaternion));
    std::vector<glm::vec3> boundingBox2 = obj2->transformBoundingBox(glm::toMat4(obj2->quaternion));

    // The amount of points on object2 that is on a face of object1, and vice versa
    std::vector<int> pointsOnFaces1;
    std::vector<int> pointsOnFaces2;

    // For each face in each object, check if it is currently touching a point on the other object
    for (int i = 0; i < obj1->boundingBoxFaces.size(); i++) {
        int* curFace = obj1->boundingBoxFaces.at(i);
        glm::vec3 planeVec1 = boundingBox1.at(curFace[1]) - boundingBox1.at(curFace[0]);
        glm::vec3 planeVec2 = boundingBox1.at(curFace[2]) - boundingBox1.at(curFace[0]);
        glm::vec3 planeVec3 = boundingBox1.at(curFace[2]) - boundingBox1.at(curFace[1]);
        glm::vec3 planeVec4 = boundingBox1.at(curFace[0]) - boundingBox1.at(curFace[1]);
        glm::vec3 planeVec5 = boundingBox1.at(curFace[0]) - boundingBox1.at(curFace[2]);
        glm::vec3 planeVec6 = boundingBox1.at(curFace[1]) - boundingBox1.at(curFace[2]);
        glm::vec3 planePoint = boundingBox1.at(curFace[0]);
        glm::vec3 normal = glm::cross(planeVec1, planeVec2);

        for (int j = 0; j < boundingBox2.size(); j++) {
            glm::vec3 curVector = boundingBox2.at(j) - planePoint;

            if (glm::length(curVector) < 0.0001f) { // Point is at the same position as the vertex of the face
                if (!contains(pointsOnFaces1, j)) {
                    pointsOnFaces1.push_back(j);
                }
            }
            else if (abs(glm::dot(curVector, normal)) < 0.0001f) { // Point is on the same plane as the face
                // Now we check if point is bounded by the face
                float dot1 = glm::dot(curVector, planeVec1);
                glm::vec3 tempVector1 = boundingBox1.at(curFace[1]) - boundingBox2.at(j);
                glm::vec3 tempVector2 = boundingBox1.at(curFace[2]) - boundingBox2.at(j);
                glm::vec3 tempVector3 = boundingBox1.at(curFace[2]) - boundingBox2.at(j);
                glm::vec3 tempVector4 = boundingBox1.at(curFace[0]) - boundingBox2.at(j);
                glm::vec3 tempVector5 = boundingBox1.at(curFace[0]) - boundingBox2.at(j);
                glm::vec3 tempVector6 = boundingBox1.at(curFace[1]) - boundingBox2.at(j);

                if (glm::dot(tempVector1, planeVec1) >= 0.0f && glm::dot(tempVector2, planeVec2) >= 0.0f && glm::dot(tempVector3, planeVec3) >= 0.0f &&
                    glm::dot(tempVector4, planeVec4) >= 0.0f && glm::dot(tempVector5, planeVec5) >= 0.0f && glm::dot(tempVector6, planeVec6) >= 0.0f) {
                    if (!contains(pointsOnFaces1, j)) {
                        pointsOnFaces1.push_back(j);
                    }
                }
            }
        }
    }

    for (int i = 0; i < obj1->boundingBoxFaces.size(); i++) {
        int* curFace = obj1->boundingBoxFaces.at(i);
        glm::vec3 planeVec1 = boundingBox2.at(curFace[1]) - boundingBox2.at(curFace[0]);
        glm::vec3 planeVec2 = boundingBox2.at(curFace[2]) - boundingBox2.at(curFace[0]);
        glm::vec3 planeVec3 = boundingBox2.at(curFace[2]) - boundingBox2.at(curFace[1]);
        glm::vec3 planeVec4 = boundingBox2.at(curFace[0]) - boundingBox2.at(curFace[1]);
        glm::vec3 planeVec5 = boundingBox2.at(curFace[0]) - boundingBox2.at(curFace[2]);
        glm::vec3 planeVec6 = boundingBox2.at(curFace[1]) - boundingBox2.at(curFace[2]);
        glm::vec3 planePoint = boundingBox2.at(curFace[0]);
        glm::vec3 normal = glm::cross(planeVec1, planeVec2);

        for (int j = 0; j < boundingBox1.size(); j++) {
            glm::vec3 curVector = boundingBox1.at(j) - planePoint;

            if (glm::length(curVector) < 0.0001f) { // Point is at the same position as the vertex of the face
                if (!contains(pointsOnFaces2, j)) {
                    pointsOnFaces2.push_back(j);
                }
            }
            else if (abs(glm::dot(curVector, normal)) < 0.0001f) { // Point is on the same plane as the face
                // Check if point is bounded by the face
                float dot1 = glm::dot(curVector, planeVec1);
                glm::vec3 tempVector1 = boundingBox2.at(curFace[1]) - boundingBox1.at(j);
                glm::vec3 tempVector2 = boundingBox2.at(curFace[2]) - boundingBox1.at(j);
                glm::vec3 tempVector3 = boundingBox2.at(curFace[2]) - boundingBox1.at(j);
                glm::vec3 tempVector4 = boundingBox2.at(curFace[0]) - boundingBox1.at(j);
                glm::vec3 tempVector5 = boundingBox2.at(curFace[0]) - boundingBox1.at(j);
                glm::vec3 tempVector6 = boundingBox2.at(curFace[1]) - boundingBox1.at(j);

                if (glm::dot(tempVector1, planeVec1) >= 0.0f && glm::dot(tempVector2, planeVec2) >= 0.0f && glm::dot(tempVector3, planeVec3) >= 0.0f &&
                    glm::dot(tempVector4, planeVec4) >= 0.0f && glm::dot(tempVector5, planeVec5) >= 0.0f && glm::dot(tempVector6, planeVec6) >= 0.0f) {
                    if (!contains(pointsOnFaces2, j)) {
                        pointsOnFaces2.push_back(j);
                    }
                }
            }
        }
    }

    // Go through some specific cases of collisions
    if (pointsOnFaces1.size() == 4) {
        pointOfContact = boundingBox2.at(pointsOnFaces1.at(0)) + boundingBox2.at(pointsOnFaces1.at(1));
        pointOfContact += boundingBox2.at(pointsOnFaces1.at(2)) + boundingBox2.at(pointsOnFaces1.at(3));
        pointOfContact /= 4.0f;
        success = true;
        return pointOfContact;
    }
    else if (pointsOnFaces2.size() == 4) {
        pointOfContact = boundingBox1.at(pointsOnFaces2.at(0)) + boundingBox1.at(pointsOnFaces2.at(1));
        pointOfContact += boundingBox1.at(pointsOnFaces2.at(2)) + boundingBox1.at(pointsOnFaces2.at(3));
        pointOfContact /= 4.0f;
        success = true;
        return pointOfContact;
    }
    else if (pointsOnFaces1.size() == 1) {
        pointOfContact = boundingBox2.at(pointsOnFaces1.at(0));
        success = true;
        return pointOfContact;
    }
    else if (pointsOnFaces2.size() == 1) {
        pointOfContact = boundingBox1.at(pointsOnFaces2.at(0));
        success = true;
        return pointOfContact;
    }
    else if (pointsOnFaces1.size() == 2) {
        pointOfContact = boundingBox2.at(pointsOnFaces1.at(0)) + boundingBox2.at(pointsOnFaces1.at(1));
        pointOfContact /= 2.0f;
        success = true;
        return pointOfContact;
    }
    else if (pointsOnFaces2.size() == 2) {
        pointOfContact = boundingBox1.at(pointsOnFaces2.at(0)) + boundingBox1.at(pointsOnFaces2.at(1));
        pointOfContact /= 2.0f;
        success = true;
        return pointOfContact;
    }
    else { // Edge to edge collision
        std::vector<glm::vec3*> lines1;
        std::vector<glm::vec3*> lines2;

        for (int i = 0; i < boundingBox1.size(); i++) {
            if (i != collisionInfo.closestIndex1) {
                glm::vec3 normalizedLine = glm::normalize(boundingBox1.at(collisionInfo.closestIndex1) - boundingBox1.at(i));
                if (abs(glm::length(normalizedLine - collisionInfo.originalNormal1)) <= 0.001f || abs(glm::length(-1.0f * normalizedLine - collisionInfo.originalNormal1)) <= 0.001f) {
                    glm::vec3* line = new glm::vec3[2];
                    line[0] = boundingBox1.at(collisionInfo.closestIndex1);
                    line[1] = boundingBox1.at(i);
                    line[0].x = std::ceil(line[0].x * 100.0f) / 100.0f;
                    line[0].y = std::ceil(line[0].y * 100.0f) / 100.0f;
                    line[0].z = std::ceil(line[0].z * 100.0f) / 100.0f;
                    line[1].x = std::ceil(line[1].x * 100.0f) / 100.0f;
                    line[1].y = std::ceil(line[1].y * 100.0f) / 100.0f;
                    line[1].z = std::ceil(line[1].z * 100.0f) / 100.0f;
                    lines1.push_back(line);
                }
            }
        }

        for (int i = 0; i < boundingBox2.size(); i++) {
            if (i != collisionInfo.closestIndex2) {
                glm::vec3 normalizedLine = glm::normalize(boundingBox2.at(collisionInfo.closestIndex2) - boundingBox2.at(i));
                if (normalizedLine == collisionInfo.originalNormal2 || -1.0f * normalizedLine == collisionInfo.originalNormal2) {
                    glm::vec3* line = new glm::vec3[2];
                    line[0] = boundingBox2.at(collisionInfo.closestIndex2);
                    line[1] = boundingBox2.at(i);
                    line[0].x = std::ceil(line[0].x * 100.0f) / 100.0f;
                    line[0].y = std::ceil(line[0].y * 100.0f) / 100.0f;
                    line[0].z = std::ceil(line[0].z * 100.0f) / 100.0f;
                    line[1].x = std::ceil(line[1].x * 100.0f) / 100.0f;
                    line[1].y = std::ceil(line[1].y * 100.0f) / 100.0f;
                    line[1].z = std::ceil(line[1].z * 100.0f) / 100.0f;
                    lines2.push_back(line);
                }
            }
        }

        std::vector<glm::vec3> intersectPoints;

        for (int i = 0; i < lines1.size(); i++) {
            for (int j = 0; j < lines2.size(); j++) {
                // Get line equations using info we have:
                // p1 = lines1.at(i)[0]                    |  p2 = lines2.at(j)[0]
                // d1 = lines1.at(i)[0] - lines1.at(i)[1]  |  d2 = lines2.at(j)[0] - lines2.at(j)[1]
                // l1 = lines1.at(i)[0] + d1 * t           |  l2 = lines2.at(j)[0] + d2 * s
                //
                // Find intersection point of lines:
                // p1.x + t * d1.x = l1.x  |  p2.x + s * d2.x = l2.x  
                // p1.y + t * d1.y = l1.y  |  p2.y + s * d2.y = l2.y
                // p1.z + t * d1.z = l1.z  |  p2.z + s * d2.z = l2.z
                //
                // l1.x = l2.x  |  l1.y = l2.y  |  l1.z = l2.z
                // 
                // p1.x + t * d1.x = p2.x + s * d2.x
                // p1.y + t * d1.y = p2.y + s * d2.y
                // p1.z + t * d1.z = p2.z + s * d2.z
                //
                // t * d1.x - s * d2.x = p2.x - p1.x
                // t * d1.y - s * d2.y = p2.y - p1.y
                // t * d1.z - s * d2.z = p2.z - p1.z
                // 
                // First, check if both lines overlap.
                // Normalize both direction vectors and see if they are parallel. Then see if they share a point
                // If both direction vectors are parallel and non overlapping, ignore rest of formula
                //
                // Solve for t and s using the first two equations:
                // t * d1.x - s * d2.x = p2.x - p1.x
                // t * d1.y - s * d2.y = p2.y - p1.y
                // 
                // Turn into matrix:
                // [d1.x  -d2.x] [t] = [(p2.x - p1.x)]
                // [d1.y  -d2.y] [s]   [(p2.y - p1.y)]
                //
                // If determinant of matrix == 0.0f, the x and y are parallel, and this matrix isnt very useful
                // Check if the x and z are parallel using same method. If x and y are parallel, x and z cant be parallel or else both lines would be parallel
                //
                // If determinant of matrix != 0.0f, and lines dont overlap, only one unique solution to this, thus inverse exists
                // 
                // Then solve for t and s

                glm::vec3 point1 = lines1.at(i)[0];
                glm::vec3 point2 = lines2.at(j)[0];
                glm::vec3 direction1 = lines1.at(i)[1] - lines1.at(i)[0];
                glm::vec3 direction2 = lines2.at(j)[1] - lines2.at(j)[0];

                glm::vec3 normalizeDirection1 = glm::normalize(direction1);
                glm::vec3 normalizeDirection2 = glm::normalize(direction2);

                bool overlap = false;
                bool parallel = false;
                // Check if lines overlap
                if (normalizeDirection1 == normalizeDirection2) {
                    parallel = true;
                    glm::vec3 pointsDifference = point1 - point2;
                    if (glm::length(pointsDifference) <= 0.0001f) {
                        overlap = true;
                    }
                    else {
                        if (glm::normalize(pointsDifference) == normalizeDirection1) {
                            overlap = true;
                        }
                    }
                }

                bool intersectionFound = false;
                glm::vec3 intersectionPoint;

                if (!overlap && !parallel) {
                    float detXY = direction1.x * (-direction2.y) - direction1.y * (-direction2.x);
                    float detXZ = direction1.x * (-direction2.z) - direction1.z * (-direction2.x);
                    float detYZ = direction1.y * (-direction2.z) - direction1.z * (-direction2.y);

                    if (detXY == 0.0f && detYZ == 0.0f) {
                        float t = (point2.x - point1.x) * (-direction2.z) - (point2.z - point1.z) * (direction2.x);
                        float s = (point2.z - point1.z) * (direction1.x) - (point2.x - point1.x) * (-direction2.z);
                        t /= detXZ;
                        s /= detXZ;

                        // Ensure that the line segments are checked, not the actual line rays
                        if (t <= 1.0f && s <= 1.0f) {
                            if (abs((point1.y + direction1.y * t) - (point2.y + direction2.y * s)) == 0.0f) {
                                intersectionFound = true;
                                intersectionPoint = point1 + direction1 * t;
                                intersectPoints.push_back(intersectionPoint);
                            }
                        }
                    }
                    else if (detXZ == 0.0f && detYZ == 0.0f) {
                        float t = (point2.x - point1.x) * (-direction2.y) - (point2.y - point1.y) * (direction2.x);
                        float s = (point2.y - point1.y) * (direction1.x) - (point2.x - point1.x) * (-direction2.y);
                        t /= detXY;
                        s /= detXY;

                        // Ensure that the line segments are checked, not the actual line rays
                        if (t <= 1.0f && s <= 1.0f) {
                            if (abs((point1.z + direction1.z * t) - (point2.z + direction2.z * s)) == 0.0f) {
                                intersectionFound = true;
                                intersectionPoint = point1 + direction1 * t;
                                intersectPoints.push_back(intersectionPoint);
                            }
                        }
                    }
                    else { // detXY == 0.0f && detXY == 0.0f
                        float t = (point2.y - point1.y) * (-direction2.z) - (point2.z - point1.z) * (direction2.y);
                        float s = (point2.z - point1.z) * (direction1.y) - (point2.y - point1.y) * (-direction2.z);
                        t /= detYZ;
                        s /= detYZ;

                        // Ensure that the line segments are checked, not the actual line rays
                        if (t <= 1.0f && s <= 1.0f) {
                            if (abs((point1.x + direction1.x * t) - (point2.x + direction2.x * s)) == 0.0f) {
                                intersectionFound = true;
                                intersectionPoint = point1 + direction1 * t;
                                intersectPoints.push_back(intersectionPoint);
                            }
                        }
                    }
                }
                if (overlap) {
                    intersectPoints.push_back((point1 + point2) / 2.0f);
                }
                if (intersectionFound) {
                    break;
                }
            }
        }
        if (intersectPoints.size() == 1) {
            pointOfContact = intersectPoints.at(0);
            success = true;
        }
    }

    return pointOfContact;
}

boolean MyFrame::contains(std::vector<int> vec, int a) {
    for (int i = 0; i < vec.size(); i++) {
        if (vec.at(i) == a) {
            return true;
        }
    }
    return false;
}
