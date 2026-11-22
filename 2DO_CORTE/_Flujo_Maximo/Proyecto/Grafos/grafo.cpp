#include "grafo.h"
#include <iostream>
#include <queue>
#include <stack>
#include <unordered_map>
#include <algorithm>
#include <limits>

grafo::grafo() {}

grafo::~grafo() {
	for (nodo* n : nodos) {
		delete n;
	}
}

void grafo::agregarNodo(int valor) {
	if (!buscarNodo(valor)) {
		nodos.push_back(new nodo(valor));
	}
}

// cada arista dirigida requiere una arista inversa 
void grafo::agregarArista(int origen, int destino, int capacidad) {
	nodo* nodoOrigen = buscarNodo(origen);
	nodo* nodoDestino = buscarNodo(destino);
	
	if (nodoOrigen && nodoDestino) {
		size_t idx_origen = nodoOrigen->getAdyacentes().size();
		size_t idx_destino = nodoDestino->getAdyacentes().size();
		
		nodoOrigen->agregarArista(nodoDestino, capacidad, idx_destino);
		nodoDestino->agregarArista(nodoOrigen, 0, idx_origen); // Arista inversa
	} else {
		std::cout << "Origen o destino no encontrado." << std::endl;
	}
}

nodo* grafo::buscarNodo(int valor) {
	for (nodo* n : nodos) {
		if (n->getDato() == valor) return n;
	}
	return nullptr;
}

void grafo::resetearFlujos() {
	for (nodo* n : nodos) {
		for (Arista& a : n->getAdyacentes()) {
			a.flujo = 0;
		}
	}
}

void grafo::mostrarGrafo() {
	std::cout << "\nEstructura de la Red de Flujo" << std::endl;
	for (nodo* n : nodos) {
		std::cout << "Nodo " << n->getDato() << " conecta con: ";
		for (auto& arista : n->getAdyacentes()) {
			// Solo las aristas originales para no saturar la consola
			if (arista.capacidad > 0) {
				std::cout << "[" << arista.destino->getDato() 
					<< " (Cap: " << arista.capacidad << ", Flujo: " << arista.flujo << ")] ";
			}
		}
		std::cout << std::endl;
	}
}

// FORD-FULKERSON (Usa Búsqueda en Profundidad - DFS iterativo)
int grafo::FordFulkerson(int origen, int destino) {
	nodo* nodoOrigen = buscarNodo(origen);
	nodo* nodoDestino = buscarNodo(destino);
	if (!nodoOrigen || !nodoDestino) return 0;
	
	int flujo_maximo = 0;
	
	while (true) {
		std::unordered_map<nodo*, nodo*> padre;
		std::unordered_map<nodo*, size_t> indice_arista;
		std::stack<nodo*> pila;
		
		pila.push(nodoOrigen);
		padre[nodoOrigen] = nullptr;
		
		bool camino_encontrado = false;
		
		while (!pila.empty()) {
			nodo* u = pila.top();
			pila.pop();
			
			if (u == nodoDestino) {
				camino_encontrado = true;
				break;
			}
			
			auto& adyacentes = u->getAdyacentes();
			for (size_t i = 0; i < adyacentes.size(); ++i) {
				Arista& a = adyacentes[i];
				if (padre.find(a.destino) == padre.end() && a.destino != nodoOrigen && a.capacidad > a.flujo) {
					padre[a.destino] = u;
					indice_arista[a.destino] = i;
					pila.push(a.destino);
				}
			}
		}
		
		if (!camino_encontrado) break;
		
		int flujo_camino = std::numeric_limits<int>::max();
		for (nodo* v = nodoDestino; v != nodoOrigen; v = padre[v]) {
			nodo* u = padre[v];
			size_t idx = indice_arista[v];
			Arista& a = u->getAdyacentes()[idx];
			flujo_camino = std::min(flujo_camino, a.capacidad - a.flujo);
		}
		
		for (nodo* v = nodoDestino; v != nodoOrigen; v = padre[v]) {
			nodo* u = padre[v];
			size_t idx = indice_arista[v];
			Arista& a = u->getAdyacentes()[idx];
			Arista& rev = a.destino->getAdyacentes()[a.rev_idx];
			
			a.flujo += flujo_camino;
			rev.flujo -= flujo_camino;
		}
		
		flujo_maximo += flujo_camino;
	}
	
	return flujo_maximo;
}
