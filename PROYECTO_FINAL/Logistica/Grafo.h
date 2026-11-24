#ifndef GRAFO_HPP
#define GRAFO_HPP

#include "Nodo.h"
#include <vector>
#include <map>
#include <set>
#include <string>

struct ResultadoDijkstra {
	std::map<int, int> distancias;
	std::map<int, int> predecesor;
	bool huboPesoNegativo;
	bool falloDePropagacion;
	double tiempoMicros;
};

struct ResultadoColoreado {
	int totalZonas;
	int totalAristasConflicto;
	std::map<int, std::set<int>> grafoConflictos;
	double tiempoMicros;
};

struct RutaZona {
	int idZona;
	int idDepot;
	int cargaTotal;
	
	// Comparativa Tarea 3: Dijkstra repetido vs Heuristica TSP simple
	std::vector<int> visitasDijkstraRep;
	std::vector<int> callesDijkstraRep;
	int distDijkstraRep;
	double tiempoDijkstraRepMicros;
	
	std::vector<int> visitasTSP;
	std::vector<int> callesTSP;
	int distTSP;
	double tiempoTSPMicros;
};

class grafo {
private:
	std::vector<nodo*> nodos;
	std::map<int, std::map<int, int>> matrizDistancias;
	std::map<int, std::map<int, int>> matrizPredecesores;
	
	// Metodos internos de coloreado (Welsh-Powell y DSATUR)
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
	
	// Metodos internos de ruteo
	int buscarDepotMasCercano(const std::vector<int>& idsZona) const;
	std::vector<int> construirTourTSP(int idDepot, const std::vector<int>& idsZona) const;
	std::vector<int> mejorar2Opt(const std::vector<int>& tourInicial) const;
	int evaluarCostoTour(const std::vector<int>& tour) const;
	std::vector<int> expandirCalles(const std::vector<int>& tour) const;
	
public:
	grafo();
	~grafo();
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
	
	// Carga de red vial (por defecto y desde archivo .txt)
	bool cargarDesdeTXT(const std::string& rutaArchivo, std::string& errorOut);
	int insertarEntregaConectada(const std::string& nombre, double x, double y, int demanda);
	
	// 1. Tu algoritmo Dijkstra + reconstruccion
	ResultadoDijkstra Dijkstra(int valorInicial) const;
	std::vector<int> reconstruirCamino(int origen, int destino, const std::map<int, int>& predecesor) const;
	double precalcularMatrizDijkstra();
	
	// 2. Coloreado de grafos (0 = Welsh-Powell, 1 = DSATUR)
	ResultadoColoreado colorearZonas(int umbralMetros, int capacidadMaxZona, int metodo, int modoConflicto);
	
	// 3. Planificacion de rutas intra-zona (Dijkstra repetido vs TSP simple + 2-Opt)
	std::vector<RutaZona> planificarRutasFlota() const;
};

#endif
