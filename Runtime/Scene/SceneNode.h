#pragma once

#include <Core/Structure/IDVector.h>

struct SceneNode {
	IDVector<SceneNode> children;
	SceneNode* Parent;
	struct Entity* Entity;

};