#pragma once

#include "layer.h"

#include <vector>

class LayerStack {
private:
	std::vector<Layer*> stack;
	int insert;

public:
	LayerStack();

	void pushLayer(Layer* layer);
	void popLayer(Layer* layer);
	void pushOverlay(Layer* layer);
	void popOverlay(Layer* layer);

	void init();
	void update();
	void event();
	void render();
	void close();

	std::vector<Layer*>::iterator begin();
	std::vector<Layer*>::iterator end();

	std::vector<Layer*>::reverse_iterator rbegin();
	std::vector<Layer*>::reverse_iterator rend();
};