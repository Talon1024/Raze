//-------------------------------------------------------------------------
/*
Copyright (C) 1997, 2005 - 3D Realms Entertainment

This file is part of Shadow Warrior version 1.2

Shadow Warrior is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

Original Source: 1997 - Frank Maddin and Jim Norwood
Prepared for public release: 03/28/2005 - Charlie Wiederhold, 3D Realms
*/
//-------------------------------------------------------------------------
#include "ns.h"

#include "build.h"

#include "keys.h"
#include "pal.h"
#include "game.h"

#include "common_game.h"

BEGIN_SW_NS

short f_c = 3;
static char tempbuf[256];

void
MapColors(short num, COLOR_MAP cm, short create)
{
    int i;
    float inc;

    if (create)
    {
        for (i = 0; i < 256; i++)
            tempbuf[i] = i;
    }

    if (cm.FromRange == 0 || num <= 0 || num >= 256)
    {
        return;
    }

    inc = cm.ToRange/((float)cm.FromRange);

    for (i = 0; i < cm.FromRange; i++)
        tempbuf[i + cm.FromColor] = (i*inc) + cm.ToColor;
}


#define PLAYER_COLOR_MAPS 15
static COLOR_MAP PlayerColorMap[PLAYER_COLOR_MAPS][1] =
{

    {
        {32, 32, LT_BLUE, LT_BROWN},
    },
    {
        {32, 31, LT_BLUE, LT_GREY},
    },
    {
        {32, 16, LT_BLUE, PURPLE}
    },
    {
        {32, 16, LT_BLUE, RUST_RED},
    },
    {
        {32, 16, LT_BLUE, YELLOW},
    },
    {
        {32, 16, LT_BLUE, DK_GREEN},
    },
    {
        {32, 16, LT_BLUE, GREEN},
    },
    {
        {32, 32, LT_BLUE, LT_BLUE},  // Redundant, but has to be here for position
    },
    {
        {32, 32, LT_BLUE, LT_TAN},
    },
    {
        {32, 16, LT_BLUE, RED},
    },
    {
        {32, 16, LT_BLUE, DK_GREY},
    },
    {
        {32, 16, LT_BLUE, BRIGHT_GREEN},
    },
    {
        {32, 16, LT_BLUE, DK_BLUE},
    },
    {
        {32, 16, LT_BLUE, FIRE},
    },
    {
        {32, 16, LT_BLUE, FIRE},
    }

};

void
InitPalette(void)
{
    static COLOR_MAP AllToRed[] =
    {
        {31, 16, LT_GREY, RED},
        {32, 16, LT_BROWN, RED},
        {32, 16, LT_TAN, RED},
        {16, 16, RUST_RED, RED},
        {16, 16, YELLOW, RED},
        {16, 16, BRIGHT_GREEN, RED},
        {16, 16, DK_GREEN, RED},
        {16, 16, GREEN, RED},
        {32, 16, LT_BLUE, RED},
        {16, 16, PURPLE, RED},
        {16, 16, FIRE, RED}
    };

    static COLOR_MAP AllToBlue[] =
    {
        {31, 32, LT_GREY, LT_BLUE},
        {32, 32, LT_BROWN, LT_BLUE},
        {32, 32, LT_TAN, LT_BLUE},
        {16, 32, RUST_RED, LT_BLUE},
        {16, 32, YELLOW, LT_BLUE},
        {16, 32, BRIGHT_GREEN, LT_BLUE},
        {16, 32, DK_GREEN, LT_BLUE},
        {16, 32, GREEN, LT_BLUE},
        {16, 32, RED, LT_BLUE},
        {16, 32, PURPLE, LT_BLUE},
        {16, 32, FIRE, LT_BLUE}
    };

    static COLOR_MAP AllToGreen[] =
    {
        {31, 16, LT_GREY, GREEN},
        {32, 16, LT_BROWN, GREEN},
        {32, 16, LT_TAN, GREEN},
        {16, 16, RUST_RED, GREEN},
        {16, 16, YELLOW, GREEN},
        {16, 16, BRIGHT_GREEN, GREEN},
        {16, 16, DK_GREEN, GREEN},
        {16, 16, GREEN, GREEN},
        {32, 16, LT_BLUE, GREEN},
        {16, 16, RED, GREEN},
        {16, 16, PURPLE, GREEN},
        {16, 16, FIRE, GREEN}
    };

    static COLOR_MAP NinjaBasic[] =
    {
        {32, 16, LT_TAN, DK_GREY},
        {32, 16, LT_BROWN,DK_GREY},
        {32, 31, LT_BLUE,LT_GREY},
        {16, 16, DK_GREEN,DK_GREY},
        {16, 16, GREEN,  DK_GREY},
        {16, 16, YELLOW, DK_GREY}
    };

    static COLOR_MAP NinjaRed[] =
    {
        {16, 16, DK_TAN, DK_GREY},
        {16, 16, GREEN, DK_TAN},
        {16, 8, DK_BROWN, RED + 8},

        {32, 16, LT_BLUE, RED}
    };

    static COLOR_MAP NinjaGreen[] =
    {
        {16, 16, DK_TAN, DK_GREY},
        {16, 16, GREEN, DK_TAN},
        {16, 8, DK_BROWN, GREEN + 6},

        {32, 16, LT_BLUE, GREEN}
    };

    static COLOR_MAP Illuminate[] =
    {
        {16, 8, LT_GREY, BRIGHT_GREEN},
        {16, 8, DK_GREY, BRIGHT_GREEN},
        {16, 8, LT_BROWN, BRIGHT_GREEN},
        {16, 8, DK_BROWN, BRIGHT_GREEN},
        {16, 8, LT_TAN, BRIGHT_GREEN},
        {16, 8, DK_TAN, BRIGHT_GREEN},
        {16, 8, RUST_RED, BRIGHT_GREEN},
        {16, 8, YELLOW, BRIGHT_GREEN},
        {16, 8, DK_GREEN, BRIGHT_GREEN},
        {16, 8, GREEN, BRIGHT_GREEN},
        {32, 8, LT_BLUE, BRIGHT_GREEN},
        {16, 8, RED, BRIGHT_GREEN},
        {16, 8, PURPLE, BRIGHT_GREEN},
        {16, 8, FIRE, BRIGHT_GREEN}
    };

    static COLOR_MAP BrownRipper = {31, 32, LT_GREY, LT_TAN};

    static COLOR_MAP SkelGore = {16, 16, RED, BRIGHT_GREEN};
    static COLOR_MAP ElectroGore = {16, 16, RED, DK_BLUE};

    static COLOR_MAP MenuHighlight = {16, 16, RED, FIRE};

    unsigned int i;
    short play;

    //
    // Dive palettes
    //
#define FOG_AMT 60 // is 15 in SWP.
#define LAVA_AMT 44 // is 11 in SWP.

    for (i = 0; i < 256; i++)
        tempbuf[i] = i;
    // palette for underwater
    paletteMakeLookupTable(PALETTE_DIVE, tempbuf, 0, 0, FOG_AMT, TRUE);

    for (i = 0; i < 256; i++)
        tempbuf[i] = i;
    paletteMakeLookupTable(PALETTE_FOG, tempbuf, FOG_AMT, FOG_AMT, FOG_AMT, TRUE);

    for (i = 0; i < 256; i++)
        tempbuf[i] = i;
    paletteMakeLookupTable(PALETTE_DIVE_LAVA, tempbuf, LAVA_AMT, 0, 0, TRUE);

    //
    // 1 Range changes
    //

    MapColors(PALETTE_BROWN_RIPPER, BrownRipper, TRUE);
    paletteMakeLookupTable(PALETTE_BROWN_RIPPER, tempbuf, 0, 0, 0, TRUE);

    MapColors(PALETTE_SKEL_GORE, SkelGore, TRUE);
    paletteMakeLookupTable(PALETTE_SKEL_GORE, tempbuf, 0, 0, 0, TRUE);

    MapColors(PALETTE_ELECTRO_GORE, ElectroGore, TRUE);
    paletteMakeLookupTable(PALETTE_ELECTRO_GORE, tempbuf, 0, 0, 0, TRUE);

    MapColors(PALETTE_MENU_HIGHLIGHT, MenuHighlight, TRUE);
    paletteMakeLookupTable(PALETTE_MENU_HIGHLIGHT, tempbuf, 0, 0, 0, TRUE);

    //
    // Multiple range changes
    //

    MapColors(PALETTE_BASIC_NINJA, NinjaBasic[0], TRUE);
    for (i = 1; i < SIZ(NinjaBasic); i++)
        MapColors(PALETTE_BASIC_NINJA, NinjaBasic[i], FALSE);
    paletteMakeLookupTable(PALETTE_BASIC_NINJA, tempbuf, 0, 0, 0, TRUE);

    MapColors(PALETTE_RED_NINJA, NinjaRed[0], TRUE);
    for (i = 1; i < SIZ(NinjaRed); i++)
        MapColors(PALETTE_RED_NINJA, NinjaRed[i], FALSE);
    paletteMakeLookupTable(PALETTE_RED_NINJA, tempbuf, 0, 0, 0, TRUE);

    MapColors(PALETTE_GREEN_NINJA, NinjaGreen[0], TRUE);
    for (i = 1; i < SIZ(NinjaGreen); i++)
        MapColors(PALETTE_GREEN_NINJA, NinjaGreen[i], FALSE);
    paletteMakeLookupTable(PALETTE_GREEN_NINJA, tempbuf, 0, 0, 0, TRUE);

    MapColors(PALETTE_GREEN_LIGHTING, AllToGreen[0], TRUE);
    for (i = 1; i < SIZ(AllToGreen); i++)
        MapColors(PALETTE_GREEN_LIGHTING, AllToGreen[i], FALSE);
    paletteMakeLookupTable(PALETTE_GREEN_LIGHTING, tempbuf, 0, 0, 0, TRUE);

    MapColors(PALETTE_RED_LIGHTING, AllToRed[0], TRUE);
    for (i = 1; i < SIZ(AllToRed); i++)
        MapColors(PALETTE_RED_LIGHTING, AllToRed[i], FALSE);
    paletteMakeLookupTable(PALETTE_RED_LIGHTING, tempbuf, 0, 0, 0, TRUE);

    MapColors(PALETTE_BLUE_LIGHTING, AllToBlue[0], TRUE);
    for (i = 1; i < SIZ(AllToBlue); i++)
        MapColors(PALETTE_BLUE_LIGHTING, AllToBlue[i], FALSE);
    paletteMakeLookupTable(PALETTE_BLUE_LIGHTING, tempbuf, 0, 0, 0, TRUE);

    MapColors(PALETTE_ILLUMINATE, Illuminate[0], TRUE);
    for (i = 1; i < SIZ(Illuminate); i++)
        MapColors(PALETTE_ILLUMINATE, Illuminate[i], FALSE);
    paletteMakeLookupTable(PALETTE_ILLUMINATE, tempbuf, 0, 0, 0, TRUE);

    // PLAYER COLORS - ALSO USED FOR OTHER THINGS
    for (play = 0; play < PLAYER_COLOR_MAPS; play++)
    {
        MapColors(PALETTE_PLAYER0 + play, PlayerColorMap[play][0], TRUE);
        MapColors(PALETTE_PLAYER0 + play, PlayerColorMap[play][0], FALSE);
        paletteMakeLookupTable(PALETTE_PLAYER0 + play, tempbuf, 0, 0, 0, TRUE);
    }

    //
    // Special Brown sludge
    //

    for (i = 0; i < 256; i++)
        tempbuf[i] = i;
    // invert the brown palette
    for (i = 0; i < 32; i++)
        tempbuf[LT_BROWN + i] = (LT_BROWN + 32) - i;
    paletteMakeLookupTable(PALETTE_SLUDGE, tempbuf, 0, 0, 0, TRUE);

}

END_SW_NS
