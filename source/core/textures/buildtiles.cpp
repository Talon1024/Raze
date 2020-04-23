/*
** buildtexture.cpp
** Handling Build textures
**
**---------------------------------------------------------------------------
** Copyright 2019 Christoph Oelckers
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
**
** 1. Redistributions of source code must retain the above copyright
**    notice, this list of conditions and the following disclaimer.
** 2. Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions and the following disclaimer in the
**    documentation and/or other materials provided with the distribution.
** 3. The name of the author may not be used to endorse or promote products
**    derived from this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
** IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
** OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
** IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
** INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
** NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
** THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**---------------------------------------------------------------------------
**
**
*/

#include "files.h"
#include "zstring.h"
#include "textures.h"
#include "image.h"

#include "baselayer.h"
#include "palette.h"
#include "m_crc32.h"
#include "build.h"
#include "gamecontrol.h"
#include "texturemanager.h"

enum
{
	MAXARTFILES_BASE = 200,
	MAXARTFILES_TOTAL = 220
};


BuildTiles TileFiles;

//==========================================================================
//
// 
//
//==========================================================================

picanm_t tileConvertAnimFormat(int32_t const picanimraw)
{
	// Unpack a 4 byte packed anim descriptor into something more accessible
	picanm_t anm;
	anm.num = picanimraw & 63;
	anm.xofs = (picanimraw >> 8) & 255;
	anm.yofs = (picanimraw >> 16) & 255;
	anm.sf = ((picanimraw >> 24) & 15) | (picanimraw & 192);
	anm.extra = (picanimraw >> 28) & 15;
	return anm;
}


//==========================================================================
//
// Base class for Build tile textures
// This needs a few subclasses for different use cases.
//
//==========================================================================

FBitmap FTileTexture::GetBgraBitmap(const PalEntry* remap, int* ptrans)
{
	FBitmap bmp;
	TArray<uint8_t> buffer;
	bmp.Create(Width, Height);
	auto ppix = GetRawData();
	if (ppix) bmp.CopyPixelData(0, 0, ppix, Width, Height, Height, 1, 0, remap);
	return bmp;
}

TArray<uint8_t> FTileTexture::Get8BitPixels(bool alphatex)
{
	TArray<uint8_t> buffer(Width * Height, true);
	auto p = GetRawData();
	if (p) memcpy(buffer.Data(), p, buffer.Size());
	else memset(buffer.Data(), 0, buffer.Size());
	return buffer;
}

//==========================================================================
//
// Tile textures are owned by their containing file object.
//
//==========================================================================

FArtTile* GetTileTexture(const char* name, const TArray<uint8_t>& backingstore, uint32_t offset, int width, int height)
{
	auto tex = new FArtTile(backingstore, offset, width, height);
	auto p = &backingstore[offset];
	auto siz = width * height;
	for (int i = 0; i < siz; i++, p++)
	{
		// move transparent color to index 0 to get in line with the rest of the texture management.
		if (*p == 0) *p = 255;
		else if (*p == 255) *p = 0;
	}
	if (tex)
	{
		tex->SetName(name);
	}
	return tex;
}

//==========================================================================
//
// 
//
//==========================================================================

void BuildTiles::AddTile(int tilenum, FTexture* tex, bool permap)
{
	assert(AllTiles.Find(tex) == AllTiles.Size() && AllMapTiles.Find(tex) == AllMapTiles.Size());
	auto& array = permap ? AllMapTiles : AllTiles;
	array.Push(tex);
	tiles[tilenum] = tex;
	if (!permap) tilesbak[tilenum] = tex;
}

//===========================================================================
//
// AddTiles
//
// Adds all the tiles in an artfile to the texture manager.
//
//===========================================================================

void BuildTiles::AddTiles (int firsttile, TArray<uint8_t>& RawData, bool permap)
{

	const uint8_t *tiles = RawData.Data();
//	int numtiles = LittleLong(((uint32_t *)tiles)[1]);	// This value is not reliable
	int tilestart = LittleLong(((int *)tiles)[2]);
	int tileend = LittleLong(((int *)tiles)[3]);
	const uint16_t *tilesizx = &((const uint16_t *)tiles)[8];
	const uint16_t *tilesizy = &tilesizx[tileend - tilestart + 1];
	const uint32_t *picanm = (const uint32_t *)&tilesizy[tileend - tilestart + 1];
	const uint8_t *tiledata = (const uint8_t *)&picanm[tileend - tilestart + 1];

	if (firsttile != -1)
	{
		tileend = tileend - tilestart + firsttile;
		tilestart = firsttile;
	}

	for (int i = tilestart; i <= tileend; ++i)
	{
		int pic = i - tilestart;
		int width = LittleShort(tilesizx[pic]);
		int height = LittleShort(tilesizy[pic]);
		uint32_t anm = LittleLong(picanm[pic]);
		int size = width*height;

		if (width <= 0 || height <= 0) continue;

		auto tex = GetTileTexture("", RawData, uint32_t(tiledata - tiles), width, height);
		AddTile(i, tex);
		this->tiledata[i].picanm = tileConvertAnimFormat(anm);
		tiledata += size;
	}
}

//===========================================================================
//
// Replacement textures
//
//===========================================================================

void BuildTiles::AddReplacement(int picnum, const HightileReplacement& replace)
{
	auto& Hightiles = tiledata[picnum].Hightiles;
	for (auto& ht : Hightiles)
	{
		if (replace.palnum == ht.palnum && (replace.faces[1] == nullptr) == (ht.faces[1] == nullptr))
		{
			ht = replace;
			return;
		}
	}
	Hightiles.Push(replace);
}

void BuildTiles::DeleteReplacement(int picnum, int palnum)
{
	auto& Hightiles = tiledata[picnum].Hightiles;
	for (int i = Hightiles.Size() - 1; i >= 0; i--)
	{
		if (Hightiles[i].palnum == palnum) Hightiles.Delete(i);
	}
}

//===========================================================================
//
//
//
//===========================================================================

HightileReplacement* BuildTiles::FindReplacement(int picnum, int palnum, bool skybox)
{
	auto& Hightiles = tiledata[picnum].Hightiles;
	for (;;)
	{
		for (auto& rep : Hightiles)
		{
			if (rep.palnum == palnum && (rep.faces[1] != nullptr) == skybox) return &rep;
		}
		if (!palnum || palnum >= MAXPALOOKUPS - RESERVEDPALS) break;
		palnum = 0;
	}
	return nullptr;	// no replacement found
}


//===========================================================================
//
// CountTiles
//
// Returns the number of tiles provided by an artfile
//
//===========================================================================

int CountTiles (const char *fn, const uint8_t *RawData)
{
	int version = LittleLong(*(uint32_t *)RawData);
	if (version != 1)
	{
		Printf("%s: Invalid art file version.  Must be 1, got %d\n", fn, version);
		return 0;
	}

	int tilestart = LittleLong(((uint32_t *)RawData)[2]);
	int tileend = LittleLong(((uint32_t *)RawData)[3]);

	if ((unsigned)tilestart >= MAXUSERTILES || (unsigned)tileend >= MAXUSERTILES)
	{
		Printf("%s: Invalid tilestart or tileend\n", fn);
		return 0;
	}
	if (tileend < tilestart)
	{
		Printf("%s: tileend < tilestart\n", fn);
		return 0;
	}

	return tileend >= tilestart ? tileend - tilestart + 1 : 0;
}

//===========================================================================
//
// CloseAllMapArt
//
// Closes all per-map ART files
//
//===========================================================================

void BuildTiles::CloseAllMapArt()
{
	AllMapTiles.DeleteAndClear();
	PerMapArtFiles.DeleteAndClear();
}

//===========================================================================
//
// ClearTextureCache
//
// Deletes all hardware textures
//
//===========================================================================

void BuildTiles::ClearTextureCache(bool artonly)
{
	for (auto tex : AllTiles)
	{
		tex->SystemTextures.Clean(true, true);
	}
	for (auto tex : AllMapTiles)
	{
		tex->SystemTextures.Clean(true, true);
	}
	if (!artonly)
	{
		decltype(textures)::Iterator it(textures);
		decltype(textures)::Pair* pair;
		while (it.NextPair(pair))
		{
			pair->Value->SystemTextures.Clean(true, true);
		}
	}
}

//===========================================================================
//
// InvalidateTile
//
//===========================================================================

void BuildTiles::InvalidateTile(int num)
{
	if ((unsigned) num < MAXTILES)
	{
		auto tex = tiles[num];
		tex->SystemTextures.Clean(true, true);
		for (auto &rep : tiledata[num].Hightiles)
		{
			for (auto &reptex : rep.faces)
			{
				if (reptex) reptex->SystemTextures.Clean(true, true);
			}
		}
		tiledata[num].rawCache.data.Clear();
	}
}

//===========================================================================
//
// MakeCanvas
//
// Turns texture into a canvas (i.e. camera texture)
//
//===========================================================================

void BuildTiles::MakeCanvas(int tilenum, int width, int height)
{
	auto canvas = ValidateCustomTile(tilenum, ReplacementType::Canvas);
	canvas->SetSize(width, height);
}

//===========================================================================
//
// LoadArtFile
//
// Returns the number of tiles found.
//
// let's load everything into memory on startup.
// Even for Ion Fury this will merely add 80 MB, because the engine already needs to cache the data, albeit in a compressed-per-lump form,
// so its 100MB art file will only have a partial impact on memory.
//
//===========================================================================

int BuildTiles::LoadArtFile(const char *fn, bool mapart, int firsttile)
{
	auto old = FindFile(fn);
	if (old >= ArtFiles.Size())	// Do not process if already loaded.
	{
		FileReader fr = fileSystem.OpenFileReader(fn);
		if (fr.isOpen())
		{
			auto artdata = fr.Read();
			const uint8_t *artptr = artdata.Data();
			if (artdata.Size() > 16)
			{
				if (memcmp(artptr, "BUILDART", 8) == 0)
				{
					artdata.Delete(0, 8);
				}
				// Only load the data if the header is present
				if (CountTiles(fn, artptr) > 0)
				{
					auto& descs = mapart ? PerMapArtFiles : ArtFiles;
					auto file = new BuildArtFile;
					descs.Push(file);
					file->filename = fn;
					file->RawData = std::move(artdata);
					AddTiles(firsttile, file->RawData, mapart);
				}
			}
		}
		else
		{
			//Printf("%s: file not found\n", fn);
			return -1;
		}
	}
	else
	{
		// Reuse the old one but move it to the top. (better not.)
		//auto fd = std::move(ArtFiles[old]);
		//ArtFiles.Delete(old);
		//ArtFiles.Push(std::move(fd));
	}
	return 0;
}

//==========================================================================
//
// 
//
//==========================================================================

void BuildTiles::LoadArtSet(const char* filename)
{
	for (int index = 0; index < MAXARTFILES_BASE; index++)
	{
		FStringf fn(filename, index);
		LoadArtFile(fn, false);
	}
	for (auto& addart : addedArt)
	{
		LoadArtFile(addart, false);
	}
}


//==========================================================================
//
// Checks if a custom tile has alredy been added to the list.
// For each tile index there may only be one replacement and its
// type may never change!
//
// All these uses will need some review further down the line so that the texture manager's content is immutable.
//
//==========================================================================

FTexture* BuildTiles::ValidateCustomTile(int tilenum, ReplacementType type)
{
	if (tilenum < 0 || tilenum >= MAXTILES) return nullptr;
	if (tiles[tilenum] != tilesbak[tilenum]) return nullptr;	// no mucking around with map tiles.
	auto tile = tiles[tilenum];
	auto reptype = tiledata[tilenum].replacement;
	if (reptype == type) return tile;		// already created
	if (reptype > ReplacementType::Art) return nullptr;		// different custom type - cannot replace again.
	FTexture* replacement = nullptr;
	if (type == ReplacementType::Writable)
	{
		// Creates an empty writable tile.
		// Current use cases are:
		// Camera textures (should be made to be creatable by the hardware renderer instead of falling back on the software renderer.)
		// thumbnails for savegame and loadgame (should bypass the texture manager entirely.)
		// view tilting in the software renderer (this should just use a local buffer instead of relying on the texture manager.)
		// Movie playback (like thumbnails this should bypass the texture manager entirely.)
		// Blood's 'lens' effect (apparently MP only) - combination of a camera texture with a distortion map - should be made a shader effect to be applied to the camera texture.
		replacement = new FWritableTile;
	}
	else if (type == ReplacementType::Restorable)
	{
		// This is for modifying an existing tile.
		// It only gets used for the crosshair and a few specific effects:
		// A) the fire in Blood.
		// B) the pin display in Redneck Rampage's bowling lanes.
		// C) Exhumed's menu plus one special effect tile.
		// All of these effects should probably be redone without actual texture hacking...
		if (tile->GetTexelWidth() == 0 || tile->GetTexelHeight() == 0) return nullptr;	// The base must have a size for this to work.
		// todo: invalidate hardware textures for tile.
		replacement = new FRestorableTile(tile);
	}
	else if (type == ReplacementType::Canvas)
	{
		replacement = new FCanvasTexture("camera", 0, 0);
	}
	else return nullptr;
	AddTile(tilenum, replacement);
	return replacement;
}

//==========================================================================
//
//  global interface
//
//==========================================================================

int32_t BuildTiles::artLoadFiles(const char* filename)
{
	TileFiles.LoadArtSet(filename);
	memset(gotpic, 0, sizeof(gotpic));
	return 0;
}

//==========================================================================
//
// Creates a tile for displaying custom content
//
//==========================================================================

uint8_t* BuildTiles::tileCreate(int tilenum, int width, int height)
{
	if (width <= 0 || height <= 0) return nullptr;
	auto tex = ValidateCustomTile(tilenum, ReplacementType::Writable);
	if (tex == nullptr) return nullptr;
	auto wtex = static_cast<FWritableTile*>(tex);
	if (!wtex->Resize(width, height)) return nullptr;
	return wtex->GetRawData();
}

//==========================================================================
//
// Makes a tile writable - only used for a handful of special cases
// (todo: Investigate how to get rid of this)
//
//==========================================================================

uint8_t * BuildTiles::tileMakeWritable(int num)
{
	auto tex = ValidateCustomTile(num, ReplacementType::Restorable);
	auto wtex = static_cast<FWritableTile*>(tex);
	return wtex ? wtex->GetRawData() : nullptr;
}

//==========================================================================
//
// Returns checksum for a given tile
//
//==========================================================================

int32_t tileGetCRC32(int tileNum)
{
	if ((unsigned)tileNum >= (unsigned)MAXTILES) return 0;
	auto tile = dynamic_cast<FArtTile*>(TileFiles.tiles[tileNum]);	// only consider original ART tiles.
	if (!tile) return 0;
	auto pixels = tile->GetRawData();
	if (!pixels) return 0;

	auto size = tile->GetTexelWidth() * tile->GetTexelHeight();
	if (size == 0) return 0;

	// Temporarily revert the data to its original form with 255 being transparent. Otherwise the CRC won't match.
	auto p = pixels;
	for (int i = 0; i < size; i++, p++)
	{
		// move transparent color to index 0 to get in line with the rest of the texture management.
		if (*p == 0) *p = 255;
		else if (*p == 255) *p = 0;
	}

	auto crc = crc32(0, (const Bytef*)pixels, size);

	// ... and back again.
	p = pixels;
	for (int i = 0; i < size; i++, p++)
	{
		// move transparent color to index 0 to get in line with the rest of the texture management.
		if (*p == 0) *p = 255;
		else if (*p == 255) *p = 0;
	}
	return crc;
}


//==========================================================================
//
// Import a tile from an external image.
// This has been signifcantly altered so it may not cover everything yet.
//
//==========================================================================

int tileImportFromTexture(const char* fn, int tilenum, int alphacut, int istexture)
{
	FTextureID texid = TexMan.CheckForTexture(fn, ETextureType::Any);
	if (!texid.isValid()) return -1;
	auto tex = TexMan.GetTexture(texid);
	//tex->alphaThreshold = 255 - alphacut;

	int32_t xsiz = tex->GetTexelWidth(), ysiz = tex->GetTexelHeight();

	if (xsiz <= 0 || ysiz <= 0)
		return -2;

	TileFiles.tiles[tilenum] = tex;
#pragma message("tileImportFromTexture needs rework!")	// Reminder so that this place isn't forgotten.
//#if 0
	// Does this make any difference when the texture gets *properly* inserted into the tile array?
	//if (istexture)
		tileSetHightileReplacement(tilenum, 0, fn, (float)(255 - alphacut) * (1.f / 255.f), 1.0f, 1.0f, 1.0, 1.0, 0);	// At the moment this is the only way to load the texture. The texture creation code is not ready yet for downconverting an image.
//#endif
	return 0;

}

//==========================================================================
//
// Copies a tile into another and optionally translates its palette.
//
//==========================================================================

void tileCopy(int tile, int source, int pal, int xoffset, int yoffset, int flags)
{
	// Todo. Since I do not know if some mod needs this it's of low priority now.
	// Let's get things working first.
	picanm_t* picanm = nullptr;
	picanm_t* sourceanm = nullptr;

	if (pal == -1 && tile == source)
	{
		// Only modify the picanm info.
		FTexture* tex = TileFiles.tiles[tile];
		if (!tex) return;
		picanm = &TileFiles.tiledata[tile].picanm;
		sourceanm = picanm;
	}
	else
	{
		if (source == -1) source = tile;
		FTexture* tex = TileFiles.tiles[source];
		if (!tex) return;
		sourceanm = &TileFiles.tiledata[source].picanm;

		TArray<uint8_t> buffer = tex->Get8BitPixels(false);

		if (pal != -1)
		{
			auto remap = (const uint8_t*)LookupTables[pal].GetChars();
			for (auto& pixel : buffer)
			{
				pixel = remap[pixel];
			}
		}
		tex = new FLooseTile(buffer, tex->GetTexelWidth(), tex->GetTexelHeight());
		picanm = &TileFiles.tiledata[tile].picanm;
		TileFiles.AddTile(tile, tex);
	}

	picanm->xofs = xoffset != -1024 ? clamp(xoffset, -128, 127) : sourceanm->xofs;
	picanm->yofs = yoffset != -1024 ? clamp(yoffset, -128, 127) : sourceanm->yofs;
	picanm->sf = (picanm->sf & ~PICANM_MISC_MASK) | (sourceanm->sf & PICANM_MISC_MASK) | flags;
}

//==========================================================================
//
// Clear map specific ART
//
//==========================================================================

void artClearMapArt(void)
{
	TileFiles.CloseAllMapArt();
	memcpy(TileFiles.tiles, TileFiles.tilesbak, sizeof(TileFiles.tiles));
}

//==========================================================================
//
// Load map specficied ART
//
//==========================================================================
static FString currentMapArt;

void artSetupMapArt(const char* filename)
{
	if (currentMapArt.CompareNoCase(filename)) return;
	currentMapArt = filename;
	artClearMapArt();

	FStringf firstname("%s_00.art", filename);
	auto fr = fileSystem.OpenFileReader(firstname);
	if (!fr.isOpen()) return;

	for (bssize_t i = 0; i < MAXARTFILES_TOTAL - MAXARTFILES_BASE; i++)
	{
		FStringf fullname("%s_%02d.art", filename, i);
		TileFiles.LoadArtFile(fullname, true);
	}
}

//==========================================================================
//
//
//
//==========================================================================

void tileDelete(int tile)
{
	TileFiles.tiles[tile] = TileFiles.tilesbak[tile] = TileFiles.Placeholder;
	vox_undefine(tile);
	md_undefinetile(tile);
	tileRemoveReplacement(tile);
}

//==========================================================================
//
//
//
//==========================================================================

void tileRemoveReplacement(int tile)
{
	if ((unsigned)tile >= MAXTILES) return;
	TileFiles.DeleteReplacements(tile);
}

//==========================================================================
//
//
//
//==========================================================================

void tileSetDummy(int tile, int width, int height)
{
	if (width == 0 || height == 0)
	{
		tileDelete(tile);
	}
	else if (width > 0 && height > 0)
	{
		auto dtile = new FDummyTile(width, height);
		TileFiles.AddTile(tile, dtile);
	}
}

//==========================================================================
//
//
//
//==========================================================================

int BuildTiles::findUnusedTile(void)
{
	static int lastUnusedTile = MAXUSERTILES - 1;

	for (; lastUnusedTile >= 0; --lastUnusedTile)
	{
		auto tex = TileFiles.tiles[lastUnusedTile];
		if (!tex || tex->GetTexelWidth() <= 0 || tex->GetTexelHeight() <= 0) return lastUnusedTile;
	}
	return -1;
}

int BuildTiles::tileCreateRotated(int tileNum)
{
	if ((unsigned)tileNum >= MAXTILES) return tileNum;
	auto tex = TileFiles.tiles[tileNum];
	if (!tex || tex->GetTexelWidth() <= 0 || tex->GetTexelHeight() <= 0) return tileNum;
	TArray<uint8_t> buffer = tex->Get8BitPixels(false);
	TArray<uint8_t> dbuffer(tex->GetTexelWidth() * tex->GetTexelHeight(), true);

	auto src = buffer.Data();
	auto dst = dbuffer.Data();

	auto width = tex->GetTexelWidth();
	auto height = tex->GetTexelHeight();
	for (int x = 0; x < width; ++x)
	{
		int xofs = width - x - 1;
		int yofs = height * x;

		for (int y = 0; y < height; ++y)
			*(dst + y * width + xofs) = *(src + y + yofs);
	}

	auto dtex = new FLooseTile(dbuffer, tex->GetTexelHeight(), tex->GetTexelWidth());
	int index = findUnusedTile();
	bool mapart = TileFiles.tiles[tileNum] != TileFiles.tilesbak[tileNum];
	TileFiles.AddTile(index, dtex, mapart);
	return index;
}

void tileSetAnim(int tile, const picanm_t& anm)
{

}

//==========================================================================
//
//
//
//==========================================================================

void BuildTiles::CloseAll()
{
	decltype(textures)::Iterator it(textures);
	decltype(textures)::Pair* pair;
	while (it.NextPair(pair)) delete pair->Value;
	textures.Clear();
	CloseAllMapArt();
	ArtFiles.DeleteAndClear();
	AllTiles.DeleteAndClear();
	if (Placeholder) delete Placeholder;
	Placeholder = nullptr;
}

//==========================================================================
//
//   Specifies a replacement texture for an ART tile.
//
//==========================================================================

int tileSetHightileReplacement(int picnum, int palnum, const char *filename, float alphacut, float xscale, float yscale, float specpower, float specfactor, uint8_t flags)
{
    if ((uint32_t)picnum >= (uint32_t)MAXTILES) return -1;
    if ((uint32_t)palnum >= (uint32_t)MAXPALOOKUPS) return -1;

	auto tex = TileFiles.tiles[picnum];
	if (tex->GetTexelWidth() <= 0 || tex->GetTexelHeight() <= 0)
	{
		Printf("Warning: defined hightile replacement for empty tile %d.", picnum);
		return -1;	// cannot add replacements to empty tiles, must create one beforehand
	}
	HightileReplacement replace = {};

	FTextureID texid = TexMan.CheckForTexture(filename, ETextureType::Any);
	if (!texid.isValid()) 
	{
		Printf("%s: Replacement for tile %d does not exist or is invalid\n", filename, picnum);
		return -1;
	}

	replace.faces[0] = TexMan.GetTexture(texid);
	if (replace.faces[0] == nullptr)
    replace.alphacut = min(alphacut,1.f);
	replace.scale = { xscale, yscale };
    replace.specpower = specpower; // currently unused
    replace.specfactor = specfactor; // currently unused
    replace.flags = flags;
	replace.palnum = (uint16_t)palnum;
	TileFiles.AddReplacement(picnum, replace);
    return 0;
}


//==========================================================================
//
//  Define the faces of a skybox
//
//==========================================================================

int tileSetSkybox(int picnum, int palnum, const char **facenames, int flags )
{
    if ((uint32_t)picnum >= (uint32_t)MAXTILES) return -1;
    if ((uint32_t)palnum >= (uint32_t)MAXPALOOKUPS) return -1;

	auto tex = TileFiles.tiles[picnum];
	if (tex->GetTexelWidth() <= 0 || tex->GetTexelHeight() <= 0)
	{
		Printf("Warning: defined skybox replacement for empty tile %d.", picnum);
		return -1;	// cannot add replacements to empty tiles, must create one beforehand
	}
	HightileReplacement replace = {};
	
	for (auto &face : replace.faces)
	{
		FTextureID texid = TexMan.CheckForTexture(*facenames, ETextureType::Any);
		if (!texid.isValid())
		{
			Printf("%s: Skybox image for tile %d does not exist or is invalid\n", *facenames, picnum);
			return -1;
		}
		face = TexMan.GetTexture(texid);
	}
    replace.flags = flags;
	replace.palnum = (uint16_t)palnum;
	TileFiles.AddReplacement(picnum, replace);
	return 0;
}

//==========================================================================
//
//  Remove a replacement
//
//==========================================================================

int tileDeleteReplacement(int picnum, int palnum)
{
    if ((uint32_t)picnum >= (uint32_t)MAXTILES) return -1;
    if ((uint32_t)palnum >= (uint32_t)MAXPALOOKUPS) return -1;
	auto tex = TileFiles.tiles[picnum];
	TileFiles.DeleteReplacement(picnum, palnum);
    return 0;
}


//==========================================================================
//
//  Copy a block of a tile.
//  Only used by RR's bowling lane.
//
//==========================================================================

void tileCopySection(int tilenum1, int sx1, int sy1, int xsiz, int ysiz, int tilenum2, int sx2, int sy2)
{
	int xsiz1 = tilesiz[tilenum1].x;
	int ysiz1 = tilesiz[tilenum1].y;
	int xsiz2 = tilesiz[tilenum2].x;
	int ysiz2 = tilesiz[tilenum2].y;
	if (xsiz1 > 0 && ysiz1 > 0 && xsiz2 > 0 && ysiz2 > 0)
	{
		auto p1 = tilePtr(tilenum1);
		auto p2 = tileData(tilenum2);
		if (p2 == nullptr) return;	// Error: Destination is not writable.
		
		int x1 = sx1;
		int x2 = sx2;
		for (int i=0; i<xsiz; i++)
		{
			int y1 = sy1;
			int y2 = sy2;
			for (int j=0; j<ysiz; j++)
			{
				if (x2 >= 0 && y2 >= 0 && x2 < xsiz2 && y2 < ysiz2)
				{
					auto src = p1[x1 * ysiz1 + y1];
					if (src != TRANSPARENT_INDEX)
						p2[x2 * ysiz2 + y2] = src;
				}
				
				y1++;
				y2++;
				if (y1 >= ysiz1) y1 = 0;
			}
			x1++;
			x2++;
			if (x1 >= xsiz1) x1 = 0;
		}
	}
	TileFiles.InvalidateTile(tilenum2);
}



TileSiz tilesiz;
PicAnm picanm;
