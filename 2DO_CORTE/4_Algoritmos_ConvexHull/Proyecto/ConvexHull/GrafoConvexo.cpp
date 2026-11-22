#include "GrafoConvexo.h"
#include <iostream>
#include <algorithm>
#include <cmath>

GrafoConvexo::GrafoConvexo() {}

GrafoConvexo::~GrafoConvexo() {
	limpiarGrafo();
}

void GrafoConvexo::limpiarGrafo() {
	for (Punto* p : nodos) {
		delete p;
	}
	nodos.clear();
	hull.clear();
}

void GrafoConvexo::limpiarAristasHull() {
	for (Punto* p : nodos) {
		p->limpiarAristas();
	}
	hull.clear();
}

Punto* GrafoConvexo::buscarNodo(int x, int y) const {
	for (Punto* p : nodos) {
		if (p->getX() == x && p->getY() == y) {
			return p;
		}
	}
	return nullptr;
}

void GrafoConvexo::agregarNodo(int x, int y) {
	if (!buscarNodo(x, y)) {
		nodos.push_back(new Punto(x, y));
	}
}

void GrafoConvexo::construirAristasEnvolvente() {
	if (hull.size() >= 2) {
		for (size_t i = 0; i < hull.size(); ++i) {
			Punto* actual = hull[i];
			Punto* siguiente = hull[(i + 1) % hull.size()];
			actual->agregarArista(siguiente);
		}
	}
}

// 1. MONOTONE CHAIN 
std::vector<Punto*> GrafoConvexo::calcularConvexHull() {
	limpiarAristasHull();
	
	size_t n = nodos.size();
	if (n == 0) return hull;
	if (n == 1) {
		hull.push_back(nodos[0]);
		return hull;
	}
	
	std::vector<Punto*> pts = nodos;
	std::sort(pts.begin(), pts.end(), [](const Punto* a, const Punto* b) {
		return *a < *b;
	});
	
	std::vector<Punto*> inferior, superior;
	
	for (size_t i = 0; i < n; ++i) {
		while (inferior.size() >= 2 &&
			   Punto::productoCruz(inferior[inferior.size() - 2], inferior.back(), pts[i]) <= 0) {
			inferior.pop_back();
		}
		inferior.push_back(pts[i]);
	}
	
	for (int i = static_cast<int>(n) - 1; i >= 0; --i) {
		while (superior.size() >= 2 &&
			   Punto::productoCruz(superior[superior.size() - 2], superior.back(), pts[i]) <= 0) {
			superior.pop_back();
		}
		superior.push_back(pts[i]);
	}
	
	inferior.pop_back();
	superior.pop_back();
	
	hull = inferior;
	hull.insert(hull.end(), superior.begin(), superior.end());
	
	construirAristasEnvolvente();
	return hull;
}

// 2. GRAHAM SCAN
std::vector<Punto*> GrafoConvexo::calcularGrahamScan() {
	limpiarAristasHull();
	size_t n = nodos.size();
	if (n <= 1) return calcularConvexHull();
	
	std::vector<Punto*> pts = nodos;
	int menor = 0;
	for (size_t i = 1; i < n; ++i) {
		if (pts[i]->getY() < pts[menor]->getY() || 
			(pts[i]->getY() == pts[menor]->getY() && pts[i]->getX() < pts[menor]->getX())) {
			menor = i;
		}
	}
	std::swap(pts[0], pts[menor]);
	Punto* p0 = pts[0];
	
	std::sort(pts.begin() + 1, pts.end(), [p0](const Punto* a, const Punto* b) {
		long long cruz = Punto::productoCruz(p0, a, b);
		if (cruz == 0) {
			long long distA = static_cast<long long>(a->getX() - p0->getX()) * (a->getX() - p0->getX()) +
				static_cast<long long>(a->getY() - p0->getY()) * (a->getY() - p0->getY());
			long long distB = static_cast<long long>(b->getX() - p0->getX()) * (b->getX() - p0->getX()) +
				static_cast<long long>(b->getY() - p0->getY()) * (b->getY() - p0->getY());
			return distA < distB;
		}
		return cruz > 0;
	});
	
	std::vector<Punto*> pila;
	for (size_t i = 0; i < n; ++i) {
		while (pila.size() >= 2 && 
			   Punto::productoCruz(pila[pila.size() - 2], pila.back(), pts[i]) <= 0) {
			pila.pop_back();
		}
		pila.push_back(pts[i]);
	}
	
	hull = pila;
	construirAristasEnvolvente();
	return hull;
}

// 3. JARVIS MARCH
std::vector<Punto*> GrafoConvexo::calcularJarvisMarch() {
	limpiarAristasHull();
	int n = nodos.size();
	if (n <= 1) return calcularConvexHull();
	
	int izquierdo = 0;
	for (int i = 1; i < n; i++) {
		if (nodos[i]->getX() < nodos[izquierdo]->getX()) {
			izquierdo = i;
		}
	}
	
	std::vector<Punto*> resultado;
	int p = izquierdo, q;
	do {
		resultado.push_back(nodos[p]);
		q = (p + 1) % n;
		for (int i = 0; i < n; i++) {
			if (Punto::productoCruz(nodos[p], nodos[i], nodos[q]) > 0) {
				q = i;
			}
		}
		p = q;
	} while (p != izquierdo);
	
	hull = resultado;
	construirAristasEnvolvente();
	return hull;
}

// 4. DIVIDE Y VENCERÁS
std::vector<Punto*> GrafoConvexo::calcularDivideYVenceras() {
	limpiarAristasHull();
	size_t n = nodos.size();
	if (n <= 1) return calcularConvexHull();
	
	std::vector<Punto*> pts = nodos;
	std::sort(pts.begin(), pts.end(), [](const Punto* a, const Punto* b) {
		return *a < *b;
	});
	
	hull = dividirYConquistarRecursivo(pts, 0, pts.size() - 1);
	construirAristasEnvolvente();
	return hull;
}

std::vector<Punto*> GrafoConvexo::dividirYConquistarRecursivo(std::vector<Punto*>& pts, int inicio, int fin) {
	if (fin - inicio + 1 <= 5) {
		std::vector<Punto*> sub(pts.begin() + inicio, pts.begin() + fin + 1);
		// Caso base usando Monotone Chain local
		std::sort(sub.begin(), sub.end(), [](const Punto* a, const Punto* b) { return *a < *b; });
		std::vector<Punto*> inf, sup;
		for (size_t i = 0; i < sub.size(); ++i) {
			while (inf.size() >= 2 && Punto::productoCruz(inf[inf.size() - 2], inf.back(), sub[i]) <= 0) inf.pop_back();
			inf.push_back(sub[i]);
		}
		for (int i = static_cast<int>(sub.size()) - 1; i >= 0; --i) {
			while (sup.size() >= 2 && Punto::productoCruz(sup[sup.size() - 2], sup.back(), sub[i]) <= 0) sup.pop_back();
			sup.push_back(sub[i]);
		}
		inf.pop_back();
		sup.pop_back();
		inf.insert(inf.end(), sup.begin(), sup.end());
		return inf;
	}
	
	int medio = inicio + (fin - inicio) / 2;
	std::vector<Punto*> izq = dividirYConquistarRecursivo(pts, inicio, medio);
	std::vector<Punto*> der = dividirYConquistarRecursivo(pts, medio + 1, fin);
	
	return combinarHulls(izq, der);
}

std::vector<Punto*> GrafoConvexo::combinarHulls(const std::vector<Punto*>& izq, const std::vector<Punto*>& der) {
	int nIzquierdo = izq.size();
	int nDerecho = der.size();
	
	int indiceMaxXIzq = 0;
	for (int i = 1; i < nIzquierdo; ++i) {
		if (izq[i]->getX() > izq[indiceMaxXIzq]->getX()) indiceMaxXIzq = i;
	}
	
	int indiceMinXDer = 0;
	for (int i = 1; i < nDerecho; ++i) {
		if (der[i]->getX() < der[indiceMinXDer]->getX()) indiceMinXDer = i;
	}
	
	// Tangente superior
	int iterIzq = indiceMaxXIzq;
	int iterDer = indiceMinXDer;
	bool arribaEncontrado = false;
	while (!arribaEncontrado) {
		arribaEncontrado = true;
		while (Punto::productoCruz(der[iterDer], izq[iterIzq], izq[(iterIzq - 1 + nIzquierdo) % nIzquierdo]) > 0) {
			iterIzq = (iterIzq - 1 + nIzquierdo) % nIzquierdo;
		}
		while (Punto::productoCruz(izq[iterIzq], der[iterDer], der[(iterDer + 1) % nDerecho]) < 0) {
			iterDer = (iterDer + 1) % nDerecho;
			arribaEncontrado = false;
		}
	}
	int supIzq = iterIzq;
	int supDer = iterDer;
	
	// Tangente inferior
	iterIzq = indiceMaxXIzq;
	iterDer = indiceMinXDer;
	bool abajoEncontrado = false;
	while (!abajoEncontrado) {
		abajoEncontrado = true;
		while (Punto::productoCruz(izq[iterIzq], der[iterDer], der[(iterDer - 1 + nDerecho) % nDerecho]) > 0) {
			iterDer = (iterDer - 1 + nDerecho) % nDerecho;
		}
		while (Punto::productoCruz(der[iterDer], izq[iterIzq], izq[(iterIzq + 1) % nIzquierdo]) < 0) {
			iterIzq = (iterIzq + 1) % nIzquierdo;
			abajoEncontrado = false;
		}
	}
	int infIzq = iterIzq;
	int infDer = iterDer;
	
	std::vector<Punto*> resultado;
	int actual = supIzq;
	resultado.push_back(izq[actual]);
	while (actual != infIzq) {
		actual = (actual + 1) % nIzquierdo;
		resultado.push_back(izq[actual]);
	}
	
	actual = infDer;
	resultado.push_back(der[actual]);
	while (actual != supDer) {
		actual = (actual + 1) % nDerecho;
		resultado.push_back(der[actual]);
	}
	
	return resultado;
}

void GrafoConvexo::mostrarGrafo() const {
	std::cout << "\nEstructura del Grafo (" << nodos.size() << " puntos)" << std::endl;
	for (const Punto* p : nodos) {
		std::cout << "Nodo (" << p->getX() << ", " << p->getY() << ")";
		const auto& ady = p->getAdyacentes();
		if (!ady.empty()) {
			std::cout << " -> conecta en hull con: ";
			for (const auto& arista : ady) {
				std::cout << "(" << arista.destino->getX() << ", " << arista.destino->getY() << ") ";
			}
		}
		std::cout << std::endl;
	}
}

void GrafoConvexo::mostrarConvexHull() const {
	if (hull.empty()) {
		std::cout << "\nEl Convex Hull aun no ha sido calculado o el grafo esta vacio.\n";
		return;
	}
	
	std::cout << "\nVertices del Convex Hull:\n";
	for (size_t i = 0; i < hull.size(); ++i) {
		std::cout << "(" << hull[i]->getX() << ", " << hull[i]->getY() << ")";
		if (i + 1 < hull.size()) std::cout << " -> ";
	}
	if (hull.size() >= 2) {
		std::cout << " -> (" << hull[0]->getX() << ", " << hull[0]->getY() << ")";
	}
	std::cout << "\nTotal de vertices en la frontera: " << hull.size() << std::endl;
}
