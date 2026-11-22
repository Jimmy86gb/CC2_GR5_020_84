#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <wx/frame.h>
#include <wx/panel.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/spinctrl.h>
#include <wx/choice.h>
#include <map>
#include <set>
#include <vector>
#include <functional>

#include "Grafo.h"

class MapaCanvas : public wxPanel {
public:
	MapaCanvas(wxWindow* parent, wxPoint pos, wxSize size, grafo* grafoRef);
	
	void actualizarDatosVisuales(
								 const std::map<int, std::set<int>>& conflictos,
								 const std::vector<RutaZona>& rutas,
								 bool dibujarTSP
								 );
	void establecerCaminoResaltado(const std::vector<int>& camino);
	void setZonaFiltro(int zona);
	void setMostrarConflictos(bool mostrar);
	void setMostrarPesos(bool mostrar);
	void setCallbackClicMapa(std::function<void(int, int)> callback);
	
private:
	grafo* miGrafo;
	std::map<int, std::set<int>> grafoConflictos;
	std::vector<RutaZona> rutasActivas;
	std::vector<int> caminoIndividual;
	int zonaFiltro;
	bool mostrarConflictos;
	bool mostrarPesos;
	bool usarRutasTSP;
	std::function<void(int, int)> onClicMapa;
	
	void OnPaint(wxPaintEvent& event);
	void OnMouseLeftDown(wxMouseEvent& event);
	wxColour obtenerColorZona(int indiceZona) const;
};

class MainWindow : public wxFrame {
public:
	MainWindow();
	virtual ~MainWindow();
	
private:
	grafo miGrafo;
	std::vector<int> idsComboNodos;
	
	// Seccion 1: Coloreado
	wxChoice* choiceAlgoritmoColoreado;
	wxChoice* choiceModoConflicto;
	wxSpinCtrl* spinUmbralConflicto;
	wxSpinCtrl* spinCapacidadZona;
	
	// Seccion 2: Ruteo y Capas
	wxChoice* choiceModoRuteo;
	wxCheckBox* chkMostrarConflictos;
	wxCheckBox* chkMostrarPesos;
	wxChoice* choiceFiltroZona;
	wxButton* btnEjecutarPipeline;
	
	// Seccion 3: Consulta Dijkstra
	wxChoice* choiceNodoOrigen;
	wxChoice* choiceNodoDestino;
	wxButton* btnTrazarDijkstra;
	wxButton* btnLimpiarCamino;
	
	// Seccion 4: Carga TXT y Edicion
	wxButton* btnCargarTXT;
	wxButton* btnRestaurarMapa;
	wxTextCtrl* txtNombreEntrega;
	wxSpinCtrl* spinDemandaEntrega;
	wxSpinCtrl* spinCoordX;
	wxSpinCtrl* spinCoordY;
	wxButton* btnAgregarEntrega;
	
	// Mapa y consola de metricas
	MapaCanvas* lienzoMapa;
	wxTextCtrl* txtConsolaResultados;
	
	void poblarSelectoresNodos();
	void ejecutarSimulacionCompleta();
	
	void OnEjecutarClick(wxCommandEvent& event);
	void OnFiltroZonaChange(wxCommandEvent& event);
	void OnCapasToggle(wxCommandEvent& event);
	void OnTrazarDijkstraClick(wxCommandEvent& event);
	void OnLimpiarCaminoClick(wxCommandEvent& event);
	void OnCargarTXTClick(wxCommandEvent& event);
	void OnRestaurarMapaClick(wxCommandEvent& event);
	void OnAgregarEntregaClick(wxCommandEvent& event);
};

#endif
