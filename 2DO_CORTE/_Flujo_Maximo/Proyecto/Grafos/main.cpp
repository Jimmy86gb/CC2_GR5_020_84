#include <iostream>
#include "grafo.h"

void cargarCasoDePrueba(grafo& g) {
	// Red de flujo clásica de 6 nodos
	for (int i = 0; i <= 5; i++) {
		g.agregarNodo(i);
	}
	
	// 0 es la fuente, 5 es el sumidero
	g.agregarArista(0, 1, 16);
	g.agregarArista(0, 2, 13);
	g.agregarArista(1, 2, 10);
	g.agregarArista(1, 3, 12);
	g.agregarArista(2, 1, 4);
	g.agregarArista(2, 4, 14);
	g.agregarArista(3, 2, 9);
	g.agregarArista(3, 5, 20);
	g.agregarArista(4, 3, 7);
	g.agregarArista(4, 5, 4);
	
	std::cout << "\n[!] Caso de prueba cargado (Nodos 0 a 5). Flujo Maximo esperado: 23\n";
}

void mostrarMenu() {
	std::cout << "\nFlujo Maximo\n";
	std::cout << "1. Agregar nodo\n";
	std::cout << "2. Agregar arista (origen - destino - capacidad)\n";
	std::cout << "3. Mostrar red de flujo\n";
	std::cout << "4. Ejecutar Ford-Fulkerson \n";
	std::cout << "5. Ejecutar Edmonds-Karp \n";	
	std::cout << "6. Cargar caso de prueba predeterminado\n";
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
			std::cout << "Ingrese el identificador del nodo: ";
			std::cin >> valor;
			miGrafo.agregarNodo(valor);
			break;
		}
		case 2: {
			int origen, destino, capacidad;
			std::cout << "Nodo origen: ";
			std::cin >> origen;
			std::cout << "Nodo destino: ";
			std::cin >> destino;
			std::cout << "Capacidad de la arista: ";
			std::cin >> capacidad;
			miGrafo.agregarArista(origen, destino, capacidad);
			std::cout << "Arista agregada correctamente.\n";
			break;
		}
		case 3: {
			miGrafo.mostrarGrafo();
			break;
		}
		case 4: {
			int inicio, fin;
			std::cout << "Ingrese nodo fuente (S): ";
			std::cin >> inicio;
			std::cout << "Ingrese nodo destino (D): ";
			std::cin >> fin;
			
			miGrafo.resetearFlujos(); // Limpiar calculos previos
			int maxFlow = miGrafo.FordFulkerson(inicio, fin);
			std::cout << "\n>>> Flujo Maximo (Ford-Fulkerson DFS): " << maxFlow << " <<<\n";
			break;
			
		}
		case 5: {
			
		}
		case 6: {
			cargarCasoDePrueba(miGrafo);
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
