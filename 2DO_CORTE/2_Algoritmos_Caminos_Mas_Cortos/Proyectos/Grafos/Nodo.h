#ifndef NODO_HPP
#define NODO_HPP

#include <vector>
#include <utility>

class nodo {
private:
	int dato;
	std::vector<std::pair<nodo*, int>> adyacentes;
	
public:
	nodo(int d);
	int getDato() const;
	void agregarArista(nodo* destino, int peso);
	void eliminarArista(nodo* destino);
	const std::vector<std::pair<nodo*, int>>& getAdyacentes() const;
};

#endif
