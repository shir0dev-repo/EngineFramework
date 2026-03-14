#pragma once

#include <string>

struct Text {
	std::string text;

	Text() : text("") {}
	Text(std::string text) : text(text) {}
};