#pragma once
#include<iostream>
class Chessboard;
class AttackBehavior;

class Base {
private:
	int pos;
	int team;
	int size;
	int maxHP;
	int atk;
	int atkRange;
	int soldierGenTime; // Periods to create a soldier
	int soldierGenPos; // Position to create a soldier
	Chessboard* chessboard;
	AttackBehavior* attackBehavior; // Delegate AttackBehavior to attack

public:
	int currentHP;

	Base(int pos, int team, int soldierGenPos, Chessboard* chessboard);

	int getPos();

	int getTeam();

	int getMaxHP();

	void soldierGen(); //Endlessly generate soldiers
	// Will watch and modify the chessboard

	void attack(); // Endlessly execute attack function
};