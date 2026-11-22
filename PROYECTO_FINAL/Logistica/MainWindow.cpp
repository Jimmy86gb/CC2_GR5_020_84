#include "MainWindow.h"
#include <wx/msgdlg.h>
#include <wx/dcclient.h>
#include <wx/pen.h>
#include <wx/brush.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <sstream>
#include <iomanip>
#include <cmath>

using namespace std;

// ============================================================================
// LIENZO DEL MAPA (MapaCanvas)
// ============================================================================
MapaCanvas::MapaCanvas(wxWindow* parent, wxPoint pos, wxSize size, grafo* grafoRef)
	: wxPanel(parent, wxID_ANY, pos, size, wxBORDER_SIMPLE),
	miGrafo(grafoRef),
	zonaFiltro(-1),
	mostrarConflictos(false),
	mostrarPesos(true) {
	
	SetBackgroundColour(wxColour(245, 248, 252));
	Bind(wxEVT_PAINT, &MapaCanvas::OnPaint, this);
	Bind(wxEVT_LEFT_DOWN, &MapaCanvas::OnMouseLeftDown, this);
}

void MapaCanvas::actualizarDatosVisuales(
										 const map<int, set<int>>& conflictos,
										 const vector<RutaZona>& rutas) {
	grafoConflictos = conflictos;
	rutasActivas = rutas;
	Refresh();
}

void MapaCanvas::establecerCaminoResaltado(const vector<int>& camino) {
	caminoIndividual = camino;
	Refresh();
}

void MapaCanvas::setZonaFiltro(int zona) {
	zonaFiltro = zona;
	Refresh();
}

void MapaCanvas::setMostrarConflictos(bool mostrar) {
	mostrarConflictos = mostrar;
	Refresh();
}

void MapaCanvas::setMostrarPesos(bool mostrar) {
	mostrarPesos = mostrar;
	Refresh();
}

void MapaCanvas::setCallbackClicMapa(function<void(int, int)> callback) {
	onClicMapa = callback;
}

wxColour MapaCanvas::obtenerColorZona(int indiceZona) const {
	static const wxColour paleta[8] = {
		wxColour(220, 38, 38),
			wxColour(37, 99, 235),
			wxColour(5, 150, 105),
			wxColour(217, 119, 6),
			wxColour(124, 58, 237),
			wxColour(8, 145, 178),
			wxColour(190, 24, 93),
			wxColour(101, 163, 13)
	};
	if (indiceZona < 0) return wxColour(100, 116, 139);
	return paleta[indiceZona % 8];
}

void MapaCanvas::OnMouseLeftDown(wxMouseEvent& event) {
	if (onClicMapa) {
		onClicMapa(event.GetX(), event.GetY());
	}
}

void MapaCanvas::OnPaint(wxPaintEvent& event) {
	wxPaintDC dc(this);
	dc.SetBackground(wxBrush(wxColour(245, 248, 252)));
	dc.Clear();
	
	if (!miGrafo) return;
	
	wxSize tam = GetClientSize();
	
	// 1. Cuadricula cartografica
	dc.SetPen(wxPen(wxColour(228, 234, 242), 1, wxPENSTYLE_SOLID));
	for (int x = 0; x < tam.GetWidth(); x += 45) dc.DrawLine(x, 0, x, tam.GetHeight());
	for (int y = 0; y < tam.GetHeight(); y += 45) dc.DrawLine(0, y, tam.GetWidth(), y);
	
	// 2. Halos de cobertura por zona
	for (nodo* n : miGrafo->obtenerNodosPorTipo(ENTREGA)) {
		int z = n->getColorZona();
		if (z < 0) continue;
		if (zonaFiltro != -1 && zonaFiltro != z) continue;
		
		wxColour cZona = obtenerColorZona(z);
		wxColour cHalo(
					   (cZona.Red() + 255 * 3) / 4,
					   (cZona.Green() + 255 * 3) / 4,
					   (cZona.Blue() + 255 * 3) / 4
					   );
		dc.SetPen(wxPen(cZona, 1, wxPENSTYLE_DOT));
		dc.SetBrush(wxBrush(cHalo));
		dc.DrawCircle(static_cast<int>(n->getX()), static_cast<int>(n->getY()), 28);
	}
	
	// 3. Grafo de conflictos
	if (mostrarConflictos) {
		dc.SetPen(wxPen(wxColour(239, 68, 68), 1, wxPENSTYLE_DOT));
		for (const auto& par : grafoConflictos) {
			nodo* nodoU = miGrafo->buscarNodo(par.first);
			if (!nodoU) continue;
			for (int idV : par.second) {
				if (par.first < idV) {
					nodo* nodoV = miGrafo->buscarNodo(idV);
					if (nodoV) {
						dc.DrawLine(
									static_cast<int>(nodoU->getX()), static_cast<int>(nodoU->getY()),
									static_cast<int>(nodoV->getX()), static_cast<int>(nodoV->getY())
									);
					}
				}
			}
		}
	}
	
	// 4. Red vial OSM y pesos
	dc.SetFont(wxFont(8, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
	for (nodo* u : miGrafo->getNodos()) {
		for (const auto& par : u->getAdyacentes()) {
			nodo* v = par.first;
			int peso = par.second;
			if (u->getDato() < v->getDato()) {
				int x1 = static_cast<int>(u->getX());
				int y1 = static_cast<int>(u->getY());
				int x2 = static_cast<int>(v->getX());
				int y2 = static_cast<int>(v->getY());
				
				dc.SetPen(wxPen(wxColour(180, 192, 206), 4, wxPENSTYLE_SOLID));
				dc.DrawLine(x1, y1, x2, y2);
				
				dc.SetPen(wxPen(wxColour(226, 232, 240), 2, wxPENSTYLE_SOLID));
				dc.DrawLine(x1, y1, x2, y2);
				
				if (mostrarPesos) {
					int mx = (x1 + x2) / 2;
					int my = (y1 + y2) / 2;
					wxString txtPeso;
					txtPeso << peso << "m";
					dc.SetTextForeground(wxColour(100, 116, 139));
					dc.DrawText(txtPeso, mx - 10, my - 7);
				}
			}
		}
	}
	
	// 5. Rutas TSP por zona con flechas
	for (const RutaZona& ruta : rutasActivas) {
		int z = ruta.idZona;
		if (zonaFiltro != -1 && zonaFiltro != z) continue;
		
		wxColour colorFlota = obtenerColorZona(z);
		dc.SetPen(wxPen(colorFlota, 3, wxPENSTYLE_SOLID));
		dc.SetBrush(wxBrush(colorFlota));
		
		int offset = (z % 4) * 2 - 3;
		
		const vector<int>& camino = ruta.trazadoCalles;
		for (size_t i = 0; i + 1 < camino.size(); ++i) {
			nodo* u = miGrafo->buscarNodo(camino[i]);
			nodo* v = miGrafo->buscarNodo(camino[i + 1]);
			if (!u || !v) continue;
			
			int x1 = static_cast<int>(u->getX()) + offset;
			int y1 = static_cast<int>(u->getY()) + offset;
			int x2 = static_cast<int>(v->getX()) + offset;
			int y2 = static_cast<int>(v->getY()) + offset;
			
			dc.DrawLine(x1, y1, x2, y2);
			
			double mx = (x1 + x2) * 0.5;
			double my = (y1 + y2) * 0.5;
			double ang = std::atan2(static_cast<double>(y2 - y1), static_cast<double>(x2 - x1));
			double tamFlecha = 6.0;
			
			wxPoint flecha[3];
			flecha[0] = wxPoint(static_cast<int>(mx + tamFlecha * std::cos(ang)),
								static_cast<int>(my + tamFlecha * std::sin(ang)));
			flecha[1] = wxPoint(static_cast<int>(mx - tamFlecha * std::cos(ang - 0.6)),
								static_cast<int>(my - tamFlecha * std::sin(ang - 0.6)));
			flecha[2] = wxPoint(static_cast<int>(mx - tamFlecha * std::cos(ang + 0.6)),
								static_cast<int>(my - tamFlecha * std::sin(ang + 0.6)));
			dc.DrawPolygon(3, flecha);
		}
	}
	
	// 6. Camino corto resaltado (Dijkstra vs A*)
	if (caminoIndividual.size() >= 2) {
		dc.SetPen(wxPen(wxColour(250, 204, 21), 7, wxPENSTYLE_SOLID));
		for (size_t i = 0; i + 1 < caminoIndividual.size(); ++i) {
			nodo* u = miGrafo->buscarNodo(caminoIndividual[i]);
			nodo* v = miGrafo->buscarNodo(caminoIndividual[i + 1]);
			if (u && v) {
				dc.DrawLine(static_cast<int>(u->getX()), static_cast<int>(u->getY()),
							static_cast<int>(v->getX()), static_cast<int>(v->getY()));
			}
		}
		
		dc.SetPen(wxPen(wxColour(15, 23, 42), 2, wxPENSTYLE_SOLID));
		for (size_t i = 0; i + 1 < caminoIndividual.size(); ++i) {
			nodo* u = miGrafo->buscarNodo(caminoIndividual[i]);
			nodo* v = miGrafo->buscarNodo(caminoIndividual[i + 1]);
			if (u && v) {
				dc.DrawLine(static_cast<int>(u->getX()), static_cast<int>(u->getY()),
							static_cast<int>(v->getX()), static_cast<int>(v->getY()));
			}
		}
	}
	
	// 7. Dibujar nodos
	for (nodo* n : miGrafo->getNodos()) {
		int x = static_cast<int>(n->getX());
		int y = static_cast<int>(n->getY());
		int id = n->getDato();
		
		if (n->getTipo() == DEPOSITO) {
			dc.SetPen(wxPen(wxColour(250, 204, 21), 2, wxPENSTYLE_SOLID));
			dc.SetBrush(wxBrush(wxColour(15, 23, 42)));
			dc.DrawRoundedRectangle(x - 13, y - 13, 26, 26, 5);
			
			dc.SetFont(wxFont(8, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
			dc.SetTextForeground(wxColour(255, 255, 255));
			dc.DrawText("DEP", x - 10, y - 6);
			
			dc.SetTextForeground(wxColour(15, 23, 42));
			dc.DrawText("[1] DEPOT CENTRAL", x - 48, y + 15);
			
		} else if (n->getTipo() == INTERSECCION) {
			dc.SetPen(wxPen(wxColour(255, 255, 255), 1, wxPENSTYLE_SOLID));
			dc.SetBrush(wxBrush(wxColour(100, 116, 139)));
			dc.DrawCircle(x, y, 5);
			
			dc.SetFont(wxFont(7, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
			dc.SetTextForeground(wxColour(71, 85, 105));
			wxString lbl;
			lbl << id;
			dc.DrawText(lbl, x + 6, y - 6);
			
		} else if (n->getTipo() == ENTREGA) {
			wxColour cNodo = obtenerColorZona(n->getColorZona());
			dc.SetPen(wxPen(wxColour(255, 255, 255), 2, wxPENSTYLE_SOLID));
			dc.SetBrush(wxBrush(cNodo));
			dc.DrawCircle(x, y, 10);
			
			dc.SetFont(wxFont(8, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
			dc.SetTextForeground(wxColour(255, 255, 255));
			wxString idTxt;
			idTxt << id;
			dc.DrawText(idTxt, x - (id >= 10 ? 6 : 3), y - 6);
			
			dc.SetTextForeground(wxColour(15, 23, 42));
			wxString infoPoi;
			infoPoi << n->getNombre() << " (" << n->getDemanda() << "p)";
			dc.DrawText(infoPoi, x + 12, y - 7);
		}
	}
	
	// 8. Barra de leyenda superior
	dc.SetPen(wxPen(wxColour(203, 213, 225), 1, wxPENSTYLE_SOLID));
	dc.SetBrush(wxBrush(wxColour(255, 255, 255)));
	dc.DrawRoundedRectangle(8, 6, tam.GetWidth() - 16, 24, 4);
	dc.SetFont(wxFont(8, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
	dc.SetTextForeground(wxColour(30, 41, 59));
	dc.DrawText("MAPA VIAL OSM  |  Cuadro Negro = Depot  |  Circulos Coloreados = Entregas por Zona (Flota)  |  Clic Izquierdo = Capturar X,Y", 16, 11);
}

// ============================================================================
// VENTANA PRINCIPAL (MainWindow)
// ============================================================================
MainWindow::MainWindow()
	: wxFrame(NULL, wxID_ANY, "Proyecto 3 - Segmentacion y Logistica Ultima-Milla (Dijkstra / A* + Coloreado + TSP 2-Opt)",
			  wxDefaultPosition, wxSize(1220, 740)) {
	
	wxPanel* panel = new wxPanel(this, wxID_ANY);
	panel->SetBackgroundColour(wxColour(238, 242, 246));
	
	miGrafo.cargarRedVialOSM();
	
	// SECCION 1
	wxStaticText* lblSec1 = new wxStaticText(panel, wxID_ANY, "1. SEGMENTACION DE ZONAS (COLOREADO)", wxPoint(15, 12));
	lblSec1->SetFont(wxFont(9, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
	
	new wxStaticText(panel, wxID_ANY, "Algoritmo Heuristico:", wxPoint(15, 38));
	choiceAlgoritmoColoreado = new wxChoice(panel, wxID_ANY, wxPoint(155, 34), wxSize(185, 25));
	choiceAlgoritmoColoreado->Append("Welsh-Powell (Grado)");
	choiceAlgoritmoColoreado->Append("DSATUR (Saturacion)");
	choiceAlgoritmoColoreado->SetSelection(1);
	
	new wxStaticText(panel, wxID_ANY, "Criterio de Conflicto:", wxPoint(15, 68));
	choiceModoConflicto = new wxChoice(panel, wxID_ANY, wxPoint(155, 64), wxSize(185, 25));
	choiceModoConflicto->Append("Lejania (Zonas Compactas)");
	choiceModoConflicto->Append("Vecindad (Reparto Mixto)");
	choiceModoConflicto->SetSelection(0);
	
	new wxStaticText(panel, wxID_ANY, "Umbral Conflicto (m):", wxPoint(15, 98));
	spinUmbralConflicto = new wxSpinCtrl(panel, wxID_ANY, "370", wxPoint(155, 94), wxSize(75, 24), wxSP_ARROW_KEYS, 100, 1500, 370);
	
	new wxStaticText(panel, wxID_ANY, "Carga Max. Zona (paq):", wxPoint(15, 126));
	spinCapacidadZona = new wxSpinCtrl(panel, wxID_ANY, "45", wxPoint(155, 122), wxSize(75, 24), wxSP_ARROW_KEYS, 15, 200, 45);
	
	new wxStaticText(panel, wxID_ANY, "------------------------------------------------------------------", wxPoint(15, 150));
	
	// SECCION 2
	wxStaticText* lblSec2 = new wxStaticText(panel, wxID_ANY, "2. RUTEO INTRA-ZONA (TSP) Y CAPAS", wxPoint(15, 168));
	lblSec2->SetFont(wxFont(9, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
	
	chkOptimizar2Opt = new wxCheckBox(panel, wxID_ANY, "Activar Mejora Local 2-Opt sobre TSP Greedy", wxPoint(15, 192));
	chkOptimizar2Opt->SetValue(true);
	
	chkMostrarConflictos = new wxCheckBox(panel, wxID_ANY, "Dibujar Grafo de Conflictos (lineas rojas)", wxPoint(15, 214));
	chkMostrarConflictos->SetValue(false);
	
	chkMostrarPesos = new wxCheckBox(panel, wxID_ANY, "Mostrar Distancias (m) en las Calles", wxPoint(15, 236));
	chkMostrarPesos->SetValue(true);
	
	new wxStaticText(panel, wxID_ANY, "Filtrar Vehiculo/Zona:", wxPoint(15, 264));
	choiceFiltroZona = new wxChoice(panel, wxID_ANY, wxPoint(155, 260), wxSize(185, 25));
	choiceFiltroZona->Append("Todas las Zonas");
	choiceFiltroZona->SetSelection(0);
	
	btnEjecutarPipeline = new wxButton(panel, wxID_ANY, "EJECUTAR SEGMENTACION Y RUTAS", wxPoint(15, 294), wxSize(325, 34));
	btnEjecutarPipeline->SetBackgroundColour(wxColour(52, 168, 83));
	btnEjecutarPipeline->SetForegroundColour(wxColour(255, 255, 255));
	btnEjecutarPipeline->SetFont(wxFont(9, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
	
	new wxStaticText(panel, wxID_ANY, "------------------------------------------------------------------", wxPoint(15, 332));
	
	// SECCION 3
	wxStaticText* lblSec3 = new wxStaticText(panel, wxID_ANY, "3. CAMINOS CORTOS (DIJKSTRA VS A*)", wxPoint(15, 348));
	lblSec3->SetFont(wxFont(9, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
	
	new wxStaticText(panel, wxID_ANY, "Nodo Origen:", wxPoint(15, 374));
	choiceNodoOrigen = new wxChoice(panel, wxID_ANY, wxPoint(105, 370), wxSize(235, 25));
	
	new wxStaticText(panel, wxID_ANY, "Nodo Destino:", wxPoint(15, 404));
	choiceNodoDestino = new wxChoice(panel, wxID_ANY, wxPoint(105, 400), wxSize(235, 25));
	
	btnCompararCamino = new wxButton(panel, wxID_ANY, "Trazar Dijkstra vs A*", wxPoint(15, 432), wxSize(175, 28));
	btnLimpiarCamino = new wxButton(panel, wxID_ANY, "Limpiar Trazo", wxPoint(200, 432), wxSize(140, 28));
	
	new wxStaticText(panel, wxID_ANY, "------------------------------------------------------------------", wxPoint(15, 464));
	
	// SECCION 4
	wxStaticText* lblSec4 = new wxStaticText(panel, wxID_ANY, "4. ANADIR ENTREGA (O CLIC EN MAPA)", wxPoint(15, 480));
	lblSec4->SetFont(wxFont(9, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
	
	new wxStaticText(panel, wxID_ANY, "Nombre POI:", wxPoint(15, 506));
	txtNombreEntrega = new wxTextCtrl(panel, wxID_ANY, "Drogueria Central", wxPoint(105, 503), wxSize(145, 24));
	
	new wxStaticText(panel, wxID_ANY, "Paq:", wxPoint(258, 506));
	spinDemandaEntrega = new wxSpinCtrl(panel, wxID_ANY, "9", wxPoint(288, 503), wxSize(52, 24), wxSP_ARROW_KEYS, 1, 50, 9);
	
	new wxStaticText(panel, wxID_ANY, "Coord X:", wxPoint(15, 536));
	spinCoordX = new wxSpinCtrl(panel, wxID_ANY, "420", wxPoint(75, 533), wxSize(75, 24), wxSP_ARROW_KEYS, 30, 790, 420);
	
	new wxStaticText(panel, wxID_ANY, "Coord Y:", wxPoint(165, 536));
	spinCoordY = new wxSpinCtrl(panel, wxID_ANY, "160", wxPoint(225, 533), wxSize(75, 24), wxSP_ARROW_KEYS, 35, 430, 160);
	
	btnAgregarEntrega = new wxButton(panel, wxID_ANY, "Anadir Entrega al Mapa", wxPoint(15, 566), wxSize(175, 30));
	btnRestaurarMapa = new wxButton(panel, wxID_ANY, "Restaurar Red OSM", wxPoint(200, 566), wxSize(140, 30));
	
	// DERECHA: LIENZO + CONSOLA
	lienzoMapa = new MapaCanvas(panel, wxPoint(355, 12), wxSize(835, 455), &miGrafo);
	lienzoMapa->setCallbackClicMapa([this](int cx, int cy) {
		spinCoordX->SetValue(cx);
		spinCoordY->SetValue(cy);
	});
	
	wxStaticText* lblConsola = new wxStaticText(panel, wxID_ANY,
												"REPORTE DE ZONAS COLOREADAS, RUTAS POR VEHICULO Y COMPARATIVA DE TIEMPOS:",
												wxPoint(355, 473));
	lblConsola->SetFont(wxFont(9, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
	
	txtConsolaResultados = new wxTextCtrl(panel, wxID_ANY, "",
										  wxPoint(355, 492), wxSize(835, 195),
										  wxTE_MULTILINE | wxTE_READONLY | wxTE_RICH);
	txtConsolaResultados->SetFont(wxFont(9, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
	
	// EVENTOS
	btnEjecutarPipeline->Bind(wxEVT_BUTTON, &MainWindow::OnEjecutarClick, this);
	choiceAlgoritmoColoreado->Bind(wxEVT_CHOICE, &MainWindow::OnEjecutarClick, this);
	choiceModoConflicto->Bind(wxEVT_CHOICE, &MainWindow::OnEjecutarClick, this);
	chkOptimizar2Opt->Bind(wxEVT_CHECKBOX, &MainWindow::OnEjecutarClick, this);
	
	choiceFiltroZona->Bind(wxEVT_CHOICE, &MainWindow::OnFiltroZonaChange, this);
	chkMostrarConflictos->Bind(wxEVT_CHECKBOX, &MainWindow::OnCapasToggle, this);
	chkMostrarPesos->Bind(wxEVT_CHECKBOX, &MainWindow::OnCapasToggle, this);
	
	btnCompararCamino->Bind(wxEVT_BUTTON, &MainWindow::OnCompararCaminoClick, this);
	btnLimpiarCamino->Bind(wxEVT_BUTTON, &MainWindow::OnLimpiarCaminoClick, this);
	btnAgregarEntrega->Bind(wxEVT_BUTTON, &MainWindow::OnAgregarEntregaClick, this);
	btnRestaurarMapa->Bind(wxEVT_BUTTON, &MainWindow::OnRestaurarMapaClick, this);
	
	poblarSelectoresNodos();
	ejecutarSimulacionCompleta();
}

MainWindow::~MainWindow() {}

void MainWindow::poblarSelectoresNodos() {
	choiceNodoOrigen->Clear();
	choiceNodoDestino->Clear();
	idsComboNodos.clear();
	
	for (nodo* n : miGrafo.getNodos()) {
		wxString item;
		item << "[" << n->getDato() << "] " << n->getNombre();
		choiceNodoOrigen->Append(item);
		choiceNodoDestino->Append(item);
		idsComboNodos.push_back(n->getDato());
	}
	
	if (!idsComboNodos.empty()) {
		choiceNodoOrigen->SetSelection(0);
		choiceNodoDestino->SetSelection(idsComboNodos.size() > 14 ? 14 : idsComboNodos.size() - 1);
	}
}

void MainWindow::ejecutarSimulacionCompleta() {
	miGrafo.reiniciarColores();
	lienzoMapa->establecerCaminoResaltado(vector<int>());
	
	miGrafo.precalcularMatrizDijkstra();
	
	int umbral = spinUmbralConflicto->GetValue();
	int capMax = spinCapacidadZona->GetValue();
	int algCol = choiceAlgoritmoColoreado->GetSelection();
	int modoConf = choiceModoConflicto->GetSelection();
	bool usar2Opt = chkOptimizar2Opt->GetValue();
	
	ResultadoColoreado resWP = miGrafo.colorearZonas(umbral, capMax, 0, modoConf);
	ResultadoColoreado resDSATUR = miGrafo.colorearZonas(umbral, capMax, 1, modoConf);
	ResultadoColoreado resActivo = miGrafo.colorearZonas(umbral, capMax, algCol, modoConf);
	
	vector<nodo*> depositos = miGrafo.obtenerNodosPorTipo(DEPOSITO);
	if (depositos.empty()) return;
	int idDepot = depositos.front()->getDato();
	
	vector<RutaZona> rutas = miGrafo.planificarRutasFlota(idDepot, usar2Opt);
	
	choiceFiltroZona->Clear();
	choiceFiltroZona->Append("Mostrar Todas las Zonas");
	for (const RutaZona& r : rutas) {
		wxString lblZ;
		lblZ << "Zona / Vehiculo #" << (r.idZona + 1) << " (" << r.cargaTotal << " paq)";
		choiceFiltroZona->Append(lblZ);
	}
	choiceFiltroZona->SetSelection(0);
	lienzoMapa->setZonaFiltro(-1);
	
	ostringstream oss;
	oss << fixed << setprecision(1);
	oss << "===============================================================================================\n";
	oss << "  RESUMEN EJECUTIVO: SEGMENTACION POR COLOREADO + RUTEO TSP SOBRE RED VIAL OSM\n";
	oss << "===============================================================================================\n";
	oss << "  * Red Vial Cargada        : " << miGrafo.cantidadNodos() << " Nodos (1 Depot, "
		<< miGrafo.obtenerNodosPorTipo(ENTREGA).size() << " Entregas) | "
		<< miGrafo.cantidadAristasUnicas() << " Tramos Viales\n";
	oss << "  * Grafo de Conflictos     : " << resActivo.totalAristasConflicto
		<< " aristas de restriccion (Umbral = " << umbral << " m)\n";
	oss << "  * Comparativa Coloreado   : Welsh-Powell = " << resWP.totalZonas << " zonas (" << resWP.tiempoMicros << " us)"
		<< "  |  DSATUR = " << resDSATUR.totalZonas << " zonas (" << resDSATUR.tiempoMicros << " us)\n\n";
	
	oss << "-----------------------------------------------------------------------------------------------\n";
	oss << "  DETALLE DE RUTAS POR FLOTA / ZONA COLOREADA (Capacidad Max: " << capMax << " paq)\n";
	oss << "-----------------------------------------------------------------------------------------------\n";
	
	int totalGreedy = 0;
	int total2Opt = 0;
	int totalPaquetes = 0;
	
	for (const RutaZona& r : rutas) {
		totalGreedy += r.distGreedy;
		total2Opt += r.dist2Opt;
		totalPaquetes += r.cargaTotal;
		
		oss << "  [ZONA #" << (r.idZona + 1) << "] Carga: " << setw(2) << r.cargaTotal << "/" << capMax << " paq"
			<< " | Dist. Greedy: " << setw(6) << r.distGreedy << "m (" << r.tiempoGreedyMicros << " us)"
			<< " | Dist. 2-Opt: " << setw(6) << r.dist2Opt << "m (" << r.tiempo2OptMicros << " us)\n";
		
		oss << "     -> Orden Visitas TSP : ";
		for (size_t i = 0; i < r.ordenVisitasTSP.size(); ++i) {
			oss << r.ordenVisitasTSP[i] << (i + 1 < r.ordenVisitasTSP.size() ? " -> " : "");
		}
		oss << "\n     -> Trazado Vial OSM  : ";
		for (size_t i = 0; i < r.trazadoCalles.size(); ++i) {
			oss << r.trazadoCalles[i] << (i + 1 < r.trazadoCalles.size() ? "-" : "");
		}
		oss << "\n";
	}
	
	oss << "-----------------------------------------------------------------------------------------------\n";
	oss << "  METRICAS GLOBALES DE LA FLOTA:\n";
	oss << "  * Total Vehiculos / Zonas : " << rutas.size() << " flotas activas  |  Carga Total: " << totalPaquetes << " paquetes\n";
	oss << "  * Distancia Total Greedy  : " << totalGreedy << " metros\n";
	oss << "  * Distancia Total 2-Opt   : " << total2Opt << " metros (Ahorro por 2-Opt: "
		<< (totalGreedy - total2Opt) << " metros)\n";
	oss << "===============================================================================================\n";
	
	txtConsolaResultados->SetValue(wxString(oss.str()));
	lienzoMapa->actualizarDatosVisuales(resActivo.grafoConflictos, rutas);
}

void MainWindow::OnEjecutarClick(wxCommandEvent& event) {
	ejecutarSimulacionCompleta();
}

void MainWindow::OnFiltroZonaChange(wxCommandEvent& event) {
	int sel = choiceFiltroZona->GetSelection();
	lienzoMapa->setZonaFiltro(sel <= 0 ? -1 : sel - 1);
}

void MainWindow::OnCapasToggle(wxCommandEvent& event) {
	lienzoMapa->setMostrarConflictos(chkMostrarConflictos->GetValue());
	lienzoMapa->setMostrarPesos(chkMostrarPesos->GetValue());
}

void MainWindow::OnCompararCaminoClick(wxCommandEvent& event) {
	int idxOrig = choiceNodoOrigen->GetSelection();
	int idxDest = choiceNodoDestino->GetSelection();
	if (idxOrig < 0 || idxDest < 0 ||
		idxOrig >= static_cast<int>(idsComboNodos.size()) ||
		idxDest >= static_cast<int>(idsComboNodos.size())) {
		return;
	}
		
		int idOrigen = idsComboNodos[idxOrig];
		int idDestino = idsComboNodos[idxDest];
		
		ResultadoCamino resDijk = miGrafo.Dijkstra(idOrigen);
		ResultadoCamino resAStar = miGrafo.AStar(idOrigen, idDestino);
		
		vector<int> camino = miGrafo.reconstruirCamino(idOrigen, idDestino, resDijk.predecesores);
		lienzoMapa->establecerCaminoResaltado(camino);
		
		nodo* nOrig = miGrafo.buscarNodo(idOrigen);
		nodo* nDest = miGrafo.buscarNodo(idDestino);
		
		ostringstream oss;
		oss << fixed << setprecision(2);
		oss << "\n>>> COMPARATIVA CAMINO CORTO INDIVIDUAL: [" << idOrigen << "] "
			<< (nOrig ? nOrig->getNombre() : "") << "  -->  [" << idDestino << "] "
			<< (nDest ? nDest->getNombre() : "") << " <<<\n";
		oss << "  * Distancia Optima en Red : " << resDijk.distancias[idDestino] << " metros\n";
		oss << "  * Dijkstra Clasico        : " << resDijk.nodosExpandidos << " nodos expandidos  |  Tiempo: " << resDijk.tiempoMicros << " us\n";
		oss << "  * A* (Heuristica Euclid.) : " << resAStar.nodosExpandidos << " nodos expandidos  |  Tiempo: " << resAStar.tiempoMicros << " us\n";
		oss << "  * Secuencia de Calles     : ";
		for (size_t i = 0; i < camino.size(); ++i) {
			oss << camino[i] << (i + 1 < camino.size() ? " -> " : "\n");
		}
		
		txtConsolaResultados->AppendText(wxString(oss.str()));
}

void MainWindow::OnLimpiarCaminoClick(wxCommandEvent& event) {
	lienzoMapa->establecerCaminoResaltado(vector<int>());
}

void MainWindow::OnAgregarEntregaClick(wxCommandEvent& event) {
	string nombre = string(txtNombreEntrega->GetValue().mb_str());
	if (nombre.empty()) {
		wxMessageBox("Ingrese un nombre valido para el punto de entrega.", "Campo Vacio", wxOK | wxICON_WARNING);
		return;
	}
	
	int demanda = spinDemandaEntrega->GetValue();
	double cx = static_cast<double>(spinCoordX->GetValue());
	double cy = static_cast<double>(spinCoordY->GetValue());
	
	miGrafo.insertarEntregaConectada(nombre, cx, cy, demanda);
	poblarSelectoresNodos();
	ejecutarSimulacionCompleta();
}

void MainWindow::OnRestaurarMapaClick(wxCommandEvent& event) {
	miGrafo.cargarRedVialOSM();
	poblarSelectoresNodos();
	ejecutarSimulacionCompleta();
}
