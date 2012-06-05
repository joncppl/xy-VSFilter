#ifndef __VERSION_567B55CD_F1A7_49FB_B001_4A5A82CA666C_H__
#define __VERSION_567B55CD_F1A7_49FB_B001_4A5A82CA666C_H__

#define XY_VAR_2_STR(x) #x
#define XY_VAR_2_STR2(x) XY_VAR_2_STR(x)

#define XY_VSFILTER_VERSION_MAJOR 3
#define XY_VSFILTER_VERSION_MINOR 0
#define XY_VSFILTER_VERSION_PATCH  0
#define XY_VSFILTER_VERSION_COMMIT 16

#define XY_VSFILTER_VERSION_NUM      XY_VSFILTER_VERSION_MAJOR,XY_VSFILTER_VERSION_MINOR,XY_VSFILTER_VERSION_PATCH ,XY_VSFILTER_VERSION_COMMIT
#define XY_VSFILTER_VERSION_STR      XY_VAR_2_STR2(XY_VSFILTER_VERSION_MAJOR) "," XY_VAR_2_STR2(XY_VSFILTER_VERSION_MINOR) "," XY_VAR_2_STR2(XY_VSFILTER_VERSION_PATCH ) "," XY_VAR_2_STR2(XY_VSFILTER_VERSION_COMMIT)

#endif // __VERSION_567B55CD_F1A7_49FB_B001_4A5A82CA666C_H__
