// based on https://github.com/nem0/LumixEngine/blob/master/external/imgui/imgui_dock.h

#ifndef IMGUI_DOCK_H
#define IMGUI_DOCK_H

//namespace Lumix { namespace FS { class OsFile; } }
//struct lua_State;

typedef struct ImVec2 ImVec2;
typedef int ImGuiWindowFlags;
    
typedef enum ImGuiDockSlot {
    ImGuiDockSlot_Left,
    ImGuiDockSlot_Right,
    ImGuiDockSlot_Top,
    ImGuiDockSlot_Bottom,
    ImGuiDockSlot_Tab,

    ImGuiDockSlot_Float,
    ImGuiDockSlot_None
} ImGuiDockSlot;

extern void igBeginWorkspace();
extern void igEndWorkspace();
extern void igShutdownDock();
extern void igSetNextDock(ImGuiDockSlot slot);
extern bool igBeginDock(const char* label, bool* opened, ImGuiWindowFlags extra_flags);
extern void igEndDock();
extern void igSetDockActive();
extern void igDockDebugWindow();
//void SaveDock(Lumix::FS::OsFile& file);
//void LoadDock(lua_State* L);

#endif // IMGUI_DOCK_H
