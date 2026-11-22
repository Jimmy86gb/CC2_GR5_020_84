#include <iostream>
#include <chrono>
#include <random>
#include <vector>
#include <algorithm>
#include "grafo.h"

void mostrarMenu() {
	std::cout << "\n=================== MENU DE GRAFOS ===================\n";
	std::cout << "1. Agregar nodo\n";
	std::cout << "2. Agregar arista\n";
	std::cout << "3. Eliminar nodo\n";
	std::cout << "4. Eliminar arista\n";
	std::cout << "5. Mostrar grafo\n";
	std::cout << "6. Ejecutar Dijkstra (Caminos minimos)\n";
	std::cout << "7. Ejecutar Bellman-Ford (Caminos minimos)\n";
	std::cout << "8. Ejecutar Floyd-Warshall (Matriz de distancias)\n";
	std::cout << "9. Ejecutar Kruskal (Arbol de Recubrimiento Minimo con cronometro)\n";
	std::cout << "10. Ejecutar Prim (Arbol de Recubrimiento Minimo con cronometro)\n";
	std::cout << "11. Generar grafo aleatorio (n nodos, media m de conexiones)\n";
	std::cout << "0. Salir\n";
	std::cout << "Seleccione una opcion: ";
}

int main() {
	grafo miGrafo;
	int opcion;
	
	do {
		mostrarMenu();
		std::cin >> opcion;
		
		switch (opcion) {
		case 1: {
			int valor;
			std::cout << "Ingrese el valor del nodo: ";
			std::cin >> valor;
			miGrafo.agregarNodo(valor);
			break;
		}
		case 2: {
			int origen, destino, peso;
			std::cout << "Nodo origen: ";
			std::cin >> origen;
			std::cout << "Nodo destino: ";
			std::cin >> destino;
			std::cout << "Peso de la arista: ";
			std::cin >> peso;
			// Grafo no dirigido (bidireccional)
			miGrafo.agregarArista(origen, destino, peso);
			miGrafo.agregarArista(destino, origen, peso);
			std::cout << "Arista agregada bidireccionalmente.\n";
			break;
		}
		case 3: {
			int valor;
			std::cout << "Ingrese el valor del nodo a eliminar: ";
			std::cin >> valor;
			miGrafo.eliminarNodo(valor);
			break;
		}
		case 4: {
			int origen, destino;
			std::cout << "Nodo origen: ";
			std::cin >> origen;
			std::cout << "Nodo destino: ";
			std::cin >> destino;
			miGrafo.eliminarArista(origen, destino);
			miGrafo.eliminarArista(destino, origen);
			break;
		}
		case 5: {
			miGrafo.mostrarGrafo();
			break;
		}
		case 6: {
			int inicio;
			std::cout << "Ingrese el nodo inicial para Dijkstra: ";
			std::cin >> inicio;
			
			auto tInicio = std::chrono::high_resolution_clock::now();
			miGrafo.Dijkstra(inicio);
			auto tFin = std::chrono::high_resolution_clock::now();
			
			std::chrono::duration<double, std::milli> duracion = tFin - tInicio;
			std::cout << "[Tiempo de ejecucion (Dijkstra): " << duracion.count() << " ms]\n";
			break;
		}
		case 7: {
			int inicio;
			std::cout << "Ingrese el nodo inicial para Bellman-Ford: ";
			std::cin >> inicio;
			
			auto tInicio = std::chrono::high_resolution_clock::now();
			miGrafo.BellmanFord(inicio);
			auto tFin = std::chrono::high_resolution_clock::now();
			
			std::chrono::duration<double, std::milli> duracion = tFin - tInicio;
			std::cout << "[Tiempo de ejecucion (Bellman-Ford): " << duracion.count() << " ms]\n";
			break;
		}
		case 8: {
			auto tInicio = std::chrono::high_resolution_clock::now();
			miGrafo.FloydWarshall();
			auto tFin = std::chrono::high_resolution_clock::now();
			
			std::chrono::duration<double, std::milli> duracion = tFin - tInicio;
			std::cout << "[Tiempo de ejecucion (Floyd-Warshall): " << duracion.count() << " ms]\n";
			break;
		}
		case 9: { // Kruskal cronometrado
			auto tInicio = std::chrono::high_resolution_clock::now();
			miGrafo.Kruskal();
			auto tFin = std::chrono::high_resolution_clock::now();
			
			std::chrono::duration<double, std::milli> duracion = tFin - tInicio;
			std::cout << "[Tiempo de ejecucion (Kruskal): " << duracion.count() << " ms]\n";
			break;
		}
		case 10: { // Prim cronometrado
			int inicio;
			std::cout << "Ingrese el nodo inicial para Prim: ";
			std::cin >> inicio;
			
			auto tInicio = std::chrono::high_resolution_clock::now();
			miGrafo.Prim(inicio);
			auto tFin = std::chrono::high_resolution_clock::now();
			
			std::chrono::duration<double, std::milli> duracion = tFin - tInicio;
			std::cout << "[Tiempo de ejecucion (Prim): " << duracion.count() << " ms]\n";
			break;
		}
		case 11: { // Generación aleatoria sin cronometrar
			int n, m;
			std::cout << "Ingrese el numero de nodos (n): ";
			std::cin >> n;
			std::cout << "Ingrese la media de conexiones por nodo (m): ";
			std::cin >> m;
			
			if (n <= 0 || m < 0 || m >= n) {
				std::cout << "Valores invalidos. Asegúrese de que 0 <= m < n.\n";
				break;
			}
			
			// Reiniciar el grafo
			miGrafo = grafo();
			
			// Crear los n nodos (identificados del 1 al n)
			std::vector<int> nodosIds(n);
			for (int i = 0; i < n; ++i) {
				nodosIds[i] = i + 1;
				miGrafo.agregarNodo(nodosIds[i]);
			}
			
			std::random_device rd;
			std::mt19937 gen(rd());
			std::uniform_int_distribution<int> distPeso(1, 100);
			
			// 1. Garantizar conectividad (Árbol base no dirigido)
			std::shuffle(nodosIds.begin(), nodosIds.end(), gen);
			for (int i = 0; i < n - 1; ++i) {
				int u = nodosIds[i];
				int v = nodosIds[i + 1];
				int peso = distPeso(gen);
				miGrafo.agregarArista(u, v, peso);
				miGrafo.agregarArista(v, u, peso);
			}
			
			// 2. Añadir aristas adicionales para cumplir con la media 'm'
			int aristasObjetivo = (n * m) / 2;
			int aristasActuales = n - 1;
			int intentos = 0;
			
			while (aristasActuales < aristasObjetivo && intentos < n * n * 2) {
				intentos++;
				std::uniform_int_distribution<int> distNodo(0, n - 1);
				int idx1 = distNodo(gen);
				int idx2 = distNodo(gen);
				if (idx1 == idx2) continue;
				
				int u = nodosIds[idx1];
				int v = nodosIds[idx2];
				
				nodo* nodoU = miGrafo.buscarNodo(u);
				bool existe = false;
				if (nodoU) {
					for (auto& par : nodoU->getAdyacentes()) {
						if (par.first->getDato() == v) {
							existe = true;
							break;
						}
					}
				}
				
				if (!existe) {
					int peso = distPeso(gen);
					miGrafo.agregarArista(u, v, peso);
					miGrafo.agregarArista(v, u, peso);
					aristasActuales++;
				}
			}
			
			std::cout << "Grafo aleatorio no dirigido generado con exito \n";
			break;
		}
		case 0:
			std::cout << "Saliendo del programa...\n";
			break;
		default:
			std::cout << "Opcion invalida. Intente de nuevo.\n";
		}
	} while (opcion != 0);
	
	return 0;
}
