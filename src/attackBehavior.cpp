#include<iostream>
#include "chessboard.h"
#include "base.h"
#include "soldier.h"
#include "attackBehavior.h"

BaseAttack::BaseAttack(int atk, Chessboard* chessboard) :
	atk(atk), chessboard(chessboard) {};


// There is certain data race (UB) in BaseAttack::attack as well as SoldierNormalAttack::attack
void BaseAttack::attack(int pos) {
	if (chessboard->getSquare(pos)->hasSoldier) { // UB 1: hasSoldier does not synchronize with setSoldier/removeSoldier
		chessboard->getSquare(pos)->soldier->currentHP -= atk; // UB 2: currentHP is not synchronized with Soldier::move or Soldier::action, etc.
		if (chessboard->getSquare(pos)->soldier->currentHP < 0) // UB 2
			chessboard->getSquare(pos)->soldier->currentHP = 0; // UB 2
		if (chessboard->getSquare(pos)->soldier->currentHP == 0) {
			chessboard->atk_mtx.lock();
			chessboard->removeSoldier(pos); // Mutex lock only protects the internal data of chessboard, not the soldier's shared data
			chessboard->atk_mtx.unlock();
		}
		else {
			chessboard->notifyObservers(); // Any changes of HP need to be printed
		}
	}
}

SoldierNormalAttack::SoldierNormalAttack(int atk, Chessboard* chessboard) :
	atk(atk), chessboard(chessboard) {};

void SoldierNormalAttack::attack(int pos) {
	if (chessboard->getSquare(pos)->hasSoldier) {
		chessboard->getSquare(pos)->soldier->currentHP -= atk;
		if (chessboard->getSquare(pos)->soldier->currentHP < 0)
			chessboard->getSquare(pos)->soldier->currentHP = 0;
		if (chessboard->getSquare(pos)->soldier->currentHP == 0) {
			chessboard->atk_mtx.lock();
			chessboard->removeSoldier(pos);
			chessboard->atk_mtx.unlock();
		}
		else {
			chessboard->notifyObservers(); // Any changes of HP need to be printed
		}
	}
}

SoldierBuildingAttack::SoldierBuildingAttack(int atk, Chessboard* chessboard) :
	atk(atk), chessboard(chessboard) {};

void SoldierBuildingAttack::attack(int pos) {
	chessboard->getSquare(pos)->base->currentHP -= atk;
	if (chessboard->getSquare(pos)->base->currentHP < 0)
		chessboard->getSquare(pos)->base->currentHP = 0;
	chessboard->notifyObservers(); // Any changes of HP need to be printed
}