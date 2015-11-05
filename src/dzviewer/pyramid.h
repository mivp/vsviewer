#ifndef _PYRAMID_H__
#define _PYRAMID_H__

#include <vector>
using std::vector;

// for fast look up
class Level
{
public:
	int level;
	int ncols, nrows;
	vector<int> mark;

	Level();
	~Level();
	int build(int _level, int _ncols, int _nrows);
	int setValue(int col, int row, bool value);
	int getValue(int col, int row);
};

class Pyramid
{
private:
	int maxlevel;
	vector<Level*> levels;
public:
	Pyramid();
	~Pyramid();
	int build(int64_t width, int64_t height, int tilesize);
	int setValue(int level, int col, int row, int value);
	int getValue(int level, int col, int row);
	int print();
};

#endif