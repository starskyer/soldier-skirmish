#include<iostream>
#include<stdexcept>
#include<SDL.h>
#include<SDL_image.h>
#include<SDL_ttf.h>
#include "base.h"
#include "vision.h"
#include "soldier.h"
#include "model.h"
#include "view.h"
#include "controller.h"
#include "chessboard.h"
#include "globalParameters.h"
#include "runtimeAssets.h"

View::View(Model* model, Controller* controller) {
	vision = 0;
	start = 0;
	changeVision = 1;
	busy = false;
	this->model = model;
	this->controller = controller;
	model->registerObservers(this);
}

SDL_Texture* View::Texture_load(std::string filePath) {
	SDL_Texture* texture;
	SDL_Surface* surface = IMG_Load(filePath.c_str());
	if (surface == NULL) {
		std::cout << "Failed to load image from " << filePath << std::endl;
		return NULL;
	}

	texture = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_FreeSurface(surface);

	if (texture == NULL) {
		std::cout << "Failed to create texture from surface." << std::endl;
		return NULL;
	}

	return texture;
}

SDL_Texture* View::CreateFontTexture(std::string text, SDL_Color color, int* width, int* height) {
	SDL_Texture* texture;
	SDL_Surface* surface = TTF_RenderText_Solid(Font, text.c_str(), color);
	if (surface == NULL) {
		std::cout << "Failed to render text surface." << std::endl;
		throw std::runtime_error(TTF_GetError());
	}

	texture = SDL_CreateTextureFromSurface(renderer, surface);
	*width = surface->w;
	*height = surface->h;
	SDL_FreeSurface(surface);

	if (texture == NULL) {
		std::cout << "Failed to create texture from surface." << std::endl;
		throw std::runtime_error(SDL_GetError());
	}

	return texture;
}

bool View::SDLinitialize() {
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		std::cout << "Failed to initialize SDL: " << SDL_GetError() << std::endl;
		return false;
	}

	if (TTF_Init() == -1) {
		std::cout << "Failed to initialize SDL_ttf: " << SDL_GetError() << std::endl;
		return false;
	}
	if ((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) == 0) {
		std::cerr << "Failed to initialize PNG support: " << IMG_GetError() << '\n';
		return false;
	}

	window = SDL_CreateWindow("Soldier Skirmish", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
	if (window == NULL) {
		std::cout << "Failed to create window: " << SDL_GetError() << std::endl;
		return false;
	}

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (!renderer) renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
	if (renderer == NULL) {
		std::cout << "Failed to create renderer: " << SDL_GetError() << std::endl;
		return false;
	}

	Infantry1Texture = Texture_load(assetPath("texture/Infantry1.png"));
	Infantry2Texture = Texture_load(assetPath("texture/Infantry2.png"));
	Archer1Texture = Texture_load(assetPath("texture/Archer1.png"));
	Archer2Texture = Texture_load(assetPath("texture/Archer2.png"));
	SiegeGun1Texture = Texture_load(assetPath("texture/SiegeGun1.png"));
	SiegeGun2Texture = Texture_load(assetPath("texture/SiegeGun2.png"));
	BaseTexture = Texture_load(assetPath("texture/Base.png"));
	Font = TTF_OpenFont(assetPath("font/NotoSans.ttf").c_str(), 28);
	if (!Infantry1Texture || !Infantry2Texture || !Archer1Texture || !Archer2Texture ||
		!SiegeGun1Texture || !SiegeGun2Texture || !BaseTexture || !Font) {
		std::cerr << "Texture/font loading failed: " << TTF_GetError() << '\n';
		return false;
	}

	std::cout << "SDL successfully initialized." << std::endl;
	return true;
}

void View::printBoardConsole() {
	std::cout << "\033[22A";
	model->chessboard->printBoardConsole();
	std::cout.flush();
}

void View::printBoardSDL() {
	int squareLength = WINDOW_WIDTH / model->chessboard->getLength(); // x
	int squareWidth = (WINDOW_HEIGHT - MENU_HEIGHT - BASEBAR_HEIGHT) / model->chessboard->getWidth(); // y

	// Background color
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // white
	SDL_RenderClear(renderer);

	// Grid line
	SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255); // grey
	for (int i = 0; i < model->chessboard->getLength() - 1; ++i)
		SDL_RenderDrawLine(renderer, (i + 1) * squareLength, BASEBAR_HEIGHT, (i + 1) * squareLength, WINDOW_HEIGHT - MENU_HEIGHT);
	for (int i = 0; i < model->chessboard->getWidth() - 1; ++i)
		SDL_RenderDrawLine(renderer, 0, (i + 1) * squareWidth + BASEBAR_HEIGHT, WINDOW_WIDTH, (i + 1) * squareWidth + BASEBAR_HEIGHT);

	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // black
	SDL_RenderDrawLine(renderer, 0, BASEBAR_HEIGHT, WINDOW_WIDTH, BASEBAR_HEIGHT);
	SDL_RenderDrawLine(renderer, 0, WINDOW_HEIGHT - MENU_HEIGHT, WINDOW_WIDTH, WINDOW_HEIGHT - MENU_HEIGHT);
	SDL_RenderDrawLine(renderer, WINDOW_WIDTH / 2, 0, WINDOW_WIDTH / 2, BASEBAR_HEIGHT);

	// Menu (Start, Pause and Change Vision buttons)
	SDL_Color textColor = { 0, 0, 0, 255 }; // black text color
	SDL_Texture* textTexture;
	SDL_Rect rect, textRect;
	int textWidth, textHeight;

	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // black
	SDL_RenderDrawLine(renderer, WINDOW_WIDTH / 12, WINDOW_HEIGHT - 3 * MENU_HEIGHT / 4, 3 * WINDOW_WIDTH / 12, WINDOW_HEIGHT - 3 * MENU_HEIGHT / 4);
	SDL_RenderDrawLine(renderer, WINDOW_WIDTH / 12, WINDOW_HEIGHT - MENU_HEIGHT / 4, 3 * WINDOW_WIDTH / 12, WINDOW_HEIGHT - MENU_HEIGHT / 4);
	SDL_RenderDrawLine(renderer, WINDOW_WIDTH / 12, WINDOW_HEIGHT - 3 * MENU_HEIGHT / 4, WINDOW_WIDTH / 12, WINDOW_HEIGHT - MENU_HEIGHT / 4);
	SDL_RenderDrawLine(renderer, 3 * WINDOW_WIDTH / 12, WINDOW_HEIGHT - 3 * MENU_HEIGHT / 4, 3 * WINDOW_WIDTH / 12, WINDOW_HEIGHT - MENU_HEIGHT / 4);
	rect = { WINDOW_WIDTH / 12 + 1, WINDOW_HEIGHT - 3 * MENU_HEIGHT / 4 + 1, WINDOW_WIDTH / 6 - 2, MENU_HEIGHT / 2 - 2 };
	if (start == 1) {
		SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255); // grey
		SDL_RenderFillRect(renderer, &rect);
	}
	textTexture = CreateFontTexture("Start", textColor, &textWidth, &textHeight);
	textRect = { WINDOW_WIDTH / 6 - textWidth / 2, WINDOW_HEIGHT - MENU_HEIGHT / 2 - textHeight / 2, textWidth, textHeight };
	SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
	SDL_DestroyTexture(textTexture);

	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // black
	SDL_RenderDrawLine(renderer, 9 * WINDOW_WIDTH / 12, WINDOW_HEIGHT - 3 * MENU_HEIGHT / 4, 11 * WINDOW_WIDTH / 12, WINDOW_HEIGHT - 3 * MENU_HEIGHT / 4);
	SDL_RenderDrawLine(renderer, 9 * WINDOW_WIDTH / 12, WINDOW_HEIGHT - MENU_HEIGHT / 4, 11 * WINDOW_WIDTH / 12, WINDOW_HEIGHT - MENU_HEIGHT / 4);
	SDL_RenderDrawLine(renderer, 9 * WINDOW_WIDTH / 12, WINDOW_HEIGHT - 3 * MENU_HEIGHT / 4, 9 * WINDOW_WIDTH / 12, WINDOW_HEIGHT - MENU_HEIGHT / 4);
	SDL_RenderDrawLine(renderer, 11 * WINDOW_WIDTH / 12, WINDOW_HEIGHT - 3 * MENU_HEIGHT / 4, 11 * WINDOW_WIDTH / 12, WINDOW_HEIGHT - MENU_HEIGHT / 4);
	rect = { 9 * WINDOW_WIDTH / 12 + 1, WINDOW_HEIGHT - 3 * MENU_HEIGHT / 4 + 1, WINDOW_WIDTH / 6 - 2, MENU_HEIGHT / 2 - 2 };
	if (changeVision == 1) {
		SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255); // grey
		SDL_RenderFillRect(renderer, &rect);
	}
	textTexture = CreateFontTexture("Change Vision", textColor, &textWidth, &textHeight);
	textRect = { 5 * WINDOW_WIDTH / 6 - textWidth / 2, WINDOW_HEIGHT - MENU_HEIGHT / 2 - textHeight / 2, textWidth, textHeight };
	SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
	SDL_DestroyTexture(textTexture);

	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // black
	SDL_RenderDrawLine(renderer, 5 * WINDOW_WIDTH / 12, WINDOW_HEIGHT - 3 * MENU_HEIGHT / 4, 7 * WINDOW_WIDTH / 12, WINDOW_HEIGHT - 3 * MENU_HEIGHT / 4);
	SDL_RenderDrawLine(renderer, 5 * WINDOW_WIDTH / 12, WINDOW_HEIGHT - MENU_HEIGHT / 4, 7 * WINDOW_WIDTH / 12, WINDOW_HEIGHT - MENU_HEIGHT / 4);
	SDL_RenderDrawLine(renderer, 5 * WINDOW_WIDTH / 12, WINDOW_HEIGHT - 3 * MENU_HEIGHT / 4, 5 * WINDOW_WIDTH / 12, WINDOW_HEIGHT - MENU_HEIGHT / 4);
	SDL_RenderDrawLine(renderer, 7 * WINDOW_WIDTH / 12, WINDOW_HEIGHT - 3 * MENU_HEIGHT / 4, 7 * WINDOW_WIDTH / 12, WINDOW_HEIGHT - MENU_HEIGHT / 4);
	rect = { 5 * WINDOW_WIDTH / 12 + 1, WINDOW_HEIGHT - 3 * MENU_HEIGHT / 4 + 1, WINDOW_WIDTH / 6 - 2, MENU_HEIGHT / 2 - 2 };
	if (model->chessboard->pause == true) {
		SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255); // grey
		SDL_RenderFillRect(renderer, &rect);
		textTexture = CreateFontTexture("Paused", textColor, &textWidth, &textHeight);
	}
	else {
		textTexture = CreateFontTexture("Pause", textColor, &textWidth, &textHeight);
	}	
	textRect = { 3 * WINDOW_WIDTH / 6 - textWidth / 2, WINDOW_HEIGHT - MENU_HEIGHT / 2 - textHeight / 2, textWidth, textHeight };
	SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
	SDL_DestroyTexture(textTexture);

	// Base HP
	SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // red
	int width1 = (int)(model->chessboard->bases[0]->currentHP / (double)model->chessboard->bases[0]->getMaxHP() * (WINDOW_WIDTH / 2));
	SDL_Rect base1HPBar = { 0, 0, width1 - 1, BASEBAR_HEIGHT };
	SDL_RenderFillRect(renderer, &base1HPBar);

	SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255); // blue
	int width2 = (int)(model->chessboard->bases[1]->currentHP / (double)model->chessboard->bases[1]->getMaxHP() * (WINDOW_WIDTH / 2));
	SDL_Rect base2HPBar = { WINDOW_WIDTH - width2 + 1, 0, width2, BASEBAR_HEIGHT };
	SDL_RenderFillRect(renderer, &base2HPBar);

	// Draw chessboard
	// Both bases takes up multiple squares but only needs to be drawn once
	int base1x = model->chessboard->bases[0]->getPos() % model->chessboard->getLength() - (BASE_SIZE / 2);
	int base1y = model->chessboard->bases[0]->getPos() / model->chessboard->getLength() - (BASE_SIZE / 2);
	SDL_Rect base1Rect = { base1x * squareLength + 1, base1y * squareWidth + BASEBAR_HEIGHT + 1, BASE_SIZE * squareLength - 2, BASE_SIZE * squareWidth - 2 };
	bool base1DrawnFlag = false;

	int base2x = model->chessboard->bases[1]->getPos() % model->chessboard->getLength() - (BASE_SIZE / 2);
	int base2y = model->chessboard->bases[1]->getPos() / model->chessboard->getLength() - (BASE_SIZE / 2);
	SDL_Rect base2Rect = { base2x * squareLength + 1, base2y * squareWidth + BASEBAR_HEIGHT + 1, BASE_SIZE * squareLength - 2, BASE_SIZE * squareWidth - 2 };
	bool base2DrawnFlag = false;

	for (int i = 0; i < model->chessboard->getWidth(); ++i) { // y
		for (int j = 0; j < model->chessboard->getLength(); ++j) { // x
			SDL_Rect rect = { j * squareLength + 1, i * squareWidth + BASEBAR_HEIGHT + 1, squareLength - 2, squareWidth - 2 };
			SDL_Rect SoldierRect = { j * squareLength + 1, i * squareWidth + squareWidth / 10 + BASEBAR_HEIGHT + 1, squareLength - 2, 9 * squareWidth / 10 - 2 };
			int pos = i * model->chessboard->getLength() + j;
			Square square = model->chessboard->chessboard[pos];

			bool hasVision = model->chessboard->visions[vision]->hasVision[pos];
			// Draw base
			if (hasVision) {
				// Set according flag to true to avoid repeated drawing
				if (square.hasBase) {
					if (square.base->getTeam() == 1 && !base1DrawnFlag) {
						SDL_RenderCopy(renderer, BaseTexture, NULL, &base1Rect);
						base1DrawnFlag = true;
					}
					if (square.base->getTeam() == 2 && !base2DrawnFlag) {
						SDL_RenderCopy(renderer, BaseTexture, NULL, &base2Rect);
						base2DrawnFlag = true;
					}
				}

				// Draw soldier
				else if (square.hasSoldier) {
					int type = square.soldier->type();
					int team = square.soldier->team;
					// Draw HP Bar
					if (team == 1) 
						SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // red
					else
						SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255); // blue
					SDL_Rect HPRect = { j * squareLength + 1, i * squareWidth + BASEBAR_HEIGHT + 1,
						square.soldier->currentHP * (squareLength - 2) / square.soldier->maxHP, squareWidth / 10 };
					SDL_RenderFillRect(renderer, &HPRect);
					// Copy texture
					switch (type) {
					case 0: {
						if (team == 1)
							SDL_RenderCopy(renderer, Infantry1Texture, NULL, &SoldierRect);
						else
							SDL_RenderCopy(renderer, Infantry2Texture, NULL, &SoldierRect);
						break;
					}
					case 1: {
						if (team == 1)
							SDL_RenderCopy(renderer, Archer1Texture, NULL, &SoldierRect);
						else
							SDL_RenderCopy(renderer, Archer2Texture, NULL, &SoldierRect);
						break;
					}
					case 2: {
						if (team == 1)
							SDL_RenderCopy(renderer, SiegeGun1Texture, NULL, &SoldierRect);
						else
							SDL_RenderCopy(renderer, SiegeGun2Texture, NULL, &SoldierRect);
						break;
					}
					}
				}

				// Draw barrier
				else if (square.type != 0) {
					SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // black
					SDL_RenderFillRect(renderer, &rect);
				}
			}

			else {
				SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255); // grey
				SDL_RenderFillRect(renderer, &rect);
			}				
		}
	}

	// Display on window
	SDL_RenderPresent(renderer);
}

void View::handleEvent(SDL_Event* e) {
	// e.type will only be SDL_MOUSEBUTTONDOWN or SDL_MOUSEBUTTONUP
	int x, y;
	SDL_GetMouseState(&x, &y);

	bool inStart = (x > WINDOW_WIDTH / 12) && (x < 3 * WINDOW_WIDTH / 12) &&
		(y > WINDOW_HEIGHT - 3 * MENU_HEIGHT / 4) && (y < WINDOW_HEIGHT - MENU_HEIGHT / 4);
	bool inPause = (x > 5 * WINDOW_WIDTH / 12) && (x < 7 * WINDOW_WIDTH / 12) &&
		(y > WINDOW_HEIGHT - 3 * MENU_HEIGHT / 4) && (y < WINDOW_HEIGHT - MENU_HEIGHT / 4);
	bool inChangeVision = (x > 9 * WINDOW_WIDTH / 12) && (x < 11 * WINDOW_WIDTH / 12) &&
		(y > WINDOW_HEIGHT - 3 * MENU_HEIGHT / 4) && (y < WINDOW_HEIGHT - MENU_HEIGHT / 4);

	// Click on Start button (Available only before started)
	if (inStart && start == 0 && e->type == SDL_MOUSEBUTTONDOWN) {
		start = 1;
		changeVision = 0;
		busy = true;
		controller->start();
	}

	// Click on Pause button (Available only after started)
	if (inPause && start == 1 && e->type == SDL_MOUSEBUTTONDOWN) {
		busy = true;
		controller->pause();
	}

	// Click on Change Vision button (Available only after started)
	if (inChangeVision && start == 1) {
		if (e->type == SDL_MOUSEBUTTONDOWN) {
			changeVision = 1;
			busy = true;			
			controller->setVision();				
		}
		else {
			changeVision = 0;
			busy = true;
		}					
	}
}

void View::SDLWork() {
	bool quit = false;
	while (!quit) {
		// Avoid always checking, which leads to memory cost
		// std::this_thread::sleep_for(std::chrono::milliseconds(1));

		while (SDL_PollEvent(&e) != 0) {
			if (e.type == SDL_QUIT) {
				quit = true;
			}
			else if (e.type == SDL_MOUSEBUTTONDOWN || e.type == SDL_MOUSEBUTTONUP) {
				handleEvent(&e);
			}
		}

		if (busy == true) {
			printBoardSDL();
			busy = false;
			if (model->chessboard->bases[0]->currentHP == 0 || model->chessboard->bases[1]->currentHP == 0)
				quit = true;
		}	
	}

	//// Print result in a new window
	//resultWindow = SDL_CreateWindow("Result", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 4, SDL_WINDOW_SHOWN);
	//resultRenderer = SDL_CreateRenderer(resultWindow, -1, SDL_RENDERER_ACCELERATED);
	//SDL_SetRenderDrawColor(resultRenderer, 255, 255, 255, 255); // white
	//SDL_RenderClear(resultRenderer);

	//int textWidth, textHeight, winner;
	//if (model->chessboard->bases[1]->currentHP == 0)	winner = 1;
	//else	winner = 2;

	//SDL_Color textColor = { 0, 0, 0, 255 }; // black text color
	//std::string resultText = "Team " + std::to_string(winner) + " wins!! Exit in 3 seconds.";
	//SDL_Texture* textTexture = CreateFontTexture(resultText, textColor, &textWidth, &textHeight);
	////SDL_Rect textRect = { WINDOW_WIDTH / 4 - textWidth / 2, WINDOW_HEIGHT / 8 - textHeight / 2, textWidth, textHeight };
	//SDL_Rect textRect = { 0, 0, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 4};
	//SDL_RenderCopy(resultRenderer, textTexture, NULL, &textRect);
	////SDL_SetRenderDrawColor(resultRenderer, 128, 128, 128, 255); // grey
	////SDL_RenderFillRect(resultRenderer, &textRect);
	//SDL_RenderPresent(resultRenderer);

	// Print result
	if (model->chessboard->bases[0]->currentHP == 0 || model->chessboard->bases[1]->currentHP == 0) {
		SDL_Rect rect = { WINDOW_WIDTH / 4, 3 * WINDOW_HEIGHT / 8, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 4 };
		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // white
		SDL_RenderFillRect(renderer, &rect);
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // black
		SDL_RenderDrawLine(renderer, WINDOW_WIDTH / 4, 3 * WINDOW_HEIGHT / 8, 3 * WINDOW_WIDTH / 4, 3 * WINDOW_HEIGHT / 8);
		SDL_RenderDrawLine(renderer, WINDOW_WIDTH / 4, 5 * WINDOW_HEIGHT / 8, 3 * WINDOW_WIDTH / 4, 5 * WINDOW_HEIGHT / 8);
		SDL_RenderDrawLine(renderer, WINDOW_WIDTH / 4, 3 * WINDOW_HEIGHT / 8, WINDOW_WIDTH / 4, 5 * WINDOW_HEIGHT / 8);
		SDL_RenderDrawLine(renderer, 3 * WINDOW_WIDTH / 4, 3 * WINDOW_HEIGHT / 8, 3 * WINDOW_WIDTH / 4, 5 * WINDOW_HEIGHT / 8);

		int textWidth, textHeight, winner;
		if (model->chessboard->bases[1]->currentHP == 0)	winner = 1;
		else	winner = 2;
		std::string resultText = "Team " + std::to_string(winner) + " wins!! Game quit in 3 seconds.";

		SDL_Color textColor = { 0, 0, 0, 255 }; // black text color	
		SDL_Texture* textTexture = CreateFontTexture(resultText, textColor, &textWidth, &textHeight);
		SDL_Rect textRect = { WINDOW_WIDTH / 2 - textWidth / 2, WINDOW_HEIGHT / 2 - textHeight / 2, textWidth, textHeight };
		SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
		SDL_DestroyTexture(textTexture);
		SDL_RenderPresent(renderer);

		// Delay final result display for 3000ms
		SDL_Delay(3000);
	}
	

	close();
}

void View::close() {
	SDL_DestroyTexture(Infantry1Texture);
	SDL_DestroyTexture(Infantry2Texture);
	SDL_DestroyTexture(Archer1Texture);
	SDL_DestroyTexture(Archer2Texture);
	SDL_DestroyTexture(SiegeGun1Texture);
	SDL_DestroyTexture(SiegeGun2Texture);
	SDL_DestroyTexture(BaseTexture);
	if (Font) TTF_CloseFont(Font);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyRenderer(resultRenderer);
	SDL_DestroyWindow(window);
	SDL_DestroyWindow(resultWindow);
	TTF_Quit();
	IMG_Quit();
	SDL_Quit();
}
