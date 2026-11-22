#include <iostream>
#include <sstream>
#include <string>
#include "GrafoColoreado.h"

bool leerEnteroEnRango(const std::string& mensaje, int minimo, int maximo, int& valor) {
	std::string linea;
	while (true) {
		std::cout << mensaje;
		if (!std::getline(std::cin, linea)) {
			return false;
		}
		std::istringstream entrada(linea);
		char sobrante;
		if ((entrada >> valor) && !(entrada >> sobrante) &&
			valor >= minimo && valor <= maximo) {
			return true;
		}
		std::cout << "Ingrese un numero entero entre " << minimo
		          << " y " << maximo << ".\n";
	}
}

void cargarEjemplo(GrafoColoreado& gc) {
	for (int id = 1; id <= 6; ++id) {
		gc.agregarNodo(id);
	}
	gc.agregarArista(1, 2);
	gc.agregarArista(1, 3);
	gc.agregarArista(2, 3);
	gc.agregarArista(3, 4);
	gc.agregarArista(4, 5);
	gc.agregarArista(4, 6);
	gc.agregarArista(5, 6);
}

bool cargarGrafoUsuario(GrafoColoreado& gc) {
	int cantidadNodos;
	if (!leerEnteroEnRango("Cantidad de nodos (1 a 100): ", 1, 100, cantidadNodos)) {
		return false;
	}
	for (int id = 1; id <= cantidadNodos; ++id) {
		gc.agregarNodo(id);
	}

	std::cout << "Los nodos se llaman 1, 2, ..., " << cantidadNodos << ".\n";
	int maximoAristas = cantidadNodos * (cantidadNodos - 1) / 2;
	int cantidadAristas;
	if (!leerEnteroEnRango("Cantidad de aristas: ", 0, maximoAristas,
	                       cantidadAristas)) {
		return false;
	}

	for (int i = 1; i <= cantidadAristas;) {
		std::cout << "\nArista " << i << " de " << cantidadAristas << ":\n";
		int origen, destino;
		if (!leerEnteroEnRango("  Nodo de origen: ", 1, cantidadNodos, origen) ||
			!leerEnteroEnRango("  Nodo de destino: ", 1, cantidadNodos, destino)) {
			return false;
		}
		if (!gc.agregarArista(origen, destino)) {
			std::cout << "Arista invalida: los nodos deben ser distintos y "
			          << "la arista no debe estar repetida. Intente de nuevo.\n";
			continue;
		}
		++i;
	}
	return true;
}

void ejecutarMetodos(GrafoColoreado& gc) {
    while (true) {
        std::cout << "\n=== METODOS DE COLOREADO ===\n"
                  << "1. Voraz (orden de insercion)\n"
                  << "2. Welsh-Powell (grado descendente)\n"
                  << "3. DSATUR (mayor saturacion)\n"
                  << "4. RLF (clases independientes)\n"
                  << "5. Exacto (backtracking, hasta 16 nodos)\n"
                  << "6. Comparar todos los metodos disponibles\n"
                  << "0. Volver al menu principal\n";
        int metodo;
        if (!leerEnteroEnRango("Metodo: ", 0, 6, metodo) || metodo == 0) return;
        if ((metodo == 5 || metodo == 6) && gc.getCantidadNodos() > 16 && metodo == 5) {
            std::cout << "El metodo exacto esta limitado a 16 nodos por su costo exponencial.\n";
            continue;
        }
        if (metodo == 6) {
            std::cout << "\nComparacion de cantidad de colores:\n";
            std::cout << "Voraz: " << gc.coloreadoVoraz() << '\n';
            std::cout << "Welsh-Powell: " << gc.coloreadoWelshPowell() << '\n';
            std::cout << "DSATUR: " << gc.coloreadoDSATUR() << '\n';
            std::cout << "RLF: " << gc.coloreadoRLF() << '\n';
            if (gc.getCantidadNodos() <= 16)
                std::cout << "Exacto (numero cromatico): " << gc.coloreadoExacto() << '\n';
            else
                std::cout << "Exacto: omitido (mas de 16 nodos)\n";
            std::cout << "Se muestra la asignacion del ultimo metodo ejecutado.\n";
        } else {
            switch (metodo) {
                case 1: gc.coloreadoVoraz(); break;
                case 2: gc.coloreadoWelshPowell(); break;
                case 3: gc.coloreadoDSATUR(); break;
                case 4: gc.coloreadoRLF(); break;
                case 5: gc.coloreadoExacto(); break;
            }
        }
        gc.mostrarGrafo();
        gc.mostrarColoreado();
    }
}

int main() {
	while (true) {
		std::cout << "\n=== COLOREADO DE GRAFOS ===\n"
		          << "1. Usar el grafo de ejemplo\n"
		          << "2. Crear mi propio grafo\n"
		          << "0. Salir\n";
		int opcion;
		if (!leerEnteroEnRango("Opcion: ", 0, 2, opcion) || opcion == 0) {
			break;
		}

		GrafoColoreado gc;
		if (opcion == 1) {
			cargarEjemplo(gc);
		} else if (!cargarGrafoUsuario(gc)) {
			break;
		}

		ejecutarMetodos(gc);
	}
	return 0;
}
