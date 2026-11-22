#include "Grafo.h"
#include <fstream>
#include <sstream>
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

const std::vector<nodo*>& grafo::getNodos() const { return nodos; }

std::vector<nodo*> grafo::obtenerNodosPorTipo(TipoNodo tipo) const {
	std::vector<nodo*> res;
	for (nodo* n : nodos) {
		if (n->getTipo() == tipo) res.push_back(n);
	}
	return res;
}

void grafo::reiniciarColores() {
	for (nodo* n : nodos) n->setColorZona(-1);
}

int grafo::cantidadNodos() const { return static_cast<int>(nodos.size()); }

int grafo::cantidadAristasUnicas() const {
	int total = 0;
	for (nodo* n : nodos) total += static_cast<int>(n->getAdyacentes().size());
	return total / 2;
}

// ============================================================================
// CARGA DE RED VIAL (BASE OSM Y DESDE ARCHIVO .TXT)
// ============================================================================
void grafo::cargarRedVialOSM() {
	limpiar();
	
	// 1. Deposito Central
	agregarNodo(1, "Depot Central", 390, 230, DEPOSITO, 0);
	
	// 2. Intersecciones viales OSM
	agregarNodo(2, "Glorieta Norte", 390, 95, INTERSECCION, 0);
	agregarNodo(3, "Cruce Av. Santander", 210, 120, INTERSECCION, 0);
	agregarNodo(4, "Cruce Cra 15 Oriente", 570, 120, INTERSECCION, 0);
	agregarNodo(5, "Parque Central", 265, 225, INTERSECCION, 0);
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
	agregarNodo(22, "Polideportivo", 485, 418, ENTREGA, 8);
	agregarNodo(23, "Centro Comercial Sur", 640, 395, ENTREGA, 13);
	
	// 4. Calles (Aristas bidireccionales con peso en metros)
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

// Formato de archivo .txt soportado:
//   # Lineas de comentario
//   NODO <id> <DEPOSITO|INTERSECCION|ENTREGA> <x> <y> <demanda> <Nombre del lugar>
//   ARISTA <origen> <destino> <peso_metros>   (si peso <= 0 se calcula automatico)
bool grafo::cargarDesdeTXT(const std::string& rutaArchivo, std::string& errorOut) {
	std::ifstream archivo(rutaArchivo);
	if (!archivo.is_open()) {
		errorOut = "No se pudo abrir el archivo seleccionado.";
		return false;
	}
	
	struct DatoNodoTXT {
		int id;
		TipoNodo tipo;
		double x, y;
		int demanda;
		std::string nombre;
	};
	struct DatoAristaTXT {
		int u, v, peso;
	};
	
	std::vector<DatoNodoTXT> nodosLeidos;
	std::vector<DatoAristaTXT> aristasLeidas;
	int cantDepots = 0;
	int cantEntregas = 0;
	
	std::string linea;
	int numLinea = 0;
	while (std::getline(archivo, linea)) {
		numLinea++;
		if (!linea.empty() && linea.back() == '\r') linea.pop_back();
		
		std::istringstream iss(linea);
		std::string etiqueta;
		if (!(iss >> etiqueta) || etiqueta[0] == '#') continue;
		
		if (etiqueta == "NODO") {
			int id, demanda;
			double x, y;
			std::string tipoStr;
			if (!(iss >> id >> tipoStr >> x >> y >> demanda)) {
				errorOut = "Formato invalido en NODO (linea " + std::to_string(numLinea) + ").";
				return false;
			}
			std::string nombre;
			std::getline(iss >> std::ws, nombre);
			if (nombre.empty()) nombre = "Nodo " + std::to_string(id);
			
			TipoNodo tipo = INTERSECCION;
			if (tipoStr == "DEPOSITO" || tipoStr == "DEPOT") {
				tipo = DEPOSITO;
				cantDepots++;
			} else if (tipoStr == "ENTREGA" || tipoStr == "POI") {
				tipo = ENTREGA;
				cantEntregas++;
			}
			
			nodosLeidos.push_back({id, tipo, x, y, demanda, nombre});
		} else if (etiqueta == "ARISTA") {
			int u, v, peso = 0;
			if (!(iss >> u >> v)) {
				errorOut = "Formato invalido en ARISTA (linea " + std::to_string(numLinea) + ").";
				return false;
			}
			iss >> peso; // Opcional: si no viene o es <= 0, se calcula por distancia
			aristasLeidas.push_back({u, v, peso});
		}
	}
	
	if (cantDepots == 0 || cantEntregas == 0) {
		errorOut = "El archivo .txt debe contener al menos 1 DEPOSITO y 1 ENTREGA.";
		return false;
	}
	
	limpiar();
	for (const auto& dn : nodosLeidos) {
		agregarNodo(dn.id, dn.nombre, dn.x, dn.y, dn.tipo, dn.demanda);
	}
	for (const auto& da : aristasLeidas) {
		if (da.peso > 0) {
			agregarAristaBidireccional(da.u, da.v, da.peso);
		} else {
			agregarAristaAutomatica(da.u, da.v);
		}
	}
	
	return true;
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
// 1. DIJKSTRA (tu implementacion original adaptada para devolver resultados)
// ============================================================================
ResultadoDijkstra grafo::Dijkstra(int valorInicial) const {
	auto tInicio = std::chrono::high_resolution_clock::now();
	
	ResultadoDijkstra res;
	res.huboPesoNegativo = false;
	res.falloDePropagacion = false;
	res.tiempoMicros = 0.0;
	
	nodo* nodoInicial = buscarNodo(valorInicial);
	if (!nodoInicial) return res;
	
	std::map<int, bool> visitado;
	for (auto nod : nodos) {
		res.distancias[nod->getDato()] = std::numeric_limits<int>::max();
		visitado[nod->getDato()] = false;
	}
	res.distancias[valorInicial] = 0;
	
	using p = std::pair<int, nodo*>;
	std::priority_queue<p, std::vector<p>, std::greater<p>> cola;
	cola.push({0, nodoInicial});
	
	while (!cola.empty()) {
		auto elemento = cola.top();
		int disActual = elemento.first;
		nodo* nodoActual = elemento.second;
		cola.pop();
		
		int u = nodoActual->getDato();
		if (visitado[u]) continue;
		visitado[u] = true;
		
		for (auto par : nodoActual->getAdyacentes()) {
			nodo* vecino = par.first;
			int peso = par.second;
			
			if (peso < 0) res.huboPesoNegativo = true;
			
			int v = vecino->getDato();
			int nuevaDistancia = disActual + peso;
			
			if (nuevaDistancia < res.distancias[v]) {
				if (visitado[v]) res.falloDePropagacion = true;
				res.distancias[v] = nuevaDistancia;
				res.predecesor[v] = u;
				cola.push({nuevaDistancia, vecino});
			}
		}
	}
	
	auto tFin = std::chrono::high_resolution_clock::now();
	res.tiempoMicros = std::chrono::duration<double, std::micro>(tFin - tInicio).count();
	return res;
}

std::vector<int> grafo::reconstruirCamino(int origen, int destino, const std::map<int, int>& predecesor) const {
	std::vector<int> ruta;
	if (origen == destino) {
		ruta.push_back(origen);
		return ruta;
	}
	if (predecesor.find(destino) == predecesor.end()) return ruta;
	
	int actual = destino;
	int pasos = 0;
	while (actual != origen && predecesor.count(actual) && pasos <= static_cast<int>(nodos.size())) {
		ruta.push_back(actual);
		actual = predecesor.at(actual);
		pasos++;
	}
	if (actual != origen) return std::vector<int>();
	ruta.push_back(origen);
	std::reverse(ruta.begin(), ruta.end());
	return ruta;
}

double grafo::precalcularMatrizDijkstra() {
	auto t0 = std::chrono::high_resolution_clock::now();
	matrizDistancias.clear();
	matrizPredecesores.clear();
	
	for (nodo* n : nodos) {
		int id = n->getDato();
		ResultadoDijkstra res = Dijkstra(id);
		matrizDistancias[id] = res.distancias;
		matrizPredecesores[id] = res.predecesor;
	}
	
	auto t1 = std::chrono::high_resolution_clock::now();
	return std::chrono::duration<double, std::micro>(t1 - t0).count();
}

// ============================================================================
// 2. COLOREADO DE GRAFOS (WELSH-POWELL Y DSATUR CON BALANCEO DE CARGA)
// ============================================================================
ResultadoColoreado grafo::colorearZonas(int umbralMetros, int capacidadMaxZona, int metodo, int modoConflicto) {
	if (matrizDistancias.empty()) precalcularMatrizDijkstra();
	
	auto tInicio = std::chrono::high_resolution_clock::now();
	
	std::vector<nodo*> entregas = obtenerNodosPorTipo(ENTREGA);
	std::map<int, std::set<int>> conflictos;
	for (nodo* e : entregas) conflictos[e->getDato()] = std::set<int>();
	
	const int INF = std::numeric_limits<int>::max();
	int totalConflictos = 0;
	
	for (size_t i = 0; i < entregas.size(); ++i) {
		for (size_t j = i + 1; j < entregas.size(); ++j) {
			int u = entregas[i]->getDato();
			int v = entregas[j]->getDato();
			int distRed = INF;
			
			auto itU = matrizDistancias.find(u);
			if (itU != matrizDistancias.end() && itU->second.count(v)) {
				distRed = itU->second.at(v);
			}
			
			// modoConflicto == 0: Lejania (> umbral => zonas compactas)
			// modoConflicto == 1: Vecinos cercanos (<= umbral => vecinos no comparten color)
			bool hayConflicto = (modoConflicto == 0) ? (distRed > umbralMetros) : (distRed <= umbralMetros);
			if (hayConflicto) {
				conflictos[u].insert(v);
				conflictos[v].insert(u);
				totalConflictos++;
			}
		}
	}
	
	std::map<int, int> asignacion = (metodo == 0)
		? colorearWelshPowell(entregas, conflictos, capacidadMaxZona)
		: colorearDSATUR(entregas, conflictos, capacidadMaxZona);
	
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
			if (colorNodo.count(vecino)) prohibidos.insert(colorNodo[vecino]);
		}
		
		int color = 0;
		while (prohibidos.count(color) ||
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
				if (colorNodo.count(vec)) coloresVecinos.insert(colorNodo[vec]);
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
			if (colorNodo.count(vec)) prohibidos.insert(colorNodo[vec]);
		}
		
		int color = 0;
		while (prohibidos.count(color) ||
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
// 3. RUTEO INTRA-ZONA: DIJKSTRA REPETIDO vs HEURISTICA TSP SIMPLE (2-OPT)
// ============================================================================
int grafo::buscarDepotMasCercano(const std::vector<int>& idsZona) const {
	std::vector<nodo*> depots = obtenerNodosPorTipo(DEPOSITO);
	if (depots.empty()) return -1;
	
	int mejorDepot = depots.front()->getDato();
	long long menorSuma = std::numeric_limits<long long>::max();
	const int INF = std::numeric_limits<int>::max();
	
	for (nodo* d : depots) {
		int idD = d->getDato();
		long long suma = 0;
		auto itD = matrizDistancias.find(idD);
		for (int idE : idsZona) {
			if (itD != matrizDistancias.end() && itD->second.count(idE) && itD->second.at(idE) != INF) {
				suma += itD->second.at(idE);
			}
		}
		if (suma < menorSuma) {
			menorSuma = suma;
			mejorDepot = idD;
		}
	}
	return mejorDepot;
}

std::vector<RutaZona> grafo::planificarRutasFlota() const {
	std::map<int, std::vector<int>> entregasPorZona;
	for (nodo* e : obtenerNodosPorTipo(ENTREGA)) {
		if (e->getColorZona() >= 0) {
			entregasPorZona[e->getColorZona()].push_back(e->getDato());
		}
	}
	
	std::vector<RutaZona> rutas;
	const int INF = std::numeric_limits<int>::max();
	
	for (const auto& par : entregasPorZona) {
		int idZona = par.first;
		const std::vector<int>& idsZona = par.second;
		
		int cargaTotal = 0;
		for (int id : idsZona) {
			nodo* n = buscarNodo(id);
			if (n) cargaTotal += n->getDemanda();
		}
		
		int idDepot = buscarDepotMasCercano(idsZona);
		if (idDepot == -1) continue;
		
		// --- A. Dijkstra repetido paso a paso ---
		auto t0 = std::chrono::high_resolution_clock::now();
		std::vector<int> visitasDijk;
		std::vector<int> callesDijk;
		int distDijk = 0;
		
		std::set<int> pendientes(idsZona.begin(), idsZona.end());
		int actual = idDepot;
		visitasDijk.push_back(actual);
		
		while (!pendientes.empty()) {
			ResultadoDijkstra resPaso = Dijkstra(actual);
			int mejorSig = -1;
			int minDist = INF;
			
			for (int cand : pendientes) {
				if (resPaso.distancias.count(cand) && resPaso.distancias[cand] < minDist) {
					minDist = resPaso.distancias[cand];
					mejorSig = cand;
				}
			}
			if (mejorSig == -1) break;
			
			distDijk += minDist;
			std::vector<int> tramo = reconstruirCamino(actual, mejorSig, resPaso.predecesor);
			if (!tramo.empty()) {
				if (!callesDijk.empty()) tramo.erase(tramo.begin());
				callesDijk.insert(callesDijk.end(), tramo.begin(), tramo.end());
			}
			visitasDijk.push_back(mejorSig);
			pendientes.erase(mejorSig);
			actual = mejorSig;
		}
		
		ResultadoDijkstra resVuelta = Dijkstra(actual);
		if (resVuelta.distancias.count(idDepot) && resVuelta.distancias[idDepot] != INF) {
			distDijk += resVuelta.distancias[idDepot];
			std::vector<int> tramo = reconstruirCamino(actual, idDepot, resVuelta.predecesor);
			if (!tramo.empty()) {
				if (!callesDijk.empty()) tramo.erase(tramo.begin());
				callesDijk.insert(callesDijk.end(), tramo.begin(), tramo.end());
			}
		}
		visitasDijk.push_back(idDepot);
		
		auto t1 = std::chrono::high_resolution_clock::now();
		double microsDijk = std::chrono::duration<double, std::micro>(t1 - t0).count();
		
		// --- B. Heuristica TSP simple (Vecino Mas Cercano + 2-Opt sobre matriz) ---
		auto t2 = std::chrono::high_resolution_clock::now();
		std::vector<int> tour = construirTourTSP(idDepot, idsZona);
		if (tour.size() > 4) tour = mejorar2Opt(tour);
		int distTSP = evaluarCostoTour(tour);
		std::vector<int> callesTSP = expandirCalles(tour);
		auto t3 = std::chrono::high_resolution_clock::now();
		double microsTSP = std::chrono::duration<double, std::micro>(t3 - t2).count();
		
		rutas.push_back({
			idZona, idDepot, cargaTotal,
				visitasDijk, callesDijk, distDijk, microsDijk,
				tour, callesTSP, distTSP, microsTSP
		});
	}
	
	return rutas;
}

std::vector<int> grafo::construirTourTSP(int idDepot, const std::vector<int>& idsZona) const {
	std::vector<int> tour;
	std::set<int> pendientes(idsZona.begin(), idsZona.end());
	int actual = idDepot;
	tour.push_back(actual);
	
	while (!pendientes.empty()) {
		int mejorSig = -1;
		int minDist = std::numeric_limits<int>::max();
		auto itU = matrizDistancias.find(actual);
		
		for (int cand : pendientes) {
			if (itU != matrizDistancias.end() && itU->second.count(cand)) {
				int d = itU->second.at(cand);
				if (d < minDist) {
					minDist = d;
					mejorSig = cand;
				}
			}
		}
		if (mejorSig == -1) break;
		tour.push_back(mejorSig);
		pendientes.erase(mejorSig);
		actual = mejorSig;
	}
	tour.push_back(idDepot);
	return tour;
}

std::vector<int> grafo::mejorar2Opt(const std::vector<int>& tourInicial) const {
	std::vector<int> mejor = tourInicial;
	bool mejoro = true;
	int n = static_cast<int>(mejor.size());
	
	while (mejoro) {
		mejoro = false;
		int mejorCosto = evaluarCostoTour(mejor);
		for (int i = 1; i < n - 2; ++i) {
			for (int k = i + 1; k < n - 1; ++k) {
				std::vector<int> cand = mejor;
				std::reverse(cand.begin() + i, cand.begin() + k + 1);
				int nuevoCosto = evaluarCostoTour(cand);
				if (nuevoCosto < mejorCosto) {
					mejor = cand;
					mejorCosto = nuevoCosto;
					mejoro = true;
				}
			}
		}
	}
	return mejor;
}

int grafo::evaluarCostoTour(const std::vector<int>& tour) const {
	int total = 0;
	const int INF = std::numeric_limits<int>::max();
	for (size_t i = 0; i + 1 < tour.size(); ++i) {
		auto itU = matrizDistancias.find(tour[i]);
		if (itU != matrizDistancias.end() && itU->second.count(tour[i + 1])) {
			int d = itU->second.at(tour[i + 1]);
			if (d != INF) total += d;
		}
	}
	return total;
}

std::vector<int> grafo::expandirCalles(const std::vector<int>& tour) const {
	std::vector<int> calles;
	for (size_t i = 0; i + 1 < tour.size(); ++i) {
		auto itP = matrizPredecesores.find(tour[i]);
		if (itP != matrizPredecesores.end()) {
			std::vector<int> tramo = reconstruirCamino(tour[i], tour[i + 1], itP->second);
			if (!tramo.empty()) {
				if (!calles.empty()) tramo.erase(tramo.begin());
				calles.insert(calles.end(), tramo.begin(), tramo.end());
			}
		}
	}
	return calles;
}
