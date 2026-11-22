#include "nodo.h"

nodo::nodo(int d) : dato(d) {}

int nodo::getDato() const {
	return dato;
}

void nodo::agregarArista(nodo* destino, int capacidad, int rev_idx) {
	// el flujo inicial es 0
	adyacentes.push_back({destino, capacidad, 0, rev_idx});
}

std::vector<Arista>& nodo::getAdyacentes() {
	return adyacentes;
}
