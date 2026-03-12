#pragma once

class Game {
public:
	void Run();

protected:
	virtual void ExecuteLogic();
	virtual void Render();
	virtual void cleanup();
};