#ifndef GRAFO_CONVEXO_H
#define GRAFO_CONVEXO_H

#include "Punto.h"
#include <vector>

class GrafoConvexo {
private:
	std::vector<Punto*> nodos;
	std::vector<Punto*> hull;
	
	void limpiarAristasHull();
	void construirAristasEnvolvente();
	

	std::vector<Punto*> dividirYConquistarRecursivo(std::vector<Punto*>& pts, int inicio, int fin);
	std::vector<Punto*> combinarHulls(const std::vector<Punto*>& izq, const std::vector<Punto*>& der);
	
public:
	GrafoConvexo();
	~GrafoConvexo();
	
	void agregarNodo(int x, int y);
	Punto* buscarNodo(int x, int y) const;
	void limpiarGrafo();
	

	std::vector<Punto*> calcularConvexHull();           // Monotone Chain 
	std::vector<Punto*> calcularGrahamScan();           // Graham Scan
	std::vector<Punto*> calcularJarvisMarch();          // Jarvis March 
	std::vector<Punto*> calcularDivideYVenceras();     // Divide y Vencerás
	
	void mostrarGrafo() const;
	void mostrarConvexHull() const;
};

#endif
