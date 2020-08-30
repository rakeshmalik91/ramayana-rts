/*****************************************************************************************************
 * Subject                   : Pathfinding Algorithms (BFS, A*, HPA*)                                *
 * Author                    : Rakesh Malik                                                          *
 *****************************************************************************************************/

#include "stdafx.h"
 
#include "algorithm.h"
#include "graphics.h"

#define MAX_MAP_SIZE		1000

using namespace math;

namespace algorithm {
	
	typedef vector<Point2Di> Path;
	
	namespace bfs {
		bool visited[MAX_MAP_SIZE][MAX_MAP_SIZE], visitabilityChecked[MAX_MAP_SIZE][MAX_MAP_SIZE];
		Point2Di visitPathBackPointer[MAX_MAP_SIZE][MAX_MAP_SIZE];
		Path getPath(Point2Di src, Point2Di dst, bool visitable(void*, int, int), void* param, int width, int height) {
			for(int r=0; r<height; r++) {
				for(int c=0; c<width; c++) {
					visited[r][c]=false;
				}
			}
			queue<Point2Di> q;
			q.push(Point2Di(src.x, src.y));
			visited[src.y][src.x]=true;
			int dx[8]={-1, 1, 0, 0,-1,-1, 1, 1};
			int dy[8]={ 0, 0,-1, 1,-1, 1,-1, 1};
			while(!visited[dst.y][dst.x] && !q.empty()) {
				Point2Di p=q.front();
				q.pop();
				for(int i=0; i<8; i++) {
					int nx=p.x+dx[i], ny=p.y+dy[i];
					if(nx>=0 && ny>=0 && nx<width && ny<height && !visited[ny][nx] && visitable(param, nx, ny)) {
						visited[ny][nx]=true;
						visitPathBackPointer[ny][nx]=p;
						q.push(Point2Di(nx, ny));
					}
				}
			}
			Path path;
			if(visited[dst.y][dst.x]) {
				for(Point2Di p(dst.x, dst.y); !(p.x==(src.x) && p.y==(src.y)); p=visitPathBackPointer[p.y][p.x]) {
					if(visitable(param, p.x, p.y))
						path.insert(path.begin(), Point2Di(p.x, p.y));
					else
						path.clear();
				}
			} else {
				for(int r=0; r<height; r++) {
					for(int c=0; c<width; c++) 
						visitabilityChecked[r][c]=false;
				}
				while(!q.empty()) q.pop();
				q.push(Point2Di(dst.x, dst.y));
				while(!q.empty()) {
					Point2Di p=q.front();
					q.pop();
					for(int i=0; i<8; i++) {
						int nx=p.x+dx[i], ny=p.y+dy[i];
						if(nx>=0 && ny>=0 && nx<width && ny<height && !visitabilityChecked[ny][nx]) {
							if(visited[ny][nx]) {
								dst=Point2Di(nx, ny);
								while(!q.empty()) q.pop();
								break;
							} else {
								visitabilityChecked[ny][nx]=true;
								q.push(Point2Di(nx, ny));
							}
						}
					}
				}
				for(Point2Di p(dst.x, dst.y); !(p.x==(src.x) && p.y==(src.y)); p=visitPathBackPointer[p.y][p.x]) {
					if(visitable(param, p.x, p.y))
						path.insert(path.begin(), Point2Di(p.x, p.y));
					else
						path.clear();
				}
			}
			return path;
		}
		void recalculatePath(Path& path, Point2Di src, Point2Di dst, bool visitable(void*, int, int), void* param, int width, int height, float threshold) {
			if(path.size()<=1)
				return;
			int obstacled_path_length;
			for(obstacled_path_length=0; obstacled_path_length<path.size(); obstacled_path_length++)
				if(visitable(param, path[obstacled_path_length].x, path[obstacled_path_length].y))
					break;
			if(obstacled_path_length>=path.size()) {
				path=getPath(src, dst, visitable, param, width, height);
			} else {
				Path newpath=getPath(src, path[obstacled_path_length], visitable, param, width, height);
				if(newpath.size()<=threshold*obstacled_path_length) {
					for(int i=0; i<obstacled_path_length; i++)
						path.erase(path.begin());
					while(!newpath.empty()) {
						path.insert(path.begin(), *(newpath.end()-1));
						newpath.erase(newpath.end()-1);
					}
				}
			}
		}
	}

	namespace astar {
		struct Cell {
			unsigned char valid:1, visitable:1, in_closedset:1, in_openset:1, in_came_from:1;
			Point2Di came_from;
			int g_score, f_score;
			Cell() :valid(0), visitable(0), in_closedset(0), in_openset(0), in_came_from(0), came_from(0, 0), g_score(0), f_score(0) {}
		} cell[MAX_MAP_SIZE][MAX_MAP_SIZE];
		vector<Point2Di> openset;
		Point2Di neighbor_offset[]={Point2Di(1, 0), Point2Di(1, 1), Point2Di(0, 1), Point2Di(-1, 1), Point2Di(-1, 0), Point2Di(-1, -1), Point2Di(0, -1), Point2Di(1, -1)};
		void clear(int width, int height) {
			for(int r=0; r<height; r++)
				for(int c=0; c<width; c++)
					cell[r][c] = Cell();
			openset.clear();
		}
		void addToOpenset(Point2Di p) {
			cell[p.y][p.x].in_openset = 1;
			openset.push_back(p);
		}
		void removeFromOpenset(Point2Di p) {
			for(vector<Point2Di>::iterator i=openset.begin(); i!=openset.end(); )
				if(*i==p) i=openset.erase(i);
				else i++;
				cell[p.y][p.x].in_openset = 0;
		}
		void addToClosedset(Point2Di p) {
			cell[p.y][p.x].in_closedset = 1;
		}
		Point2Di getLowestFScoreInOpenSet() {
			Point2Di p;
			float min_f_score=FLT_MAX;
			for(int i=0; i<openset.size(); i++) {
				if(cell[openset[i].y][openset[i].x].f_score<min_f_score) {
					min_f_score = cell[openset[i].y][openset[i].x].f_score;
					p=openset[i];
				}
			}
			return p;
		}
		Path reconstructPath(Point2Di current_node) {
			Path path;
			for(Point2Di node = current_node; cell[node.y][node.x].in_came_from; node = cell[node.y][node.x].came_from)
				path.insert(path.begin(), node);
			return path;
		}
		int heuristicCostEstimate(Point2Di src, Point2Di dst) {
			return dist(src, dst);
		}
		bool _visitable(bool visitable(void*, int, int), void* param, int x, int y) {
			if(cell[y][x].valid) {
				return cell[y][x].visitable;
			} else {
				cell[y][x].valid = 1;
				return cell[y][x].visitable=visitable(param, x, y)?1:0;
			}
		}
		Point2Di getNearestPointInCameFrom(Point2Di src, Point2Di dst, bool visitable(void*, int, int), void* param, int width, int height, int step) {
			unsigned int startTime=SDL_GetTicks(), time=manhattanDist(src, dst)/2;
			for(int r=0; r<height; r++)
				for(int c=0; c<width; c++) {
					cell[r][c].in_closedset=false;
					cell[r][c].in_openset=false;
				}
			openset.clear();
			openset.insert(openset.end(), dst);
			while(!openset.empty()) {
				Point2Di current=openset.front();
				if(cell[current.y][current.x].in_came_from)
					return current;
				if(SDL_GetTicks()-startTime>time)
					break;
				openset.erase(openset.begin());
				for(int n=0; n<8; n++) {
					Point2Di neighbor=current+neighbor_offset[n]*step;
					if(neighbor.in(0, 0, width - 1, height - 1) && !cell[neighbor.y][neighbor.x].in_openset)
						openset.insert(openset.end(), neighbor);
				}
			}
			Path line=lineBresenham(dst, src);
			for(int i=0; i<line.size(); i++)
			if(cell[line[i].y][line[i].x].in_came_from && visitable(param, line[i].x, line[i].y))
					return line[i];
			return src;
		}
		Path getPath(Point2Di src, Point2Di dst, bool visitable(void*, int, int), void* param, int width, int height, int step) {
			unsigned int startTime=SDL_GetTicks(), time=dist(src, dst)+10;
			int sqrstep=step*step;
			clear(width, height);
			addToOpenset(src);
			cell[src.y][src.x].g_score = 0;
			cell[src.y][src.x].f_score = cell[src.y][src.x].g_score + heuristicCostEstimate(src, dst);
			while(!openset.empty()) {
				Point2Di current=getLowestFScoreInOpenSet();
				if(squareDist(current, dst)<sqrstep)
					return reconstructPath(current);
				if(SDL_GetTicks()-startTime>time)
					break;
				removeFromOpenset(current);
				addToClosedset(current);
				for(int n=0; n<8; n++) {
					Point2Di neighbor=current+neighbor_offset[n]*step;
					if(neighbor.in(0, 0, width-1, height-1) && _visitable(visitable, param, neighbor.x, neighbor.y)) {
						float tentative_g_score = cell[current.y][current.x].g_score + dist(current, neighbor);
						if(cell[neighbor.y][neighbor.x].in_closedset && tentative_g_score >= cell[neighbor.y][neighbor.x].g_score)
							continue;
						if(!cell[neighbor.y][neighbor.x].in_closedset || tentative_g_score<cell[neighbor.y][neighbor.x].g_score) {
							cell[neighbor.y][neighbor.x].came_from = current;
							cell[neighbor.y][neighbor.x].in_came_from = true;
							cell[neighbor.y][neighbor.x].g_score=tentative_g_score;
							cell[neighbor.y][neighbor.x].f_score = cell[neighbor.y][neighbor.x].g_score + heuristicCostEstimate(neighbor, dst);
							if(!cell[neighbor.y][neighbor.x].in_openset)
								addToOpenset(neighbor);
						}
					}
				}
			}
			Point2Di nearestPoint=getNearestPointInCameFrom(src, dst, visitable, param, width, height, step);
			return reconstructPath(nearestPoint);
		}
		Path recalculatePath(Path path, Point2Di src, Point2Di dst, bool visitable(void*, int, int), void* param, int width, int height, float threshold, int step) {
			if(path.size()<=1)
				return path;
			int obstacled_path_length;
			for(obstacled_path_length=0; obstacled_path_length<path.size(); obstacled_path_length++)
				if(visitable(param, path[obstacled_path_length].x, path[obstacled_path_length].y))
					break;
			if(obstacled_path_length>=path.size()) {
				path=getPath(src, dst, visitable, param, width, height, step);
			} else {
				Path newpath=getPath(src, path[obstacled_path_length], visitable, param, width, height, step);
				if(newpath.size()<=threshold*obstacled_path_length) {
					for(int i=0; i<obstacled_path_length; i++)
						path.erase(path.begin());
					while(!newpath.empty()) {
						path.insert(path.begin(), *(newpath.end()-1));
						newpath.erase(newpath.end()-1);
					}
				}
			}
			return path;
		}
	}

	namespace hpastar {
		Path getPath(Point2Di src, Point2Di dst, bool visitable(void*, int, int), void* param, int width, int height) {
			Path path=astar::getPath(src, dst, visitable, param, width, height, 8);
			if(path.size()>0)
				path=astar::getPath(src, path.front(), visitable, param, width, height, 1);
			else
				path=astar::getPath(src, dst, visitable, param, width, height, 1);
			return path;
		}
		Path recalculatePath(Path path, Point2Di src, Point2Di dst, bool visitable(void*, int, int), void* param, int width, int height, float threshold) {
			if(path.size()<=1)
				return path;
			int obstacled_path_length;
			for(obstacled_path_length=0; obstacled_path_length<path.size(); obstacled_path_length++)
				if(visitable(param, path[obstacled_path_length].x, path[obstacled_path_length].y))
					break;
			if(obstacled_path_length>=path.size()) {
				path=getPath(src, dst, visitable, param, width, height);
			} else {
				Path newpath=getPath(src, path[obstacled_path_length], visitable, param, width, height);
				if(newpath.size()<=threshold*obstacled_path_length) {
					for(int i=0; i<obstacled_path_length; i++)
						path.erase(path.begin());
					while(!newpath.empty()) {
						path.insert(path.begin(), *(newpath.end()-1));
						newpath.erase(newpath.end()-1);
					}
				}
			}
			return path;
		}
	}


	const Point2Di PathfinderAStar::neighbor_offset[] = { 
		Point2Di(1, 0), Point2Di(1, 1), Point2Di(0, 1), Point2Di(-1, 1), 
		Point2Di(-1, 0), Point2Di(-1, -1), Point2Di(0, -1), Point2Di(1, -1) 
	};
	PathfinderAStar::PathfinderAStar(int width, int height)
		: width(width), height(height), cell(allocate<Cell>(width, height)) {
	}
	PathfinderAStar::~PathfinderAStar() {
		deallocate(cell, width, height);
	}
	void PathfinderAStar::clear() {
		for (int r = 0; r<height; r++)
		for (int c = 0; c<width; c++)
			cell[r][c] = Cell();
		openset.clear();
	}
	void PathfinderAStar::addToOpenset(Point2Di p) {
		cell[p.y][p.x].in_openset = 1;
		openset.push_back(p);
	}
	void PathfinderAStar::removeFromOpenset(Point2Di p) {
		for (vector<Point2Di>::iterator i = openset.begin(); i != openset.end();)
		if (*i == p) i = openset.erase(i);
		else i++;
		cell[p.y][p.x].in_openset = 0;
	}
	void PathfinderAStar::addToClosedset(Point2Di p) {
		cell[p.y][p.x].in_closedset = 1;
	}
	Point2Di PathfinderAStar::getLowestFScoreInOpenSet() {
		Point2Di p;
		float min_f_score = FLT_MAX;
		for (int i = 0; i<openset.size(); i++) {
			if (cell[openset[i].y][openset[i].x].f_score<min_f_score) {
				min_f_score = cell[openset[i].y][openset[i].x].f_score;
				p = openset[i];
			}
		}
		return p;
	}
	Path PathfinderAStar::reconstructPath(Point2Di current_node) {
		Path path;
		for (Point2Di node = current_node; cell[node.y][node.x].in_came_from; node = cell[node.y][node.x].came_from)
			path.insert(path.begin(), node);
		return path;
	}
	int PathfinderAStar::heuristicCostEstimate(Point2Di src, Point2Di dst) {
		return dist(src, dst);
	}
	bool PathfinderAStar::intelligentVisitable(int x, int y, bool(*visitable)(void*, int, int), void* param) {
		if (cell[y][x].valid) {
			return cell[y][x].visitable;
		} else {
			cell[y][x].valid = 1;
			return cell[y][x].visitable = visitable(param, x, y) ? 1 : 0;
		}
	}
	Point2Di PathfinderAStar::getNearestPointInCameFrom(Point2Di src, Point2Di dst, bool(*visitable)(void*, int, int), void* param, int step) {
		unsigned int startTime = SDL_GetTicks(), time = manhattanDist(src, dst) / 2;
		for (int r = 0; r<height; r++)
		for (int c = 0; c<width; c++) {
			cell[r][c].in_closedset = false;
			cell[r][c].in_openset = false;
		}
		openset.clear();
		openset.insert(openset.end(), dst);
		while (!openset.empty()) {
			Point2Di current = openset.front();
			if (cell[current.y][current.x].in_came_from)
				return current;
			if (SDL_GetTicks() - startTime>time)
				break;
			openset.erase(openset.begin());
			for (int n = 0; n<8; n++) {
				Point2Di neighbor = current + neighbor_offset[n] * step;
				if (neighbor.in(0, 0, width - 1, height - 1) && !cell[neighbor.y][neighbor.x].in_openset)
					openset.insert(openset.end(), neighbor);
			}
		}
		Path line = lineBresenham(dst, src);
		for (int i = 0; i<line.size(); i++)
		if (cell[line[i].y][line[i].x].in_came_from && visitable(param, line[i].x, line[i].y)) {
			return line[i];
		}
		return src;
	}
	Path PathfinderAStar::findPath(Point2Di src, Point2Di dst, bool(*visitable)(void*, int, int), void* param, int step) {
		unsigned int startTime = SDL_GetTicks(), time = dist(src, dst) + 10;
		int sqrstep = step*step;
		clear();
		addToOpenset(src);
		cell[src.y][src.x].g_score = 0;
		cell[src.y][src.x].f_score = cell[src.y][src.x].g_score + heuristicCostEstimate(src, dst);
		while (!openset.empty()) {
			Point2Di current = getLowestFScoreInOpenSet();
			if (squareDist(current, dst)<sqrstep)
				return reconstructPath(current);
			if (SDL_GetTicks() - startTime>time)
				break;
			removeFromOpenset(current);
			addToClosedset(current);
			for (int n = 0; n<8; n++) {
				Point2Di neighbor = current + neighbor_offset[n] * step;
				if (neighbor.in(0, 0, width - 1, height - 1) && intelligentVisitable(neighbor.x, neighbor.y, visitable, param)) {
					float tentative_g_score = cell[current.y][current.x].g_score + dist(current, neighbor);
					if (cell[neighbor.y][neighbor.x].in_closedset && tentative_g_score >= cell[neighbor.y][neighbor.x].g_score)
						continue;
					if (!cell[neighbor.y][neighbor.x].in_closedset || tentative_g_score<cell[neighbor.y][neighbor.x].g_score) {
						cell[neighbor.y][neighbor.x].came_from = current;
						cell[neighbor.y][neighbor.x].in_came_from = true;
						cell[neighbor.y][neighbor.x].g_score = tentative_g_score;
						cell[neighbor.y][neighbor.x].f_score = cell[neighbor.y][neighbor.x].g_score + heuristicCostEstimate(neighbor, dst);
						if (!cell[neighbor.y][neighbor.x].in_openset)
							addToOpenset(neighbor);
					}
				}
			}
		}
		Point2Di nearestPoint = getNearestPointInCameFrom(src, dst, visitable, param, step);
		return reconstructPath(nearestPoint);
	}
	Path PathfinderAStar::recalculatePath(Path path, Point2Di src, Point2Di dst, bool(*visitable)(void*, int, int), void* param, float threshold, int step) {
		if (path.size() <= 1)
			return path;
		int obstacled_path_length;
		for (obstacled_path_length = 0; obstacled_path_length<path.size(); obstacled_path_length++)
		if (visitable(param, path[obstacled_path_length].x, path[obstacled_path_length].y))
			break;
		if (obstacled_path_length >= path.size()) {
			path = findPath(src, dst, visitable, param, step);
		} else {
			Path newpath = findPath(src, path[obstacled_path_length], visitable, param, step);
			if (newpath.size() <= threshold*obstacled_path_length) {
				for (int i = 0; i<obstacled_path_length; i++)
					path.erase(path.begin());
				while (!newpath.empty()) {
					path.insert(path.begin(), *(newpath.end() - 1));
					newpath.erase(newpath.end() - 1);
				}
			}
		}
		return path;
	}
};
