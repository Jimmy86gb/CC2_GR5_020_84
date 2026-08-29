#include <iostream>
#include <cmath>
#include <cstdint>
using namespace std;
// [es una funcion que puse para facilitar el proceso de BBP, y que no se desborde]
long long calcularPotenciaModular(long long base, long long exponente, long long modulo) {
	long long resultado = 1;
	base = base % modulo;
	while (exponente > 0) {
		if (exponente % 2 == 1) {
			resultado = (resultado * base) % modulo;
		}
		base = (base * base) % modulo;
		exponente /= 2;
	}
	return resultado;
}

/*	[Basicamente esta funcion lo que hace es calcular en una posicion que digito de pi esta, es la que encontre con este comportamiento
	sirve pq nos ayuda a aleatorizar, desordenar mejor el mensaje, pero no encontre algo parecido para base 10]
*/
double calcularSerieBBP(int j, int n) {
	double suma = 0.0;
	for (int k = 0; k <= n; ++k) {
		long long r = 8 * k + j;
		double termino = (double)calcularPotenciaModular(16, n - k, r) / r;
		suma = suma + termino - (int)suma;
	}
	for (int k = n + 1; k <= n + 10; ++k) {
		long long r = 8 * k + j;
		double termino = pow(16.0, n - k) / r;
		suma = suma + termino - (int)suma;
	}
	return suma;
}

// [es la funcion que dirije el proceso de hallar el digito]
uint8_t obtenerDigitoPi(int posicion) {
	double fraccionario = 4.0 * calcularSerieBBP(1, posicion) - 2.0 * calcularSerieBBP(4, posicion) - 1.0 * calcularSerieBBP(5, posicion) - 1.0 * calcularSerieBBP(6, posicion);
	fraccionario = fraccionario - (int)fraccionario;
	if (fraccionario < 0) {
		fraccionario += 1.0;
	}
	return (uint8_t)(fraccionario * 16.0);
}

// [basicamente juega un poco con los bits como nos menciono el profesor hoy, aparte los niega pq me interesaba añadirlo y probar]
uint32_t mezclarEstado(uint32_t estadoActual, uint8_t digitoPi, int bitsDesplazamiento) {
	estadoActual = (estadoActual ^ (estadoActual >> bitsDesplazamiento)) + digitoPi;
	return ~estadoActual;
}

// [solo aplica el modulo y ya]
uint32_t aplicarLimite(uint32_t estadoActual, uint32_t limite) {
	if (limite == 0) return 0;
	return estadoActual % limite;
}


int main() {
	int posicionInicial, salto, desplazamientoBits, cantidadNumeros;
	uint32_t limiteSuperior;
	
	cout << "Posicion inicial en Pi (S0): ";
	//esta seria la semilla
	cin >> posicionInicial;
	
	
	cout << "Salto entre digitos (J): ";
	//es una variable que determina los saltos entre digitos de pi, es mejor si es primo
	cin >> salto;
	
	
	cout << "Bits a correr a la derecha (k): ";
	//que tantos bits correra a la derecha cuando juegue con ellos, no puede ser 0
	cin >> desplazamientoBits;
	
	
	cout << "Limite superior de los numeros (L): ";
	//esto lo podemos dejar constante en 20 o dejarlo asi por si necesitamos despues
	cin >> limiteSuperior;
	
	cout << "Cantidad de numeros a generar: ";
	//para tantear muestras, tarda un cuanto en mi pc generar 10000 digitos, pero no creo que se extienda tanto
	cin >> cantidadNumeros;
	
	uint32_t estado = 67; //es nuestro numero, no tiene signo para evitar errores, y aprovecha ese bit extra como info
	int iteracionesCalentamiento = 15;
	
	// calentamiento del estado, basicamente unos intentos para que la poca capacidad de "corromper" de los digitos de pi, tengan tiempo a hacer efecto
	for (int i = 0; i < iteracionesCalentamiento; ++i) {
		int posicionActual = posicionInicial + (i * salto);
		uint8_t digitoPi = obtenerDigitoPi(posicionActual);
		estado = mezclarEstado(estado, digitoPi, desplazamientoBits);
	}
	
	// solo imprime y ya
	int posicionActual = posicionInicial + (iteracionesCalentamiento * salto);
	for (int n = 0; n < cantidadNumeros; ++n) {
		uint8_t digitoPi = obtenerDigitoPi(posicionActual);
		estado = mezclarEstado(estado, digitoPi, desplazamientoBits);
		
		uint32_t numeroGenerado = aplicarLimite(estado, limiteSuperior);
		cout << numeroGenerado << " ";
		
		posicionActual += salto;
	}
	cout << "\n";
	
	return 0;
}
