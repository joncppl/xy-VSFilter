#include "stdafx.h"
#include "overlay_config.h"

BOOL is_enabled = def_is_enabled;
BOOL do_relaunch = def_do_relaunch;
BOOL override_size = def_override_size;
POINT render_size = def_render_size;
unsigned short opacity = def_opacity;

char configfile[] = "\\xyoverlay.ini";

char *ConfigFilePath()
{
	char* docdir = getenv("USERPROFILE");
	static char configPath[256];
	sprintf(&configPath[0], "%s%s", docdir, configfile);
	return configPath;
}


char trueString[] = "TRUE";
char falseString[] = "FALSE";
char *BoolToString(BOOL b)
{
	return b ? trueString : falseString;
}

void SaveCurrentConfig()
{
	FILE *file = fopen(ConfigFilePath(), "w");
	if (file)
	{
		fprintf(file, "ENABLED=%s\n", BoolToString(is_enabled));
		fprintf(file, "RELOAD=%s\n", BoolToString(do_relaunch));
		fprintf(file, "SIZE_OVERRIDE=%s\n", BoolToString(override_size));
		fprintf(file, "SIZE_X=%d\n", render_size.x);
		fprintf(file, "SIZE_Y=%d\n", render_size.y);
		fprintf(file, "OPACITY=%d\n", opacity);
		fclose(file);
	}
}

void LoadConfig()
{
	FILE *file = fopen(ConfigFilePath(), "r");
	if (!file)
	{
		SaveCurrentConfig();
		return;
	}
	char line[256];
	char *dataPtr;
	while (fgets(line, 256, file) != NULL)
	{
		if (strstr(line, "ENABLED") != NULL)
		{
			dataPtr = strstr(line, "=");
			if (strncmp(++dataPtr, "TRUE", 4) == 0)
			{
				is_enabled = TRUE;
			}
			else
			{
				is_enabled = FALSE;
			}
		}
		else if (strstr(line, "RELOAD") != NULL) 
		{
			dataPtr = strstr(line, "=");
			if (strncmp(++dataPtr, "TRUE", 4) == 0)
			{
				do_relaunch = TRUE;
			}
			else
			{
				do_relaunch = FALSE;
			}
		}
		else if (strstr(line, "SIZE_OVERRIDE") != NULL)
		{
			dataPtr = strstr(line, "=");
			if (strncmp(++dataPtr, "TRUE", 4) == 0)
			{
				override_size = TRUE;
			}
			else
			{
				override_size = FALSE;
			}
		}
		else if (strstr(line, "SIZE_X") != NULL)
		{
			dataPtr = strstr(line, "=");
			render_size.x = atoi(++dataPtr);
		}
		else if (strstr(line, "SIZE_Y") != NULL)
		{
			dataPtr = strstr(line, "=");
			render_size.y = atoi(++dataPtr);
		}
		else if (strstr(line, "OPACITY") != NULL)
		{
			dataPtr = strstr(line, "=");
			opacity = atoi(++dataPtr);
		}
	}
	fclose(file);
	
	// so as to add any missing values
	SaveCurrentConfig();
}

BOOL IsEnabled()
{
	return is_enabled;
}

BOOL DoRelaunch()
{
	return do_relaunch;
}

BOOL OverrideSize()
{
	return override_size;
}

POINT RenderSize()
{
	return render_size;
}

unsigned short Opacity()
{
	return opacity;
}