#include<iostream>
#include<iomanip>
#include<fstream>
#include<thread>
#include<algorithm>
#include<filesystem>
#include "base.h"
#include "soldier.h"
#include "vision.h"
#include "chessboard.h"
#include "model.h"
#include "globalParameters.h"

Square::Square() {
	type = 0;
	hasSoldier = false;
	hasBase = false;
	soldier = NULL;
	base = NULL;
}

void Square::setBase(Base* base) {
	this->hasBase = true;
	this->base = base;
}

void Square::setSoldier(Soldier* soldier) {
	hasSoldier = true;
	this->soldier = soldier;
}

void Square::removeSoldier() {
	hasSoldier = false;
	this->soldier = NULL;
}

Chessboard::Chessboard(std::string mapPath, std::string basePath) {
	busy = false;
	pause = false;
	// Initialize chessboard from map path
	std::ifstream ifsMap;
	ifsMap.open(std::filesystem::u8path(mapPath), std::ios::in);
	std::string buf;
	int team, pos, soldierGenPos;
	std::getline(ifsMap, buf);
	this->size = std::stoi(buf);

	visions = {}; soldiers = {};	
	chessboard = new Square[size];

	pos = 0;
	while (std::getline(ifsMap, buf)) {
		if (!buf.empty() && buf.back() == '\r') buf.pop_back();
		// buf is a line of map.txt
		this->length = buf.size();
		for (int i = 0; i < buf.size(); ++i, ++pos) {
			int type = buf[i] - '0';
			if (type != 0) {
				chessboard[pos].type = type;
			}
		}
	}
	this->width = size / length;
	ifsMap.close();

	// Initialize bases from base path
	std::ifstream ifsBase;
	ifsBase.open(std::filesystem::u8path(basePath), std::ios::in);
	
	std::vector<int> baseSquare;
	for (int i = 0; i < 2; ++i) {
		ifsBase >> team >> pos >> soldierGenPos;

		// Create base and detached threads for base->soldierGen() and base->attack()
		bases[i] = new Base(pos, team, soldierGenPos, this);

		// Set corresponding squares to base square
		baseSquare = posAround(pos, 0, BASE_SIZE / 2);
		for (int index : baseSquare) {
			chessboard[index].setBase(bases[i]);
		}
	}		
	ifsBase.close();

	//Initialize vision
	VisionAll* vision0 = new VisionAll(this);
	VisionTeam* vision1 = new VisionTeam(this, 1);
	VisionTeam* vision2 = new VisionTeam(this, 2);
}

void Chessboard::registerVision(Vision* vision) {
	visions.push_back(vision);
}

void Chessboard::registerModel(Model* model) {
	this->model = model;
}

void Chessboard::notifyObservers() {
	// Will only notify observers when observers are not busy
	if (busy == false) {
		busy = true;
		for (int i = 0; i < visions.size(); ++i)
			visions[i]->visionUpdate();
		model->notifyObservers();
		busy = false;
	}
	return;
}

int Chessboard::getLength() {
	return length;
}

int Chessboard::getWidth() {
	return width;
}

int Chessboard::getSize() {
	return size;
}

//int Chessboard::Cor2Pos(Coordinate* cor) {
//	return cor->y * length + cor->x;
//}
//
//void Chessboard::Pos2Cor(Coordinate* cor, int pos) {
//	cor->x = pos % length;
//	cor->y = pos / length;
//	return;
//}

std::vector<int> Chessboard::posAround(int pos, int distance) {
	return Chessboard::posAround(pos, 1, distance);
}

std::vector<int> Chessboard::posAround(int pos, int distanceFrom, int distanceTo) {
	std::vector<int> results;
	int result;
	int d, i, j;
	for (d = distanceFrom; d <= distanceTo; ++d) {
		for (i = -d; i <= d; ++i) {
			result = (0 <= pos % length + i) && (pos % length + i < length) ?
				(pos - d * length + i) : -1;
			if (0 <= result && result < size)
				results.push_back(result);
		}
		for (j = -d + 1; j <= d - 1; ++j) {
			result = (0 <= pos % length - d) ?
				(pos + j * length - d) : -1;
			if (0 <= result && result < size)
				results.push_back(result);
			result = (pos % length + d < length) ?
				(pos + j * length + d) : -1;
			if (0 <= result && result < size)
				results.push_back(result);
		}
		for (i = -d; i <= d; ++i) {
			result = (0 <= pos % length + i) && (pos % length + i < length) ?
				(pos + d * length + i) : -1;
			if (0 <= result && result < size)
				results.push_back(result);
		}
	}
	return results;
}

void Chessboard::setSoldier(int pos, Soldier* soldier) {
	mtx.lock();
	chessboard[pos].setSoldier(soldier);
	soldiers.push_back(soldier);
	notifyObservers(); // notifyObservers should not be called inside mutex lock as it may cause potential deadlock
	mtx.unlock();
}

void Chessboard::removeSoldier(int pos) {
	mtx.lock();
	Soldier* soldier = chessboard[pos].soldier;
	chessboard[pos].removeSoldier();
	// TIP: The thread of the removed soldier will also stop
	auto index = std::find(soldiers.begin(), soldiers.end(), soldier);
	if (index != soldiers.end()) {
		soldiers.erase(index);
	}	
	notifyObservers();
	mtx.unlock();
}

Square* Chessboard::getSquare(int pos) {
	return &(chessboard[pos]);
}

int Chessboard::getEnemyBasePos(int team) {
	if (bases[0]->getTeam() != team)
		return bases[0]->getPos();
	else
		return bases[1]->getPos();
}

void Chessboard::printBoardConsole() {
	// Print the chessboard in console window.
	int i, j, pos, type, team;
	for (j = 0; j < width; ++j) {
		for (i = 0; i < length; ++i) {
			pos = j * length + i;
			if (chessboard[pos].hasBase) {
				if (chessboard[pos].base->getTeam() == 1)
					std::cout << 'B';
				else
					std::cout << 'b';
			}
			else if (chessboard[pos].hasSoldier) {
				type = chessboard[pos].soldier->type();
				team = chessboard[pos].soldier->team;
				switch (type) {
				case 0: {
					if (team == 1)	std::cout << 'I';
					else			std::cout << 'i';
					break;
				}
				case 1: {
					if (team == 1)	std::cout << 'A';
					else			std::cout << 'a';
					break;
				}
				case 2: {
					if (team == 1)	std::cout << 'S';
					else			std::cout << 's';
					break;
				}
				}
			}
			else if (chessboard[pos].type != 0) {
				std::cout << chessboard[pos].type;
			}
			else
				std::cout << ' ';
				
		}
		std::cout << std::endl;
	}
	std::cout << "Team 1 base HP: " << std::setw(3) << bases[0]->currentHP << std::endl;
	std::cout << "Team 2 base HP: " << std::setw(3) << bases[1]->currentHP << std::endl;
}
