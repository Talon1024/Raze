//-------------------------------------------------------------------------
/*
Copyright (C) 2010 EDuke32 developers and contributors

This file is part of EDuke32.

EDuke32 is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License version 2
as published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/
//-------------------------------------------------------------------------
#include "ns.h"
#include "compat.h"
#include "build.h"
#include "common.h"
#include "exhumed.h"
#include "osdcmds.h"
#include "view.h"
#include "mapinfo.h"

BEGIN_PS_NS


static int osdcmd_god(CCmdFuncPtr UNUSED(parm))
{
    UNREFERENCED_CONST_PARAMETER(parm);

    if (!nNetPlayerCount && !bInDemo)
    {
        DoPassword(3);
    }
    else
        Printf("god: Not in a single-player game.\n");

    return OSDCMD_OK;
}

static int osdcmd_noclip(CCmdFuncPtr UNUSED(parm))
{
    UNREFERENCED_CONST_PARAMETER(parm);

    if (!nNetPlayerCount && !bInDemo)
    {
        DoPassword(6);
    }
    else
    {
        Printf("noclip: Not in a single-player game.\n");
    }

    return OSDCMD_OK;
}

static int osdcmd_map(CCmdFuncPtr parm)
{
    if (parm->numparms != 1)
    {
        return OSDCMD_SHOWHELP;
    }
    FString mapname = parm->parms[0];

    if (!fileSystem.Lookup(mapname, "MAP"))
    {
        Printf(TEXTCOLOR_RED "map: file \"%s\" not found.\n", mapname.GetChars());
        return OSDCMD_OK;
    }
	
	// Check if the map is already defined.
    for (int i = 0; i <= ISDEMOVER? 4 : 32; i++)
    {
        if (mapList[i].labelName.CompareNoCase(mapname) == 0)
        {
			levelnew = i;
			levelnum = i;
			return OSDCMD_OK;
        }
    }
    return OSDCMD_OK;
}

static int osdcmd_changelevel(CCmdFuncPtr parm)
{
    char* p;

    if (parm->numparms != 1) return OSDCMD_SHOWHELP;

    int nLevel = strtol(parm->parms[0], &p, 10);
    if (p[0]) return OSDCMD_SHOWHELP;

    if (nLevel < 0) return OSDCMD_SHOWHELP;

    int nMaxLevels;

    if (!ISDEMOVER) {
        nMaxLevels = 32;
    }
    else {
        nMaxLevels = 4;
    }

    if (nLevel > nMaxLevels)
    {
        Printf("changelevel: invalid level number\n");
        return OSDCMD_SHOWHELP;
    }

    levelnew = nLevel;
    levelnum = nLevel;

    return OSDCMD_OK;
}


int32_t registerosdcommands(void)
{
    //if (VOLUMEONE)
    C_RegisterFunction("changelevel","changelevel <level>: warps to the given level", osdcmd_changelevel);
    C_RegisterFunction("map","map <mapname>: loads the given map", osdcmd_map);
    //    C_RegisterFunction("demo","demo <demofile or demonum>: starts the given demo", osdcmd_demo);
    //}

    //C_RegisterFunction("cmenu","cmenu <#>: jumps to menu", osdcmd_cmenu);


    //C_RegisterFunction("give","give <all|health|weapons|ammo|armor|keys|inventory>: gives requested item", osdcmd_give);
    C_RegisterFunction("god","god: toggles god mode", osdcmd_god);
    //C_RegisterFunction("activatecheat","activatecheat <id>: activates a cheat code", osdcmd_activatecheat);

    C_RegisterFunction("noclip","noclip: toggles clipping mode", osdcmd_noclip);
    //C_RegisterFunction("restartmap", "restartmap: restarts the current map", osdcmd_restartmap);
    //C_RegisterFunction("restartsound","restartsound: reinitializes the sound system",osdcmd_restartsound);

    //C_RegisterFunction("spawn","spawn <picnum> [palnum] [cstat] [ang] [x y z]: spawns a sprite with the given properties",osdcmd_spawn);

    return 0;
}


END_PS_NS
