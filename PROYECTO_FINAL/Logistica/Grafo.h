#ifndef GRAFO_HPP
#define GRAFO_HPP

#include "Nodo.h"
#include <vector>
#include <map>
#include <set>
#include <string>

struct ResultadoCamino {
	std::map<int, int> distancias;
	std::map<int, int> predecesores;
	int nodosExpandidos;
	double tiempoMicros;
};

struct ResultadoColoreado {
	int totalZonas;
	int totalAristasConflicto;
	std::map<int, std::set<int>> grafoConflictos;
	std::map<int, int> coloresAsignados;
	double tiempoMicros;
};

struct RutaZona {
	int idZona;
	std::vector<int> ordenVisitasTSP;
	std::vector<int> trazadoCalles;
	int distGreedy;
	int dist2Opt;
	double tiempoGreedyMicros;
	double tiempo2OptMicros;
	int cargaTotal;
};

class grafo {
private:
	std::vector<nodo*> nodos;
	std::map<int, std::map<int, int>> matrizDistancias;
	std::map<int, std::map<int, int>> matrizPredecesores;
	
	std::map<int, int> colorearWelshPowell(
										   const std::vector<nodo*>& entregas,
										   const std::map<int, std::set<int>>& conflictos,
										   int capacidadMaxZona
										   ) const;
	
	std::map<int, int> colorearDSATUR(
									  const std::vector<nodo*>& entregas,
									  const std::map<int, std::set<int>>& conflictos,
									  int capacidadMaxZona
									  ) const;
	
	std::vector<int> construirTourGreedy(int idDeposito, const std::vector<int>& entregasZona) const;
	std::vector<int> optimizarTour2Opt(const std::vector<int>& tourInicial) const;
	int evaluarCostoTour(const std::vector<int>& tour) const;
	
public:
	grafo();
	~grafo();
	
	// Evita copias accidentales de punteros (previene doble liberacion de memoria)
	grafo(const grafo&) = delete;
	grafo& operator=(const grafo&) = delete;
	
	void limpiar();
	void agregarNodo(int valor, const std::string& nombre = "", double x = 0.0, double y = 0.0,
					 TipoNodo tipo = INTERSECCION, int demanda = 0);
	void agregarArista(int origen, int destino, int peso);
	void agregarAristaBidireccional(int origen, int destino, int peso);
	void agregarAristaAutomatica(int origen, int destino);
	void eliminarNodo(int valor);
	void eliminarArista(int origen, int destino);
	
	nodo* buscarNodo(int valor) const;
	const std::vector<nodo*>& getNodos() const;
	std::vector<nodo*> obtenerNodosPorTipo(TipoNodo tipo) const;
	void reiniciarColores();
	int cantidadNodos() const;
	int cantidadAristasUnicas() const;
	
	// Dataset OSM y edicion dinamica
	void cargarRedVialOSM();
	int insertarEntregaConectada(const std::string& nombre, double x, double y, int demanda);
	
	// Algoritmos de caminos cortos
	ResultadoCamino Dijkstra(int valorInicial) const;
	ResultadoCamino AStar(int valorInicial, int valorDestino) const;
	std::vector<int> reconstruirCamino(int origen, int destino, const std::map<int, int>& predecesores) const;
	void precalcularMatrizDijkstra();
	
	// Algoritmo de coloreado de grafos (0 = Welsh-Powell, 1 = DSATUR)
	ResultadoColoreado colorearZonas(int umbralMetros, int capacidadMaxZona, int metodo, int modoConflicto);
	
	// Planificacion de rutas intra-zona (TSP Vecino Mas Cercano + 2-Opt)
	std::vector<RutaZona> planificarRutasFlota(int idDeposito, bool aplicar2Opt) const;
};

#endif
