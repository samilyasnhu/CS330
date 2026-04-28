#include <GLFW\glfw3.h>
#include "linmath.h"
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <iostream>
#include <vector>
#include <windows.h>
#include <time.h>
#include <cmath>

using namespace std;

const float DEG2RAD = 3.14159 / 180;

void processInput(GLFWwindow* window);

enum BRICKTYPE { REFLECTIVE, DESTRUCTABLE };
enum ONOFF { ON, OFF };

class Brick
{
public:
	float red, green, blue;
	float x, y, width;
	int hitsRemaining; // NEW
	BRICKTYPE brick_type;
	ONOFF onoff;

	Brick(BRICKTYPE bt, float xx, float yy, float ww, float rr, float gg, float bb)
	{
		brick_type = bt;
		x = xx; y = yy; width = ww;
		red = rr; green = gg; blue = bb;
		onoff = ON;

		hitsRemaining = (bt == DESTRUCTABLE) ? 3 : -1; // 3 hits required
	};

	void drawBrick()
	{
		if (onoff == ON)
		{
			double halfside = width / 2;

			glColor3d(red, green, blue);
			glBegin(GL_POLYGON);

			glVertex2d(x + halfside, y + halfside);
			glVertex2d(x + halfside, y - halfside);
			glVertex2d(x - halfside, y - halfside);
			glVertex2d(x - halfside, y + halfside);

			glEnd();
		}
	}
};


class Circle
{
public:
	float red, green, blue;
	float radius;
	float x;
	float y;
	float speed = 0.03;
	int direction; // 1=up 2=right 3=down 4=left 5 = up right   6 = up left  7 = down right  8= down left

	Circle(double xx, double yy, double rr, int dir, float rad, float r, float g, float b)
	{
		x = xx;
		y = yy;
		radius = rr;
		red = r;
		green = g;
		blue = b;
		radius = rad;
		direction = dir;
	}

	void CheckCollision(Brick* brk)
	{
		if (brk->brick_type == REFLECTIVE)
		{
			if ((x > brk->x - brk->width && x <= brk->x + brk->width) && (y > brk->y - brk->width && y <= brk->y + brk->width))
			{
				direction = GetRandomDirection();
				x = x + 0.03;
				y = y + 0.04;
			}
		}
		else if (brk->brick_type == DESTRUCTABLE)
		{
			if ((x > brk->x - brk->width && x <= brk->x + brk->width) &&
				(y > brk->y - brk->width && y <= brk->y + brk->width))
			{
				brk->hitsRemaining--;

				// Change color as it gets damaged
				brk->red *= 0.7f;
				brk->green *= 0.7f;
				brk->blue *= 0.7f;

				if (brk->hitsRemaining <= 0)
				{
					brk->onoff = OFF;
				}

				// Bounce instead of random teleport
				direction = 9 - direction; // simple flip
			}
		}
	}

	int GetRandomDirection()
	{
		return (rand() % 8) + 1;
	}

	void MoveOneStep()
	{
		if (y <= -1 + radius || y >= 1 - radius)
		{
			speed *= 1.05f; // slight speed increase
			direction = (direction == 1) ? 3 : (direction == 3) ? 1 : direction;
		}

		if (x <= -1 + radius || x >= 1 - radius)
		{
			speed *= 1.05f;
			direction = (direction == 2) ? 4 : (direction == 4) ? 2 : direction;
		}
	}

	void DrawCircle()
	{
		glColor3f(red, green, blue);
		glBegin(GL_POLYGON);
		for (int i = 0; i < 360; i++) {
			float degInRad = i * DEG2RAD;
			glVertex2f((cos(degInRad) * radius) + x, (sin(degInRad) * radius) + y);
		}
		glEnd();
	}
};


vector<Circle> world;

float paddleX = 0.0f;

int main(void) {
	srand(time(NULL));

	if (!glfwInit()) {
		exit(EXIT_FAILURE);
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

	GLFWwindow* window = glfwCreateWindow(480, 480, "8-2 Assignment", NULL, NULL);
	if (!window) {
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	// ==========================
	// CREATE BRICK GRID
	// ==========================
	vector<Brick> bricks;

	for (int i = -2; i <= 2; i++)
	{
		for (int j = 1; j <= 3; j++)
		{
			float x = i * 0.3f;
			float y = j * 0.25f;

			if ((i + j) % 2 == 0)
				bricks.push_back(Brick(DESTRUCTABLE, x, y, 0.2, 0.2f, 0.8f, 0.3f));
			else
				bricks.push_back(Brick(REFLECTIVE, x, y, 0.2, 0.9f, 0.9f, 0.2f));
		}
	}

	// ==========================
	// MAIN LOOP
	// ==========================
	while (!glfwWindowShouldClose(window)) {

		// Setup View
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT);

		processInput(window);

		// Setup View
		glClear(GL_COLOR_BUFFER_BIT);

		processInput(window);

		// ==========================
		// 🔹 CIRCLE vs CIRCLE COLLISION (PUT IT HERE)
		// ==========================
		for (int i = 0; i < world.size(); i++)
		{
			for (int j = i + 1; j < world.size(); j++)
			{
				float dx = world[i].x - world[j].x;
				float dy = world[i].y - world[j].y;
				float dist = sqrt(dx * dx + dy * dy);

				if (dist < world[i].radius + world[j].radius)
				{
					// Change colors on collision
					world[i].red = 1.0f;
					world[j].blue = 1.0f;
				}
			}
		}

		// ==========================
		// UPDATE + DRAW CIRCLES
		// ==========================
		for (int i = 0; i < world.size(); i++)
		{
			// Check collision with all bricks
			for (int j = 0; j < bricks.size(); j++)
			{
				world[i].CheckCollision(&bricks[j]);
			}

			world[i].MoveOneStep();
			world[i].DrawCircle();
		}

		// ==========================
		// DRAW BRICKS
		// ==========================
		for (int j = 0; j < bricks.size(); j++)
		{
			bricks[j].drawBrick();
		}

		// ==========================
		// DRAW PADDLE
		// ==========================
		glColor3f(1.0f, 1.0f, 1.0f);
		glBegin(GL_POLYGON);
		glVertex2f(paddleX - 0.2f, -0.9f);
		glVertex2f(paddleX + 0.2f, -0.9f);
		glVertex2f(paddleX + 0.2f, -0.85f);
		glVertex2f(paddleX - 0.2f, -0.85f);
		glEnd();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}


void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		paddleX -= 0.05f;

	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		paddleX += 0.05f;
}