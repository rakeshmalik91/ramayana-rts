#ifndef __ALGORITHM_H
#define __ALGORITHM_H

#include <iostream>
#include <vector>

#include "math.h"

using namespace std;
using namespace math;

namespace algorithm {	
	namespace bfs {
		Path getPath(Point2Di src, Point2Di dst, bool visitable(void*, int, int), void* param, int width, int height);
		void recalculatePath(Path& path, Point2Di src, Point2Di dst, bool visitable(void*, int, int), void* param, int width, int height, float threshold=2.0);
	}

	namespace astar {
		Path getPath(Point2Di src, Point2Di dst, bool visitable(void*, int, int), void* param, int width, int height, int step=1);
		Path recalculatePath(Path path, Point2Di src, Point2Di dst, bool visitable(void*, int, int), void* param, int width, int height, float threshold=2.0, int step=1);
	}

	namespace hpastar {
		Path getPath(Point2Di src, Point2Di dst, bool visitable(void*, int, int), void* param, int width, int height);
		Path recalculatePath(Path path, Point2Di src, Point2Di dst, bool visitable(void*, int, int), void* param, int width, int height, float threshold=2.0);
	}

	class PathfinderAStar {
		static const Point2Di neighbor_offset[];
		int width, height;
		struct Cell {
			unsigned char valid : 1, visitable : 1, in_closedset : 1, in_openset : 1, in_came_from : 1;
			Point2Di came_from;
			int g_score, f_score;
			Cell() :valid(0), visitable(0), in_closedset(0), in_openset(0), in_came_from(0), came_from(0, 0), g_score(0), f_score(0) {}
		} **cell;
		vector<Point2Di> openset;
		Path path;
	protected:
		void clear();
		void addToOpenset(Point2Di p);
		void removeFromOpenset(Point2Di p);
		void addToClosedset(Point2Di p);
		Point2Di getLowestFScoreInOpenSet();
		Path reconstructPath(Point2Di current_node);
		int heuristicCostEstimate(Point2Di src, Point2Di dst);
		bool intelligentVisitable(int x, int y, bool(*visitable)(void*, int, int), void* param);
		Point2Di getNearestPointInCameFrom(Point2Di src, Point2Di dst, bool(*visitable)(void*, int, int), void* param, int step);
		static int pathfinderThread(void*);
	public:
		PathfinderAStar(int width, int height);
		~PathfinderAStar();
		Path findPath(Point2Di src, Point2Di dst, bool(*visitable)(void*, int, int), void* param, int step = 1);
		Path recalculatePath(Path path, Point2Di src, Point2Di dst, bool(*visitable)(void*, int, int), void* param, float threshold = 2.0, int step = 1);
	};
};

#endif
