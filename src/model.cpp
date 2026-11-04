#include<iostream>
#include<thread>
#include<string>
#include "base.h"
#include "chessboard.h"
#include "soldier.h"
#include "model.h"
#include "view.h"

Model::Model(std::string mapPath, std::string basePath) {
	this->mapPath = mapPath; this->basePath = basePath;
	chessboard = new Chessboard(mapPath, basePath);
	chessboard->registerModel(this);
	
}

void Model::start() {
	// Game start
	std::thread soldierGenThread, attackThread;
	soldierGenThread = std::thread([&]() {
		chessboard->bases[0]->soldierGen();
		});
	soldierGenThread.detach();
	soldierGenThread = std::thread([&]() {
		chessboard->bases[1]->soldierGen();
		});
	soldierGenThread.detach();

	// Game continues until either base is destroyed
	while (chessboard->bases[0]->currentHP > 0 && chessboard->bases[1]->currentHP > 0);
	// Wait until print board is finished
	while (chessboard->busy == 1);
	if (chessboard->bases[1]->currentHP == 0)
		std::cout << "Team 1 wins!!" << std::endl;
	else
		std::cout << "Team 2 wins!!" << std::endl;
	return;
}

void Model::start_test() {
	//SoldierFactory factory;
	//Soldier* soldier1 = factory.createSoldier(0, 61, 1, chessboard);
	//std::thread soldier1Thread([&]() {
	//	soldier1->soldierAction();
	//	});
	//soldier1Thread.detach();
	//chessboard->setSoldier(61, soldier1); // Notify Observers
	//Soldier* soldier2 = factory.createSoldier(0, 338, 2, chessboard);
	//std::thread soldier2Thread([&]() {
	//	soldier2->soldierAction();
	//	});
	//soldier2Thread.detach();
	//chessboard->setSoldier(338, soldier2);
	std::thread soldierGenThread, attackThread;
	soldierGenThread = std::thread([&]() {
		chessboard->bases[0]->soldierGen();
		});
	soldierGenThread.detach();
	soldierGenThread = std::thread([&]() {
		chessboard->bases[1]->soldierGen();
		});
	soldierGenThread.detach();

	//for (int i = 0; i < 1; ++i) {
	//	soldierGenThread = std::thread([&]() {
	//		chessboard->bases[i]->soldierGen();
	//		});
	//	soldierGenThread.detach();
	//	//attackThread = std::thread([&]() {
	//	//	chessboard->bases[i]->attack();
	//	//	});
	//	//attackThread.detach();
	//}

	// Game continues until either base is destroyed
	while (chessboard->bases[0]->currentHP > 0 && chessboard->bases[1]->currentHP > 0);
	// Wait until print board is finished
	while (chessboard->busy == 1);
	if (chessboard->bases[1]->currentHP == 0)
		std::cout << "Team 1 wins!!" << std::endl;
	else
		std::cout << "Team 2 wins!!" << std::endl;
	return;
}

void Model::registerObservers(View* view) {
	views.push_back(view);
}

void Model::notifyObservers() {
	// Notify view to update itself
	for (int i = 0; i < views.size(); ++i) {
		//views[i]->printBoardConsole();
		//views[i]->printBoardSDL();
		if (views[i]->busy == false) {
			views[i]->busy = true;
			// wait until view completes print board
			while (views[i]->busy == true);
		}
	}
}

void Model::pause() {
	if(chessboard->pause)
		chessboard->pause = false;
	else
		chessboard->pause = true;
}