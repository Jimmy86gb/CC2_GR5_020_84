#ifndef NODO_H
#define NODO_H

#include <vector>

class nodo; // Declaración adelantada

struct Arista {
	nodo* destino;
	int capacidad;
	int flujo;
	int rev_idx; // Índice de la arista inversa en el nodo destino
};

class nodo {
private:
	int dato;
	std::vector<Arista> adyacentes;
	
public:
	nodo(int d);
	int getDato() const;
	void agregarArista(nodo* destino, int capacidad, int rev_idx);
	std::vector<Arista>& getAdyacentes(); // Retorna referencia para poder modificar el flujo
};

#endif
