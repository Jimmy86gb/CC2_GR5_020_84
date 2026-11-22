#include "Nodo.h"
#include <algorithm>

Nodo::Nodo(int id) : id(id), color(-1) {}

int Nodo::getId() const {
	return id;
}

int Nodo::getColor() const {
	return color;
}

void Nodo::setColor(int nuevoColor) {
	color = nuevoColor;
}

void Nodo::agregarAdyacente(Nodo* destino) {
	if (destino != nullptr && destino != this &&
		std::find(adyacentes.begin(), adyacentes.end(), destino) == adyacentes.end()) {
		adyacentes.push_back(destino);
	}
}

const std::vector<Nodo*>& Nodo::getAdyacentes() const {
	return adyacentes;
}
