#pragma once

extern void LoadConfig();
extern BOOL IsEnabled();
extern BOOL DoRelaunch();
extern BOOL OverrideSize();
extern POINT RenderSize();
extern unsigned short Opacity();

//Defaults:
BOOL def_is_enabled = TRUE;
BOOL def_do_relaunch = FALSE;
BOOL def_override_size = TRUE;
POINT def_render_size = { 1920, 1080 };
unsigned short def_opacity = 80;

