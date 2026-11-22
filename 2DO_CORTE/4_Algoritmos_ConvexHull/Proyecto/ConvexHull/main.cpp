#include <iostream>
#include "GrafoConvexo.h"
using namespace std;

void mostrarMenu() {
	cout << "\n===============================" << endl;
	cout << "     MENU GRAFO CONVEXO        " << endl;
	cout << "===============================" << endl;
	cout << "1. Añadir un nuevo punto" << endl;
	cout << "2. Limpiar grafo" << endl;
	cout << "3. Calcular Convex Hull (Monotone Chain)" << endl;
	cout << "4. Calcular Convex Hull (Graham Scan)" << endl;
	cout << "5. Calcular Convex Hull (Jarvis March)" << endl;
	cout << "6. Calcular Convex Hull (Divide y Venceras)" << endl;
	cout << "7. Mostrar grafo y conexiones" << endl;
	cout << "8. Precargar grafo de prueba" << endl;
	cout << "0. Salir" << endl;
	cout << "Elige una opcion: ";
}

int main (int argc, char *argv[]) {
	GrafoConvexo gc;
	gc.agregarNodo(0, 3);
	gc.agregarNodo(1, 1);
	gc.agregarNodo(2, 2);
	gc.agregarNodo(4, 4);
	gc.agregarNodo(0, 0);
	gc.agregarNodo(1, 2);
	gc.agregarNodo(3, 1);
	gc.agregarNodo(3, 3);
	

	gc.calcularConvexHull(); 
	
	// gc.calcularGrahamScan();
	// gc.calcularJarvisMarch();
	// gc.calcularDivideAndConquer();
	
	gc.mostrarConvexHull();
	gc.mostrarGrafo();
	
	gc.limpiarGrafo();
	for(int i = 0; i<9; i++){
		cout<<"\n";
	}
	
	int opcion;
	
	do {
		mostrarMenu();
		if (!(cin >> opcion)) {
			cout << "Entrada invalida. Por favor, introduce un numero." << endl;
			cin.clear();
			cin.ignore(10000, '\n');
			continue;
		}
		
		switch (opcion) {
		case 1: {
			int x, y;
			cout << "Introduce la coordenada X: ";
			cin >> x;
			cout << "Introduce la coordenada Y: ";
			cin >> y;
			gc.agregarNodo(x, y);
			cout << "Punto (" << x << ", " << y << ") aniadido con exito." << endl;
			break;
		}
		case 2:
			gc.limpiarGrafo();
			cout << "El grafo ha sido limpiado por completo." << endl;
			break;
			
		case 3:
			gc.calcularConvexHull();
			cout << "\n[Monotone Chain ejecutado]" << endl;
			gc.mostrarConvexHull();
			break;
			
		case 4:
			gc.calcularGrahamScan();
			cout << "\n[Graham Scan ejecutado]" << endl;
			gc.mostrarConvexHull();
			break;
			
		case 5:
			gc.calcularJarvisMarch();
			cout << "\n[Jarvis March ejecutado]" << endl;
			gc.mostrarConvexHull();
			break;
			
		case 6:
			gc.calcularDivideYVenceras();
			cout << "\n[Divide y Venceras ejecutado]" << endl;
			gc.mostrarConvexHull();
			break;
			
		case 7:
			gc.mostrarGrafo();
			gc.mostrarConvexHull();
			break;
			
		case 8: {
			gc.limpiarGrafo();
			
			int puntosPrueba[][2] = {
				{0, 3}, {1, 1}, {2, 2}, {4, 4}, 
				{0, 0}, {3, 0}, {3, 3}, {1, 4}
			};
			
			for (auto& p : puntosPrueba) {
				gc.agregarNodo(p[0], p[1]);
			}
			cout << "Grafo precargado con 8 puntos de prueba." << endl;
			break;
		}
		case 0:
			cout << "Saliendo del programa..." << endl;
			break;
			
		default:
			cout << "Opcion no valida. Intenta de nuevo." << endl;
		}
		
	} while (opcion != 0);
	
	
	
	return 0;
}
