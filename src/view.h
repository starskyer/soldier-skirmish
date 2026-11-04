#pragma once
#include<iostream>
#include<SDL.h>
#include<SDL_image.h>
#include<SDL_ttf.h>
#undef main
class Model;
class Controller;

class View {
private:
	SDL_Renderer *renderer = nullptr, *resultRenderer = nullptr;
	SDL_Window *window = nullptr, *resultWindow = nullptr;
	SDL_Event e;
	SDL_Texture* Infantry1Texture = nullptr;
	SDL_Texture* Infantry2Texture = nullptr;
	SDL_Texture* Archer1Texture = nullptr;
	SDL_Texture* Archer2Texture = nullptr;
	SDL_Texture* SiegeGun1Texture = nullptr;
	SDL_Texture* SiegeGun2Texture = nullptr;
	SDL_Texture* BaseTexture = nullptr;
	TTF_Font* Font = nullptr;

	Model* model;
	Controller* controller;

	int start;
	int changeVision;

public:
	int vision;
	bool busy;

	View(Model* model, Controller* controller);

	SDL_Texture* Texture_load(std::string filePath);

	SDL_Texture* CreateFontTexture(std::string text, SDL_Color color, int* width, int* height);

	bool SDLinitialize();

	void printBoardConsole();

	void printBoardSDL();

	void handleEvent(SDL_Event* e);

	void SDLWork();
	void close();
};
