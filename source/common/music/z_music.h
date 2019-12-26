#pragma once

// Totally minimalistic interface - should be all the game modules need.

void Mus_Init();
int Mus_Play(const char *mapname, const char *fn, bool loop);
void Mus_Stop();
bool Mus_IsPlaying();
void Mus_Fade(double seconds);
void Mus_SetPaused(bool on);
void Mus_ResumeSaved();
