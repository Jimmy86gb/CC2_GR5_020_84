#include "nodo.h"

nodo::nodo(int d) : dato(d) {}

int nodo::getDato() const {
	return dato;
}

void nodo::agregarArista(nodo* destino, int peso) {
	adyacentes.push_back({destino, peso});
}

void nodo::eliminarArista(nodo* destino) {
	for (auto it = adyacentes.begin(); it != adyacentes.end(); ) {
		if (it->first == destino) {
			it = adyacentes.erase(it); // erase devuelve el iterador al siguiente elemento
		} else {
			++it;
		}
	}
}

const std::vector<std::pair<nodo*, int>>& nodo::getAdyacentes() const {
	return adyacentes;
}
