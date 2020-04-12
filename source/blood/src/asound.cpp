//-------------------------------------------------------------------------
/*
Copyright (C) 2010-2019 EDuke32 developers and contributors
Copyright (C) 2019 Nuke.YKT

This file is part of NBlood.

NBlood is free software; you can redistribute it and/or
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
#include "common_game.h"
//#include "blood.h"
#include "view.h"
#include "config.h"
#include "db.h"
#include "player.h"
#include "resource.h"
#include "sound.h"
#include "loadsave.h"
#include "raze_sound.h"

BEGIN_BLD_NS

#define kMaxAmbChannel 64

struct AMB_CHANNEL
{
    FSoundID soundID;
    int distance;
    int check;
};

AMB_CHANNEL ambChannels[kMaxAmbChannel];
int nAmbChannels = 0;

void ambProcess(void)
{
    if (!SoundEnabled())
        return;
    for (int nSprite = headspritestat[kStatAmbience]; nSprite >= 0; nSprite = nextspritestat[nSprite])
    {
        spritetype *pSprite = &sprite[nSprite];
        if (pSprite->owner < 0 || pSprite->owner >= kMaxAmbChannel)
            continue;
        int nXSprite = pSprite->extra;
        if (nXSprite > 0 && nXSprite < kMaxXSprites)
        {
            XSPRITE *pXSprite = &xsprite[nXSprite];
            if (pXSprite->state)
            {
                int dx = pSprite->x-gMe->pSprite->x;
                int dy = pSprite->y-gMe->pSprite->y;
                int dz = pSprite->z-gMe->pSprite->z;
                dx >>= 4;
                dy >>= 4;
                dz >>= 8;
                int nDist = ksqrt(dx*dx+dy*dy+dz*dz);
                int vs = mulscale16(pXSprite->data4, pXSprite->busy);
                ambChannels[pSprite->owner].distance += ClipRange(scale(nDist, pXSprite->data1, pXSprite->data2, vs, 0), 0, vs);
            }
        }
    }
    AMB_CHANNEL *pChannel = ambChannels;
    for (int i = 0; i < nAmbChannels; i++, pChannel++)
    {
        if (soundEngine->IsSourcePlayingSomething(SOURCE_Ambient, pChannel, CHAN_BODY, -1))
        {
            if (pChannel->distance > 0)
            {
                soundEngine->ChangeSoundVolume(SOURCE_Ambient, pChannel, CHAN_BODY, pChannel->distance / 255.f);
            }
            else
            {
                // Stop the sound if it cannot be heard so that it doesn't occupy a physical channel.
                soundEngine->StopSound(SOURCE_Ambient, pChannel, CHAN_BODY);
            }
        }
        else if (pChannel->distance > 0)
        {
            FVector3 pt{};
            soundEngine->StartSound(SOURCE_Ambient, pChannel, &pt, CHAN_BODY, CHANF_LOOP|CHANF_TRANSIENT, pChannel->soundID, pChannel->distance / 255.f, ATTN_NONE);
        }
        pChannel->distance = 0;
    }
}

void ambKillAll(void)
{
    AMB_CHANNEL *pChannel = ambChannels;
    for (int i = 0; i < nAmbChannels; i++, pChannel++)
    {
        soundEngine->StopSound(SOURCE_Ambient, pChannel, CHAN_BODY);
        pChannel->soundID = 0;
    }
    nAmbChannels = 0;
}

void ambInit(void)
{
    ambKillAll();
    memset(ambChannels, 0, sizeof(ambChannels));
    for (int nSprite = headspritestat[kStatAmbience]; nSprite >= 0; nSprite = nextspritestat[nSprite]) {
        if (sprite[nSprite].extra <= 0 || sprite[nSprite].extra >= kMaxXSprites) continue;
        
        XSPRITE *pXSprite = &xsprite[sprite[nSprite].extra];
        if (pXSprite->data1 >= pXSprite->data2) continue;
        
        int i; AMB_CHANNEL *pChannel = ambChannels;
        for (i = 0; i < nAmbChannels; i++, pChannel++)
            if (pXSprite->data3 == pChannel->check) break;
        
        if (i == nAmbChannels) {
            
            if (i >= kMaxAmbChannel) {
                sprite[nSprite].owner = -1;
                continue;
            }
                    
            int nSFX = pXSprite->data3;
            auto snd = soundEngine->FindSoundByResID(nSFX);
            if (!snd) {
                //ThrowError("Missing sound #%d used in ambient sound generator %d\n", nSFX);
                viewSetSystemMessage("Missing sound #%d used in ambient sound generator #%d\n", nSFX, nSprite);
                actPostSprite(nSprite, kStatDecoration);
                continue;
            }

            pChannel->soundID = FSoundID(snd);
            pChannel->check = nSFX;
            pChannel->distance = 0;
            nAmbChannels++;

        }

        sprite[nSprite].owner = i;
    }
}

END_BLD_NS
