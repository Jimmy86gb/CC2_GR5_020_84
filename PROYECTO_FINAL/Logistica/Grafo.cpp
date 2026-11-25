#include "Grafo.h"
#include <fstream>
#include <sstream>
#include <limits>
#include <queue>
#include <algorithm>
#include <chrono>
#include <cmath>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#endif

/* 
tiempoActualMicros
* - Entrega / Devuelve: Un número decimal (`double`) que representa el instante
*   de tiempo actual medido en microsegundos (útil para medir cuánto tarda un
*   proceso restando un tiempo inicial a un tiempo final).
* - Flujo de datos:
*   1. Si estamos en Windows (`_WIN32`), usa un reloj de alta precisión del sistema
*      y convierte los ticks a microsegundos mediante un factor calculado una sola vez.
*   2. En otros sistemas operativos, obtiene el tiempo transcurrido en el reloj
*      estable (`steady_clock`) y lo convierte directamente a microsegundos.
*   3. Devuelve el valor numérico resultante.
 */
static double tiempoActualMicros() {
	//  Paso 1: En Windows, calcular factor y leer contador de alta resolución 
#ifdef _WIN32
	static double factorMicros = 0.0;
	if (factorMicros == 0.0) {
		LARGE_INTEGER freq;
		QueryPerformanceFrequency(&freq);
		factorMicros = 1000000.0 / static_cast<double>(freq.QuadPart);
	}
	LARGE_INTEGER ahora;
	QueryPerformanceCounter(&ahora);
	//  Paso 3: Retornar tiempo en microsegundos (Windows) 
	return static_cast<double>(ahora.QuadPart) * factorMicros;
#else
	// Paso 2: En otros sistemas, leer reloj estable y convertir a microsegundos 
	auto ahora = std::chrono::steady_clock::now().time_since_epoch();
	//  Paso 3: Retornar tiempo en microsegundos (no Windows) 
	return std::chrono::duration<double, std::micro>(ahora).count();
#endif
}

/
grafo::grafo() {
}

grafo::~grafo() {
	limpiar();
}

/* 
impiar
* - Flujo de datos:
*   1. Recorre cada puntero a `nodo` guardado en `nodos` y libera su memoria con `delete`.
*   2. Vacía el vector `nodos` para no dejar punteros inválidos.
*   3. Limpia `matrizDistancias` y `matrizPredecesores` para borrar rutas calculadas antes.
*/
void grafo::limpiar() {
	// Paso 1: Liberar memoria de cada nodo existente
	for (nodo* n : nodos) {
		delete n;
	}
	// Paso 2: Vaciar el contenedor de nodos
	nodos.clear();
	// Paso 3: Borrar matrices precalculadas de distancias y predecesores
	matrizDistancias.clear();
	matrizPredecesores.clear();
}

/* 
agregarNodo
*   Recibe:
*   * `valor` (`int`): identificador único del nodo.
*   * `nombre` (`const std::string&`): nombre o etiqueta del lugar/punto.
*   * `x` (`double`) y `y` (`double`): coordenadas espaciales en el mapa.
*   * `tipo` (`TipoNodo`): categoría del punto (por ejemplo, depósito, intersección o entrega).
*   * `demanda` (`int`): cantidad de carga o pedido asociada al nodo.
* - Flujo de datos:
*   1. Busca si ya existe un nodo con el `valor` indicado usando `buscarNodo(valor)`.
*   2. Si no existe, crea un nuevo `nodo` con esos datos y lo agrega al vector `nodos`.
*/
void grafo::agregarNodo(int valor, const std::string& nombre, double x, double y,
						TipoNodo tipo, int demanda) {
	//  Paso 1: Verificar si ya existe un nodo con ese identificador 
	if (!buscarNodo(valor)) {
		//  Paso 2: Crear y registrar el nuevo nodo en el grafo 
		nodos.push_back(new nodo(valor, nombre, x, y, tipo, demanda));
	}
}

/* 
agregarArista
* - Recibe:
*   * `origen` (`int`): ID del nodo desde donde sale la conexión.
*   * `destino` (`int`): ID del nodo hacia donde llega la conexión.
*   * `peso` (`int`): costo o distancia de ese tramo (por ejemplo, en metros).
* - Flujo de datos:
*   1. Localiza los punteros correspondientes a `origen` y `destino`.
*   2. Si ambos nodos existen, agrega la arista con su `peso` desde `nodoOrigen` a `nodoDestino`.
*/
void grafo::agregarArista(int origen, int destino, int peso) {
	// Paso 1: Buscar nodos de origen y destino
	nodo* nodoOrigen = buscarNodo(origen);
	nodo* nodoDestino = buscarNodo(destino);
	// Paso 2: Si ambos existen, añadir la conexión dirigida
	if (nodoOrigen && nodoDestino) {
		nodoOrigen->agregarArista(nodoDestino, peso);
	}
}

/*
* LEYENDA: grafo::agregarAristaBidireccional
* - Recibe:
*   * `origen` (`int`): ID de uno de los extremos de la conexión.
*   * `destino` (`int`): ID del otro extremo de la conexión.
*   * `peso` (`int`): distancia o costo del trayecto en ambos sentidos.
* - Flujo de datos:
*   1. Agrega la arista desde `origen` hacia `destino` con el `peso` dado.
*   2. Agrega la arista inversa desde `destino` hacia `origen` con el mismo `peso`.
*/
void grafo::agregarAristaBidireccional(int origen, int destino, int peso) {
	// Paso 1: Crear conexión de origen a destino 
	agregarArista(origen, destino, peso);
	//  Paso 2: Crear conexión de regreso de destino a origen 
	agregarArista(destino, origen, peso);
}

/*
agregarAristaAutomatica
* - Recibe:
*   * `origen` (`int`): ID del primer nodo.
*   * `destino` (`int`): ID del segundo nodo.
* - Flujo de datos:
*   1. Busca `nodoOrigen` y `nodoDestino` por sus IDs.
*   2. Si ambos existen, calcula la distancia euclidiana entre ellos, la multiplica por
*      `1.18` (factor de ajuste vial/curvatura) y redondea a entero (`peso`).
*   3. Asegura que el peso sea al menos `1` y registra la conexión bidireccional.
 */
void grafo::agregarAristaAutomatica(int origen, int destino) {
	//  Paso 1: Buscar ambos nodos 
	nodo* nodoOrigen = buscarNodo(origen);
	nodo* nodoDestino = buscarNodo(destino);
	if (nodoOrigen && nodoDestino) {
		//  Paso 2: Calcular peso estimado según distancia euclidiana * 1.18 
		int peso = static_cast<int>(std::round(nodoOrigen->distanciaEuclidiana(nodoDestino) * 1.18));
		//  Paso 3: Garantizar peso mínimo de 1 y crear arista bidireccional 
		if (peso < 1) peso = 1;
		agregarAristaBidireccional(origen, destino, peso);
	}
}

/* 
buscarNodo
* - Recibe:
*   * `valor` (`int`): identificador del nodo que se desea encontrar.
* - Entrega / Devuelve: Un puntero a `nodo` (`nodo*`) que apunta al nodo encontrado,
*   o `nullptr` si no existe ningún nodo con ese ID.
* - Flujo de datos:
*   1. Revisa secuencialmente la lista `nodos`.
*   2. Si encuentra un nodo cuyo `getDato()` coincide con `valor`, devuelve su puntero.
*   3. Si termina el recorrido sin hallarlo, devuelve `nullptr`.
*/
nodo* grafo::buscarNodo(int valor) const {
	//  Paso 1 y 2: Recorrer nodos y retornar coincidencia si existe 
	for (nodo* n : nodos) {
		if (n->getDato() == valor) return n;
	}
	//  Paso 3: Devolver nullptr si no se encontró el nodo 
	return nullptr;
}

/*
eliminarArista
* - Recibe:
*   * `origen` (`int`): ID del nodo desde donde parte la arista.
*   * `destino` (`int`): ID del nodo hacia donde apunta la arista.
* - Flujo de datos:
*   1. Busca los nodos correspondientes a `origen` y `destino`.
*   2. Si ambos existen, pide a `nodoOrigen` que elimine la arista hacia `nodoDestino`.
*/
void grafo::eliminarArista(int origen, int destino) {
	//  Paso 1: Localizar nodos de origen y destino 
	nodo* nodoOrigen = buscarNodo(origen);
	nodo* nodoDestino = buscarNodo(destino);
	//  Paso 2: Eliminar arista saliente si ambos nodos existen 
	if (nodoOrigen && nodoDestino) {
		nodoOrigen->eliminarArista(nodoDestino);
	}
}

/* 
eliminarNodo
* - Recibe:
*   * `valor` (`int`): identificador del nodo que se quiere quitar del grafo.
* - Flujo de datos:
*   1. Busca el nodo con `valor`; si no existe, termina sin cambios.
*   2. Recorre todos los nodos del grafo para borrar cualquier arista dirigida hacia el nodo objetivo.
*   3. Busca el puntero del nodo dentro de `nodos`, lo quita del vector y libera su memoria con `delete`.
 */
void grafo::eliminarNodo(int valor) {
	// Paso 1: Buscar el nodo a eliminar y salir si no existe 
	nodo* nodoAEliminar = buscarNodo(valor);
	if (!nodoAEliminar) return;
	
	//  Paso 2: Eliminar referencias/aristas hacia ese nodo en el resto del grafo 
	for (nodo* n : nodos) {
		n->eliminarArista(nodoAEliminar);
	}
	
	//  Paso 3: Quitar el nodo del vector y liberar su memoria
	for (auto it = nodos.begin(); it != nodos.end(); ++it) {
		if (*it == nodoAEliminar) {
			nodos.erase(it);
			break;
		}
	}
	delete nodoAEliminar;
}

/* 
getNodos
* - Entrega / Devuelve: Una referencia constante (`const std::vector<nodo*>&`) al
*   vector interno de punteros a todos los nodos del grafo (representa el catálogo
*   actual de nodos sin permitir modificar el vector desde fuera).
* - Flujo de datos:
*   1. Devuelve directamente la referencia constante a `nodos`.
 */
const std::vector<nodo*>& grafo::getNodos() const {
	//  Paso 1: Retornar referencia constante al vector de nodos 
	return nodos;
}

/* 
obtenerNodosPorTipo
* - Recibe:
*   * `tipo` (`TipoNodo`): el tipo de nodo que se desea filtrar (por ejemplo,
*     `DEPOSITO`, `INTERSECCION` o `ENTREGA`).
* - Entrega / Devuelve: Un `std::vector<nodo*>` con los punteros a todos los nodos
*   que coinciden con el `tipo` solicitado.
* - Flujo de datos:
*   1. Crea un vector vacío `res` para almacenar coincidencias.
*   2. Recorre `nodos` y añade a `res` aquellos cuyo `getTipo()` sea igual a `tipo`.
*   3. Devuelve `res`.
 */
std::vector<nodo*> grafo::obtenerNodosPorTipo(TipoNodo tipo) const {
	// Paso 1: Preparar vector de resultados 
	std::vector<nodo*> res;
	//  Paso 2: Filtrar nodos que coincidan con el tipo buscado 
	for (nodo* n : nodos) {
		if (n->getTipo() == tipo) res.push_back(n);
	}
	//  Paso 3: Devolver los nodos encontrados 
	return res;
}

/* 
reiniciarColores
* - Flujo de datos:
*   1. Recorre cada nodo en `nodos` y llama a `setColorZona(-1)`.
*  */
void grafo::reiniciarColores() {
	//  Paso 1: Asignar color -1 a cada nodo 
	for (nodo* n : nodos) n->setColorZona(-1);
}

/* 
* LEYENDA: grafo::cantidadNodos
* - Entrega / Devuelve: Un entero (`int`) que representa el número total de nodos
*   actualmente almacenados en el grafo.
* - Flujo de datos:
*   1. Obtiene el tamaño de `nodos`, lo convierte a `int` y lo devuelve.
*  */
int grafo::cantidadNodos() const {
	//  Paso 1: Convertir tamaño del vector a int y retornarlo 
	return static_cast<int>(nodos.size());
}

/* 
grafo::cantidadAristasUnicas
* - Entrega / Devuelve: Un entero (`int`) que representa la cantidad aproximada/única
*   de conexiones no dirigidas (asumiendo aristas bidireccionales guardadas en ambos
*   sentidos, por eso divide entre 2).
* - Flujo de datos:
*   1. Suma la cantidad de adyacentes de cada nodo en `nodos`.
*   2. Divide el total entre `2` y devuelve el resultado.
 */
int grafo::cantidadAristasUnicas() const {
	//  Paso 1: Sumar todas las adyacencias registradas por nodo 
	int total = 0;
	for (nodo* n : nodos) total += static_cast<int>(n->getAdyacentes().size());
	//  Paso 2: Dividir entre 2 para estimar conexiones únicas y retornar 
	return total / 2;
}



/* 
cargarDesdeTXT
* - Recibe:
*   * `rutaArchivo` (`const std::string&`): ruta o nombre del archivo `.txt` a leer.
*   * `errorOut` (`std::string&`): variable de salida por referencia donde se
*     escribe un mensaje descriptivo si ocurre algún error de apertura o formato.
* - Entrega / Devuelve: Un booleano (`bool`): `true` si el archivo se leyó y cargó
*   correctamente en el grafo, o `false` si hubo algún error.
* - Flujo de datos:
*   1. Abre el archivo; si falla, guarda el mensaje en `errorOut` y devuelve `false`.
*   2. Define estructuras temporales y lee línea por línea ignorando líneas vacías o
*      comentarios (`#`).
*   3. Si la línea empieza con `NODO`, extrae id, tipo, coordenadas, demanda y nombre;
*      cuenta depósitos y entregas, y guarda el registro en `nodosLeidos`.
*   4. Si la línea empieza con `ARISTA`, extrae origen, destino y peso opcional, y lo
*      guarda en `aristasLeidas`.
*   5. Valida que haya al menos 1 depósito y 1 entrega; si no, reporta error y devuelve `false`.
*   6. Limpia el grafo actual, crea los nodos leídos y agrega las aristas (manuales o
*      automáticas según su peso), devolviendo `true` al finalizar.
 */
bool grafo::cargarDesdeTXT(const std::string& rutaArchivo, std::string& errorOut) {
	//  Paso 1: Intentar abrir el archivo de texto 
	std::ifstream archivo(rutaArchivo);
	if (!archivo.is_open()) {
		errorOut = "No se pudo abrir el archivo seleccionado.";
		return false;
	}
	
	//  Paso 2: Preparar estructuras temporales y leer líneas 
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
		
		//  Paso 3: Procesar línea tipo NODO 
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
			//  Paso 4: Procesar línea tipo ARISTA 
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
	
	//  Paso 5: Validar requisitos mínimos de depósitos y entregas 
	if (cantDepots == 0 || cantEntregas == 0) {
		errorOut = "El archivo .txt debe contener al menos 1 DEPOSITO y 1 ENTREGA.";
		return false;
	}
	
	//  Paso 6: Reemplazar contenido del grafo con los datos leídos y retornar true 
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

/* 
insertarEntregaConectada
* - Recibe:
*   * `nombre` (`const std::string&`): nombre del nuevo punto de entrega.
*   * `x` (`double`) y `y` (`double`): posición del nuevo punto en el plano.
*   * `demanda` (`int`): demanda o pedido que tendrá este nuevo punto.
* - Entrega / Devuelve: Un entero (`int`) que representa el nuevo ID asignado
*   al nodo de entrega recién creado y conectado.
* - Flujo de datos:
*   1. Calcula un `nuevoId` mayor que cualquiera de los IDs ya existentes.
*   2. Agrega el nuevo nodo de tipo `ENTREGA` al grafo y obtiene su puntero.
*   3. Calcula la distancia euclidiana desde el nuevo nodo hacia todos los demás
*      nodos y ordena los candidatos de más cercano a más lejano.
*   4. Conecta automáticamente el nuevo nodo con hasta los `2` vecinos más cercanos
*      y devuelve `nuevoId`.
*  */
int grafo::insertarEntregaConectada(const std::string& nombre, double x, double y, int demanda) {
	//  Paso 1: Generar un ID nuevo que no colisione con los existentes 
	int nuevoId = 1;
	for (nodo* n : nodos) {
		if (n->getDato() >= nuevoId) nuevoId = n->getDato() + 1;
	}
	
	//  Paso 2: Crear el nodo de entrega y obtener su puntero 
	agregarNodo(nuevoId, nombre, x, y, ENTREGA, demanda);
	nodo* nuevoNodo = buscarNodo(nuevoId);
	
	//  Paso 3: Calcular distancias a los demás nodos y ordenar por cercanía 
	std::vector<std::pair<double, int>> cercanos;
	for (nodo* n : nodos) {
		if (n->getDato() == nuevoId) continue;
		cercanos.push_back({nuevoNodo->distanciaEuclidiana(n), n->getDato()});
	}
	std::sort(cercanos.begin(), cercanos.end());
	
	//  Paso 4: Conectar con hasta 2 nodos más cercanos y devolver nuevoId 
	int enlaces = std::min(2, static_cast<int>(cercanos.size()));
	for (int i = 0; i < enlaces; ++i) {
		agregarAristaAutomatica(nuevoId, cercanos[i].second);
	}
	return nuevoId;
}


/* 
 grafo::Dijkstra
* - Recibe:
*   * `valorInicial` (`int`): ID del nodo desde donde comienza el cálculo de caminos
*     más cortos.
* - Entrega / Devuelve: Una estructura `ResultadoDijkstra` que contiene:
*   * `distancias`: distancia mínima acumulada desde `valorInicial` a cada nodo.
*   * `predecesor`: nodo anterior en la ruta más corta hacia cada nodo.
*   * `huboPesoNegativo`: indica si se detectó alguna arista con peso negativo.
*   * `falloDePropagacion`: indica si se intentó mejorar un nodo ya marcado como visitado.
*   * `tiempoMicros`: duración de la ejecución en microsegundos.
* - Flujo de datos:
*   1. Registra el tiempo inicial e inicializa `res` con valores por defecto; si
*      `valorInicial` no existe, devuelve `res` vacío.
*   2. Inicializa distancias en infinito (excepto `valorInicial` en `0`) y marca
*      todos los nodos como no visitados.
*   3. Inserta `{0, nodoInicial}` en una cola de prioridad mínima (`min-heap`).
*   4. Extrae el nodo más cercano pendiente; si ya fue visitado, lo omite y si no,
*      lo marca como visitado.
*   5. Revisa sus vecinos: detecta pesos negativos, calcula `nuevaDistancia` y, si
*      mejora la distancia conocida, actualiza distancia, predecesor y mete el vecino
*      a la cola.
*   6. Calcula `tiempoMicros` al terminar el bucle y devuelve `res`.
 */
ResultadoDijkstra grafo::Dijkstra(int valorInicial) const {
	//  Paso 1: Medir tiempo de inicio e inicializar estructura de resultado 
	double tInicio = tiempoActualMicros();
	
	ResultadoDijkstra res;
	res.huboPesoNegativo = false;
	res.falloDePropagacion = false;
	res.tiempoMicros = 0.0;
	
	nodo* nodoInicial = buscarNodo(valorInicial);
	if (!nodoInicial) return res;
	
	//  Paso 2: Inicializar distancias y estado de visitado para todos los nodos 
	std::map<int, bool> visitado;
	for (auto nod : nodos) {
		res.distancias[nod->getDato()] = std::numeric_limits<int>::max();
		visitado[nod->getDato()] = false;
	}
	res.distancias[valorInicial] = 0;
	
	//  Paso 3: Preparar cola de prioridad mínima con el nodo inicial 
	using p = std::pair<int, nodo*>;
	std::priority_queue<p, std::vector<p>, std::greater<p>> cola;
	cola.push({0, nodoInicial});
	
	//  Paso 4: Extraer el nodo con menor distancia acumulada pendiente 
	while (!cola.empty()) {
		auto elemento = cola.top();
		int disActual = elemento.first;
		nodo* nodoActual = elemento.second;
		cola.pop();
		
		int u = nodoActual->getDato();
		if (visitado[u]) continue;
		visitado[u] = true;
		
		//  Paso 5: Relajar aristas hacia vecinos y actualizar rutas 
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
	
	//  Paso 6: Guardar tiempo transcurrido y devolver resultado 
	res.tiempoMicros = tiempoActualMicros() - tInicio;
	return res;
}

/* 
reconstruirCamino
* - Recibe:
*   * `origen` (`int`): ID del nodo inicial de la ruta.
*   * `destino` (`int`): ID del nodo final al que se quiere llegar.
*   * `predecesor` (`const std::map<int, int>&`): mapa que asocia cada nodo con
*     el nodo previo en el camino más corto.
* - Entrega / Devuelve: Un `std::vector<int>` con la secuencia ordenada de IDs de
*   nodos desde `origen` hasta `destino` (vacío si no hay camino válido).
* - Flujo de datos:
*   1. Si `origen` y `destino` son iguales, devuelve un vector con solo `origen`.
*   2. Verifica que `destino` tenga predecesor registrado; si no, devuelve vector vacío.
*   3. Retrocede desde `destino` usando `predecesor` hasta llegar a `origen` (controlando
*      un límite de pasos para evitar ciclos).
*   4. Si no se logró alcanzar `origen`, devuelve vector vacío; si sí, agrega `origen`,
*      invierte el vector para dejarlo en orden de ida y lo devuelve.
*  */
std::vector<int> grafo::reconstruirCamino(int origen, int destino, const std::map<int, int>& predecesor) const {
	//  Paso 1: Manejar caso trivial donde origen y destino coinciden 
	std::vector<int> ruta;
	if (origen == destino) {
		ruta.push_back(origen);
		return ruta;
	}
	//  Paso 2: Comprobar si existe predecesor para el destino 
	if (predecesor.find(destino) == predecesor.end()) return ruta;
	
	//  Paso 3: Caminar hacia atrás desde destino hasta origen 
	int actual = destino;
	int pasos = 0;
	while (actual != origen && predecesor.count(actual) && pasos <= static_cast<int>(nodos.size())) {
		ruta.push_back(actual);
		actual = predecesor.at(actual);
		pasos++;
	}
	//  Paso 4: Validar llegada a origen, invertir secuencia y retornar 
	if (actual != origen) return std::vector<int>();
	ruta.push_back(origen);
	std::reverse(ruta.begin(), ruta.end());
	return ruta;
}

/* 
precalcularMatrizDijkstra
* - Entrega / Devuelve: Un número decimal (`double`) que representa el tiempo total
*   tardado en microsegundos para calcular las rutas desde todos los nodos.
*   Además, llena `matrizDistancias` y `matrizPredecesores` del grafo.
* - Flujo de datos:
*   1. Registra el tiempo inicial `t0` y limpia `matrizDistancias` y `matrizPredecesores`.
*   2. Recorre cada nodo del grafo, ejecuta `Dijkstra(id)` y guarda su mapa de
*      distancias y predecesores indexado por `id`.
*   3. Devuelve el tiempo total transcurrido en microsegundos.
*  */
double grafo::precalcularMatrizDijkstra() {
	//  Paso 1: Marcar inicio y limpiar matrices previas 
	double t0 = tiempoActualMicros();
	matrizDistancias.clear();
	matrizPredecesores.clear();
	
	//  Paso 2: Ejecutar Dijkstra desde cada nodo y almacenar resultados
	for (nodo* n : nodos) {
		int id = n->getDato();
		ResultadoDijkstra res = Dijkstra(id);
		matrizDistancias[id] = res.distancias;
		matrizPredecesores[id] = res.predecesor;
	}
	
	//  Paso 3: Retornar tiempo de ejecución en microsegundos 
	return tiempoActualMicros() - t0;
}


/* 
colorearZonas
* - Recibe:
*   * `umbralMetros` (`int`): distancia límite (en metros) usada para definir cuándo
*     dos entregas entran en conflicto.
*   * `capacidadMaxZona` (`int`): carga/demanda máxima permitida por zona o color.
*   * `metodo` (`int`): algoritmo de coloreado (`0` para Welsh-Powell, otro valor para DSATUR).
*   * `modoConflicto` (`int`): criterio de conflicto (`0` si distancia > umbral para
*     agrupar cercanos; `1` si distancia <= umbral para separar vecinos cercanos).
* - Entrega / Devuelve: Una estructura `ResultadoColoreado` con:
*   * Cantidad de colores/zonas únicas utilizadas.
*   * `totalConflictos` detectados entre pares de entregas.
*   * El mapa de `conflictos` por nodo.
*   * El tiempo de ejecución en microsegundos.
*   Además, actualiza el color de zona (`setColorZona`) en cada nodo de entrega.
* - Flujo de datos:
*   1. Si `matrizDistancias` está vacía, llama a `precalcularMatrizDijkstra()`.
*   2. Inicia el cronómetro y obtiene todos los nodos de tipo `ENTREGA`.
*   3. Compara cada par de entregas usando `matrizDistancias`, registra conflictos
*      según `modoConflicto` y cuenta el total.
*   4. Ejecuta `colorearWelshPowell` o `colorearDSATUR` según `metodo` para obtener
*      la asignación de colores.
*   5. Asigna el color a cada entrega, registra los colores únicos, mide el tiempo
*      y devuelve `ResultadoColoreado`.
 */
ResultadoColoreado grafo::colorearZonas(int umbralMetros, int capacidadMaxZona, int metodo, int modoConflicto) {
	//  Paso 1: Asegurar que exista la matriz de distancias 
	if (matrizDistancias.empty()) precalcularMatrizDijkstra();
	
	//  Paso 2: Iniciar medición de tiempo y obtener nodos de entrega 
	double tInicio = tiempoActualMicros();
	
	std::vector<nodo*> entregas = obtenerNodosPorTipo(ENTREGA);
	std::map<int, std::set<int>> conflictos;
	for (nodo* e : entregas) conflictos[e->getDato()] = std::set<int>();
	
	const int INF = std::numeric_limits<int>::max();
	int totalConflictos = 0;
	
	//  Paso 3: Construir grafo de conflictos entre pares de entregas 
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
	
	//  Paso 4: Asignar colores usando Welsh-Powell o DSATUR 
	std::map<int, int> asignacion = (metodo == 0)
		? colorearWelshPowell(entregas, conflictos, capacidadMaxZona)
		: colorearDSATUR(entregas, conflictos, capacidadMaxZona);
	
	//  Paso 5: Guardar color en cada entrega, contar colores únicos y retornar 
	std::set<int> coloresUnicos;
	for (nodo* e : entregas) {
		int c = asignacion[e->getDato()];
		e->setColorZona(c);
		coloresUnicos.insert(c);
	}
	
	double micros = tiempoActualMicros() - tInicio;
	
	return {
		static_cast<int>(coloresUnicos.size()),
			totalConflictos,
			conflictos,
			micros
	};
}

/* 
colorearWelshPowell
* - Recibe:
*   * `entregas` (`const std::vector<nodo*>&`): lista de nodos de entrega a colorear.
*   * `conflictos` (`const std::map<int, std::set<int>>&`): conjunto de nodos en
*     conflicto para cada entrega.
*   * `capacidadMaxZona` (`int`): carga máxima permitida por color/zona.
* - Entrega / Devuelve: Un `std::map<int, int>` que asocia cada ID de entrega
*   (`clave`) con el número de color/zona asignado (`valor`).
* - Flujo de datos:
*   1. Copia `entregas` a `ordenados` y los ordena de mayor a menor cantidad de
*      conflictos (y ante empate, por mayor `demanda`).
*   2. Recorre cada entrega ordenada y junta en `prohibidos` los colores ya usados
*      por sus vecinos en conflicto.
*   3. Busca el menor color `>= 0` que no esté prohibido y que no supere
*      `capacidadMaxZona` (salvo que la zona esté vacía, `cargaPorColor[color] == 0`).
*   4. Registra el color del nodo, suma su demanda a `cargaPorColor[color]` y
*      devuelve `colorNodo`.
 */
std::map<int, int> grafo::colorearWelshPowell(
											  const std::vector<nodo*>& entregas,
											  const std::map<int, std::set<int>>& conflictos,
											  int capacidadMaxZona) const {
	
	//  Paso 1: Ordenar entregas por mayor número de conflictos y luego por demanda 
	std::vector<nodo*> ordenados = entregas;
	std::sort(ordenados.begin(), ordenados.end(), [&](nodo* a, nodo* b) {
		size_t gA = conflictos.at(a->getDato()).size();
		size_t gB = conflictos.at(b->getDato()).size();
		if (gA != gB) return gA > gB;
		return a->getDemanda() > b->getDemanda();
	});
	
	std::map<int, int> colorNodo;
	std::map<int, int> cargaPorColor;
	
	//  Paso 2: Recorrer entregas y recolectar colores prohibidos de vecinos 
	for (nodo* uNodo : ordenados) {
		int u = uNodo->getDato();
		int demandaU = uNodo->getDemanda();
		std::set<int> prohibidos;
		
		for (int vecino : conflictos.at(u)) {
			if (colorNodo.count(vecino)) prohibidos.insert(colorNodo[vecino]);
		}
		
		//  Paso 3: Elegir color válido respetando conflictos y capacidad máxima 
		int color = 0;
		while (prohibidos.count(color) ||
			   (cargaPorColor[color] + demandaU > capacidadMaxZona && cargaPorColor[color] > 0)) {
			color++;
		}
			   
			   //  Paso 4: Asignar color, acumular carga y devolver mapa final 
			   colorNodo[u] = color;
			   cargaPorColor[color] += demandaU;
	}
	
	return colorNodo;
}

/* 
colorearDSATUR
* - Recibe:
*   * `entregas` (`const std::vector<nodo*>&`): lista de nodos de entrega a colorear.
*   * `conflictos` (`const std::map<int, std::set<int>>&`): relaciones de conflicto
*     entre entregas.
*   * `capacidadMaxZona` (`int`): límite de carga por color/zona.
* - Entrega / Devuelve: Un `std::map<int, int>` con el color asignado a cada ID de
*   entrega.
* - Flujo de datos:
*   1. Inicializa mapas de colores y cargas, y coloca todas las entregas en `pendientes`.
*   2. Mientras queden nodos pendientes, calcula para cada candidato su saturación
*      (`sat`: cuántos colores distintos ya tienen sus vecinos coloreados) y su
*      grado de conflicto (`grado`).
*   3. Selecciona el nodo con mayor saturación (desempatando por mayor grado).
*   4. Obtiene los colores prohibidos por vecinos ya coloreados y busca un color
*      libre que respete `capacidadMaxZona`.
*   5. Asigna el color, actualiza la carga de esa zona, elimina el nodo de
*      `pendientes` y, al terminar el ciclo, devuelve `colorNodo`.
 */
std::map<int, int> grafo::colorearDSATUR(
										 const std::vector<nodo*>& entregas,
										 const std::map<int, std::set<int>>& conflictos,
										 int capacidadMaxZona) const {
	
	//  Paso 1: Inicializar estructuras y conjunto de nodos pendientes 
	std::map<int, int> colorNodo;
	std::map<int, int> cargaPorColor;
	std::set<nodo*> pendientes(entregas.begin(), entregas.end());
	
	while (!pendientes.empty()) {
		//  Paso 2 y 3: Elegir nodo pendiente con mayor saturación (y mayor grado) 
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
		
		//  Paso 4: Determinar colores prohibidos y buscar color compatible con capacidad 
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
			   
			   //  Paso 5: Guardar asignación, actualizar carga y retirar de pendientes 
			   colorNodo[u] = color;
			   cargaPorColor[color] += demandaU;
			   pendientes.erase(mejorNodo);
	}
	
	return colorNodo;
}


/* 
buscarDepotMasCercano
* - Recibe:
*   * `idsZona` (`const std::vector<int>&`): lista de IDs de entregas que
*     pertenecen a una misma zona.
* - Entrega / Devuelve: Un entero (`int`) con el ID del depósito (`DEPOSITO`) que
*   tiene la menor suma de distancias hacia las entregas de `idsZona` (o `-1` si no
*   hay depósitos).
* - Flujo de datos:
*   1. Obtiene todos los nodos de tipo `DEPOSITO`; si no hay ninguno, devuelve `-1`.
*   2. Inicializa `mejorDepot` con el primero y `menorSuma` con un valor máximo.
*   3. Para cada depósito, suma las distancias válidas (distintas de `INF`) desde
*      ese depósito hacia cada entrega de `idsZona`.
*   4. Si la suma es menor que `menorSuma`, actualiza el mínimo y `mejorDepot`, y
*      al final devuelve `mejorDepot`.
 */
int grafo::buscarDepotMasCercano(const std::vector<int>& idsZona) const {
	//  Paso 1: Obtener depósitos disponibles
	std::vector<nodo*> depots = obtenerNodosPorTipo(DEPOSITO);
	if (depots.empty()) return -1;
	
	//  Paso 2: Inicializar mejor depósito y menor distancia acumulada 
	int mejorDepot = depots.front()->getDato();
	long long menorSuma = std::numeric_limits<long long>::max();
	const int INF = std::numeric_limits<int>::max();
	
	//  Paso 3 y 4: Evaluar suma de distancias por depósito y elegir el menor 
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

/* 
planificarRutasFlota
* - Entrega / Devuelve: Un `std::vector<RutaZona>` donde cada elemento describe
*   la planificación de una zona coloreada: ID de zona, depósito asignado, carga
*   total, y los resultados/tiempos de dos métodos de ruteo (Dijkstra repetido
*   paso a paso vs. heurística TSP con vecino más cercano + 2-Opt).
* - Flujo de datos:
*   1. Agrupa las entregas coloreadas (`colorZona >= 0`) por número de zona.
*   2. Para cada zona, calcula su `cargaTotal` sumando demandas y busca el depósito
*      más cercano (`idDepot`).
*   3. Ejecuta el método A (Dijkstra repetido): parte de `idDepot`, calcula
*      `Dijkstra(actual)` en cada paso, elige el pendiente más cercano, acumula
*      distancia y calles (`reconstruirCamino`), y al final regresa a `idDepot`.
*   4. Ejecuta el método B (TSP + 2-Opt): construye un tour inicial con
*      `construirTourTSP`, lo mejora con `mejorar2Opt` si tiene más de 4 nodos,
*      calcula su costo (`evaluarCostoTour`) y expande sus calles (`expandirCalles`).
*   5. Agrega la estructura `RutaZona` al vector `rutas` y lo devuelve al terminar.
*  */
std::vector<RutaZona> grafo::planificarRutasFlota() const {
	//  Paso 1: Agrupar entregas por color de zona 
	std::map<int, std::vector<int>> entregasPorZona;
	for (nodo* e : obtenerNodosPorTipo(ENTREGA)) {
		if (e->getColorZona() >= 0) {
			entregasPorZona[e->getColorZona()].push_back(e->getDato());
		}
	}
	
	std::vector<RutaZona> rutas;
	const int INF = std::numeric_limits<int>::max();
	
	//  Paso 2: Calcular carga total y depósito más cercano para cada zona 
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
		
		//  Paso 3: Método A - Dijkstra repetido paso a paso 
		double t0 = tiempoActualMicros();
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
		
		double microsDijk = tiempoActualMicros() - t0;
		
		//  Paso 4: Método B - Heurística TSP (Vecino Más Cercano + 2-Opt) 
		double t2 = tiempoActualMicros();
		std::vector<int> tour = construirTourTSP(idDepot, idsZona);
		if (tour.size() > 4) tour = mejorar2Opt(tour);
		int distTSP = evaluarCostoTour(tour);
		std::vector<int> callesTSP = expandirCalles(tour);
		double microsTSP = tiempoActualMicros() - t2;
		
		// Paso 5: Registrar ruta completa de la zona 
		rutas.push_back({
			idZona, idDepot, cargaTotal,
				visitasDijk, callesDijk, distDijk, microsDijk,
				tour, callesTSP, distTSP, microsTSP
		});
	}
	
	return rutas;
}

/* 
construirTourTSP
* - Recibe:
*   * `idDepot` (`int`): ID del depósito donde inicia y termina el recorrido.
*   * `idsZona` (`const std::vector<int>&`): IDs de las entregas a visitar.
* - Entrega / Devuelve: Un `std::vector<int>` con el orden de visita (comenzando
*   en `idDepot`, pasando por las entregas según el vecino más cercano en
*   `matrizDistancias` y regresando a `idDepot`).
* - Flujo de datos:
*   1. Coloca `idsZona` en un conjunto `pendientes`, inicia en `actual = idDepot` y
*      lo agrega a `tour`.
*   2. Mientras haya nodos pendientes, busca en `matrizDistancias[actual]` el nodo
*      pendiente con menor distancia.
*   3. Si encuentra uno, lo agrega a `tour`, lo quita de `pendientes` y avanza
*      `actual` hacia él (si no hay ninguno alcanzable, corta el ciclo).
*   4. Agrega `idDepot` al final para cerrar el recorrido y devuelve `tour`.
*/
std::vector<int> grafo::construirTourTSP(int idDepot, const std::vector<int>& idsZona) const {
	//  Paso 1: Inicializar pendientes y comenzar el tour en el depósito 
	std::vector<int> tour;
	std::set<int> pendientes(idsZona.begin(), idsZona.end());
	int actual = idDepot;
	tour.push_back(actual);
	
	//  Paso 2 y 3: Seleccionar iterativamente el nodo pendiente más cercano 
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
	//  Paso 4: Cerrar el tour volviendo al depósito y retornar 
	tour.push_back(idDepot);
	return tour;
}

/* 
* LEYENDA: grafo::mejorar2Opt
* - Recibe:
*   * `tourInicial` (`const std::vector<int>&`): secuencia de nodos del recorrido
*     inicial (se asume que empieza y termina en el depósito).
* - Entrega / Devuelve: Un `std::vector<int>` con el mejor recorrido encontrado
*   tras aplicar intercambios 2-Opt que reduzcan el costo total.
* - Flujo de datos:
*   1. Copia `tourInicial` en `mejor` y habilita la bandera `mejoro = true`.
*   2. Mientras haya mejoras, calcula `mejorCosto` del recorrido actual.
*   3. Prueba invertir subsegmentos internos entre posiciones `i` y `k` (sin tocar
*      los extremos `0` y `n - 1`) y evalúa el `nuevoCosto` con `evaluarCostoTour`.
*   4. Si `nuevoCosto < mejorCosto`, guarda el nuevo tour, actualiza `mejorCosto` y
*      marca `mejoro = true`; cuando ya no haya mejoras, devuelve `mejor`.
 */
std::vector<int> grafo::mejorar2Opt(const std::vector<int>& tourInicial) const {
	//  Paso 1: Copiar tour inicial y preparar ciclo de búsqueda local 
	std::vector<int> mejor = tourInicial;
	bool mejoro = true;
	int n = static_cast<int>(mejor.size());
	
	while (mejoro) {
		//  Paso 2: Calcular costo base de la ruta actual 
		mejoro = false;
		int mejorCosto = evaluarCostoTour(mejor);
		//  Paso 3 y 4: Invertir tramos [i..k] y conservar si el costo mejora 
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

/* 
evaluarCostoTour
* - Recibe:
*   * `tour` (`const std::vector<int>&`): lista ordenada de nodos que forman un
*     recorrido.
* - Entrega / Devuelve: Un entero (`int`) que representa la distancia o costo
*   total sumado a lo largo de todos los tramos consecutivos del `tour`.
* - Flujo de datos:
*   1. Inicializa `total` en `0`.
*   2. Recorre cada par consecutivo `(tour[i], tour[i + 1])` y consulta su
*      distancia en `matrizDistancias`.
*   3. Si existe y no es `INF`, suma esa distancia a `total`, y al terminar lo devuelve.
*  */
int grafo::evaluarCostoTour(const std::vector<int>& tour) const {
	// Paso 1: Inicializar acumulador de costo 
	int total = 0;
	const int INF = std::numeric_limits<int>::max();
	// Paso 2 y 3: Sumar distancias de cada tramo consecutivo y retornar 
	for (size_t i = 0; i + 1 < tour.size(); ++i) {
		auto itU = matrizDistancias.find(tour[i]);
		if (itU != matrizDistancias.end() && itU->second.count(tour[i + 1])) {
			int d = itU->second.at(tour[i + 1]);
			if (d != INF) total += d;
		}
	}
	return total;
}

/* 
expandirCalles
* - Recibe:
*   * `tour` (`const std::vector<int>&`): secuencia de paradas principales (nodos)
*     que componen el recorrido.
* - Entrega / Devuelve: Un `std::vector<int>` con todos los IDs de nodos intermedios
*   y finales (calles/puntos de paso) que unen cada salto del `tour` sin duplicar
*   el nodo de empalme entre tramos.
* - Flujo de datos:
*   1. Crea un vector vacío `calles`.
*   2. Para cada tramo `(tour[i], tour[i + 1])`, busca el mapa de predecesores de
*      `tour[i]` en `matrizPredecesores` y reconstruye el camino con `reconstruirCamino`.
*   3. Si el tramo no está vacío, elimina su primer elemento cuando `calles` ya tiene
*      elementos (para no repetir el nodo de conexión) y anexa el resto a `calles`.
*   4. Devuelve `calles`.
*  */
std::vector<int> grafo::expandirCalles(const std::vector<int>& tour) const {
	//  Paso 1: Preparar vector expandido de nodos/calles 
	std::vector<int> calles;
	//  Paso 2 y 3: Reconstruir cada tramo y concatenar sin duplicar empalmes 
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
	//  Paso 4: Retornar secuencia completa de calles 
	return calles;
}

