#include<iostream>
#include<thread>
#include<chrono>
#include<cstdlib>
#include<ctime>
#include "base.h"
#include "chessboard.h"
#include "attackBehavior.h"
#include "soldier.h"
#include "globalParameters.h"

Base::Base(int pos, int team, int soldierGenPos, Chessboard* chessboard): 
	pos(pos), team(team), size(BASE_SIZE), maxHP(BASE_HP), atk(BASE_ATK), atkRange(BASE_ATKRG), 
	soldierGenTime(BASE_GENTIME), soldierGenPos(soldierGenPos), chessboard(chessboard) 
{
	currentHP = BASE_HP;
	attackBehavior = new BaseAttack(atk, chessboard);
}

int Base::getPos() {
	return pos;
}

int Base::getTeam() {
	return team;
}

int Base::getMaxHP() {
	return maxHP;
}

// The following function uses std::thread, which is now considered unsafe with detach method.
// In the comment, a safer method using std::shared_ptr and std::condition_variable is suggested for future improvement.
// Or it is recommeneded to use std::jthread instead of detached std::thread in C++20 and later.
// std::jthread and std::stop_token can be used to safely stop the thread when Base is destructed.
void Base::soldierGen() {
	Soldier* soldier; // Recommend all corresponding functions to update to std::shared_ptr<Soldier>
	SoldierFactory factory;
	std::thread soldierThread;
	int type;
	srand(time(NULL));
	while (currentHP > 0) {
		while (chessboard->pause); // Busy wait, could be optimized with condition_variable
		if (!chessboard->getSquare(soldierGenPos)->hasSoldier) {
			type = rand() % 3; // Random type of soldier

			// Create a soldier according to soldier type
			// As created soldier will endlessly do action, detached thread is needed;
			soldier = factory.createSoldier(type, soldierGenPos, team, chessboard);
			chessboard->setSoldier(soldierGenPos, soldier); // Notify Observers

			soldierThread = std::thread([soldier]() { // soldierThread = std::thread([&]() {
				soldier->soldierAction();
				});
			soldierThread.detach();			
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(soldierGenTime * TIME));
	}
}

void Base::attack() {
	int target;
	while (currentHP > 0) {
		target = -1;
		std::vector<int> attackablePos = chessboard->posAround(pos, atkRange);
		for (int pos : attackablePos) { 
			// If there is any enemy in attack range
			if (chessboard->getSquare(pos)->hasSoldier &&
				chessboard->getSquare(pos)->soldier->team != this->team) {
				target = pos;
				break;
			}
		}
		while (chessboard->pause);
		if(target >= 0)
			attackBehavior->attack(target);
		else
			std::this_thread::sleep_for(std::chrono::milliseconds(TIME));
	}	
}
