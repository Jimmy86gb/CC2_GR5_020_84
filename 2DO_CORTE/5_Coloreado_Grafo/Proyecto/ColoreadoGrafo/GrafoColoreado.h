#ifndef GRAFO_COLOREADO_H
#define GRAFO_COLOREADO_H

#include "Nodo.h"
#include <vector>

class GrafoColoreado {
private:
    std::vector<Nodo*> nodos;
    int totalColores;
    void reiniciarColores();
    int colorearEnOrden(const std::vector<Nodo*>& orden);

public:
    GrafoColoreado();
    ~GrafoColoreado();
    GrafoColoreado(const GrafoColoreado&) = delete;
    GrafoColoreado& operator=(const GrafoColoreado&) = delete;

    void agregarNodo(int id);
    Nodo* buscarNodo(int id) const;
    bool agregarArista(int origen, int destino);
    void limpiarGrafo();
    int calcularColoreado(); // Compatibilidad: Welsh-Powell
    int coloreadoVoraz();
    int coloreadoWelshPowell();
    int coloreadoDSATUR();
    int coloreadoRLF();
    int coloreadoExacto(); // Backtracking; recomendado para grafos pequenos
    int getCantidadNodos() const;
    void mostrarGrafo() const;
    void mostrarColoreado() const;
};

#endif
