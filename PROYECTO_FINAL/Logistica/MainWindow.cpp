#include "MainWindow.h"
#include <wx/msgdlg.h>
#include <wx/filedlg.h>
#include <wx/dcclient.h>
#include <wx/pen.h>
#include <wx/brush.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <limits>

using namespace std;

// ============================================================================
// LIENZO DEL MAPA (MapaCanvas)
// ============================================================================
MapaCanvas::MapaCanvas(wxWindow* parent, wxPoint pos, wxSize size, grafo* grafoRef)
	: wxPanel(parent, wxID_ANY, pos, size, wxBORDER_SIMPLE),
	miGrafo(grafoRef),
	zonaFiltro(-1),
	mostrarConflictos(false),
	mostrarPesos(true),
	usarRutasTSP(true) {
	
	SetBackgroundColour(wxColour(245, 248, 252));
	Bind(wxEVT_PAINT, &MapaCanvas::OnPaint, this);
	Bind(wxEVT_LEFT_DOWN, &MapaCanvas::OnMouseLeftDown, this);
}

void MapaCanvas::actualizarDatosVisuales(
										 const map<int, set<int>>& conflictos,
										 const vector<RutaZona>& rutas,
										 bool dibujarTSP) {
	grafoConflictos = conflictos;
	rutasActivas = rutas;
	usarRutasTSP = dibujarTSP;
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
	if (onClicMapa) onClicMapa(event.GetX(), event.GetY());
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
	
	// 2. Halos de zonas coloreadas
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
		dc.DrawCircle(static_cast<int>(n->getX()), static_cast<int>(n->getY()), 26);
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
	
	// 4. Red vial OSM y distancias
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
					wxString txtPeso;
					txtPeso << peso << "m";
					dc.SetTextForeground(wxColour(100, 116, 139));
					dc.DrawText(txtPeso, (x1 + x2) / 2 - 10, (y1 + y2) / 2 - 7);
				}
			}
		}
	}
	
	// 5. Rutas por zona con flechas
	for (const RutaZona& ruta : rutasActivas) {
		int z = ruta.idZona;
		if (zonaFiltro != -1 && zonaFiltro != z) continue;
		
		wxColour colorFlota = obtenerColorZona(z);
		dc.SetPen(wxPen(colorFlota, 3, wxPENSTYLE_SOLID));
		dc.SetBrush(wxBrush(colorFlota));
		
		int offset = (z % 4) * 2 - 3;
		const vector<int>& camino = usarRutasTSP ? ruta.callesTSP : ruta.callesDijkstraRep;
		
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
			double tamF = 6.0;
			
			wxPoint flecha[3];
			flecha[0] = wxPoint(static_cast<int>(mx + tamF * std::cos(ang)),
								static_cast<int>(my + tamF * std::sin(ang)));
			flecha[1] = wxPoint(static_cast<int>(mx - tamF * std::cos(ang - 0.6)),
								static_cast<int>(my - tamF * std::sin(ang - 0.6)));
			flecha[2] = wxPoint(static_cast<int>(mx - tamF * std::cos(ang + 0.6)),
								static_cast<int>(my - tamF * std::sin(ang + 0.6)));
			dc.DrawPolygon(3, flecha);
		}
	}
	
	// 6. Camino corto individual resaltado (Dijkstra)
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
			wxString tag;
			tag << "D" << id;
			dc.DrawText(tag, x - 7, y - 6);
			
			dc.SetTextForeground(wxColour(15, 23, 42));
			wxString lblDep;
			lblDep << "[" << id << "] " << n->getNombre();
			dc.DrawText(lblDep, x - 40, y + 15);
			
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
	
	// 8. Leyenda superior
	dc.SetPen(wxPen(wxColour(203, 213, 225), 1, wxPENSTYLE_SOLID));
	dc.SetBrush(wxBrush(wxColour(255, 255, 255)));
	dc.DrawRoundedRectangle(8, 6, tam.GetWidth() - 16, 24, 4);
	dc.SetFont(wxFont(8, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
	dc.SetTextForeground(wxColour(30, 41, 59));
	dc.DrawText("MAPA VIAL OSM  |  Cuadro Negro = Depot  |  Circulos Coloreados = Entregas por Zona  |  Clic = Capturar X,Y", 16, 11);
}

// ============================================================================
// VENTANA PRINCIPAL (MainWindow)
// ============================================================================
MainWindow::MainWindow(grafo* grafoRef, const ConfigApp& config)
	: wxFrame(NULL, wxID_ANY, "Proyecto 3 - Logistica Ultima-Milla (Dijkstra + Coloreado + Ruteo)",
			  wxDefaultPosition, wxSize(1220, 745)),
	miGrafo(grafoRef),
	configDefault(config) {
	
	wxPanel* panel = new wxPanel(this, wxID_ANY);
	panel->SetBackgroundColour(wxColour(238, 242, 246));
	
	// SECCION 1: COLOREADO DE ZONAS
	wxStaticText* lblSec1 = new wxStaticText(panel, wxID_ANY, "1. SEGMENTACION DE ZONAS (COLOREADO)", wxPoint(15, 10));
	lblSec1->SetFont(wxFont(9, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
	
	new wxStaticText(panel, wxID_ANY, "Algoritmo:", wxPoint(15, 36));
	choiceAlgoritmoColoreado = new wxChoice(panel, wxID_ANY, wxPoint(155, 32), wxSize(185, 24));
	choiceAlgoritmoColoreado->Append("Welsh-Powell (Grado)");
	choiceAlgoritmoColoreado->Append("DSATUR (Saturacion)");
	choiceAlgoritmoColoreado->SetSelection(configDefault.algoritmoColoreadoDefault);
	
	new wxStaticText(panel, wxID_ANY, "Regla Conflicto:", wxPoint(15, 64));
	choiceModoConflicto = new wxChoice(panel, wxID_ANY, wxPoint(155, 60), wxSize(185, 24));
	choiceModoConflicto->Append("Lejania (Zonas Compactas)");
	choiceModoConflicto->Append("Vecindad (Reparto Mixto)");
	choiceModoConflicto->SetSelection(configDefault.modoConflictoDefault);
	
	new wxStaticText(panel, wxID_ANY, "Umbral Conflicto (m):", wxPoint(15, 92));
	spinUmbralConflicto = new wxSpinCtrl(
										 panel, wxID_ANY, to_string(configDefault.umbralConflictoDefault),
										 wxPoint(155, 88), wxSize(75, 24), wxSP_ARROW_KEYS, 80, 1500, configDefault.umbralConflictoDefault
										 );
	
	new wxStaticText(panel, wxID_ANY, "Carga Max. Zona (paq):", wxPoint(15, 120));
	spinCapacidadZona = new wxSpinCtrl(
									   panel, wxID_ANY, to_string(configDefault.capacidadZonaDefault),
									   wxPoint(155, 116), wxSize(75, 24), wxSP_ARROW_KEYS, 15, 200, configDefault.capacidadZonaDefault
									   );
	
	new wxStaticText(panel, wxID_ANY, "------------------------------------------------------------------", wxPoint(15, 144));
	
	// SECCION 2: RUTEO INTRA-ZONA Y CAPAS
	wxStaticText* lblSec2 = new wxStaticText(panel, wxID_ANY, "2. RUTEO POR ZONA Y CAPAS DEL MAPA", wxPoint(15, 160));
	lblSec2->SetFont(wxFont(9, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
	
	new wxStaticText(panel, wxID_ANY, "Mostrar Rutas:", wxPoint(15, 186));
	choiceModoRuteo = new wxChoice(panel, wxID_ANY, wxPoint(115, 182), wxSize(225, 24));
	choiceModoRuteo->Append("Heuristica TSP Simple + 2-Opt");
	choiceModoRuteo->Append("Dijkstra Repetido (Paso a Paso)");
	choiceModoRuteo->SetSelection(configDefault.modoRuteoDefault);
	
	chkMostrarConflictos = new wxCheckBox(panel, wxID_ANY, "Dibujar Grafo de Conflictos (lineas rojas)", wxPoint(15, 212));
	chkMostrarConflictos->SetValue(false);
	
	chkMostrarPesos = new wxCheckBox(panel, wxID_ANY, "Mostrar Distancias (m) en las Calles", wxPoint(15, 234));
	chkMostrarPesos->SetValue(true);
	
	new wxStaticText(panel, wxID_ANY, "Filtrar Vehiculo:", wxPoint(15, 262));
	choiceFiltroZona = new wxChoice(panel, wxID_ANY, wxPoint(125, 258), wxSize(215, 24));
	choiceFiltroZona->Append("Todas las Zonas");
	choiceFiltroZona->SetSelection(0);
	
	btnEjecutarPipeline = new wxButton(panel, wxID_ANY, "EJECUTAR SEGMENTACION Y RUTAS", wxPoint(15, 290), wxSize(325, 32));
	btnEjecutarPipeline->SetBackgroundColour(wxColour(52, 168, 83));
	btnEjecutarPipeline->SetForegroundColour(wxColour(255, 255, 255));
	btnEjecutarPipeline->SetFont(wxFont(9, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
	
	new wxStaticText(panel, wxID_ANY, "------------------------------------------------------------------", wxPoint(15, 326));
	
	// SECCION 3: CONSULTA DIJKSTRA
	wxStaticText* lblSec3 = new wxStaticText(panel, wxID_ANY, "3. CONSULTA DIJKSTRA (DEPOTS / POIs)", wxPoint(15, 342));
	lblSec3->SetFont(wxFont(9, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
	
	new wxStaticText(panel, wxID_ANY, "Origen:", wxPoint(15, 368));
	choiceNodoOrigen = new wxChoice(panel, wxID_ANY, wxPoint(80, 364), wxSize(260, 24));
	
	new wxStaticText(panel, wxID_ANY, "Destino:", wxPoint(15, 396));
	choiceNodoDestino = new wxChoice(panel, wxID_ANY, wxPoint(80, 392), wxSize(260, 24));
	
	btnTrazarDijkstra = new wxButton(panel, wxID_ANY, "Trazar Camino Dijkstra", wxPoint(15, 424), wxSize(180, 28));
	btnLimpiarCamino = new wxButton(panel, wxID_ANY, "Limpiar Trazo", wxPoint(205, 424), wxSize(135, 28));
	
	new wxStaticText(panel, wxID_ANY, "------------------------------------------------------------------", wxPoint(15, 456));
	
	// SECCION 4: CARGAR MAPA TXT Y AGREGAR ENTREGAS
	wxStaticText* lblSec4 = new wxStaticText(panel, wxID_ANY, "4. ARCHIVOS .TXT Y NUEVAS ENTREGAS", wxPoint(15, 472));
	lblSec4->SetFont(wxFont(9, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
	
	btnCargarTXT = new wxButton(panel, wxID_ANY, "Cargar Mapa (.txt)...", wxPoint(15, 496), wxSize(175, 30));
	btnCargarTXT->SetBackgroundColour(wxColour(37, 99, 235));
	btnCargarTXT->SetForegroundColour(wxColour(255, 255, 255));
	btnCargarTXT->SetFont(wxFont(9, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
	
	btnRestaurarMapa = new wxButton(panel, wxID_ANY, "Default (.txt)", wxPoint(200, 496), wxSize(140, 30));
	
	new wxStaticText(panel, wxID_ANY, "Nombre POI:", wxPoint(15, 536));
	txtNombreEntrega = new wxTextCtrl(panel, wxID_ANY, "Drogueria Central", wxPoint(100, 533), wxSize(150, 24));
	
	new wxStaticText(panel, wxID_ANY, "Paq:", wxPoint(258, 536));
	spinDemandaEntrega = new wxSpinCtrl(panel, wxID_ANY, "9", wxPoint(288, 533), wxSize(52, 24), wxSP_ARROW_KEYS, 1, 50, 9);
	
	new wxStaticText(panel, wxID_ANY, "Coord X:", wxPoint(15, 566));
	spinCoordX = new wxSpinCtrl(panel, wxID_ANY, "420", wxPoint(75, 563), wxSize(75, 24), wxSP_ARROW_KEYS, 30, 790, 420);
	
	new wxStaticText(panel, wxID_ANY, "Coord Y:", wxPoint(165, 566));
	spinCoordY = new wxSpinCtrl(panel, wxID_ANY, "160", wxPoint(225, 563), wxSize(75, 24), wxSP_ARROW_KEYS, 35, 430, 160);
	
	btnAgregarEntrega = new wxButton(panel, wxID_ANY, "Anadir Entrega al Mapa (o clic en mapa)", wxPoint(15, 596), wxSize(325, 28));
	
	// DERECHA: LIENZO + CONSOLA
	lienzoMapa = new MapaCanvas(panel, wxPoint(355, 12), wxSize(835, 455), miGrafo);
	lienzoMapa->setCallbackClicMapa([this](int cx, int cy) {
		spinCoordX->SetValue(cx);
		spinCoordY->SetValue(cy);
	});
	
	wxStaticText* lblConsola = new wxStaticText(panel, wxID_ANY,
												"REPORTE DE DISTANCIAS DIJKSTRA, ZONAS COLOREADAS Y COMPARATIVA DE RUTAS:",
												wxPoint(355, 473));
	lblConsola->SetFont(wxFont(9, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
	
	txtConsolaResultados = new wxTextCtrl(panel, wxID_ANY, "",
										  wxPoint(355, 492), wxSize(835, 198),
										  wxTE_MULTILINE | wxTE_READONLY | wxTE_RICH);
	txtConsolaResultados->SetFont(wxFont(9, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
	
	// EVENTOS
	btnEjecutarPipeline->Bind(wxEVT_BUTTON, &MainWindow::OnEjecutarClick, this);
	choiceAlgoritmoColoreado->Bind(wxEVT_CHOICE, &MainWindow::OnEjecutarClick, this);
	choiceModoConflicto->Bind(wxEVT_CHOICE, &MainWindow::OnEjecutarClick, this);
	choiceModoRuteo->Bind(wxEVT_CHOICE, &MainWindow::OnEjecutarClick, this);
	
	choiceFiltroZona->Bind(wxEVT_CHOICE, &MainWindow::OnFiltroZonaChange, this);
	chkMostrarConflictos->Bind(wxEVT_CHECKBOX, &MainWindow::OnCapasToggle, this);
	chkMostrarPesos->Bind(wxEVT_CHECKBOX, &MainWindow::OnCapasToggle, this);
	
	btnTrazarDijkstra->Bind(wxEVT_BUTTON, &MainWindow::OnTrazarDijkstraClick, this);
	btnLimpiarCamino->Bind(wxEVT_BUTTON, &MainWindow::OnLimpiarCaminoClick, this);
	btnCargarTXT->Bind(wxEVT_BUTTON, &MainWindow::OnCargarTXTClick, this);
	btnRestaurarMapa->Bind(wxEVT_BUTTON, &MainWindow::OnRestaurarMapaClick, this);
	btnAgregarEntrega->Bind(wxEVT_BUTTON, &MainWindow::OnAgregarEntregaClick, this);
	
	poblarSelectoresNodos();
	ejecutarSimulacionCompleta();
}

MainWindow::~MainWindow() {}

void MainWindow::poblarSelectoresNodos() {
	choiceNodoOrigen->Clear();
	choiceNodoDestino->Clear();
	idsComboNodos.clear();
	
	if (!miGrafo) return;
	
	for (nodo* n : miGrafo->getNodos()) {
		wxString item;
		item << "[" << n->getDato() << "] " << n->getNombre();
		choiceNodoOrigen->Append(item);
		choiceNodoDestino->Append(item);
		idsComboNodos.push_back(n->getDato());
	}
	
	if (!idsComboNodos.empty()) {
		choiceNodoOrigen->SetSelection(0);
		choiceNodoDestino->SetSelection(idsComboNodos.size() > 1 ? idsComboNodos.size() - 1 : 0);
	}
}

void MainWindow::ejecutarSimulacionCompleta() {
	if (!miGrafo || miGrafo->cantidadNodos() == 0) {
		choiceFiltroZona->Clear();
		choiceFiltroZona->Append("Sin mapa cargado");
		choiceFiltroZona->SetSelection(0);
		lienzoMapa->establecerCaminoResaltado(vector<int>());
		lienzoMapa->actualizarDatosVisuales(map<int, set<int>>(), vector<RutaZona>(), true);
		txtConsolaResultados->SetValue(
									   "No hay ningun mapa cargado en memoria.\n"
									   "Asegurese de guardar 'mapa_default.txt' en la carpeta del proyecto o use 'Cargar Mapa (.txt)...'."
									   );
		return;
	}
	
	miGrafo->reiniciarColores();
	lienzoMapa->establecerCaminoResaltado(vector<int>());
	
	double tiempoMatriz = miGrafo->precalcularMatrizDijkstra();
	
	int umbral = spinUmbralConflicto->GetValue();
	int capMax = spinCapacidadZona->GetValue();
	int algCol = choiceAlgoritmoColoreado->GetSelection();
	int modoConf = choiceModoConflicto->GetSelection();
	bool usarTSP = (choiceModoRuteo->GetSelection() == 0);
	
	// Comparar Welsh-Powell vs DSATUR y dejar aplicado el seleccionado
	ResultadoColoreado resWP = miGrafo->colorearZonas(umbral, capMax, 0, modoConf);
	ResultadoColoreado resDSATUR = miGrafo->colorearZonas(umbral, capMax, 1, modoConf);
	ResultadoColoreado resActivo = miGrafo->colorearZonas(umbral, capMax, algCol, modoConf);
	
	vector<RutaZona> rutas = miGrafo->planificarRutasFlota();
	
	choiceFiltroZona->Clear();
	choiceFiltroZona->Append("Mostrar Todas las Zonas");
	for (const RutaZona& r : rutas) {
		wxString lblZ;
		lblZ << "Zona #" << (r.idZona + 1) << " (D" << r.idDepot << " | " << r.cargaTotal << " paq)";
		choiceFiltroZona->Append(lblZ);
	}
	choiceFiltroZona->SetSelection(0);
	lienzoMapa->setZonaFiltro(-1);
	
	// Reporte completo
	ostringstream oss;
	oss << fixed << setprecision(1);
	oss << "===============================================================================================\n";
	oss << "  1. RED VIAL Y DISTANCIAS DIJKSTRA (Matriz completa: " << tiempoMatriz << " us)\n";
	oss << "===============================================================================================\n";
	
	vector<nodo*> depots = miGrafo->obtenerNodosPorTipo(DEPOSITO);
	vector<nodo*> entregas = miGrafo->obtenerNodosPorTipo(ENTREGA);
	oss << "  Red: " << miGrafo->cantidadNodos() << " Nodos (" << depots.size() << " Depot(s), "
		<< entregas.size() << " Entregas) | " << miGrafo->cantidadAristasUnicas() << " Calles\n";
	
	if (!depots.empty()) {
		int idD0 = depots.front()->getDato();
		ResultadoDijkstra d0 = miGrafo->Dijkstra(idD0);
		oss << "  Distancias desde Depot D" << idD0 << " hacia entregas:\n";
		for (nodo* e : entregas) {
			int idE = e->getDato();
			int dist = d0.distancias.count(idE) ? d0.distancias[idE] : numeric_limits<int>::max();
			oss << "    [" << setw(2) << idE << "] " << left << setw(24) << e->getNombre() << right
				<< " (" << setw(2) << e->getDemanda() << " paq) -> "
				<< (dist == numeric_limits<int>::max() ? "INF" : to_string(dist) + " m")
				<< " | Zona #" << (e->getColorZona() + 1) << "\n";
		}
	}
	
	oss << "\n===============================================================================================\n";
	oss << "  2. COLOREADO DE ZONAS (Conflictos: " << resActivo.totalAristasConflicto
		<< " aristas | Umbral: " << umbral << "m | Cap. Max: " << capMax << " paq)\n";
	oss << "===============================================================================================\n";
	oss << "  * Welsh-Powell : " << resWP.totalZonas << " zonas (" << resWP.tiempoMicros << " us)\n";
	oss << "  * DSATUR       : " << resDSATUR.totalZonas << " zonas (" << resDSATUR.tiempoMicros << " us)\n";
	
	oss << "\n===============================================================================================\n";
	oss << "  3. RUTAS POR VEHICULO (DIJKSTRA REPETIDO vs HEURISTICA TSP + 2-OPT)\n";
	oss << "===============================================================================================\n";
	
	int sumDijk = 0, sumTSP = 0, sumPaq = 0;
	double tSumDijk = 0.0, tSumTSP = 0.0;
	
	for (const RutaZona& r : rutas) {
		sumDijk += r.distDijkstraRep;
		sumTSP += r.distTSP;
		sumPaq += r.cargaTotal;
		tSumDijk += r.tiempoDijkstraRepMicros;
		tSumTSP += r.tiempoTSPMicros;
		
		oss << "  [ZONA #" << (r.idZona + 1) << " | Depot D" << r.idDepot
			<< " | Carga: " << r.cargaTotal << "/" << capMax << " paq]\n";
		oss << "    - Dijkstra Repetido : " << setw(5) << r.distDijkstraRep << " m (" << setw(6)
			<< r.tiempoDijkstraRepMicros << " us) | Visitas: ";
		for (size_t i = 0; i < r.visitasDijkstraRep.size(); ++i)
			oss << r.visitasDijkstraRep[i] << (i + 1 < r.visitasDijkstraRep.size() ? "->" : "\n");
		
		oss << "    - TSP Simple + 2Opt : " << setw(5) << r.distTSP << " m (" << setw(6)
			<< r.tiempoTSPMicros << " us) | Visitas: ";
		for (size_t i = 0; i < r.visitasTSP.size(); ++i)
			oss << r.visitasTSP[i] << (i + 1 < r.visitasTSP.size() ? "->" : "\n");
	}
	
	oss << "-----------------------------------------------------------------------------------------------\n";
	oss << "  TOTAL FLOTA (" << rutas.size() << " vehiculos | " << sumPaq << " paquetes):\n";
	oss << "    * Dijkstra Repetido : " << sumDijk << " m  | Tiempo total: " << tSumDijk << " us\n";
	oss << "    * TSP Simple + 2Opt : " << sumTSP << " m  | Tiempo total: " << tSumTSP
		<< " us | Ahorro: " << (sumDijk - sumTSP) << " m\n";
	oss << "===============================================================================================\n";
	
	txtConsolaResultados->SetValue(wxString(oss.str()));
	lienzoMapa->actualizarDatosVisuales(resActivo.grafoConflictos, rutas, usarTSP);
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

void MainWindow::OnTrazarDijkstraClick(wxCommandEvent& event) {
	if (!miGrafo) return;
	int idxOrig = choiceNodoOrigen->GetSelection();
	int idxDest = choiceNodoDestino->GetSelection();
	if (idxOrig < 0 || idxDest < 0 ||
		idxOrig >= static_cast<int>(idsComboNodos.size()) ||
		idxDest >= static_cast<int>(idsComboNodos.size())) {
		return;
	}
		
		int idOrigen = idsComboNodos[idxOrig];
		int idDestino = idsComboNodos[idxDest];
		
		ResultadoDijkstra res = miGrafo->Dijkstra(idOrigen);
		vector<int> camino = miGrafo->reconstruirCamino(idOrigen, idDestino, res.predecesor);
		lienzoMapa->establecerCaminoResaltado(camino);
		
		nodo* nO = miGrafo->buscarNodo(idOrigen);
		nodo* nD = miGrafo->buscarNodo(idDestino);
		
		ostringstream oss;
		oss << fixed << setprecision(2);
		oss << "\n>>> CONSULTA DIJKSTRA: [" << idOrigen << "] " << (nO ? nO->getNombre() : "")
			<< " --> [" << idDestino << "] " << (nD ? nD->getNombre() : "") << " <<<\n";
		if (res.huboPesoNegativo) {
			oss << "  [Aviso] Se detectaron pesos negativos en el grafo.\n";
		}
		int dist = res.distancias.count(idDestino) ? res.distancias[idDestino] : numeric_limits<int>::max();
		oss << "  * Distancia optima : " << (dist == numeric_limits<int>::max() ? "No alcanzable" : to_string(dist) + " metros")
			<< " | Tiempo: " << res.tiempoMicros << " us\n";
		oss << "  * Ruta por calles  : ";
		for (size_t i = 0; i < camino.size(); ++i) {
			oss << camino[i] << (i + 1 < camino.size() ? " -> " : "\n");
		}
		
		txtConsolaResultados->AppendText(wxString(oss.str()));
}

void MainWindow::OnLimpiarCaminoClick(wxCommandEvent& event) {
	lienzoMapa->establecerCaminoResaltado(vector<int>());
}

void MainWindow::OnCargarTXTClick(wxCommandEvent& event) {
	if (!miGrafo) return;
	wxFileDialog dialogo(
						 this,
						 "Seleccionar archivo de mapa (.txt)",
						 "",
						 "",
						 "Archivos de texto (*.txt)|*.txt|Todos los archivos (*.*)|*.*",
						 wxFD_OPEN | wxFD_FILE_MUST_EXIST
						 );
	
	if (dialogo.ShowModal() == wxID_CANCEL) return;
	
	string ruta = string(dialogo.GetPath().mb_str());
	string error;
	if (!miGrafo->cargarDesdeTXT(ruta, error)) {
		wxMessageBox(wxString(error), "Error al cargar archivo", wxOK | wxICON_ERROR);
		return;
	}
	
	poblarSelectoresNodos();
	ejecutarSimulacionCompleta();
}

void MainWindow::OnRestaurarMapaClick(wxCommandEvent& event) {
	if (!miGrafo) return;
	string error;
	if (!miGrafo->cargarDesdeTXT(configDefault.rutaMapaDefault, error)) {
		wxMessageBox(
					 "No se pudo recargar '" + configDefault.rutaMapaDefault + "'.\n" + error,
					 "Error al cargar default",
					 wxOK | wxICON_ERROR
					 );
		return;
	}
	poblarSelectoresNodos();
	ejecutarSimulacionCompleta();
}

void MainWindow::OnAgregarEntregaClick(wxCommandEvent& event) {
	if (!miGrafo) return;
	string nombre = string(txtNombreEntrega->GetValue().mb_str());
	if (nombre.empty()) {
		wxMessageBox("Ingrese un nombre valido para el punto de entrega.", "Campo Vacio", wxOK | wxICON_WARNING);
		return;
	}
	
	int demanda = spinDemandaEntrega->GetValue();
	double cx = static_cast<double>(spinCoordX->GetValue());
	double cy = static_cast<double>(spinCoordY->GetValue());
	
	miGrafo->insertarEntregaConectada(nombre, cx, cy, demanda);
	poblarSelectoresNodos();
	ejecutarSimulacionCompleta();
}
