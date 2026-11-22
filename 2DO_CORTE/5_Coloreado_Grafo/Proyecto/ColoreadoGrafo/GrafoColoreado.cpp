#include "GrafoColoreado.h"
#include <algorithm>
#include <cstddef>
#include <functional>
#include <iostream>
#include <set>
#include <vector>

GrafoColoreado::GrafoColoreado() : totalColores(0) {}
GrafoColoreado::~GrafoColoreado() { limpiarGrafo(); }

void GrafoColoreado::limpiarGrafo() {
    for (Nodo* nodo : nodos) delete nodo;
    nodos.clear();
    totalColores = 0;
}

Nodo* GrafoColoreado::buscarNodo(int id) const {
    for (Nodo* nodo : nodos) if (nodo->getId() == id) return nodo;
    return nullptr;
}

void GrafoColoreado::agregarNodo(int id) {
    if (buscarNodo(id) == nullptr) {
        nodos.push_back(new Nodo(id));
        reiniciarColores();
    }
}

bool GrafoColoreado::agregarArista(int origen, int destino) {
    Nodo* a = buscarNodo(origen);
    Nodo* b = buscarNodo(destino);
    if (a == nullptr || b == nullptr || a == b) return false;
    const std::vector<Nodo*>& vecinos = a->getAdyacentes();
    if (std::find(vecinos.begin(), vecinos.end(), b) != vecinos.end()) return false;
    a->agregarAdyacente(b);
    b->agregarAdyacente(a);
    reiniciarColores();
    return true;
}

void GrafoColoreado::reiniciarColores() {
    for (Nodo* nodo : nodos) nodo->setColor(-1);
    totalColores = 0;
}

int GrafoColoreado::colorearEnOrden(const std::vector<Nodo*>& orden) {
    reiniciarColores();
    for (Nodo* nodo : orden) {
        std::vector<bool> ocupado(nodos.size() + 1, false);
        for (const Nodo* vecino : nodo->getAdyacentes()) {
            if (vecino->getColor() >= 0)
                ocupado[static_cast<std::size_t>(vecino->getColor())] = true;
        }
        int color = 0;
        while (ocupado[static_cast<std::size_t>(color)]) ++color;
        nodo->setColor(color);
        totalColores = std::max(totalColores, color + 1);
    }
    return totalColores;
}

// Voraz: utiliza el orden en que fueron agregados los nodos.
int GrafoColoreado::coloreadoVoraz() {
    return colorearEnOrden(nodos);
}

// Welsh-Powell: voraz con nodos ordenados por grado descendente.
int GrafoColoreado::coloreadoWelshPowell() {
    std::vector<Nodo*> orden = nodos;
    std::sort(orden.begin(), orden.end(), [](const Nodo* a, const Nodo* b) {
        if (a->getAdyacentes().size() != b->getAdyacentes().size())
            return a->getAdyacentes().size() > b->getAdyacentes().size();
        return a->getId() < b->getId();
    });
    return colorearEnOrden(orden);
}

int GrafoColoreado::calcularColoreado() { return coloreadoWelshPowell(); }

// DSATUR: selecciona el nodo con mayor numero de colores distintos en vecinos.
// Desempata por grado y luego por identificador.
int GrafoColoreado::coloreadoDSATUR() {
    reiniciarColores();
    for (std::size_t paso = 0; paso < nodos.size(); ++paso) {
        Nodo* elegido = nullptr;
        int mejorSaturacion = -1;
        std::size_t mejorGrado = 0;
        for (Nodo* nodo : nodos) {
            if (nodo->getColor() != -1) continue;
            std::set<int> distintos;
            for (const Nodo* vecino : nodo->getAdyacentes())
                if (vecino->getColor() >= 0) distintos.insert(vecino->getColor());
            int saturacion = static_cast<int>(distintos.size());
            std::size_t grado = nodo->getAdyacentes().size();
            if (elegido == nullptr || saturacion > mejorSaturacion ||
                (saturacion == mejorSaturacion && grado > mejorGrado) ||
                (saturacion == mejorSaturacion && grado == mejorGrado && nodo->getId() < elegido->getId())) {
                elegido = nodo;
                mejorSaturacion = saturacion;
                mejorGrado = grado;
            }
        }
        std::set<int> usados;
        for (const Nodo* vecino : elegido->getAdyacentes())
            if (vecino->getColor() >= 0) usados.insert(vecino->getColor());
        int color = 0;
        while (usados.count(color) != 0) ++color;
        elegido->setColor(color);
        totalColores = std::max(totalColores, color + 1);
    }
    return totalColores;
}

// RLF: construye una clase independiente completa para cada color.
int GrafoColoreado::coloreadoRLF() {
    reiniciarColores();
    std::set<Nodo*> pendientes(nodos.begin(), nodos.end());
    int color = 0;
    while (!pendientes.empty()) {
        std::set<Nodo*> candidatos = pendientes;
        while (!candidatos.empty()) {
            Nodo* elegido = nullptr;
            int mejorBloqueo = -1;
            int mejorGrado = -1;
            for (Nodo* nodo : candidatos) {
                int bloqueo = 0;
                int grado = 0;
                for (Nodo* vecino : nodo->getAdyacentes()) {
                    if (pendientes.count(vecino) == 0) continue;
                    if (candidatos.count(vecino) == 0) ++bloqueo;
                    else ++grado;
                }
                if (elegido == nullptr || bloqueo > mejorBloqueo ||
                    (bloqueo == mejorBloqueo && grado > mejorGrado) ||
                    (bloqueo == mejorBloqueo && grado == mejorGrado && nodo->getId() < elegido->getId())) {
                    elegido = nodo;
                    mejorBloqueo = bloqueo;
                    mejorGrado = grado;
                }
            }
            elegido->setColor(color);
            pendientes.erase(elegido);
            candidatos.erase(elegido);
            for (Nodo* vecino : elegido->getAdyacentes()) candidatos.erase(vecino);
        }
        ++color;
    }
    totalColores = color;
    return totalColores;
}

// Backtracking exacto con poda y seleccion dinamica tipo DSATUR.
// Calcula el numero cromatico; el coste puede ser exponencial.
int GrafoColoreado::coloreadoExacto() {
    int cotaSuperior = coloreadoDSATUR();
    if (nodos.empty()) return 0;
    const int n = static_cast<int>(nodos.size());
    std::vector<int> mejores(n);
    for (int i = 0; i < n; ++i) mejores[i] = nodos[i]->getColor();
    std::vector<int> colores(n, -1);
    std::vector<std::vector<bool>> aristas(n, std::vector<bool>(n, false));
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            aristas[i][j] = std::find(nodos[i]->getAdyacentes().begin(),
                                     nodos[i]->getAdyacentes().end(), nodos[j]) != nodos[i]->getAdyacentes().end();

    std::function<void(int, int)> buscar = [&](int asignados, int utilizados) {
        if (utilizados >= cotaSuperior) return;
        if (asignados == n) {
            cotaSuperior = utilizados;
            mejores = colores;
            return;
        }
        int elegido = -1, maximaSat = -1, maximoGrado = -1;
        for (int i = 0; i < n; ++i) {
            if (colores[i] != -1) continue;
            std::set<int> coloresVecinos;
            int grado = 0;
            for (int j = 0; j < n; ++j) {
                if (!aristas[i][j]) continue;
                ++grado;
                if (colores[j] != -1) coloresVecinos.insert(colores[j]);
            }
            int sat = static_cast<int>(coloresVecinos.size());
            if (elegido < 0 || sat > maximaSat ||
                (sat == maximaSat && grado > maximoGrado) ||
                (sat == maximaSat && grado == maximoGrado && nodos[i]->getId() < nodos[elegido]->getId())) {
                elegido = i;
                maximaSat = sat;
                maximoGrado = grado;
            }
        }
        for (int color = 0; color <= utilizados && color < cotaSuperior; ++color) {
            bool valido = true;
            for (int j = 0; j < n; ++j)
                if (aristas[elegido][j] && colores[j] == color) { valido = false; break; }
            if (!valido) continue;
            colores[elegido] = color;
            buscar(asignados + 1, std::max(utilizados, color + 1));
            colores[elegido] = -1;
        }
    };
    buscar(0, 0);
    for (int i = 0; i < n; ++i) nodos[i]->setColor(mejores[i]);
    totalColores = cotaSuperior;
    return totalColores;
}

int GrafoColoreado::getCantidadNodos() const { return static_cast<int>(nodos.size()); }

void GrafoColoreado::mostrarGrafo() const {
    std::cout << "\nEstructura del Grafo (" << nodos.size() << " nodos):\n";
    for (const Nodo* nodo : nodos) {
        std::cout << "Nodo " << nodo->getId() << " -> ";
        for (const Nodo* vecino : nodo->getAdyacentes()) std::cout << vecino->getId() << " ";
        std::cout << '\n';
    }
}

void GrafoColoreado::mostrarColoreado() const {
    if (nodos.empty()) { std::cout << "\nEl grafo esta vacio.\n"; return; }
    if (totalColores == 0) { std::cout << "\nPrimero debe calcular el coloreado.\n"; return; }
    std::cout << "\nColoreado del Grafo:\n";
    for (const Nodo* nodo : nodos)
        std::cout << "Nodo " << nodo->getId() << ": color " << nodo->getColor() + 1 << '\n';
    std::cout << "Total de colores utilizados: " << totalColores << '\n';
}
