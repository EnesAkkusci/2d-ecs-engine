#include <SDL_image.h>
#include <cstddef>
#include "./AssetManager.h"
#include "../Logger/Logger.h"
#include "SDL_mixer.h"
#include "SDL_render.h"
#include "SDL_surface.h"
#include "SDL_ttf.h"

#ifndef ASSETS_PATH
#define ASSETS_PATH "./assets/"
#endif

AssetManager::AssetManager() {
	Logger::Confirm("Asset Manager consturcted.");
}

AssetManager::~AssetManager() {
	ClearAssets();
	Logger::Confirm("Asset Manager destructed.");
}

void AssetManager::ClearAssets() { 
	for (auto texture : textures) {
		SDL_DestroyTexture(texture.second);
	}
	textures.clear();

	for (auto font : fonts) {
		TTF_CloseFont(font.second);
	}
	fonts.clear();

	for (auto sfx : soundEffects) {
		Mix_FreeChunk(sfx.second);
	}
	soundEffects.clear();
}

void AssetManager::AddTexture(SDL_Renderer* renderer, const std::string_view& assetName, const std::string& filePath) { 
	auto address = ASSETS_PATH"images/"+ filePath;
	SDL_Surface* surface = IMG_Load(address.c_str());
	if(!surface) Logger::Err("Failed to load image at: " + address);
	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_FreeSurface(surface);
	textures.emplace(assetName, texture);
}

SDL_Texture* AssetManager::GetTexture(const std::string& assetName) { 
	auto it = textures.find(assetName);

	#ifdef DEBUG
	Logger::Assert((it != textures.end()), "Non existent asset name: " + assetName);
	#endif

	return it->second;
}

void AssetManager::AddFont(const std::string_view& fontName, const std::string& filePath, int fontSize) {
	auto address = ASSETS_PATH"fonts/" + filePath;
	TTF_Font* font = TTF_OpenFont(address.c_str(), fontSize);
	if(!font) Logger::Err("Failed to load font at: " + address + " .With possible error: " + TTF_GetError());
	fonts.emplace(fontName, font);
}

TTF_Font* AssetManager::GetFont(const std::string& fontName) { 
	auto it = fonts.find(fontName);
	Logger::Assert((it != fonts.end()), "Non existent font name: " + fontName);
	return it->second;
}

void  AssetManager::AddSFX(const std::string_view& sfxName, const std::string& filePath) {
	auto address = ASSETS_PATH"sound-effects/" + filePath;
	Mix_Chunk* sfx = Mix_LoadWAV( address.c_str());
	if(!sfx) Logger::Err("Failed to load sfx at: " + address + " .With possible error: " + Mix_GetError());
	soundEffects.emplace(sfxName, sfx);
}

Mix_Chunk* AssetManager::GetSFX(const std::string& sfxName) {
	auto it = soundEffects.find(sfxName);
	Logger::Assert((it != soundEffects.end()), "Non existent font name: " + sfxName);
	return it->second;
}
