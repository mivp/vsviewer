#include "pyramid.h"
#include <math.h>
#include <iostream>

using namespace std;

// Level
Level::Level(): level(-1), ncols(0), nrows(0)
{
	mark.clear();
}
Level::~Level()
{
	mark.clear();
}

int Level::build(int _level, int _ncols, int _nrows)
{
	mark.clear();
	level = _level;
	ncols = _ncols;
	nrows = _nrows;
	for(int i=0; i < nrows*ncols; i++)
		mark.push_back(0);
}

int Level::setValue(int col, int row, bool value)
{
	int index = row * ncols + col;
	if(index >= nrows*ncols)
		return -1;
	mark[index] = value;
	return 0;
}

int Level::getValue(int row, int col)
{
	int index = row * ncols + col;
	if(index >= nrows*ncols)
		return -1;
	return mark[index];
}

// Pyramid
Pyramid::Pyramid(): maxlevel(0)
{
	levels.clear();
}

Pyramid::~Pyramid()
{
	levels.clear();
}

int Pyramid::build(int64_t width, int64_t height, int tilesize)
{
	levels.clear();

	maxlevel = 0;
	while(pow(2,maxlevel)*tilesize <= width)
		maxlevel++;
	
	int64_t w, h;
	int ncols, nrows;
	for(int level=0; level <= maxlevel; level++)
	{
		w = 1.0 * width / pow(2, (maxlevel-level));
		ncols = w / tilesize + 1;
		h = 1.0 * height / pow(2, (maxlevel-level));
		nrows = h / tilesize + 1;
		Level* l = new Level();
		l->build(level, ncols, nrows);
		levels.push_back(l);
	}

	return 0;
}

int Pyramid::setValue(int level, int col, int row, int value)
{
	if(level > maxlevel)
		return -1;
	return ((Level*)levels[level])->setValue(col, row, value);
}

int Pyramid::getValue(int level, int col, int row)
{
	if(level > maxlevel)
		return -1;
	return ((Level*)levels[level])->getValue(col, row);
}

int Pyramid::print()
{
	for(int level=0; level <=maxlevel; level++)
	{
		Level* l = levels[level];
		cout << "Level: " << level << " ncols: " << l->ncols << " nrows: " << l->nrows << endl;
	}
	return 0;
}