#ifndef PUNTO_H
#define PUNTO_H

#include <vector>

class Punto;

struct AristaConvexa {
	Punto* destino;
};

class Punto {
private:
	int x;
	int y;
	std::vector<AristaConvexa> adyacentes;
	
public:
	Punto(int x, int y);
	
	int getX() const;
	int getY() const;
	
	void agregarArista(Punto* destino);
	void limpiarAristas();
	const std::vector<AristaConvexa>& getAdyacentes() const;
	
	
	bool operator<(const Punto& otro) const;
	bool operator==(const Punto& otro) const;
	
	// > 0: giro antihorario (izquierda)
	// < 0: giro horario (derecha)
	// = 0: puntos colinealesx
	static long long productoCruz(const Punto* O, const Punto* A, const Punto* B);
};

#endif
