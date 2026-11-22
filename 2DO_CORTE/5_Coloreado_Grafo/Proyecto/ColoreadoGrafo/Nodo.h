#ifndef NODO_H
#define NODO_H

#include <vector>

class Nodo {
private:
	int id;
	int color;
	std::vector<Nodo*> adyacentes;

public:
	explicit Nodo(int id);
	int getId() const;
	int getColor() const;
	void setColor(int nuevoColor);
	void agregarAdyacente(Nodo* destino);
	const std::vector<Nodo*>& getAdyacentes() const;
};

#endif
