#include "Nodo.h"
#include <cmath>

nodo::nodo(int d, const std::string& nom, double px, double py, TipoNodo t, int dem)
	: dato(d), nombre(nom), x(px), y(py), tipo(t), demanda(dem), colorZona(-1) {}

int nodo::getDato() const {
	return dato;
}

std::string nodo::getNombre() const {
	return nombre;
}

double nodo::getX() const {
	return x;
}

double nodo::getY() const {
	return y;
}

TipoNodo nodo::getTipo() const {
	return tipo;
}

int nodo::getDemanda() const {
	return demanda;
}

int nodo::getColorZona() const {
	return colorZona;
}

void nodo::setColorZona(int color) {
	colorZona = color;
}

double nodo::distanciaEuclidiana(const nodo* otro) const {
	if (!otro) return 0.0;
	double dx = x - otro->x;
	double dy = y - otro->y;
	return std::hypot(dx, dy);
}

void nodo::agregarArista(nodo* destino, int peso) {
	if (!destino || destino == this) return;
	for (const auto& par : adyacentes) {
		if (par.first == destino) return;
	}
	adyacentes.push_back({destino, peso});
}

void nodo::eliminarArista(nodo* destino) {
	for (auto it = adyacentes.begin(); it != adyacentes.end(); ) {
		if (it->first == destino) {
			it = adyacentes.erase(it);
		} else {
			++it;
		}
	}
}

const std::vector<std::pair<nodo*, int>>& nodo::getAdyacentes() const {
	return adyacentes;
}
