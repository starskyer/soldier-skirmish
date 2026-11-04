#include<iostream>
#include<iomanip>
#include<algorithm>
#include<thread>
#include<chrono>
#include<vector>
#include<queue>
#include<random>
#include<cstdlib>
#include<ctime>
#include "chessboard.h"
#include "soldier.h"
#include "globalParameters.h"

bool validSquare(int pos, Chessboard* chessboard) {
	bool inbound = (0 <= pos && pos < chessboard->getSize());
	if (!inbound) return false;
	bool isObstacle = (chessboard->getSquare(pos)->type != 0);
	bool hasSoldier = (chessboard->getSquare(pos)->hasSoldier);
	bool hasBase = (chessboard->getSquare(pos)->hasBase);
	return (inbound && !isObstacle && !hasSoldier && !hasBase);
}

std::vector<int> surroundSquare(int pos, Chessboard* chessboard) {
	std::vector<int> results;
	int length = chessboard->getLength();
	int up = pos - length;
	int down = pos + length;
	int left = pos % length > 0 ? pos - 1 : -1;
	int right = pos % length < length - 1 ? pos + 1 : -1;
	if (validSquare(up, chessboard))
		results.push_back(up);
	if (validSquare(down, chessboard))
		results.push_back(down);
	if (validSquare(left, chessboard))
		results.push_back(left);
	if (validSquare(right, chessboard))
		results.push_back(right);
	return results;
}

bool markChessboard(int pos, int target, Chessboard* chessboard, int* marks) {
	// Please ensure that start position is different from target position
	// If there is a path from pos to target, return true
	// Also return a modified "marks" array with moves on it
	// marks[pos] = 1; marks[unreachable pos] = 0;
	int move = 1;
	std::queue<int> markQueue;
	marks[pos] = move;
	markQueue.push(pos);	
	++move;

	while (!markQueue.empty()) {
		while (marks[markQueue.front()] == move - 1) {
			int front = markQueue.front();
			markQueue.pop();
			std::vector<int> surround = surroundSquare(front, chessboard);
			for (int square : surround) {
				if (marks[square] == 0) {
					marks[square] = move;
					if (square == target) {
						return true;
					}
					markQueue.push(square);					
				}
			}
			if (markQueue.empty())
				return false;
		}
		++move;
	}

	return false;
}

int randomNext(int pos, Chessboard* chessboard, int* marks, int next) {
	// Please ensure that current position at least has a NEXT position
	// next == -1: trace back; next == 1: trace forward
	std::vector<int> nexts;
	int length = chessboard->getLength();
	int up = pos - length;
	int down = pos + length;
	int left = pos % length > 0 ? pos - 1 : -1;
	int right = pos % length < length - 1 ? pos + 1 : -1;
	if (up >= 0 && marks[up] == marks[pos] + next)
		nexts.push_back(up);
	if (down < chessboard->getSize() && marks[down] == marks[pos] + next)
		nexts.push_back(down);
	if (left >= 0 && marks[left] == marks[pos] + next)
		nexts.push_back(left);
	if (right >= 0 && marks[right] == marks[pos] + next)
		nexts.push_back(right);

	if (nexts.size() == 0)
		return pos;
	if (nexts.size() == 1)
		return nexts[0];

	// Randomly pick a next position
	thread_local std::mt19937 generator(std::random_device{}());
	std::uniform_int_distribution<int> distribution(0, static_cast<int>(nexts.size()) - 1);
	int choice = distribution(generator);
	return nexts[choice];
}

std::vector<int> findPath(int pos, int target, int speed, Chessboard* chessboard) {
	// Return a vector of positions to the target position when path exists,
	// Or return a vector of "speed" length with randomly picked positions
	// when path does not exist,
	// Excluding start pos and including target pos
	int* marks = new int[chessboard->getSize()];
	for (int i = 0; i < chessboard->getSize(); ++i) {
		marks[i] = 0;
	}
	std::vector<int> path;
	bool existPath = markChessboard(pos, target, chessboard, marks);

	//for (int i = 0; i < 20; ++i) {
	//	for (int j = 0; j < 20; ++j) {
	//		std::cout << std::setw(3) << marks[20 * i + j];
	//	}
	//	std::cout << std::endl;
	//}

	if (existPath) {
		int curPos = target;
		while (curPos != pos) {
			path.push_back(curPos);
			curPos = randomNext(curPos, chessboard, marks, -1);			
		}
		std::reverse(path.begin(), path.end());		
	}
	else {
		int curPos = pos;
		while (path.size() < speed) {
			curPos = randomNext(curPos, chessboard, marks, 1);
			path.push_back(curPos);
		}
	}

	//for (int i = 0; i < path.size(); ++i)
	//	std::cout << std::setw(4) << path[i];
	//std::cout << std::endl;

	delete[] marks;
	return path;
}

void Soldier::move(int target) {
	// Every move needs to notify observers
	int index;
	std::vector<int> targets;
	if (target == this->pos) {
		std::this_thread::sleep_for(std::chrono::milliseconds(TIME));
		return;
	}

	// Update target position if target position is occupied by soldier
	if (chessboard->getSquare(target)->hasSoldier) {
		targets = surroundSquare(target, chessboard);
		srand(time(NULL));
		while (targets.empty()) {
			targets = chessboard->posAround(target, 1);
			index = rand() % targets.size();
			target = targets[index];
			targets = surroundSquare(target, chessboard);
		}
		index = rand() % targets.size();
		target = targets[index];
	}
	// Update target position if target position is occupied by base
	else if (chessboard->getSquare(target)->hasBase) {
		targets = chessboard->posAround(target, BASE_SIZE / 2 + 1, BASE_SIZE / 2 + 1);
		for (int i = 0; i < targets.size(); ++i) {
			if (validSquare(targets[i], chessboard)) {
				target = targets[i];
				break;
			}				
		}
	}

	std::vector<int> path;
	for (int i = 0; i < this->moveSpeed; ++i) {
		while (chessboard->pause);
		if (this->pos != target && this->currentHP > 0) { // UB: currentHP is not synchronized
			path = findPath(this->pos, target, this->moveSpeed - i, chessboard);
			chessboard->move_mtx.lock();
			// Wait until chessboard set soldier to this square
			// while (chessboard->getSquare(this->pos)->hasSoldier == false);
			chessboard->removeSoldier(this->pos);
			this->pos = path[0];
			chessboard->setSoldier(this->pos, this);
			chessboard->move_mtx.unlock();
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(TIME / moveSpeed));
	}
}
