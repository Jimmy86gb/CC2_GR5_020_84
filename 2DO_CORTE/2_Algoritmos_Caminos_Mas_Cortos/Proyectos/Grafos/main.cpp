#include <iostream>
#include "grafo.h"

void mostrarMenu() {
	std::cout << "\n MENU \n";
	std::cout << "--- Construccion ---\n";
	std::cout << "1. Agregar Nodo\n";
	std::cout << "2. Agregar Arista\n";
	std::cout << "3. Eliminar Nodo\n";
	std::cout << "4. Eliminar Arista\n";
	std::cout << "5. Mostrar Estructura del Grafo\n";
	std::cout << "--- Algoritmos ---\n";
	std::cout << "6. Ejecutar Dijkstra (Origen unico)\n";
	std::cout << "7. Ejecutar Bellman-Ford (Origen unico)\n";
	std::cout << "8. Ejecutar Floyd-Warshall (Todos contra todos)\n";
	std::cout << "--- Pruebas ---\n";
	std::cout << "9. Cargar Grafo Prueba 1 (Pesos positivos)\n";
	std::cout << "10. Cargar Grafo Prueba 2 (Pesos negativos)\n";
	std::cout << "11. Salir\n";
	std::cout << "\nElige una opcion: ";
}

int main() {
	grafo miGrafo;
	int opcion;
	int u, v, peso;
	
	do {
		mostrarMenu();
		std::cin >> opcion;
		
		switch (opcion) {
		case 1:
			std::cout << "Ingrese el valor del nodo: ";
			std::cin >> u;
			miGrafo.agregarNodo(u);
			std::cout << "Nodo agregado.\n";
			break;
		case 2:
			std::cout << "Ingrese origen, destino y peso (ej. 1 2 5): ";
			std::cin >> u >> v >> peso;
			miGrafo.agregarArista(u, v, peso);
			break;
		case 3:
			std::cout << "Ingrese el valor del nodo a eliminar: ";
			std::cin >> u;
			miGrafo.eliminarNodo(u);
			break;
		case 4:
			std::cout << "Ingrese origen y destino de la arista a eliminar (ej. 1 2): ";
			std::cin >> u >> v;
			miGrafo.eliminarArista(u, v);
			break;
		case 5:
			miGrafo.mostrarGrafo();
			break;
		case 6:
			std::cout << "Ingrese nodo origen para Dijkstra: ";
			std::cin >> u;
			miGrafo.Dijkstra(u);
			break;
		case 7:
			std::cout << "Ingrese nodo origen para Bellman-Ford: ";
			std::cin >> u;
			miGrafo.BellmanFord(u);
			break;
		case 8:
			miGrafo.FloydWarshall();
			break;
		case 9:
			miGrafo = grafo(); 
			for (int i = 1; i <= 5; ++i) miGrafo.agregarNodo(i);
			miGrafo.agregarArista(1, 2, 4);
			miGrafo.agregarArista(1, 3, 2);
			miGrafo.agregarArista(2, 3, 1);
			miGrafo.agregarArista(2, 4, 5);
			miGrafo.agregarArista(3, 4, 8);
			miGrafo.agregarArista(3, 5, 10);
			miGrafo.agregarArista(4, 5, 2);
			std::cout << "Grafo de prueba 1 cargado.\n";
			break;
		case 10:
			miGrafo = grafo(); 
			for (int i = 1; i <= 4; ++i) miGrafo.agregarNodo(i);
			miGrafo.agregarArista(1, 2, 2);
			miGrafo.agregarArista(1, 3, 5);
			miGrafo.agregarArista(2, 4, 3);
			miGrafo.agregarArista(3, 2, -10);
			std::cout << "Grafo de prueba 2 cargado.\n";
			break;
		case 11:
			std::cout << "Saliendo...\n";
			break;
		default:
			std::cout << "Opcion invalida.\n";
		}
	} while (opcion != 11);
	
	return 0;
}
