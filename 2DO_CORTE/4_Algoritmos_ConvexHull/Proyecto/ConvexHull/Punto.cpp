#include "Punto.h"

Punto::Punto(int x, int y) : x(x), y(y) {}

int Punto::getX() const {
	return x;
}

int Punto::getY() const {
	return y;
}

void Punto::agregarArista(Punto* destino) {
	adyacentes.push_back({destino});
}

void Punto::limpiarAristas() {
	adyacentes.clear();
}

const std::vector<AristaConvexa>& Punto::getAdyacentes() const {
	return adyacentes;
}

bool Punto::operator<(const Punto& otro) const {
	if (x != otro.x) return x < otro.x;
	return y < otro.y;
}

bool Punto::operator==(const Punto& otro) const {
	return x == otro.x && y == otro.y;
}

long long Punto::productoCruz(const Punto* O, const Punto* A, const Punto* B) {
	return static_cast<long long>(A->x - O->x) * (B->y - O->y) -
		static_cast<long long>(A->y - O->y) * (B->x - O->x);
}
