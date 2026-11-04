#include<iostream>
#include<vector>
#include "chessboard.h"
#include "base.h"
#include "soldier.h"
#include "vision.h"
#include "globalParameters.h"

void Vision::visionUpdate() {
	getVision();
}

VisionTeam::VisionTeam(Chessboard* chessboard, int team) : team(team) {
	this->chessboard = chessboard;
	chessboard->registerVision(this);
	size = chessboard->getSize();
	hasVision = new bool[size]{};
}

void VisionTeam::getVision() {
	std::vector<int> vision;
	int pos, distance;
	for (int i = 0; i < size; ++i)
		hasVision[i] = false;

	pos = chessboard->bases[team - 1]->getPos();
	distance = BASE_ATKRG;
	vision = chessboard->posAround(pos, 0, distance);
	for (int index : vision)
		hasVision[index] = true;

	for (Soldier* soldier : chessboard->soldiers) {
		if(soldier->team == this->team){
			pos = soldier->pos;
			distance = soldier->visionRange;
			vision = chessboard->posAround(pos, 0, distance);
			for (int index : vision)
				hasVision[index] = true;
		}
	}
}

VisionAll::VisionAll(Chessboard* chessboard) {
	this->chessboard = chessboard;
	chessboard->registerVision(this);
	size = chessboard->getSize();
	hasVision = new bool[size]{};
}

void VisionAll::getVision() {
	std::vector<int> vision;
	int pos, distance;
	for (int i = 0; i < size; ++i)
		hasVision[i] = false;

	for (int i = 0; i < 2; ++i) {
		pos = chessboard->bases[i]->getPos();
		distance = BASE_ATKRG;
		vision = chessboard->posAround(pos, 0, distance);
		for (int index : vision)
			hasVision[index] = true;
	}

	for (Soldier* soldier : chessboard->soldiers) {
		pos = soldier->pos;
		distance = soldier->visionRange;
		vision = chessboard->posAround(pos, 0, distance);
		for (int index : vision)
			hasVision[index] = true;
	}
}
