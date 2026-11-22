#include "grafo.h"
#include <limits>
#include <queue>
#include <algorithm>
#include <chrono>
#include <cmath>

grafo::grafo() {}

grafo::~grafo() {
	limpiar();
}

void grafo::limpiar() {
	for (nodo* n : nodos) {
		delete n;
	}
	nodos.clear();
	matrizDistancias.clear();
	matrizPredecesores.clear();
}

void grafo::agregarNodo(int valor, const std::string& nombre, double x, double y,
						TipoNodo tipo, int demanda) {
	if (!buscarNodo(valor)) {
		nodos.push_back(new nodo(valor, nombre, x, y, tipo, demanda));
	}
}

void grafo::agregarArista(int origen, int destino, int peso) {
	nodo* nodoOrigen = buscarNodo(origen);
	nodo* nodoDestino = buscarNodo(destino);
	if (nodoOrigen && nodoDestino) {
		nodoOrigen->agregarArista(nodoDestino, peso);
	}
}

void grafo::agregarAristaBidireccional(int origen, int destino, int peso) {
	agregarArista(origen, destino, peso);
	agregarArista(destino, origen, peso);
}

void grafo::agregarAristaAutomatica(int origen, int destino) {
	nodo* nodoOrigen = buscarNodo(origen);
	nodo* nodoDestino = buscarNodo(destino);
	if (nodoOrigen && nodoDestino) {
		int peso = static_cast<int>(std::round(nodoOrigen->distanciaEuclidiana(nodoDestino) * 1.18));
		if (peso < 1) peso = 1;
		agregarAristaBidireccional(origen, destino, peso);
	}
}

nodo* grafo::buscarNodo(int valor) const {
	for (nodo* n : nodos) {
		if (n->getDato() == valor) return n;
	}
	return nullptr;
}

void grafo::eliminarArista(int origen, int destino) {
	nodo* nodoOrigen = buscarNodo(origen);
	nodo* nodoDestino = buscarNodo(destino);
	if (nodoOrigen && nodoDestino) {
		nodoOrigen->eliminarArista(nodoDestino);
	}
}

void grafo::eliminarNodo(int valor) {
	nodo* nodoAEliminar = buscarNodo(valor);
	if (!nodoAEliminar) return;
	
	for (nodo* n : nodos) {
		n->eliminarArista(nodoAEliminar);
	}
	
	for (auto it = nodos.begin(); it != nodos.end(); ++it) {
		if (*it == nodoAEliminar) {
			nodos.erase(it);
			break;
		}
	}
	
	delete nodoAEliminar;
}

const std::vector<nodo*>& grafo::getNodos() const {
	return nodos;
}

std::vector<nodo*> grafo::obtenerNodosPorTipo(TipoNodo tipo) const {
	std::vector<nodo*> res;
	for (nodo* n : nodos) {
		if (n->getTipo() == tipo) res.push_back(n);
	}
	return res;
}

void grafo::reiniciarColores() {
	for (nodo* n : nodos) {
		n->setColorZona(-1);
	}
}

int grafo::cantidadNodos() const {
	return static_cast<int>(nodos.size());
}

int grafo::cantidadAristasUnicas() const {
	int total = 0;
	for (nodo* n : nodos) {
		total += static_cast<int>(n->getAdyacentes().size());
	}
	return total / 2;
}

void grafo::cargarRedVialOSM() {
	limpiar();
	
	// 1. Deposito Central
	agregarNodo(1, "Depot Central Logistico", 390, 230, DEPOSITO, 0);
	
	// 2. Intersecciones de la red vial OSM
	agregarNodo(2, "Glorieta Norte", 390, 95, INTERSECCION, 0);
	agregarNodo(3, "Cruce Av. Santander", 210, 120, INTERSECCION, 0);
	agregarNodo(4, "Cruce Oriental Cra 15", 570, 120, INTERSECCION, 0);
	agregarNodo(5, "Interseccion Parque Central", 265, 225, INTERSECCION, 0);
	agregarNodo(6, "Cruce Comercial Este", 520, 230, INTERSECCION, 0);
	agregarNodo(7, "Puente Occidente", 120, 240, INTERSECCION, 0);
	agregarNodo(8, "Diagonal Industrial", 665, 240, INTERSECCION, 0);
	agregarNodo(9, "Cruce Hospital Sur", 235, 335, INTERSECCION, 0);
	agregarNodo(10, "Glorieta Suroriental", 515, 340, INTERSECCION, 0);
	agregarNodo(11, "Av. Circunvalar Sur", 390, 365, INTERSECCION, 0);
	
	// 3. Puntos de Entrega (POIs con demanda en paquetes)
	agregarNodo(12, "Panaderia La Espiga", 115, 65, ENTREGA, 8);
	agregarNodo(13, "Supermercado Express", 305, 50, ENTREGA, 14);
	agregarNodo(14, "Farmacia Norte", 485, 50, ENTREGA, 7);
	agregarNodo(15, "Colegio San Patricio", 690, 75, ENTREGA, 15);
	agregarNodo(16, "Clinica Santa Fe", 65, 165, ENTREGA, 10);
	agregarNodo(17, "Torres del Parque", 175, 185, ENTREGA, 12);
	agregarNodo(18, "Plaza de Mercado", 625, 175, ENTREGA, 16);
	agregarNodo(19, "Ferreteria Industrial", 710, 325, ENTREGA, 9);
	agregarNodo(20, "Urbanizacion El Bosque", 80, 340, ENTREGA, 11);
	agregarNodo(21, "Campus Universitario", 285, 415, ENTREGA, 15);
	agregarNodo(22, "Polideportivo Municipal", 485, 418, ENTREGA, 8);
	agregarNodo(23, "Centro Comercial Sur", 640, 395, ENTREGA, 13);
	
	// 4. Calles (Aristas no dirigidas con peso en metros)
	agregarAristaAutomatica(1, 2);
	agregarAristaAutomatica(1, 5);
	agregarAristaAutomatica(1, 6);
	agregarAristaAutomatica(1, 11);
	agregarAristaAutomatica(1, 9);
	agregarAristaAutomatica(1, 10);
	
	agregarAristaAutomatica(2, 3);
	agregarAristaAutomatica(2, 4);
	agregarAristaAutomatica(2, 13);
	agregarAristaAutomatica(2, 14);
	
	agregarAristaAutomatica(3, 12);
	agregarAristaAutomatica(3, 13);
	agregarAristaAutomatica(3, 17);
	agregarAristaAutomatica(3, 5);
	
	agregarAristaAutomatica(4, 14);
	agregarAristaAutomatica(4, 15);
	agregarAristaAutomatica(4, 18);
	agregarAristaAutomatica(4, 6);
	
	agregarAristaAutomatica(5, 17);
	agregarAristaAutomatica(5, 7);
	agregarAristaAutomatica(5, 9);
	
	agregarAristaAutomatica(6, 18);
	agregarAristaAutomatica(6, 8);
	agregarAristaAutomatica(6, 10);
	
	agregarAristaAutomatica(7, 16);
	agregarAristaAutomatica(7, 17);
	agregarAristaAutomatica(7, 20);
	
	agregarAristaAutomatica(8, 15);
	agregarAristaAutomatica(8, 18);
	agregarAristaAutomatica(8, 19);
	
	agregarAristaAutomatica(9, 20);
	agregarAristaAutomatica(9, 11);
	agregarAristaAutomatica(9, 21);
	
	agregarAristaAutomatica(10, 11);
	agregarAristaAutomatica(10, 19);
	agregarAristaAutomatica(10, 22);
	agregarAristaAutomatica(10, 23);
	
	agregarAristaAutomatica(11, 21);
	agregarAristaAutomatica(11, 22);
	
	agregarAristaAutomatica(12, 16);
	agregarAristaAutomatica(13, 14);
	agregarAristaAutomatica(19, 23);
	agregarAristaAutomatica(20, 21);
	agregarAristaAutomatica(21, 22);
	agregarAristaAutomatica(22, 23);
}

int grafo::insertarEntregaConectada(const std::string& nombre, double x, double y, int demanda) {
	int nuevoId = 1;
	for (nodo* n : nodos) {
		if (n->getDato() >= nuevoId) nuevoId = n->getDato() + 1;
	}
	
	agregarNodo(nuevoId, nombre, x, y, ENTREGA, demanda);
	nodo* nuevoNodo = buscarNodo(nuevoId);
	
	std::vector<std::pair<double, int>> cercanos;
	for (nodo* n : nodos) {
		if (n->getDato() == nuevoId) continue;
		cercanos.push_back({nuevoNodo->distanciaEuclidiana(n), n->getDato()});
	}
	std::sort(cercanos.begin(), cercanos.end());
	
	int enlaces = std::min(2, static_cast<int>(cercanos.size()));
	for (int i = 0; i < enlaces; ++i) {
		agregarAristaAutomatica(nuevoId, cercanos[i].second);
	}
	return nuevoId;
}

// ============================================================================
// DIJKSTRA
// ============================================================================
ResultadoCamino grafo::Dijkstra(int valorInicial) const {
	auto tInicio = std::chrono::high_resolution_clock::now();
	
	std::map<int, int> distancias;
	std::map<int, bool> visitado;
	std::map<int, int> predecesor;
	const int INF = std::numeric_limits<int>::max();
	
	nodo* nodoInicial = buscarNodo(valorInicial);
	if (!nodoInicial) {
		return {distancias, predecesor, 0, 0.0};
	}
	
	for (nodo* nod : nodos) {
		distancias[nod->getDato()] = INF;
		visitado[nod->getDato()] = false;
	}
	distancias[valorInicial] = 0;
	
	using p = std::pair<int, nodo*>;
	std::priority_queue<p, std::vector<p>, std::greater<p>> cola;
	cola.push({0, nodoInicial});
	
	int expandidos = 0;
	
	while (!cola.empty()) {
		auto elemento = cola.top();
		int disActual = elemento.first;
		nodo* nodoActual = elemento.second;
		cola.pop();
		
		int u = nodoActual->getDato();
		if (visitado[u]) continue;
		visitado[u] = true;
		expandidos++;
		
		for (const auto& par : nodoActual->getAdyacentes()) {
			nodo* vecino = par.first;
			int peso = par.second;
			int v = vecino->getDato();
			
			if (disActual != INF && disActual + peso < distancias[v]) {
				distancias[v] = disActual + peso;
				predecesor[v] = u;
				cola.push({distancias[v], vecino});
			}
		}
	}
	
	auto tFin = std::chrono::high_resolution_clock::now();
	double micros = std::chrono::duration<double, std::micro>(tFin - tInicio).count();
	
	return {distancias, predecesor, expandidos, micros};
}

// ============================================================================
// A* (A-STAR) CON HEURISTICA EUCLIDIANA
// ============================================================================
ResultadoCamino grafo::AStar(int valorInicial, int valorDestino) const {
	auto tInicio = std::chrono::high_resolution_clock::now();
	
	std::map<int, int> gScore;
	std::map<int, bool> cerrado;
	std::map<int, int> predecesor;
	const int INF = std::numeric_limits<int>::max();
	
	nodo* nodoInicial = buscarNodo(valorInicial);
	nodo* nodoMeta = buscarNodo(valorDestino);
	if (!nodoInicial || !nodoMeta) {
		return {gScore, predecesor, 0, 0.0};
	}
	
	for (nodo* nod : nodos) {
		gScore[nod->getDato()] = INF;
		cerrado[nod->getDato()] = false;
	}
	gScore[valorInicial] = 0;
	
	using pf = std::pair<double, nodo*>;
	std::priority_queue<pf, std::vector<pf>, std::greater<pf>> openSet;
	openSet.push({nodoInicial->distanciaEuclidiana(nodoMeta), nodoInicial});
	
	int expandidos = 0;
	
	while (!openSet.empty()) {
		nodo* nodoActual = openSet.top().second;
		openSet.pop();
		
		int u = nodoActual->getDato();
		if (u == valorDestino) {
			expandidos++;
			break;
		}
		if (cerrado[u]) continue;
		cerrado[u] = true;
		expandidos++;
		
		for (const auto& par : nodoActual->getAdyacentes()) {
			nodo* vecino = par.first;
			int peso = par.second;
			int v = vecino->getDato();
			if (cerrado[v]) continue;
			
			if (gScore[u] != INF && gScore[u] + peso < gScore[v]) {
				gScore[v] = gScore[u] + peso;
				predecesor[v] = u;
				double fScore = static_cast<double>(gScore[v]) + vecino->distanciaEuclidiana(nodoMeta);
				openSet.push({fScore, vecino});
			}
		}
	}
	
	auto tFin = std::chrono::high_resolution_clock::now();
	double micros = std::chrono::duration<double, std::micro>(tFin - tInicio).count();
	
	return {gScore, predecesor, expandidos, micros};
}

std::vector<int> grafo::reconstruirCamino(int origen, int destino, const std::map<int, int>& predecesores) const {
	std::vector<int> camino;
	if (origen == destino) {
		camino.push_back(origen);
		return camino;
	}
	if (predecesores.find(destino) == predecesores.end()) {
		return camino;
	}
	
	int actual = destino;
	while (actual != origen) {
		camino.push_back(actual);
		auto it = predecesores.find(actual);
		if (it == predecesores.end()) return std::vector<int>();
		actual = it->second;
	}
	camino.push_back(origen);
	std::reverse(camino.begin(), camino.end());
	return camino;
}

void grafo::precalcularMatrizDijkstra() {
	matrizDistancias.clear();
	matrizPredecesores.clear();
	
	for (nodo* n : nodos) {
		int id = n->getDato();
		ResultadoCamino res = Dijkstra(id);
		matrizDistancias[id] = res.distancias;
		matrizPredecesores[id] = res.predecesores;
	}
}

// ============================================================================
// COLOREADO DE GRAFOS (WELSH-POWELL Y DSATUR CON BALANCEO DE CARGA)
// ============================================================================
ResultadoColoreado grafo::colorearZonas(int umbralMetros, int capacidadMaxZona, int metodo, int modoConflicto) {
	if (matrizDistancias.empty()) {
		precalcularMatrizDijkstra();
	}
	
	auto tInicio = std::chrono::high_resolution_clock::now();
	
	std::vector<nodo*> entregas = obtenerNodosPorTipo(ENTREGA);
	std::map<int, std::set<int>> conflictos;
	for (nodo* e : entregas) {
		conflictos[e->getDato()] = std::set<int>();
	}
	
	const int INF = std::numeric_limits<int>::max();
	int totalConflictos = 0;
	
	for (size_t i = 0; i < entregas.size(); ++i) {
		for (size_t j = i + 1; j < entregas.size(); ++j) {
			int u = entregas[i]->getDato();
			int v = entregas[j]->getDato();
			int distRed = INF;
			
			auto itU = matrizDistancias.find(u);
			if (itU != matrizDistancias.end()) {
				auto itV = itU->second.find(v);
				if (itV != itU->second.end()) distRed = itV->second;
			}
			
			bool hayConflicto = (modoConflicto == 0) ? (distRed > umbralMetros) : (distRed <= umbralMetros);
			if (hayConflicto) {
				conflictos[u].insert(v);
				conflictos[v].insert(u);
				totalConflictos++;
			}
		}
	}
	
	std::map<int, int> asignacion;
	if (metodo == 0) {
		asignacion = colorearWelshPowell(entregas, conflictos, capacidadMaxZona);
	} else {
		asignacion = colorearDSATUR(entregas, conflictos, capacidadMaxZona);
	}
	
	std::set<int> coloresUnicos;
	for (nodo* e : entregas) {
		int c = asignacion[e->getDato()];
		e->setColorZona(c);
		coloresUnicos.insert(c);
	}
	
	auto tFin = std::chrono::high_resolution_clock::now();
	double micros = std::chrono::duration<double, std::micro>(tFin - tInicio).count();
	
	return {
		static_cast<int>(coloresUnicos.size()),
			totalConflictos,
			conflictos,
			asignacion,
			micros
	};
}

std::map<int, int> grafo::colorearWelshPowell(
											  const std::vector<nodo*>& entregas,
											  const std::map<int, std::set<int>>& conflictos,
											  int capacidadMaxZona) const {
	
	std::vector<nodo*> ordenados = entregas;
	std::sort(ordenados.begin(), ordenados.end(), [&](nodo* a, nodo* b) {
		size_t gA = conflictos.at(a->getDato()).size();
		size_t gB = conflictos.at(b->getDato()).size();
		if (gA != gB) return gA > gB;
		return a->getDemanda() > b->getDemanda();
	});
	
	std::map<int, int> colorNodo;
	std::map<int, int> cargaPorColor;
	
	for (nodo* uNodo : ordenados) {
		int u = uNodo->getDato();
		int demandaU = uNodo->getDemanda();
		std::set<int> prohibidos;
		
		for (int vecino : conflictos.at(u)) {
			auto it = colorNodo.find(vecino);
			if (it != colorNodo.end()) prohibidos.insert(it->second);
		}
		
		int color = 0;
		while (prohibidos.find(color) != prohibidos.end() ||
			   (cargaPorColor[color] + demandaU > capacidadMaxZona && cargaPorColor[color] > 0)) {
			color++;
		}
			   
			   colorNodo[u] = color;
			   cargaPorColor[color] += demandaU;
	}
	
	return colorNodo;
}

std::map<int, int> grafo::colorearDSATUR(
										 const std::vector<nodo*>& entregas,
										 const std::map<int, std::set<int>>& conflictos,
										 int capacidadMaxZona) const {
	
	std::map<int, int> colorNodo;
	std::map<int, int> cargaPorColor;
	std::set<nodo*> pendientes(entregas.begin(), entregas.end());
	
	while (!pendientes.empty()) {
		nodo* mejorNodo = nullptr;
		int maxSat = -1;
		int maxGrado = -1;
		
		for (nodo* cand : pendientes) {
			int idC = cand->getDato();
			std::set<int> coloresVecinos;
			for (int vec : conflictos.at(idC)) {
				auto it = colorNodo.find(vec);
				if (it != colorNodo.end()) coloresVecinos.insert(it->second);
			}
			int sat = static_cast<int>(coloresVecinos.size());
			int grado = static_cast<int>(conflictos.at(idC).size());
			
			if (sat > maxSat || (sat == maxSat && grado > maxGrado)) {
				maxSat = sat;
				maxGrado = grado;
				mejorNodo = cand;
			}
		}
		
		int u = mejorNodo->getDato();
		int demandaU = mejorNodo->getDemanda();
		std::set<int> prohibidos;
		
		for (int vec : conflictos.at(u)) {
			auto it = colorNodo.find(vec);
			if (it != colorNodo.end()) prohibidos.insert(it->second);
		}
		
		int color = 0;
		while (prohibidos.find(color) != prohibidos.end() ||
			   (cargaPorColor[color] + demandaU > capacidadMaxZona && cargaPorColor[color] > 0)) {
			color++;
		}
			   
			   colorNodo[u] = color;
			   cargaPorColor[color] += demandaU;
			   pendientes.erase(mejorNodo);
	}
	
	return colorNodo;
}

// ============================================================================
// PLANIFICACION DE RUTAS POR ZONA (TSP GREEDY VECINO MAS CERCANO + 2-OPT)
// ============================================================================
std::vector<RutaZona> grafo::planificarRutasFlota(int idDeposito, bool aplicar2Opt) const {
	std::map<int, std::vector<int>> entregasPorZona;
	for (nodo* e : obtenerNodosPorTipo(ENTREGA)) {
		if (e->getColorZona() >= 0) {
			entregasPorZona[e->getColorZona()].push_back(e->getDato());
		}
	}
	
	std::vector<RutaZona> rutas;
	
	for (const auto& par : entregasPorZona) {
		int colorZona = par.first;
		const std::vector<int>& idsZona = par.second;
		
		int cargaTotal = 0;
		for (int id : idsZona) {
			nodo* n = buscarNodo(id);
			if (n) cargaTotal += n->getDemanda();
		}
		
		auto t0 = std::chrono::high_resolution_clock::now();
		std::vector<int> tourGreedy = construirTourGreedy(idDeposito, idsZona);
		int distGreedy = evaluarCostoTour(tourGreedy);
		auto t1 = std::chrono::high_resolution_clock::now();
		double microsGreedy = std::chrono::duration<double, std::micro>(t1 - t0).count();
		
		auto t2 = std::chrono::high_resolution_clock::now();
		std::vector<int> tour2Opt = tourGreedy;
		if (aplicar2Opt && tour2Opt.size() > 4) {
			tour2Opt = optimizarTour2Opt(tourGreedy);
		}
		int dist2Opt = evaluarCostoTour(tour2Opt);
		auto t3 = std::chrono::high_resolution_clock::now();
		double micros2Opt = std::chrono::duration<double, std::micro>(t3 - t2).count();
		
		const std::vector<int>& tourFinal = aplicar2Opt ? tour2Opt : tourGreedy;
		
		std::vector<int> trazadoCalles;
		for (size_t i = 0; i + 1 < tourFinal.size(); ++i) {
			int u = tourFinal[i];
			int v = tourFinal[i + 1];
			auto itPred = matrizPredecesores.find(u);
			if (itPred != matrizPredecesores.end()) {
				std::vector<int> tramo = reconstruirCamino(u, v, itPred->second);
				if (!tramo.empty()) {
					if (!trazadoCalles.empty()) tramo.erase(tramo.begin());
					trazadoCalles.insert(trazadoCalles.end(), tramo.begin(), tramo.end());
				}
			}
		}
		
		rutas.push_back({
			colorZona,
				tourFinal,
				trazadoCalles,
				distGreedy,
				dist2Opt,
				microsGreedy,
				micros2Opt,
				cargaTotal
		});
	}
	
	return rutas;
}

std::vector<int> grafo::construirTourGreedy(int idDeposito, const std::vector<int>& entregasZona) const {
	std::vector<int> tour;
	std::set<int> pendientes(entregasZona.begin(), entregasZona.end());
	int actual = idDeposito;
	tour.push_back(actual);
	
	while (!pendientes.empty()) {
		int mejorSig = -1;
		int minDist = std::numeric_limits<int>::max();
		
		auto itU = matrizDistancias.find(actual);
		for (int cand : pendientes) {
			int d = std::numeric_limits<int>::max();
			if (itU != matrizDistancias.end()) {
				auto itV = itU->second.find(cand);
				if (itV != itU->second.end()) d = itV->second;
			}
			if (d < minDist) {
				minDist = d;
				mejorSig = cand;
			}
		}
		
		if (mejorSig == -1) break;
		tour.push_back(mejorSig);
		pendientes.erase(mejorSig);
		actual = mejorSig;
	}
	
	tour.push_back(idDeposito);
	return tour;
}

std::vector<int> grafo::optimizarTour2Opt(const std::vector<int>& tourInicial) const {
	std::vector<int> mejorTour = tourInicial;
	bool mejoro = true;
	int n = static_cast<int>(mejorTour.size());
	
	while (mejoro) {
		mejoro = false;
		int mejorCosto = evaluarCostoTour(mejorTour);
		
		for (int i = 1; i < n - 2; ++i) {
			for (int k = i + 1; k < n - 1; ++k) {
				std::vector<int> candidato = mejorTour;
				std::reverse(candidato.begin() + i, candidato.begin() + k + 1);
				int nuevoCosto = evaluarCostoTour(candidato);
				if (nuevoCosto < mejorCosto) {
					mejorTour = candidato;
					mejorCosto = nuevoCosto;
					mejoro = true;
				}
			}
		}
	}
	return mejorTour;
}

int grafo::evaluarCostoTour(const std::vector<int>& tour) const {
	int total = 0;
	const int INF = std::numeric_limits<int>::max();
	
	for (size_t i = 0; i + 1 < tour.size(); ++i) {
		int u = tour[i];
		int v = tour[i + 1];
		auto itU = matrizDistancias.find(u);
		if (itU != matrizDistancias.end()) {
			auto itV = itU->second.find(v);
			if (itV != itU->second.end() && itV->second != INF) {
				total += itV->second;
			}
		}
	}
	return total;
}
