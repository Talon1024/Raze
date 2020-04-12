//-------------------------------------------------------------------------
/*
Copyright (C) 2016 EDuke32 developers and contributors
Copyright (C) 2019 Christoph Oelckers

This is free software; you can redistribute it and/or
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

#include "ns.h"	// Must come before everything else!
#include "build.h"
#include "osd.h"

#include "keys.h"
#include "names2.h"
#include "panel.h"
#include "game.h"
#include "tags.h"
#include "sector.h"
#include "sprite.h"
#include "weapon.h"
#include "player.h"
#include "jsector.h"
#include "menus.h"
#include "sw_strs.h"
#include "pal.h"
#include "demo.h"
#include "keydef.h"

#include "gamecontrol.h"
#include "gamedefs.h"
#include "config.h"
#include "network.h"
#include "text.h"
#include "version.h"
#include "network.h"

#include "colormap.h"
#include "config.h"
#include "menu/menu.h"
#include "raze_sound.h"
#include "sounds.h"

#include "../../glbackend/glbackend.h"


BEGIN_SW_NS

int handle1;

void Menu_Init(void)
{


}

//----------------------------------------------------------------------------
//
// Implements the native looking menu used for the main menu
// and the episode/skill selection screens, i.e. the parts
// that need to look authentic
//
//----------------------------------------------------------------------------

class SWMainMenu : public DListMenu
{
	void Ticker() override
	{
		// Dynamically enable and disable the save option
		for (unsigned e = 0; e < mDesc->mItems.Size(); ++e)
		{
			auto entry = mDesc->mItems[e];
			if (entry->GetAction(nullptr) == NAME_Savegamemenu)
			{
				entry->mEnabled = gi->CanSave();
			}
		}
	}

	void PreDraw() override
	{
        rotatesprite(160 << 16, 15 << 16, 65536, 0, pic_shadow_warrior,
                     m_defshade, 0, ROTATE_SPRITE_SCREEN_CLIP, 0, 0, xdim - 1, ydim - 1);
	}
};

static bool DidOrderSound;
static int zero = 0;
class SWOrderMenu : public DImageScrollerMenu
{
public:
	SWOrderMenu()
	{
		if (SW_SHAREWARE && !DidOrderSound)
		{
			DidOrderSound = true;
			int choose_snd = STD_RANDOM_RANGE(1000);
			if (choose_snd > 500)
				PlaySound(DIGI_WANGORDER1, v3df_dontpan, CHAN_BODY, CHANF_UI);
			else 
				PlaySound(DIGI_WANGORDER2, v3df_dontpan, CHAN_BODY, CHANF_UI);
		}
	}
};

//----------------------------------------------------------------------------
//
// Menu related game interface functions
//
//----------------------------------------------------------------------------

void GameInterface::DrawNativeMenuText(int fontnum, int state, double xpos, double ypos, float fontscale, const char* text, int flags)
{
	short w, h;
	switch (fontnum)
	{
		case NIT_BigFont:
			MNU_MeasureStringLarge(text, &w, &h);
			if (flags & LMF_Centered) xpos -= w/2;
			MNU_DrawStringLarge(short(xpos), short(ypos), text, state == NIT_InactiveState? 20 : 0);
			break;
		
		case NIT_SmallFont:
			MNU_MeasureString(text, &w, &h);
			if (flags & LMF_Centered) xpos -= w/2;
			MNU_DrawString(short(xpos), short(ypos), text, state == NIT_InactiveState? 20 : 0, 16);
			break;
		
		case NIT_TinyFont:
			MNU_MeasureSmallString(text, &w, &h);
			if (flags & LMF_Centered) xpos -= w/2;
			MNU_DrawSmallString(short(xpos), short(ypos), text, state == NIT_InactiveState? 20 : 0, 16);
			break;
	}
	if (state == NIT_SelectedState)
	{
		int x = int(xpos), y = int(ypos);
		int scale = 65536;
		short w,h;

		if (text)
		{
			scale /= 2;
			x -= mulscale17(tilesiz[pic_yinyang].x,scale) + 2;
			y += 4;
		}
		else
		{
			scale -= (1<<13);
			x -= ((tilesiz[pic_yinyang].x) / 2) - 3;
			y += 8;
		}

		rotatesprite(x << 16, y << 16,
					 scale, 0, pic_yinyang, 2, 0, MenuDrawFlags, 0, 0, xdim - 1, ydim - 1);
	}
}


void GameInterface::MenuOpened()
{
}

void GameInterface::MenuSound(EMenuSounds snd)
{
	switch (snd)
	{
		case CursorSound:
            PlaySound(DIGI_STAR, v3df_dontpan, CHAN_BODY, CHANF_UI);
			break;

		case AdvanceSound:
			PlaySound(DIGI_SWORDSWOOSH, v3df_dontpan, CHAN_BODY, CHANF_UI);
			break;
			
		case CloseSound:
			PlaySound(DIGI_STARCLINK, v3df_dontpan, CHAN_BODY, CHANF_UI);
			break;

		default:
			return;
	}
}

void GameInterface::MenuClosed()
{
	if (!LoadGameOutsideMoveLoop)
	{
		ResumeGame();
		SetRedrawScreen(&Player[myconnectindex]);
	}
}

extern SWBOOL InMenuLevel;
extern SWBOOL DemoMode;
extern SWBOOL ExitLevel, NewGame;

bool GameInterface::CanSave()
{
    return (!CommEnabled && numplayers ==1 && !DemoMode && !InMenuLevel && !TEST(Player[myconnectindex].Flags, PF_DEAD));
}

void GameInterface::StartGame(FGameStartup& gs)
{
    PLAYERp pp = Player + screenpeek;
    int handle = 0;
    int zero = 0;

    // always assumed that a demo is playing

    ready2send = 0;

    if (gs.Episode >= 1)
        Level = 5;
    else
        Level = 1;

    DemoPlaying = FALSE;
    ExitLevel = TRUE;
    NewGame = TRUE;
    DemoMode = FALSE;
    CameraTestMode = FALSE;
	Skill = gs.Skill;
	StopFX();

    //InitNewGame();

    if (Skill == 0)
        PlaySound(DIGI_TAUNTAI3, v3df_none, CHAN_VOICE, CHANF_UI);
    else if (Skill == 1)
        PlaySound(DIGI_NOFEAR, v3df_none, CHAN_VOICE, CHANF_UI);
    else if (Skill == 2)
        PlaySound(DIGI_WHOWANTSWANG, v3df_none, CHAN_VOICE, CHANF_UI);
    else if (Skill == 3)
        PlaySound(DIGI_NOPAIN, v3df_none, CHAN_VOICE, CHANF_UI);

	while (soundEngine->IsSourcePlayingSomething(SOURCE_None, nullptr, CHAN_VOICE))
	{
		DoUpdateSounds();
		handleevents();
	}
}

FSavegameInfo GameInterface::GetSaveSig()
{
	return { SAVESIG_SW, MINSAVEVER_SW, SAVEVER_SW };
}

void GameInterface::DrawMenuCaption(const DVector2& origin, const char* text)
{
	short w, h;
	// Draw the backdrop bar
	rotatesprite(10 << 16, (5-3) << 16, 65536, 0, 2427,
				 2, 0, MenuDrawFlags|ROTATE_SPRITE_CORNER, 0, 0, xdim - 1, ydim - 1);
	MNU_MeasureStringLarge(text, &w, &h);
	MNU_DrawStringLarge(TEXT_XCENTER(w), 5, text, 1);
}

void GameInterface::DrawCenteredTextScreen(const DVector2& origin, const char* text, int position, bool bg)
{
	if (text)
	{
		short width, height = 0;
		MNU_MeasureString("T", &width, &height);

		auto lines = FString(text).Split("\n");
		int y = 100 - (height * lines.Size() / 2);
		for (auto& l : lines)
		{
			short lheight = 0;
			MNU_MeasureString(l, &width, &lheight);
			int x = 160 - width / 2;
			MNU_DrawString(x, y, l, 0, 0);
			y += height;
		}
	}
}



END_SW_NS

//----------------------------------------------------------------------------
//
// Class registration
//
//----------------------------------------------------------------------------


static TMenuClassDescriptor<ShadowWarrior::SWMainMenu> _mm("ShadowWarrior.MainMenu");
static TMenuClassDescriptor<ShadowWarrior::SWOrderMenu> _so("ShadowWarrior.OrderMenu");

void RegisterSWMenus()
{
	menuClasses.Push(&_mm);
	menuClasses.Push(&_so);
}
