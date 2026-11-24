#include "Application.h"
#include "MainWindow.h"
#include <wx/msgdlg.h>
#include <fstream>
#include <string>

IMPLEMENT_APP(mxApplication)
	
	static std::string resolverRutaDefault(const std::string& nombreArchivo) {
	std::ifstream f1(nombreArchivo);
	if (f1.good()) return nombreArchivo;
	
	std::string padre = "../" + nombreArchivo;
	std::ifstream f2(padre);
	if (f2.good()) return padre;
	
	return nombreArchivo;
}
	
	bool mxApplication::OnInit() {
		// 1. Opciones y valores por defecto del sistema
		ConfigApp config;
		config.rutaMapaDefault = resolverRutaDefault("mapa_default.txt");
		config.algoritmoColoreadoDefault = 1; // 0 = Welsh-Powell, 1 = DSATUR
		config.modoConflictoDefault = 0;      // 0 = Lejania (compactas), 1 = Vecindad
		config.umbralConflictoDefault = 370;  // metros
		config.capacidadZonaDefault = 45;     // paquetes maximos por vehiculo
		config.modoRuteoDefault = 0;          // 0 = TSP + 2-Opt, 1 = Dijkstra Repetido
		
		// 2. Carga inicial del mapa desde archivo .txt
		std::string errorCarga;
		if (!miGrafo.cargarDesdeTXT(config.rutaMapaDefault, errorCarga)) {
			wxMessageBox(
						 "No se encontro o no se pudo leer '" + config.rutaMapaDefault + "'.\n"
						 "Detalle: " + errorCarga + "\n\n"
						 "Puedes cargar cualquier archivo desde el boton 'Cargar Mapa (.txt)...'.",
						 "Aviso de carga inicial",
						 wxOK | wxICON_INFORMATION
						 );
		}
		
		// 3. Lanzar interfaz grafica pasandole el grafo y la configuracion
		MainWindow* ventanaPrincipal = new MainWindow(&miGrafo, config);
		ventanaPrincipal->Show(true);
		return true;
	}
