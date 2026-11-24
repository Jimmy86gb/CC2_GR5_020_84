#ifndef NODO_HPP
#define NODO_HPP

#include <vector>
#include <utility>
#include <string>

enum TipoNodo {
	DEPOSITO,
	INTERSECCION,
	ENTREGA
};

class nodo {
private:
	int dato;
	std::string nombre;
	double x;
	double y;
	TipoNodo tipo;
	int demanda;
	int colorZona;
	std::vector<std::pair<nodo*, int>> adyacentes;
	
public:
	nodo(int d, const std::string& nom = "", double px = 0.0, double py = 0.0,
		 TipoNodo t = INTERSECCION, int dem = 0);
	
	int getDato() const;
	std::string getNombre() const;
	double getX() const;
	double getY() const;
	TipoNodo getTipo() const;
	int getDemanda() const;
	int getColorZona() const;
	void setColorZona(int color);
	
	double distanciaEuclidiana(const nodo* otro) const;
	
	void agregarArista(nodo* destino, int peso);
	void eliminarArista(nodo* destino);
	const std::vector<std::pair<nodo*, int>>& getAdyacentes() const;
};

#endif
