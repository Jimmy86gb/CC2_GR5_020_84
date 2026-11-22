#ifndef GRAFO_HPP
#define GRAFO_HPP

#include "nodo.h"
#include <vector>
#include <map>

class grafo {
private:
	std::vector<nodo*> nodos;
	
public:
	grafo();
	~grafo();
	
	void agregarNodo(int valor);
	void agregarArista(int origen, int destino, int peso);
	void eliminarNodo(int valor);
	void eliminarArista(int origen, int destino);
	void mostrarGrafo();
	nodo* buscarNodo(int valor);
	
	// Algoritmos de caminos minimos
	void Dijkstra(int valorInicial);
	void BellmanFord(int valorInicial);
	void FloydWarshall();
	
	// Algoritmos de Árbol de Recubrimiento Mínimo (MST)
	void Kruskal();
	void Prim(int valorInicial);
};

#endif
