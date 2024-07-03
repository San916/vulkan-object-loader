# Vulkan Object Loader
An object loader capable of loading 3D objects and animations using Vulkan.

Overview
--------
<h3> The user can: </h3>

 - Load 3D **.obj** and animated **.md5mesh/.md5anim** files from a file tree made with **wxWidgets**

![object-loading-gif](https://github.com/San916/vulkan-object-loader/blob/main/gifs/object_loading.gif)

 - Move around the **3D environment** using either **UI** or **mouse controls**
 - **Manipulate objects** using the UI controls, with **mouse picking** for object selection

![object-and-camera-manipulation-gif](https://github.com/San916/vulkan-object-loader/blob/main/gifs/object_camera_manipulation.gif)

- Simulate **rigid body physics** using a **physics engine built from scratch** (physics currently works on .obj files, .md5 files are a WIP)
    - Cube bouncing on a plane ![cube-bouncing-demonstration-gif](https://github.com/San916/vulkan-object-loader/blob/main/gifs/physics_1.gif)
    - Cubes stacking on each other ![cube-stacking-demonstration-gif](https://github.com/San916/vulkan-object-loader/blob/main/gifs/physics_2.gif)
    - Cubes colliding with each other ![cube-collision-demonstration-gif](https://github.com/San916/vulkan-object-loader/blob/main/gifs/physics_3.gif)
    - And many other rigid-body interactions!

Setup
--------

<h3> Installing the libraries </h3>

 - Install the **Vulkan SDK** at https://vulkan.lunarg.com/ into the C: folder (using version 1.3.275.0)
 - Download the 64-bit Windows binaries for **glfw** at https://www.glfw.org/download (using version 3.4)
 - Download the **glm** library on github at https://github.com/g-truc/glm
 - Download **boost** at https://www.boost.org/users/download/ (using version 1_84_0)
 - Download **stb-master** on github at https://github.com/nothings/stb
 - Install **wxWidgets** at https://www.wxwidgets.org/downloads/ (using version 3.2.4) 

 Place the **glfw, glm, boost, stb-master, and wxWidgets** libraries into a single parent directory.

 To build wxWidgets, open `wxWidgets-3.2.4\build\msw\wx_vc15.sln`, then select the `build solution` option after right-clicking the solution in the `Solution Explorer`.

 For boost, run `boost_1_84_0\bootstrap.bat`, then run the file `boost_1_84_0\b2.exe` which should have appeared after running the previous file. A new folder `boost_1_84_0\stage` should appear. If you are having trouble (or are using a different version of boost), follow the guide at https://stackoverflow.com/questions/2629421/how-to-use-boost-in-visual-studio-2010/2655683#2655683.

 This is what your directory should look like for the project.

 <img src="./images/file-explorer-setup.JPG" alt="Image of a file explorer" width="900">

<h3> Setting up the solution </h3>

Drag the contents of the folder named `files-to-place-in-minimal` into `wxWidgets-3.2.4\samples\minimal`, then open up `wxWidgets-3.2.4\samples\minimal\minimal_vc15.sln`.

 - At the top of Visual Studio, go into `View -> Other Windows -> Property Manager`
 - In the `Property Manager`, drop down `minimal` and right-click `Debug|x64`, select `Add Existing Property Sheet...` and select the file `PropertySheet.props`
 - Go into `minimal -> Properties -> General -> C++ Language Stantard` while in the `Debug x64` configuration, and select the option with `C++17`
 - At the top of the `Solution Explorer`, select `Show All Files`, right click every file shown in the image below (these are all the relevant files), and select `Include In Project`

<img src="./images/solution-explorer.JPG" alt="Image of a Visual Studio Solution Explorer" height="700">

 - In `minimal.cpp` at the top, fill out the `ROOT_HOME_FOLDER_NAME` and `ROOT_DIR_NAME` to correspond to whichever directory you like

 <img src="./images/root-folder-names.JPG" alt="Image of a c++ file contents" width="900">

If you use different versions of libraries or have placed them in different locations, you may need to change some of the file paths in `Properties -> VC++ Directories -> Include Directories | Library Directories`, `Properties -> C/C++ -> Additional Include Directories`, and `Properties -> Linker ->  General -> Additional Library Directories`.

Controls
-----
- Scrollwheel: Camera zooming
- Left click: Camera panning
- Right click: Camera rotating (around the camera target)
- Shift + Left click: Object selection

Creators
-----

Made by San Kang

Resources
-----
Chalet.obj and corresponding texture files can be found on: 
https://github.com/heitaoflower/vulkan-tutorial/tree/master/Tutorial29

Bob Lamp files can be found on: 
https://www.katsbits.com/download/models/
