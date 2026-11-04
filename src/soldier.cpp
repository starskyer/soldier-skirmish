#include<iostream>
#include<chrono>
#include<thread>
#include "attackBehavior.h"
#include "base.h"
#include "soldier.h"
#include "chessboard.h"
#include "globalParameters.h"

int Soldier::decide(int* target) {
	// Return a decision: 0 is for move (with target/destination),
	// 1 is for attack soldier, 2 is for attack base,
	// 3 is for stop (soldier will not stop without controller command);
	std::vector<int> visiblePos = chessboard->posAround(pos, visionRange);
	std::vector<int> attackablePos = chessboard->posAround(pos, atkRange);
	std::vector<int> unattackablePos = chessboard->posAround(pos, atkRange + 1, visionRange);

	// Soldier will first attack enemies in attack range,
	// And then move to attack enemies out of attack range but in vision range.
	for (int pos : attackablePos) {
		if (chessboard->getSquare(pos)->hasSoldier &&
			chessboard->getSquare(pos)->soldier->team != this->team) {
			*target = pos;
			return 1;
		}			
	}

	for (int pos : unattackablePos) {
		if (chessboard->getSquare(pos)->hasSoldier &&
			chessboard->getSquare(pos)->soldier->team != this->team) {
			*target = pos;
			return 0;
		}
	}

	// Soldier will not attack enemy's base
	// Until there are no enemy soldiers in vision range.
	for (int pos : attackablePos) {
		if (chessboard->getSquare(pos)->hasBase &&
			chessboard->getSquare(pos)->base->getTeam() != this->team) {
			*target = pos;
			return 2;
		}
	}

	// Soldier will move towards enemy's base when there's nothing to attack.
	*target = chessboard->getEnemyBasePos(this->team);
	return 0;
}

int SiegeGun::decide(int* target) {
	std::vector<int> attackablePos = chessboard->posAround(pos, atkRange);

	// Siege Gun will only attack enemy's base
	for (int pos : attackablePos) {
		if (chessboard->getSquare(pos)->hasBase &&
			chessboard->getSquare(pos)->base->getTeam() != this->team) {
			*target = pos;
			return 2;
		}
	}

	// Soldier will move towards enemy's base when there's nothing to attack.
	*target = chessboard->getEnemyBasePos(this->team);
	return 0;
}

void Soldier::attack(int type, int target) {
	// Type 1: Attack soldier, Type 2: Attack base
	while (chessboard->pause);
	switch (type) {
	case 1:
	{
		attackBehavior1->attack(target);
		std::this_thread::sleep_for(std::chrono::milliseconds(TIME));
		break;
	}
	case 2:
	{
		attackBehavior2->attack(target);
		std::this_thread::sleep_for(std::chrono::milliseconds(TIME));
		break;
	}
	default:
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(TIME));
		break;
	}
	}
}

void Soldier::stop() {
	while (chessboard->pause);
	std::this_thread::sleep_for(std::chrono::milliseconds(TIME));
}

void Soldier::soldierAction() {
	int target, decision;
	while (currentHP > 0) {
		decision = decide(&target);
		switch (decision) {
		case 0: {
			move(target);
			break;
		}
		case 1: {
			attack(1, target);
			break;
		}
		case 2: {
			attack(2, target);
			break;
		}
		case 3: {
			stop();
			break;
		}
		default: {
			stop();
			break;
		}
		}
	}
}

Infantry::Infantry(int pos, int team, Chessboard* chessboard) {
	maxHP = INFANTRY_HP;  atk1 = INFANTRY_ATK1; atk2 = INFANTRY_ATK2;
	atkRange = INFANTRY_ATKRG; moveSpeed = INFANTRY_MVSPD; visionRange = INFANTRY_VSNRG;
	this->pos = pos;
	this->team = team;
	this->chessboard = chessboard;
	attackBehavior1 = new SoldierNormalAttack(atk1, chessboard);
	attackBehavior2 = new SoldierBuildingAttack(atk2, chessboard);
	currentHP = maxHP;
}

int Infantry::type() {
	return 0;
}

Archer::Archer(int pos, int team, Chessboard* chessboard) {
	maxHP = ARCHER_HP;  atk1 = ARCHER_ATK1; atk2 = ARCHER_ATK2;
	atkRange = ARCHER_ATKRG; moveSpeed = ARCHER_MVSPD; visionRange = ARCHER_VSNRG;
	this->pos = pos;
	this->team = team;
	this->chessboard = chessboard;
	attackBehavior1 = new SoldierNormalAttack(atk1, chessboard);
	attackBehavior2 = new SoldierBuildingAttack(atk2, chessboard);
	currentHP = maxHP;
}

int Archer::type() {
	return 1;
}

SiegeGun::SiegeGun(int pos, int team, Chessboard* chessboard) {
	maxHP = SIEGEGUN_HP;  atk1 = SIEGEGUN_ATK1; atk2 = SIEGEGUN_ATK2;
	atkRange = SIEGEGUN_ATKRG; moveSpeed = SIEGEGUN_MVSPD; visionRange = SIEGEGUN_VSNRG;
	this->pos = pos;
	this->team = team;
	this->chessboard = chessboard;
	attackBehavior1 = NULL;
	attackBehavior2 = new SoldierBuildingAttack(atk2, chessboard);
	currentHP = maxHP;
}

int SiegeGun::type() {
	return 2;
}

Soldier* SoldierFactory::createSoldier(int type, int pos, int team, Chessboard* chessboard) {
	Soldier* soldier;
	switch (type) {
	case 0: {
		soldier = new Infantry(pos, team, chessboard);
		break;
	}
	case 1: {
		soldier = new Archer(pos, team, chessboard);
		break;
	}
	case 2: {
		soldier = new SiegeGun(pos, team, chessboard);
		break;
	}
	default: {
		soldier = new Infantry(pos, team, chessboard);
		break;
	}
	}	
	return soldier;
}