#ifndef GRAFO_H
#define GRAFO_H

#include "nodo.h"
#include <vector>

class grafo {
private:
	std::vector<nodo*> nodos;
	
public:
	grafo();
	~grafo();
	
	void agregarNodo(int valor);
	void agregarArista(int origen, int destino, int capacidad);
	nodo* buscarNodo(int valor);
	void resetearFlujos(); // Necesario para correr varias veces en el mismo grafo
	void mostrarGrafo();
	
	// Algoritmos de Flujo Máximo
	int EdmondsKarp(int origen, int destino);
	int FordFulkerson(int origen, int destino);
};

#endif
