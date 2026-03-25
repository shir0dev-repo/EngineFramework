#pragma once

class Game {
public:
	void run();

protected:
	virtual void executeLogic();
	virtual void render();
	virtual void cleanup();
};