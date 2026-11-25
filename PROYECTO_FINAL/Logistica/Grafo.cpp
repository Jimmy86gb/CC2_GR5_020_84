#include "Grafo.h"
#include <fstream>
#include <sstream>
#include <limits>
#include <queue>
#include <algorithm>
#include <chrono>
#include <cmath>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN // quita cosas viejas de windows.h que no usamos
#define NOMINMAX            // evita que windows.h dane std::min y std::numeric_limits::max
#include <windows.h>
#endif

// saca la hora actual en microsegundos para poder medir cuanto se demora cada algoritmo
// entrada: ninguna
// salida: un double con el tiempo actual en microsegundos
static double tiempoActualMicros() {
#ifdef _WIN32
	// factormicros se queda guardada entre llamadas por el static, asi solo se calcula una vez
	static double factorMicros = 0.0;
	if (factorMicros == 0.0) {
		LARGE_INTEGER freq; // guarda cuantos tics por segundo da el reloj del pc
		QueryPerformanceFrequency(&freq);
		// un segundo tiene un millon de microsegundos, asi pasamos de tics a microsegundos
		factorMicros = 1000000.0 / static_cast<double>(freq.QuadPart);
	}
	LARGE_INTEGER ahora; // guarda en que tic va el reloj ahorita mismo
	QueryPerformanceCounter(&ahora);
	return static_cast<double>(ahora.QuadPart) * factorMicros;
#else
	// por si se corre en linux o mac, usa el reloj normal de c++
	auto ahora = std::chrono::steady_clock::now().time_since_epoch();
	return std::chrono::duration<double, std::micro>(ahora).count();
#endif
}

// constructor vacio, arranca el grafo sin nada
grafo::grafo() {}

// destructor, cuando se cierra el programa borra todos los nodos de memoria
grafo::~grafo() {
	limpiar();
}

// borra todo lo que tenga el grafo para dejarlo vacio sin dejar basura en ram
// entrada: ninguna | salida: ninguna
void grafo::limpiar() {
	// recorre cada puntero del vector nodos y libera su memoria con delete
	for (nodo* n : nodos) {
		delete n;
	}
	nodos.clear();              // vacia la lista de nodos
	matrizDistancias.clear();   // borra las distancias guardadas
	matrizPredecesores.clear(); // borra las rutas guardadas
}

// mete un nodo nuevo al grafo si no existe otro con el mismo id
// entrada: valor (id), nombre del lugar, posicion x y, tipo (deposito/cruce/entrega) y demanda (paquetes)
// salida: ninguna
void grafo::agregarNodo(int valor, const std::string& nombre, double x, double y,
						TipoNodo tipo, int demanda) {
	// primero revisa que ese id no este repetido
	if (!buscarNodo(valor)) {
		// crea el nodo con new y mete su direccion al vector nodos
		nodos.push_back(new nodo(valor, nombre, x, y, tipo, demanda));
	}
}

// conecta un nodo origen con un destino en un solo sentido con su peso en metros
// entrada: id origen, id destino, peso de la calle
// salida: ninguna
void grafo::agregarArista(int origen, int destino, int peso) {
	nodo* nodoOrigen = buscarNodo(origen);   // busca el puntero del nodo de salida
	nodo* nodoDestino = buscarNodo(destino); // busca el puntero del nodo de llegada
	// solo conecta si ambos existen en el grafo
	if (nodoOrigen && nodoDestino) {
		nodoOrigen->agregarArista(nodoDestino, peso);
	}
}

// conecta dos nodos en ambos sentidos porque las calles del mapa son doble via
// entrada: id origen, id destino, peso | salida: ninguna
void grafo::agregarAristaBidireccional(int origen, int destino, int peso) {
	agregarArista(origen, destino, peso);
	agregarArista(destino, origen, peso);
}

// crea una calle doble via calculando el peso solo a partir de las coordenadas x y
// entrada: id origen, id destino | salida: ninguna
void grafo::agregarAristaAutomatica(int origen, int destino) {
	nodo* nodoOrigen = buscarNodo(origen);
	nodo* nodoDestino = buscarNodo(destino);
	if (nodoOrigen && nodoDestino) {
		// saca la distancia en linea recta y le suma 18 por ciento para simular curvas de calle real
		int peso = static_cast<int>(std::round(nodoOrigen->distanciaEuclidiana(nodoDestino) * 1.18));
		if (peso < 1) peso = 1; // evita que quede una calle de peso 0
		agregarAristaBidireccional(origen, destino, peso);
	}
}

// busca un nodo por su numero de id
// entrada: valor (id a buscar) | salida: puntero al nodo encontrado o nullptr si no existe
nodo* grafo::buscarNodo(int valor) const {
	for (nodo* n : nodos) {
		if (n->getDato() == valor) return n; // si coincide el id devuelve ese nodo
	}
	return nullptr;
}

// borra la conexion que va desde origen hasta destino
// entrada: id origen, id destino | salida: ninguna
void grafo::eliminarArista(int origen, int destino) {
	nodo* nodoOrigen = buscarNodo(origen);
	nodo* nodoDestino = buscarNodo(destino);
	if (nodoOrigen && nodoDestino) {
		nodoOrigen->eliminarArista(nodoDestino);
	}
}

// borra un nodo del grafo y tambien quita todas las calles que apuntaban a el
// entrada: valor (id del nodo a borrar) | salida: ninguna
void grafo::eliminarNodo(int valor) {
	nodo* nodoAEliminar = buscarNodo(valor); // busca el nodo
	if (!nodoAEliminar) return;              // si no existe se sale
	
	// primero le quita a todos los demas nodos cualquier conexion hacia el que vamos a borrar
	for (nodo* n : nodos) {
		n->eliminarArista(nodoAEliminar);
	}
	
	// luego lo busca en el vector principal con el iterador it para sacarlo de la lista
	for (auto it = nodos.begin(); it != nodos.end(); ++it) {
		if (*it == nodoAEliminar) {
			nodos.erase(it);
			break;
		}
	}
	delete nodoAEliminar; // al final si lo borra de memoria para que no quede basura
}

// devuelve la lista completa de nodos (solo lectura)
// entrada: ninguna | salida: referencia constante al vector de punteros nodo
const std::vector<nodo*>& grafo::getNodos() const { return nodos; }

// filtra y devuelve solo los nodos de un tipo especifico (por ejemplo solo entregas o solo depositos)
// entrada: tipo de nodo buscado | salida: vector con los nodos que cumplen
std::vector<nodo*> grafo::obtenerNodosPorTipo(TipoNodo tipo) const {
	std::vector<nodo*> res; // lista donde vamos guardando los que coinciden
	for (nodo* n : nodos) {
		if (n->getTipo() == tipo) res.push_back(n);
	}
	return res;
}

// le quita el color de zona a todos los nodos dejandolos en -1 (sin zona)
// entrada: ninguna | salida: ninguna
void grafo::reiniciarColores() {
	for (nodo* n : nodos) n->setColorZona(-1);
}

// devuelve cuantos nodos hay en total en el grafo
int grafo::cantidadNodos() const { return static_cast<int>(nodos.size()); }

// cuenta cuantas calles reales hay sin contar doble las idas y vueltas
// entrada: ninguna | salida: entero con la cantidad de calles unicas
int grafo::cantidadAristasUnicas() const {
	int total = 0; // suma todas las conexiones salientes de cada nodo
	for (nodo* n : nodos) total += static_cast<int>(n->getAdyacentes().size());
	return total / 2; // se divide por 2 porque cada calle esta guardada en ambos extremos
}

// lee un archivo txt y arma todo el mapa con sus depositos, cruces, entregas y calles
// entrada: rutaarchivo (donde esta el txt) y errorout (texto por referencia para avisar si algo fallo)
// salida: true si cargo bien, false si hubo error
bool grafo::cargarDesdeTXT(const std::string& rutaArchivo, std::string& errorOut) {
	std::ifstream archivo(rutaArchivo); // abre el archivo en modo lectura
	if (!archivo.is_open()) {
		errorOut = "No se pudo abrir el archivo seleccionado.";
		return false;
	}
	
	// struct temporal para ir guardando cada nodo que leemos antes de borrar el mapa actual
	struct DatoNodoTXT {
		int id;
		TipoNodo tipo;
		double x, y;
		int demanda;
		std::string nombre;
	};
	// struct temporal para guardar cada calle (u = origen, v = destino, peso = metros)
	struct DatoAristaTXT {
		int u, v, peso;
	};
	
	std::vector<DatoNodoTXT> nodosLeidos;     // lista temporal de nodos leidos del txt
	std::vector<DatoAristaTXT> aristasLeidas; // lista temporal de aristas leidas del txt
	int cantDepots = 0;   // cuenta cuantos depositos trajo el archivo
	int cantEntregas = 0; // cuenta cuantas entregas trajo el archivo
	
	std::string linea; // guarda el texto de la linea actual
	int numLinea = 0;  // contador de lineas para decir en que linea fallo si hay error
	while (std::getline(archivo, linea)) {
		numLinea++;
		// quita el caracter \r al final que a veces meten los archivos de windows
		if (!linea.empty() && linea.back() == '\r') linea.pop_back();
		
		std::istringstream iss(linea); // sirve para partir la linea por espacios palabra por palabra
		std::string etiqueta;          // primera palabra de la linea (NODO, ARISTA o #)
		if (!(iss >> etiqueta) || etiqueta[0] == '#') continue; // salta lineas vacias o comentarios
		
		if (etiqueta == "NODO") {
			int id, demanda;
			double x, y;
			std::string tipoStr; // texto del tipo leido del archivo
			if (!(iss >> id >> tipoStr >> x >> y >> demanda)) {
				errorOut = "Formato invalido en NODO (linea " + std::to_string(numLinea) + ").";
				return false;
			}
			std::string nombre;
			// std::ws se come los espacios antes del nombre y getline lee todo el resto aunque tenga espacios
			std::getline(iss >> std::ws, nombre);
			if (nombre.empty()) nombre = "Nodo " + std::to_string(id);
			
			TipoNodo tipo = INTERSECCION; // por defecto queda como cruce de calle
			if (tipoStr == "DEPOSITO" || tipoStr == "DEPOT") {
				tipo = DEPOSITO;
				cantDepots++;
			} else if (tipoStr == "ENTREGA" || tipoStr == "POI") {
				tipo = ENTREGA;
				cantEntregas++;
			}
			
			// guarda el nodo en la lista temporal usando el struct datonodotxt
			nodosLeidos.push_back({id, tipo, x, y, demanda, nombre});
		} else if (etiqueta == "ARISTA") {
			int u, v, peso = 0;
			if (!(iss >> u >> v)) {
				errorOut = "Formato invalido en ARISTA (linea " + std::to_string(numLinea) + ").";
				return false;
			}
			iss >> peso; // si no trae peso o trae 0, despues se calcula solo por distancia
			aristasLeidas.push_back({u, v, peso});
		}
	}
	
	// revisa que el mapa sirva para la simulacion antes de reemplazar el viejo
	if (cantDepots == 0 || cantEntregas == 0) {
		errorOut = "El archivo .txt debe contener al menos 1 DEPOSITO y 1 ENTREGA.";
		return false;
	}
	
	// como todo se leyo bien, ahora si limpia el grafo viejo y mete los datos nuevos
	limpiar();
	for (const auto& dn : nodosLeidos) {
		agregarNodo(dn.id, dn.nombre, dn.x, dn.y, dn.tipo, dn.demanda);
	}
	for (const auto& da : aristasLeidas) {
		if (da.peso > 0) {
			agregarAristaBidireccional(da.u, da.v, da.peso); // usa el peso fijo del txt
		} else {
			agregarAristaAutomatica(da.u, da.v);             // calcula el peso con x y
		}
	}
	
	return true;
}

// agrega una entrega nueva desde la interfaz y la conecta a los 2 puntos mas cercanos del mapa
// entrada: nombre del sitio, posicion x y, demanda en paquetes
// salida: el id nuevo que se le asigno a esa entrega
int grafo::insertarEntregaConectada(const std::string& nombre, double x, double y, int demanda) {
	int nuevoId = 1; // busca cual es el id mas grande actual para poner el siguiente
	for (nodo* n : nodos) {
		if (n->getDato() >= nuevoId) nuevoId = n->getDato() + 1;
	}
	
	agregarNodo(nuevoId, nombre, x, y, ENTREGA, demanda);
	nodo* nuevoNodo = buscarNodo(nuevoId);
	
	// guarda pares de {distancia, id} contra todos los nodos que ya existian
	std::vector<std::pair<double, int>> cercanos;
	for (nodo* n : nodos) {
		if (n->getDato() == nuevoId) continue; // no se compara consigo mismo
		cercanos.push_back({nuevoNodo->distanciaEuclidiana(n), n->getDato()});
	}
	std::sort(cercanos.begin(), cercanos.end()); // ordena de menor a mayor distancia
	
	// conecta a maximo 2 vecinos (o menos si el mapa tiene pocos nodos)
	int enlaces = std::min(2, static_cast<int>(cercanos.size()));
	for (int i = 0; i < enlaces; ++i) {
		agregarAristaAutomatica(nuevoId, cercanos[i].second);
	}
	return nuevoId;
}

// algoritmo de dijkstra: calcula las distancias mas cortas desde un nodo origen a todos los demas
// entrada: valorinicial (id del nodo desde donde arranca)
// salida: struct resultadodijkstra con mapa de distancias, predecesores, avisos de pesos negativos y tiempo
ResultadoDijkstra grafo::Dijkstra(int valorInicial) const {
	double tInicio = tiempoActualMicros(); // marca el tiempo al arrancar
	
	// struct donde se va a devolver todo el resultado junto
	ResultadoDijkstra res;
	res.huboPesoNegativo = false;
	res.falloDePropagacion = false;
	res.tiempoMicros = 0.0;
	
	nodo* nodoInicial = buscarNodo(valorInicial);
	if (!nodoInicial) return res; // si el id no existe devuelve vacio
	
	std::map<int, bool> visitado; // marca que nodos ya quedaron listos para no repetirlos
	// arranca poniendo todas las distancias en infinito (el numero mas grande de int)
	for (auto nod : nodos) {
		res.distancias[nod->getDato()] = std::numeric_limits<int>::max();
		visitado[nod->getDato()] = false;
	}
	res.distancias[valorInicial] = 0; // la distancia al mismo nodo de salida es 0
	
	// cola que siempre deja de primero el par con menor distancia (por el std::greater)
	using p = std::pair<int, nodo*>;
	std::priority_queue<p, std::vector<p>, std::greater<p>> cola;
	cola.push({0, nodoInicial});
	
	while (!cola.empty()) {
		auto elemento = cola.top();        // saca el nodo mas cercano pendiente
		int disActual = elemento.first;    // distancia acumulada hasta aqui
		nodo* nodoActual = elemento.second; // puntero al nodo que estamos revisando
		cola.pop();
		
		int u = nodoActual->getDato(); // id del nodo actual
		if (visitado[u]) continue;     // si ya habia salido con una distancia menor, se ignora
		visitado[u] = true;
		
		// revisa cada calle que sale del nodo actual
		for (auto par : nodoActual->getAdyacentes()) {
			nodo* vecino = par.first; // nodo al que llega la calle
			int peso = par.second;    // metros que mide la calle
			
			if (peso < 0) res.huboPesoNegativo = true; // aviso por si meten pesos negativos
			
			int v = vecino->getDato();             // id del vecino
			int nuevaDistancia = disActual + peso; // cuanto costaria llegar a v pasando por u
			
			// si este camino es mas corto que el que teniamos guardado, lo actualizamos
			if (nuevaDistancia < res.distancias[v]) {
				// si ya estaba cerrado y aparecio algo menor es porque hubo peso negativo
				if (visitado[v]) res.falloDePropagacion = true;
				res.distancias[v] = nuevaDistancia;
				res.predecesor[v] = u; // guarda que a v se llega desde u
				cola.push({nuevaDistancia, vecino});
			}
		}
	}
	
	res.tiempoMicros = tiempoActualMicros() - tInicio; // resta el inicio para sacar lo que tardo
	return res;
}

// arma la lista de nodos paso a paso desde origen hasta destino usando el mapa de predecesores
// entrada: id origen, id destino, mapa predecesor que dejo dijkstra
// salida: vector con la secuencia de ids del camino (vacio si no se puede llegar)
std::vector<int> grafo::reconstruirCamino(int origen, int destino, const std::map<int, int>& predecesor) const {
	std::vector<int> ruta; // aqui se va armando el camino
	if (origen == destino) {
		ruta.push_back(origen);
		return ruta;
	}
	// si el destino no tiene predecesor significa que no hay camino hasta alla
	if (predecesor.find(destino) == predecesor.end()) return ruta;
	
	int actual = destino; // arrancamos desde el final devolviendonos hacia el origen
	int pasos = 0;        // limite de seguridad para no quedar en ciclo infinito
	while (actual != origen && predecesor.count(actual) && pasos <= static_cast<int>(nodos.size())) {
		ruta.push_back(actual);
		actual = predecesor.at(actual); // salta al nodo anterior
		pasos++;
	}
	if (actual != origen) return std::vector<int>(); // si no llego al origen devuelve vacio
	ruta.push_back(origen);
	// como se armo al reves (destino -> origen), se voltea para que quede derecho
	std::reverse(ruta.begin(), ruta.end());
	return ruta;
}

// corre dijkstra desde todos los nodos del mapa y guarda todas las distancias y rutas en memoria
// entrada: ninguna | salida: tiempo total en microsegundos que tardo en armar toda la matriz
double grafo::precalcularMatrizDijkstra() {
	double t0 = tiempoActualMicros();
	matrizDistancias.clear();
	matrizPredecesores.clear();
	
	for (nodo* n : nodos) {
		int id = n->getDato();
		ResultadoDijkstra res = Dijkstra(id); // recibe el struct con todo lo de este nodo
		matrizDistancias[id] = res.distancias;
		matrizPredecesores[id] = res.predecesor;
	}
	
	return tiempoActualMicros() - t0;
}

// arma el grafo de conflictos entre entregas y reparte las zonas usando welsh-powell o dsatur
// entrada: umbralmetros (distancia limite), capacidadmaxzona (tope de paquetes), metodo (0 wp, 1 dsatur), modoconflicto (0 lejania, 1 vecindad)
// salida: struct resultadocoloreado con total de zonas, total de conflictos, mapa de conflictos y tiempo
ResultadoColoreado grafo::colorearZonas(int umbralMetros, int capacidadMaxZona, int metodo, int modoConflicto) {
	if (matrizDistancias.empty()) precalcularMatrizDijkstra(); // si no hay matriz la calcula primero
	
	double tInicio = tiempoActualMicros();
	
	std::vector<nodo*> entregas = obtenerNodosPorTipo(ENTREGA); // solo se colorean las entregas
	std::map<int, std::set<int>> conflictos; // guarda para cada entrega con cuales otras choca
	for (nodo* e : entregas) conflictos[e->getDato()] = std::set<int>();
	
	const int INF = std::numeric_limits<int>::max();
	int totalConflictos = 0; // contador de lineas rojas de conflicto
	
	// compara cada pareja de entregas sin repetir pares
	for (size_t i = 0; i < entregas.size(); ++i) {
		for (size_t j = i + 1; j < entregas.size(); ++j) {
			int u = entregas[i]->getDato();
			int v = entregas[j]->getDato();
			int distRed = INF; // distancia real por calles entre u y v
			
			auto itU = matrizDistancias.find(u);
			if (itU != matrizDistancias.end() && itU->second.count(v)) {
				distRed = itU->second.at(v);
			}
			
			// modo 0 (lejania): si estan mas lejos que el umbral no pueden ir en la misma zona
			// modo 1 (vecindad): si estan muy cerca (menor o igual al umbral) se separan en zonas distintas
			bool hayConflicto = (modoConflicto == 0) ? (distRed > umbralMetros) : (distRed <= umbralMetros);
			if (hayConflicto) {
				conflictos[u].insert(v);
				conflictos[v].insert(u);
				totalConflictos++;
			}
		}
	}
	
	// llama al metodo que escogio el usuario y recibe un mapa {id_entrega -> numero_color}
	std::map<int, int> asignacion = (metodo == 0)
		? colorearWelshPowell(entregas, conflictos, capacidadMaxZona)
		: colorearDSATUR(entregas, conflictos, capacidadMaxZona);
	
	std::set<int> coloresUnicos; // el set no guarda repetidos, asi sabemos cuantas zonas salieron
	for (nodo* e : entregas) {
		int c = asignacion[e->getDato()];
		e->setColorZona(c); // le pinta la zona directamente al objeto nodo
		coloresUnicos.insert(c);
	}
	
	double micros = tiempoActualMicros() - tInicio;
	
	// devuelve los 4 datos empacados en el struct resultadocoloreado
	return {
		static_cast<int>(coloresUnicos.size()),
		totalConflictos,
		conflictos,
		micros
	};
}

// coloreado welsh-powell: ordena primero las entregas con mas conflictos y les va dando el primer color libre
// entrada: lista de entregas, mapa de conflictos, capacidad maxima de paquetes por zona
// salida: mapa con el color (zona) asignado a cada id de entrega
std::map<int, int> grafo::colorearWelshPowell(
											  const std::vector<nodo*>& entregas,
											  const std::map<int, std::set<int>>& conflictos,
											  int capacidadMaxZona) const {
	
	std::vector<nodo*> ordenados = entregas; // copia para ordenar sin mover la lista original
	// ordena de mayor a menor cantidad de conflictos, y si empatan va primero la que pide mas paquetes
	std::sort(ordenados.begin(), ordenados.end(), [&](nodo* a, nodo* b) {
		size_t gA = conflictos.at(a->getDato()).size(); // cuantos conflictos tiene a
		size_t gB = conflictos.at(b->getDato()).size(); // cuantos conflictos tiene b
		if (gA != gB) return gA > gB;
		return a->getDemanda() > b->getDemanda();
	});
	
	std::map<int, int> colorNodo;     // guarda que color le toco a cada nodo
	std::map<int, int> cargaPorColor; // suma cuantos paquetes lleva cada zona para no pasarse
	
	for (nodo* uNodo : ordenados) {
		int u = uNodo->getDato();
		int demandaU = uNodo->getDemanda();
		std::set<int> prohibidos; // colores que ya estan usando los vecinos en conflicto
		
		for (int vecino : conflictos.at(u)) {
			if (colorNodo.count(vecino)) prohibidos.insert(colorNodo[vecino]);
		}
		
		int color = 0; // prueba desde la zona 0 hacia arriba
		// si el color esta prohibido o si al meter este pedido se pasa de la capacidad, prueba el siguiente color
		// la parte de cargaPorColor[color] > 0 evita ciclo infinito si un solo cliente pesa mas que la capacidad
		while (prohibidos.count(color) ||
			   (cargaPorColor[color] + demandaU > capacidadMaxZona && cargaPorColor[color] > 0)) {
			color++;
		}
		
		colorNodo[u] = color;
		cargaPorColor[color] += demandaU;
	}
	
	return colorNodo;
}

// coloreado dsatur: en cada paso elige el nodo mas apretado (el que tiene mas colores distintos a su alrededor)
// entrada: lista de entregas, mapa de conflictos, capacidad maxima por zona
// salida: mapa con el color (zona) asignado a cada id de entrega
std::map<int, int> grafo::colorearDSATUR(
										 const std::vector<nodo*>& entregas,
										 const std::map<int, std::set<int>>& conflictos,
										 int capacidadMaxZona) const {
	
	std::map<int, int> colorNodo;     // color asignado a cada id
	std::map<int, int> cargaPorColor; // paquetes acumulados por zona
	std::set<nodo*> pendientes(entregas.begin(), entregas.end()); // entregas que todavia no tienen zona
	
	while (!pendientes.empty()) {
		nodo* mejorNodo = nullptr; // el candidato ganador de esta ronda
		int maxSat = -1;           // mayor cantidad de colores vecinos distintos encontrada
		int maxGrado = -1;         // desempate: mayor cantidad de conflictos
		
		for (nodo* cand : pendientes) {
			int idC = cand->getDato();
			std::set<int> coloresVecinos; // guarda colores distintos de los vecinos ya pintados
			for (int vec : conflictos.at(idC)) {
				if (colorNodo.count(vec)) coloresVecinos.insert(colorNodo[vec]);
			}
			int sat = static_cast<int>(coloresVecinos.size()); // saturacion de este candidato
			int grado = static_cast<int>(conflictos.at(idC).size()); // grado de conflicto
			
			if (sat > maxSat || (sat == maxSat && grado > maxGrado)) {
				maxSat = sat;
				maxGrado = grado;
				mejorNodo = cand;
			}
		}
		
		int u = mejorNodo->getDato();
		int demandaU = mejorNodo->getDemanda();
		std::set<int> prohibidos; // colores ocupados por vecinos en conflicto
		
		for (int vec : conflictos.at(u)) {
			if (colorNodo.count(vec)) prohibidos.insert(colorNodo[vec]);
		}
		
		int color = 0;
		// busca el primer color libre que tampoco pase la carga maxima de paquetes
		while (prohibidos.count(color) ||
			   (cargaPorColor[color] + demandaU > capacidadMaxZona && cargaPorColor[color] > 0)) {
			color++;
		}
		
		colorNodo[u] = color;
		cargaPorColor[color] += demandaU;
		pendientes.erase(mejorNodo); // lo saca de pendientes porque ya quedo pintado
	}
	
	return colorNodo;
}

// si el mapa tiene varios depositos, elige el que quede mas cerca en promedio a las entregas de esa zona
// entrada: idsZona (lista de entregas que pertenecen a una misma zona)
// salida: id del deposito mas cercano, o -1 si no hay depositos
int grafo::buscarDepotMasCercano(const std::vector<int>& idsZona) const {
	std::vector<nodo*> depots = obtenerNodosPorTipo(DEPOSITO);
	if (depots.empty()) return -1;
	
	int mejorDepot = depots.front()->getDato(); // empieza suponiendo que gana el primer deposito
	long long menorSuma = std::numeric_limits<long long>::max(); // long long para que la suma no se desborde
	const int INF = std::numeric_limits<int>::max();
	
	for (nodo* d : depots) {
		int idD = d->getDato();
		long long suma = 0; // suma de distancias desde este deposito a todas las entregas de la zona
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

// arma las rutas de cada vehiculo (una por zona) y compara dijkstra repetido contra tsp + 2-opt
// entrada: ninguna (usa los colores ya asignados a los nodos)
// salida: vector de structs rutazona, donde cada struct tiene las rutas, distancias, cargas y tiempos de una zona
std::vector<RutaZona> grafo::planificarRutasFlota() const {
	// agrupa los ids de las entregas segun su numero de zona
	std::map<int, std::vector<int>> entregasPorZona;
	for (nodo* e : obtenerNodosPorTipo(ENTREGA)) {
		if (e->getColorZona() >= 0) {
			entregasPorZona[e->getColorZona()].push_back(e->getDato());
		}
	}
	
	std::vector<RutaZona> rutas; // lista final de structs rutazona que se devuelve a la ventana
	const int INF = std::numeric_limits<int>::max();
	
	for (const auto& par : entregasPorZona) {
		int idZona = par.first;                      // numero de zona (0, 1, 2...)
		const std::vector<int>& idsZona = par.second; // lista de entregas de esa zona
		
		int cargaTotal = 0; // suma cuantos paquetes va a llevar el vehiculo de esta zona
		for (int id : idsZona) {
			nodo* n = buscarNodo(id);
			if (n) cargaTotal += n->getDemanda();
		}
		
		int idDepot = buscarDepotMasCercano(idsZona); // de cual bodega sale este carro
		if (idDepot == -1) continue;
		
		// metodo a: dijkstra repetido (en cada parada vuelve a correr dijkstra para ir al cliente mas cercano)
		double t0 = tiempoActualMicros();
		std::vector<int> visitasDijk; // solo orden de paradas (deposito y clientes)
		std::vector<int> callesDijk;  // secuencia completa incluyendo cruces de calle
		int distDijk = 0;             // metros totales recorridos con este metodo
		
		std::set<int> pendientes(idsZona.begin(), idsZona.end()); // clientes que faltan por visitar
		int actual = idDepot; // empieza parado en el deposito
		visitasDijk.push_back(actual);
		
		while (!pendientes.empty()) {
			ResultadoDijkstra resPaso = Dijkstra(actual); // corre dijkstra en vivo desde donde esta el carro
			int mejorSig = -1; // siguiente entrega mas cercana
			int minDist = INF;
			
			for (int cand : pendientes) {
				if (resPaso.distancias.count(cand) && resPaso.distancias[cand] < minDist) {
					minDist = resPaso.distancias[cand];
					mejorSig = cand;
				}
			}
			if (mejorSig == -1) break; // por si alguna entrega quedo desconectada
			
			distDijk += minDist;
			std::vector<int> tramo = reconstruirCamino(actual, mejorSig, resPaso.predecesor);
			if (!tramo.empty()) {
				// quita el primer nodo del tramo nuevo para no repetir el punto donde se pegan dos tramos
				if (!callesDijk.empty()) tramo.erase(tramo.begin());
				callesDijk.insert(callesDijk.end(), tramo.begin(), tramo.end());
			}
			visitasDijk.push_back(mejorSig);
			pendientes.erase(mejorSig);
			actual = mejorSig; // mueve el carro a la entrega que acaba de hacer
		}
		
		// corre un ultimo dijkstra para devolverse desde la ultima entrega hasta el deposito
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
		
		double microsDijk = tiempoActualMicros() - t0; // tiempo que gasto dijkstra repetido
		
		// metodo b: tsp simple + mejora 2-opt (usa la matriz ya calculada en vez de repetir dijkstra)
		double t2 = tiempoActualMicros();
		std::vector<int> tour = construirTourTSP(idDepot, idsZona); // ruta inicial de vecino mas cercano
		// solo vale la pena aplicar 2-opt si hay al menos 3 entregas (mas salida y llegada son mas de 4)
		if (tour.size() > 4) tour = mejorar2Opt(tour);
		int distTSP = evaluarCostoTour(tour);            // suma de metros del tour final
		std::vector<int> callesTSP = expandirCalles(tour); // convierte paradas en calles reales
		double microsTSP = tiempoActualMicros() - t2;    // tiempo que gasto tsp + 2-opt
		
		// guarda todo lo de esta zona dentro de un struct rutazona
		rutas.push_back({
			idZona, idDepot, cargaTotal,
			visitasDijk, callesDijk, distDijk, microsDijk,
			tour, callesTSP, distTSP, microsTSP
		});
	}
	
	return rutas;
}

// arma un recorrido inicial rapido saltando siempre al cliente mas cercano usando matrizdistancias
// entrada: iddepot (donde sale y vuelve), idszona (clientes a visitar)
// salida: vector con el orden de paradas (ej: 1 -> 14 -> 18 -> 15 -> 1)
std::vector<int> grafo::construirTourTSP(int idDepot, const std::vector<int>& idsZona) const {
	std::vector<int> tour;
	std::set<int> pendientes(idsZona.begin(), idsZona.end());
	int actual = idDepot;
	tour.push_back(actual);
	
	while (!pendientes.empty()) {
		int mejorSig = -1;
		int minDist = std::numeric_limits<int>::max();
		auto itU = matrizDistancias.find(actual); // consulta directo en la matriz precalculada
		
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
	tour.push_back(idDepot); // cierra el circuito regresando al deposito
	return tour;
}

// mejora un recorrido volteando pedazos del orden de visitas para quitar cruces o devoluciones
// entrada: tourinicial (ruta que dio construirtourtsp)
// salida: el mismo tour pero reordenado si encontro una distancia menor
std::vector<int> grafo::mejorar2Opt(const std::vector<int>& tourInicial) const {
	std::vector<int> mejor = tourInicial; // mejor orden encontrado hasta ahora
	bool mejoro = true;                   // bandera para seguir intentando mientras logre ahorrar metros
	int n = static_cast<int>(mejor.size()); // cantidad de paradas en el tour
	
	while (mejoro) {
		mejoro = false;
		int mejorCosto = evaluarCostoTour(mejor);
		// i y k se mueven solo por las entregas del medio, dejando fijos el inicio (0) y el final (n-1)
		for (int i = 1; i < n - 2; ++i) {
			for (int k = i + 1; k < n - 1; ++k) {
				std::vector<int> cand = mejor; // copia para probar el cambio
				// voltea el tramo desde la posicion i hasta k inclusive (por eso el + 1)
				std::reverse(cand.begin() + i, cand.begin() + k + 1);
				int nuevoCosto = evaluarCostoTour(cand);
				if (nuevoCosto < mejorCosto) {
					mejor = cand;
					mejorCosto = nuevoCosto;
					mejoro = true; // como ahorro metros, repite otra ronda buscando mas mejoras
				}
			}
		}
	}
	return mejor;
}

// suma los metros totales de un recorrido consultando la matriz de distancias entre cada parada seguida
// entrada: tour (vector con el orden de paradas) | salida: entero con la distancia total en metros
int grafo::evaluarCostoTour(const std::vector<int>& tour) const {
	int total = 0;
	const int INF = std::numeric_limits<int>::max();
	for (size_t i = 0; i + 1 < tour.size(); ++i) {
		auto itU = matrizDistancias.find(tour[i]);
		if (itU != matrizDistancias.end() && itU->second.count(tour[i + 1])) {
			int d = itU->second.at(tour[i + 1]);
			if (d != INF) total += d; // evita sumar infinito si algo esta desconectado
		}
	}
	return total;
}

// convierte una lista corta de paradas en la lista larga de calles y cruces reales para dibujar en el mapa
// entrada: tour (solo deposito y entregas) | salida: vector con todos los nodos intermedios por donde pasa
std::vector<int> grafo::expandirCalles(const std::vector<int>& tour) const {
	std::vector<int> calles; // ruta completa calle por calle
	for (size_t i = 0; i + 1 < tour.size(); ++i) {
		auto itP = matrizPredecesores.find(tour[i]);
		if (itP != matrizPredecesores.end()) {
			// reconstruye las esquinas intermedias entre la parada i y la parada i+1
			std::vector<int> tramo = reconstruirCamino(tour[i], tour[i + 1], itP->second);
			if (!tramo.empty()) {
				// quita el primer nodo del nuevo tramo para no duplicar el empalme
				if (!calles.empty()) tramo.erase(tramo.begin());
				calles.insert(calles.end(), tramo.begin(), tramo.end());
			}
		}
	}
	return calles;
}