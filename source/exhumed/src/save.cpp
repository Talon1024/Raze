//-------------------------------------------------------------------------
/*
Copyright (C) 2010-2019 EDuke32 developers and contributors
Copyright (C) 2019 sirlemonhead, Nuke.YKT
This file is part of PCExhumed.
PCExhumed is free software; you can redistribute it and/or
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
#include "save.h"
#include <stdio.h>
#include <stdarg.h>
#include "init.h"
#include "raze_music.h"
//#include <fcntl.h>
//#include <sys/stat.h>
//#include <io.h>
#include "engine.h"
#include "exhumed.h"
#include "mmulti.h"
#include "savegamehelp.h"
#include "sound.h"

BEGIN_PS_NS

extern int MenuExitCondition;
void SaveTextureState();
void LoadTextureState();

static TArray<SavegameHelper*> sghelpers(TArray<SavegameHelper*>::NoInit);

bool GameInterface::SaveGame(FSaveGameNode* sv)
{
    for (auto sgh : sghelpers) sgh->Save();
    SaveTextureState();
    FinishSavegameWrite();
    return 1; // CHECKME
}

bool GameInterface::LoadGame(FSaveGameNode* sv)
{

    for (auto sgh : sghelpers) sgh->Load();
    LoadTextureState();
    FinishSavegameRead();

    // reset the sky in case it hasn't been done yet.
    psky_t* pSky = tileSetupSky(0);
    pSky->tileofs[0] = 0;
    pSky->tileofs[1] = 0;
    pSky->tileofs[2] = 0;
    pSky->tileofs[3] = 0;
    pSky->yoffs = 256;
    pSky->lognumtiles = 2;
    pSky->horizfrac = 65536;
    pSky->yscale = 65536;
    parallaxtype = 2;
    g_visibility = 2048;
    ototalclock = totalclock;
    MenuExitCondition = 6;

    if (levelnum > 15)
    {
        nSwitchSound = 35;
        nStoneSound = 23;
        nElevSound = 51;
        nStopSound = 35;
    }
    else
    {
        nSwitchSound = 33;
        nStoneSound = 23;
        nElevSound = 23;
        nStopSound = 66;
    }

    Mus_ResumeSaved();
    return 1; // CHECKME
}


SavegameHelper::SavegameHelper(const char* name, ...)
{
    Name = name;
    sghelpers.Push(this);
    va_list ap;
    va_start(ap, name);
    for(;;)
    {
        void* addr = va_arg(ap, void*);
        if (!addr) break;
        size_t size = va_arg(ap, size_t);
        Elements.Push(std::make_pair(addr, size));
    }
}

void SavegameHelper::Load()
{
    auto fr = ReadSavegameChunk(Name);
    for (auto& entry : Elements)
    {
        auto read = fr.Read(entry.first, entry.second);
        if (read != entry.second) I_Error("Save game read error in %s", Name.GetChars());
    }
}
void SavegameHelper::Save()
{
    auto fw = WriteSavegameChunk(Name);
    for (auto& entry : Elements)
    {
        auto write = fw->Write(entry.first, entry.second);
        if (write != entry.second) I_Error("Save game write error in %s", Name.GetChars());
    }
}

END_PS_NS
