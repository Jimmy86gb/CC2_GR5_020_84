#include "grafo.h" 
#include <iostream>
#include <limits>
#include <queue>
#include <tuple>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <set>
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

void grafo::agregarArista(int origen, int destino, int peso) {
	nodo* nodoOrigen = buscarNodo(origen);
	nodo* nodoDestino = buscarNodo(destino);
	if (nodoOrigen && nodoDestino) {
		nodoOrigen->agregarArista(nodoDestino, peso);
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

//Elimina la conexion directa entre dos nodos
void grafo::eliminarArista(int origen, int destino) {
	nodo* nodoOrigen = buscarNodo(origen);
	nodo* nodoDestino = buscarNodo(destino);
	if (nodoOrigen && nodoDestino) {
		nodoOrigen->eliminarArista(nodoDestino);
		std::cout << "Arista eliminada." << std::endl;
	} else {
		std::cout << "Origen o destino no encontrado." << std::endl;
	}
}

// Elimina un nodo de forma segura, evitando punteros a null
void grafo::eliminarNodo(int valor) {
	nodo* nodoAEliminar = buscarNodo(valor);
	if (!nodoAEliminar) {
		std::cout << "Nodo no encontrado." << std::endl;
		return;
	}
	
	// Eliminar cualquier arista en todo el grafo que apunte a este nodo
	for (nodo* n : nodos) {
		n->eliminarArista(nodoAEliminar);
	}
	
	// Sacar el nodo de la lista principal del grafo
	for (auto it = nodos.begin(); it != nodos.end(); ++it) {
		if (*it == nodoAEliminar) {
			nodos.erase(it);
			break;
		}
	}
	
	// Liberar memoria
	delete nodoAEliminar;
	std::cout << "Nodo y todas sus conexiones entrantes/salientes eliminados." << std::endl;
}

//Muestra la estructura actual para verificar eliminaciones
void grafo::mostrarGrafo() {
	if (nodos.empty()) {
		std::cout << "El grafo esta vacio." << std::endl;
		return;
	}
	std::cout << "\nEstructura del Grafo" << std::endl;
	for (nodo* n : nodos) {
		std::cout << "Nodo " << n->getDato() << " conecta con: ";
		if (n->getAdyacentes().empty()) {
			std::cout << "[Ninguno]";
		} else {
			for (auto par : n->getAdyacentes()) {
				std::cout << "[" << par.first->getDato() << " (peso: " << par.second << ")] ";
			}
		}
		std::cout << std::endl;
	}
}

//DIJKSTRA
void grafo::Dijkstra(int valorInicial) {
	nodo* nodoInicial = buscarNodo(valorInicial);
	if (!nodoInicial) {
		std::cout << "Nodo no encontrado" << std::endl;
		return;
	}
	
	std::map<int, int> distancias;
	std::map<int, bool> visitado;
	std::map<int, int> predecesor;
	
	for (auto nod : nodos) {
		distancias[nod->getDato()] = std::numeric_limits<int>::max();
		visitado[nod->getDato()] = false;
	}
	distancias[valorInicial] = 0;
	
	using p = std::pair<int, nodo*>;
	std::priority_queue<p, std::vector<p>, std::greater<p>> cola;
	cola.push({0, nodoInicial});
	
	while (!cola.empty()) {
		auto elemento = cola.top();
		int disActual = elemento.first;
		nodo* nodoActual = elemento.second;
		cola.pop();
		
		int u = nodoActual->getDato();
		if (visitado[u]) continue;
		visitado[u] = true;
		
		for (auto par : nodoActual->getAdyacentes()) {
			nodo* vecino = par.first;
			int peso = par.second;
			
			int v = vecino->getDato();
			int nuevaDistancia = disActual + peso;
			
			if (nuevaDistancia < distancias[v]) {
				distancias[v] = nuevaDistancia;
				predecesor[v] = u;
				cola.push({nuevaDistancia, vecino});
			}
		}
	}
	
	std::cout << "\nDistancias Dijkstra (Origen: " << valorInicial << ")" << std::endl;
	for (auto par : distancias) {
		int nod = par.first;
		int distancia = par.second;
		if (distancia == std::numeric_limits<int>::max()) {
			std::cout << "Nodo " << nod << ": No alcanzable" << std::endl;
		} else {
			std::cout << "Nodo " << nod << " | Distancia: " << distancia << " | Ruta: ";
			
			// Reconstruir y mostrar la ruta
			if (nod == valorInicial) {
				std::cout << nod << std::endl;
			} else {
				std::vector<int> ruta;
				int actual = nod;
				while (actual != valorInicial) {
					ruta.push_back(actual);
					actual = predecesor[actual];
				}
				ruta.push_back(valorInicial);
				for (int i = ruta.size() - 1; i >= 0; --i) {
					std::cout << ruta[i] << (i > 0 ? " -> " : "");
				}
				std::cout << std::endl;
			}
		}
	}
}

//BELLMAN-FORD
void grafo::BellmanFord(int valorInicial) {
	nodo* nodoInicial = buscarNodo(valorInicial);
	if (!nodoInicial) {
		std::cout << "Nodo no encontrado" << std::endl;
		return;
	}
	
	std::map<int, int> distancias;
	std::map<int, int> predecesor;
	
	for (auto nod : nodos) {
		distancias[nod->getDato()] = std::numeric_limits<int>::max();
	}
	distancias[valorInicial] = 0;
	
	std::vector<std::tuple<int, int, int>> aristas;
	for (auto nod : nodos) {
		for (auto par : nod->getAdyacentes()) {
			nodo* vecino = par.first;
			int peso = par.second;
			aristas.push_back(std::make_tuple(nod->getDato(), vecino->getDato(), peso));
		}
	}
	
	int V = nodos.size();
	for (int i = 0; i < V - 1; i++) {
		for (auto arista : aristas) {
			int u = std::get<0>(arista);
			int v = std::get<1>(arista);
			int peso = std::get<2>(arista);
			
			if (distancias[u] != std::numeric_limits<int>::max() && distancias[u] + peso < distancias[v]) {
				distancias[v] = distancias[u] + peso;
				predecesor[v] = u;
			}
		}
	}
	
	bool cicloNegativo = false;
	for (auto arista : aristas) {
		int u = std::get<0>(arista);
		int v = std::get<1>(arista);
		int peso = std::get<2>(arista);
		
		if (distancias[u] != std::numeric_limits<int>::max() && distancias[u] + peso < distancias[v]) {
			cicloNegativo = true;
			break;
		}
	}
	
	std::cout << "\nDistancias Bellman-Ford (Origen: " << valorInicial << ")" << std::endl;
	if (cicloNegativo) {
		std::cout << "Se detecto un ciclo negativo. Las distancias no son confiables." << std::endl;
		return;
	}
	
	for (auto par : distancias) {
		int nod = par.first;
		int distancia = par.second;
		if (distancia == std::numeric_limits<int>::max()) {
			std::cout << "Nodo " << nod << ": No alcanzable" << std::endl;
		} else {
			std::cout << "Nodo " << nod << " | Distancia: " << distancia << " | Ruta: ";
			
			// Reconstruir y mostrar la ruta
			if (nod == valorInicial) {
				std::cout << nod << std::endl;
			} else {
				std::vector<int> ruta;
				int actual = nod;
				while (actual != valorInicial && predecesor.count(actual)) {
					ruta.push_back(actual);
					actual = predecesor[actual];
				}
				ruta.push_back(valorInicial);
				for (int i = ruta.size() - 1; i >= 0; --i) {
					std::cout << ruta[i] << (i > 0 ? " -> " : "");
				}
				std::cout << std::endl;
			}
		}
	}
}

//FLOYD-WARSHALL
void grafo::FloydWarshall() {
	int V = nodos.size();
	if (V == 0) return;
	
	std::map<int, int> nodoAIndice;
	std::map<int, int> indiceANodo;
	for (int i = 0; i < V; ++i) {
		nodoAIndice[nodos[i]->getDato()] = i;
		indiceANodo[i] = nodos[i]->getDato();
	}
	
	const int INF = 9999999;
	std::vector<std::vector<int>> dist(V, std::vector<int>(V, INF));
	
	for (int i = 0; i < V; ++i) {
		dist[i][i] = 0; 
	}
	
	for (auto nod : nodos) {
		int u = nodoAIndice[nod->getDato()];
		for (auto par : nod->getAdyacentes()) {
			nodo* vecino = par.first;
			int peso = par.second;
			int v = nodoAIndice[vecino->getDato()];
			dist[u][v] = peso;
		}
	}
	
	for (int k = 0; k < V; ++k) {
		for (int i = 0; i < V; ++i) {
			for (int j = 0; j < V; ++j) {
				if (dist[i][k] != INF && dist[k][j] != INF && dist[i][k] + dist[k][j] < dist[i][j]) {
					dist[i][j] = dist[i][k] + dist[k][j];
				}
			}
		}
	}
	
	std::cout << "\nMatriz de Distancias Minimas (Floyd-Warshall)" << std::endl;
	std::cout << "   |";
	for (int i = 0; i < V; ++i) std::cout << std::setw(5) << indiceANodo[i];
	std::cout << "\n---+";
	for (int i = 0; i < V; ++i) std::cout << "-----";
	std::cout << "\n";
	
	for (int i = 0; i < V; ++i) {
		std::cout << std::setw(2) << indiceANodo[i] << " |";
		for (int j = 0; j < V; ++j) {
			if (dist[i][j] == INF) {
				std::cout << std::setw(5) << "INF";
			} else {
				std::cout << std::setw(5) << dist[i][j];
			}
		}
		std::cout << std::endl;
	}
	
}

#include "grafo.h" 
#include <iostream>
#include <limits>
#include <queue>
#include <tuple>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <set>

// Estructura auxiliar para Union-Find (DSU) utilizada en Kruskal
struct DSU {
	std::map<int, int> padre;
	std::map<int, int> rango;
	
	void hacerSet(int i) {
		padre[i] = i;
		rango[i] = 0;
	}
	
	int encontrar(int i) {
		if (padre[i] == i)
			return i;
		return padre[i] = encontrar(padre[i]); // Compresión de ruta
	}
	
	void unir(int i, int j) {
		int raizI = encontrar(i);
		int raizJ = encontrar(j);
		if (raizI != raizJ) {
			if (rango[raizI] < rango[raizJ]) {
				padre[raizI] = raizJ;
			} else if (rango[raizI] > rango[raizJ]) {
				padre[raizJ] = raizI;
			} else {
				padre[raizJ] = raizI;
				rango[raizI]++;
			}
		}
	}
};

// KRUSKAL
void grafo::Kruskal() {
	if (nodos.empty()) {
		std::cout << "El grafo esta vacio." << std::endl;
		return;
	}
	
	// Recolectar aristas únicas (evitando duplicados en grafos no dirigidos)
	std::vector<std::tuple<int, int, int>> aristas; // {peso, origen, destino}
	std::set<std::pair<int, int>> aristasVistas;
	
	for (auto nod : nodos) {
		int u = nod->getDato();
		for (auto par : nod->getAdyacentes()) {
			int v = par.first->getDato();
			int peso = par.second;
			
			int minN = std::min(u, v);
			int maxN = std::max(u, v);
			if (aristasVistas.find({minN, maxN}) == aristasVistas.end()) {
				aristasVistas.insert({minN, maxN});
				aristas.push_back(std::make_tuple(peso, u, v));
			}
		}
	}
	
	// Ordenar aristas por peso de menor a mayor (std::tuple se ordena por su primer elemento: peso)
	std::sort(aristas.begin(), aristas.end());
	
	// Inicializar DSU
	DSU dsu;
	for (auto nod : nodos) {
		dsu.hacerSet(nod->getDato());
	}
	
	int pesoTotal = 0;
	std::vector<std::tuple<int, int, int>> mstAristas;
	
	for (auto& arista : aristas) {
		int peso = std::get<0>(arista);
		int u = std::get<1>(arista);
		int v = std::get<2>(arista);
		
		if (dsu.encontrar(u) != dsu.encontrar(v)) {
			dsu.unir(u, v);
			pesoTotal += peso;
			mstAristas.push_back(std::make_tuple(u, v, peso));
		}
	}
	
	std::cout << "\nArbol de Recubrimiento Minimo - Kruskal" << std::endl;
	if (mstAristas.size() < nodos.size() - 1 && nodos.size() > 1) {
		std::cout << "Aviso: El grafo no esta completamente conectado." << std::endl;
	}
	for (auto& a : mstAristas) {
		std::cout << "Arista: " << std::get<0>(a) << " - " << std::get<1>(a) << " | Peso: " << std::get<2>(a) << std::endl;
	}
	std::cout << "Peso total del MST: " << pesoTotal << std::endl;
}

// PRIM
void grafo::Prim(int valorInicial) {
	nodo* nodoInicial = buscarNodo(valorInicial);
	if (!nodoInicial) {
		std::cout << "Nodo no encontrado" << std::endl;
		return;
	}
	
	std::map<int, bool> visitado;
	// Cola de prioridad para almacenar {peso, {origen, destino}} ordenada de menor a mayor peso
	using AristaP = std::pair<int, std::pair<int, int>>;
	std::priority_queue<AristaP, std::vector<AristaP>, std::greater<AristaP>> pq;
	
	visitado[valorInicial] = true;
	for (auto par : nodoInicial->getAdyacentes()) {
		pq.push({par.second, {valorInicial, par.first->getDato()}});
	}
	
	int pesoTotal = 0;
	std::vector<std::tuple<int, int, int>> mstAristas;
	
	while (!pq.empty()) {
		auto frente = pq.top();
		pq.pop();
		
		int peso = frente.first;
		int u = frente.second.first;
		int v = frente.second.second;
		
		if (visitado[v]) continue;
		
		visitado[v] = true;
		pesoTotal += peso;
		mstAristas.push_back(std::make_tuple(u, v, peso));
		
		nodo* nodoV = buscarNodo(v);
		if (nodoV) {
			for (auto par : nodoV->getAdyacentes()) {
				int vecinoVal = par.first->getDato();
				int pesoArista = par.second;
				if (!visitado[vecinoVal]) {
					pq.push({pesoArista, {v, vecinoVal}});
				}
			}
		}
	}
	
	std::cout << "\nArbol de Recubrimiento Minimo - Prim (Inicio: " << valorInicial << ")" << std::endl;
	if (mstAristas.size() < nodos.size() - 1 && nodos.size() > 1) {
		std::cout << "Aviso: El grafo no esta conectado. No se pudo formar un MST completo." << std::endl;
	}
	for (auto& a : mstAristas) {
		std::cout << "Arista: " << std::get<0>(a) << " - " << std::get<1>(a) << " | Peso: " << std::get<2>(a) << std::endl;
	}
	std::cout << "Peso total del MST: " << pesoTotal << std::endl;
}
