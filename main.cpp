#include <iostream>
#include <vector>
#include <cmath>
#include <random>
#include <thread>
#include <chrono>

#ifdef _WIN32
	#include <windows.h>
#else
	#include <X11/Xlib.h>
#endif

struct Pt {
	double x, y;
};

class HM {
	std::mt19937 rng;
	std::normal_distribution<double> jd;

	Pt bz(double t, Pt p0, Pt p1, Pt p2, Pt p3) {
		double u = 1 - t;
		return {
			u*u*u*p0.x + 3 * u*u*t*p1.x + 3 * u*t*t*p2.x + t*t*t * p3.x,
			u*u*u*p0.y + 3 * u*u*t*p1.y + 3 * u*t*t*p2.y + t*t*t*p3.y
		};
	}

	void setCur(int x, int y) {
#ifdef _WIN32
		SetCursorPos(x, y);
#else
		Display* d = XOpenDisplay(NULL);
		XWarpPointer(d, None, None, 0, 0, 0, 0, x, y);
		XFlush(d);
		XCloseDisplay(d);
#endif
	}

public:
	HM() : rng(std::random_device{}()), jd(0, 1.5) {}

	void move(Pt s, Pt e) {
		double dx = e.x - s.x, dy = e.y - s.y;
		double dist = std::hypot(dx, dy);
		int steps = std::max(15, (int)(dist / 10));

		double off = dist * 0.15;
		double ang = std::atan2(dy, dx) + M_PI / 2;
		Pt c1 = {s.x + dx * 0.3 + off*std::cos(ang)*jd(rng),
		         s.y + dy * 0.3 + off*std::sin(ang)*jd(rng)
		        };
		Pt c2 = {s.x + dx * 0.7 + off*std::cos(ang)*jd(rng),
		         s.y + dy * 0.7 + off*std::sin(ang)*jd(rng)
		        };

		std::vector<Pt> path;
		for (int i = 0; i <= steps; ++i) {
			double t = (double)i / steps;
			Pt p = bz(t, s, c1, c2, e);
			p.x += jd(rng) * (0.5 + t * (1 - t) * 2);
			p.y += jd(rng) * (0.5 + t * (1 - t) * 2);
			path.push_back(p);
		}

		double dur = 100 + dist * 0.8;
		double st = dur / steps;

		std::this_thread::sleep_for(std::chrono::milliseconds(
		                                200 + std::uniform_int_distribution<>(0, 100)(rng)));

		for (int i = 0; i < path.size(); ++i) {
			double t = (double)i / (path.size() - 1);
			double ei = t < 0.5 ? 4 * t * t * t : 1 - std::pow(-2 * t + 2, 3) / 2;
			int idx = (int)(ei * (path.size() - 1));
			setCur((int)path[idx].x, (int)path[idx].y);
			std::this_thread::sleep_for(std::chrono::milliseconds((int)st));
		}
	}

	std::mt19937& getRng() {
		return rng;
	}
};

int main(int argc, char* argv[]) {
	if (argc < 3 || (argc - 1) % 2 != 0) {
		std::cout << "LuxMouse\nBilibili Follow MC_Chenzhao\nGithub StellavacuaLuxumbra\nB站关注MC_Chenzhao\n";
		std::cout << "Usage: LuxMouse.exe x1 y1 [x2 y2 ...]\n";
		return 1;
	}

	HM hm;
	std::vector<Pt> pts;
	for (int i = 1; i < argc; i += 2) {
		pts.push_back({std::atof(argv[i]), std::atof(argv[i + 1])});
	}

#ifdef _WIN32
	POINT cp;
	GetCursorPos(&cp);
	Pt cur = {(double)cp.x, (double)cp.y};
#else
	Display* d = XOpenDisplay(NULL);
	Window root, child;
	int rx, ry, wx, wy;
	unsigned mask;
	XQueryPointer(d, DefaultRootWindow(d), &root, &child, &rx, &ry, &wx, &wy, &mask);
	Pt cur = {(double)rx, (double)ry};
	XCloseDisplay(d);
#endif

	for (auto& p : pts) {
		hm.move(cur, p);
		cur = p;
		std::this_thread::sleep_for(std::chrono::milliseconds(
		                                std::uniform_int_distribution<>(300, 800)(hm.getRng())));
	}

	return 0;
}
