#include <iostream>
#include <string>
#include <cmath>
#include <sstream>
#include <iomanip>
#include <limits>
#include <cstdlib>
#include <ctime>

using namespace std;

// --- UTILIDADES HEXADECIMALES ---
string aHexadecimal(long long valor) {
	stringstream ss;
	ss << hex << uppercase << setw(8) << setfill('0') << valor;
	return ss.str();
}

long long desdeHexadecimal(const string& hexStr) {
	long long valor;
	stringstream ss(hexStr);
	ss >> hex >> valor;
	return valor;
}

// --- MATEMATICA CRIPTOGRAFICA ---
long long calcularMCD(long long a, long long b) {
	while (b != 0) {
		long long temp = b;
		b = a % b;
		a = temp;
	}
	return a;
}

long long euclidesExtendido(long long e, long long phi) {
	long long m0 = phi, y = 0, x = 1;
	if (phi == 1) return 0;
	while (e > 1) {
		long long q = e / phi;
		long long t = phi;
		phi = e % phi, e = t;
		t = y;
		y = x - q * y;
		x = t;
	}
	if (x < 0) x += m0;
	return x;
}

void generarClavesAsimetricas(long long p, long long q, long long &n, long long &e, long long &d) {
	n = p * q;
	while (n <= 255) {
		cout << "\n[ERROR] Modulo muy pequeno (n = " << n << "). Debe ser mayor a 255.\n";
		cout << "-> Nuevo primer numero primo : "; cin >> p;
		cout << "-> Nuevo segundo numero primo: "; cin >> q;
		n = p * q;
	}
	long long phi = (p - 1) * (q - 1);
	e = 3; 
	while (calcularMCD(e, phi) != 1) e++;
	d = euclidesExtendido(e, phi); 
}

long long expModular(long long base, long long exp, long long mod) {
	long long resultado = 1;
	base = base % mod;
	while (exp > 0) {
		if (exp % 2 == 1) resultado = (resultado * base) % mod;
		exp = exp >> 1;
		base = (base * base) % mod;
	}
	return resultado;
}

long long calcularHashBBP(const string& texto, long long modulo) {
	long long hash = 0;
	for (size_t k = 0; k < texto.length(); k++) {
		double terminoBBP = (4.0 / (8 * k + 1)) - (2.0 / (8 * k + 4)) - 
			(1.0 / (8 * k + 5)) - (1.0 / (8 * k + 6));
		double valorSerie = (1.0 / pow(16, k)) * terminoBBP;
		long long entropiaBBP = abs((long long)(valorSerie * 100000000));
		hash = (hash + ((long long)texto[k] ^ entropiaBBP)) % modulo;
	}
	return (hash == 0) ? 1 : abs(hash); 
}

// --- NUEVO: CIFRADO CON INYECCION DE RUIDO ---
string cifrarMensajeHex(const string& mensaje, long long clavePublica, long long modulo) {
	string cifradoHex = "";
	
	// 1. Generar la ruta secreta (cuánto ruido habrá por cada letra real)
	int salto_ruido = rand() % 4 + 2; // Número entre 2 y 5
	
	// 2. El FRAGMENTO: Cifrar la instrucción de cómo eliminar el ruido
	cifradoHex += aHexadecimal(expModular((long long)salto_ruido, clavePublica, modulo));
	
	// 3. Inyectar ruido intercalado
	for (char c : mensaje) {
		// Letra real
		cifradoHex += aHexadecimal(expModular((long long)c, clavePublica, modulo));
		
		// Generar 'salto_ruido' caracteres basura
		for (int i = 0; i < salto_ruido; i++) {
			char ruido = (char)(rand() % 94 + 32); // Caracteres ASCII imprimibles
			cifradoHex += aHexadecimal(expModular((long long)ruido, clavePublica, modulo));
		}
	}
	return cifradoHex;
}

// --- NUEVO: DESCIFRADO CON EVASIÓN DE RUIDO ---
string descifrarMensajeHex(const string& cifradoHex, long long clavePrivada, long long modulo) {
	if (cifradoHex.length() < 8) return "";
	
	// 1. Extraer el FRAGMENTO inicial para saber cómo evadir el ruido
	string fragmentoInstruccion = cifradoHex.substr(0, 8);
	long long salto_ruido = expModular(desdeHexadecimal(fragmentoInstruccion), clavePrivada, modulo);
	
	// 2. LA PRUEBA: Verificar si el fragmento tiene sentido para quien lo abrió
	bool caminoValido = (salto_ruido >= 2 && salto_ruido <= 5);
	
	if (!caminoValido) {
		cout << "\n[!] PRUEBA FALLIDA: El fragmento de instrucciones es ilegible.\n";
		cout << "    -> No eres el destinatario correcto o el mensaje fue alterado.\n";
		salto_ruido = 0; // Se fuerza a procesar el ruido como si fuera texto real
	} else {
		cout << "\n[*] PRUEBA SUPERADA: Fragmento decodificado.\n";
		cout << "    -> Camino especial encontrado: esquivando " << salto_ruido << " bloques de ruido por caracter.\n";
	}
	
	// 3. Recorrer el mensaje evadiendo el ruido
	string mensajeRecuperado = "";
	size_t i = 8; // Empezamos después del fragmento inicial
	
	while (i + 8 <= cifradoHex.length()) {
		// Leer bloque real
		string fragmento = cifradoHex.substr(i, 8);
		long long valor = desdeHexadecimal(fragmento);
		mensajeRecuperado += (char)expModular(valor, clavePrivada, modulo);
		
		i += 8; // Avanzar al siguiente bloque
		
		// Si pasó la prueba, saltar los bloques de ruido generados
		if (caminoValido) {
			i += (8 * salto_ruido);
		}
	}
	return mensajeRecuperado;
}

void limpiarBuffer() {
	cin.clear();
	cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

int main() {
	srand((unsigned)time(NULL)); // Inicializar semilla aleatoria para el ruido
	int opcion;
	
	while (true) {
		cout << "\n=======================================================\n";
		cout << "     HERRAMIENTA CRIPTOGRAFICA brap (RUIDO ACTIVO)\n";
		cout << "=======================================================\n";
		cout << " 1. Generar nuevo par de claves (Publica/Privada)\n";
		cout << " 2. Enviar: Cifrar y Firmar un mensaje\n";
		cout << " 3. Recibir: Descifrar y Verificar un mensaje\n";
		cout << " 0. Salir\n";
		cout << "-------------------------------------------------------\n";
		cout << "Elige una opcion: ";
		
		if (!(cin >> opcion)) {
			limpiarBuffer();
			continue;
		}
		
		if (opcion == 0) break;
		
		switch (opcion) {
		case 1: {
			long long p, q, n, e, d;
			cout << "\n--- GENERADOR DE CLAVES ---\n";
			cout << "Ingresa dos numeros primos grandes (ej: 47, 53, 61, 97...)\n";
			cout << "Primo 1: "; cin >> p;
			cout << "Primo 2: "; cin >> q;
			
			generarClavesAsimetricas(p, q, n, e, d);
			cout << "\n[!] GUARDA ESTOS DATOS:\n";
			cout << " * Tu Clave Privada (d) : " << d << " (¡NO LA COMPARTAS!)\n";
			cout << " * Tu Clave Publica (e) : " << e << "\n";
			cout << " * Tu Modulo de Red (n) : " << n << "\n";
			break;
		}
		case 2: {
			string mensajeOriginal;
			long long privA, modA; 
			long long pubB, modB;  
			
			cout << "\n--- MODO EMISOR ---\n";
			cout << "1. Tus credenciales (para firmar):\n";
			cout << "   - Tu Clave Privada (dA) : "; cin >> privA;
			cout << "   - Tu Modulo de Red (nA) : "; cin >> modA;
			
			cout << "\n2. Credenciales del destinatario (para cifrar):\n";
			cout << "   - Su Clave Publica (eB) : "; cin >> pubB;
			cout << "   - Su Modulo de Red (nB) : "; cin >> modB;
			
			limpiarBuffer();
			cout << "\n3. Escribe el texto confidencial a enviar: ";
			getline(cin, mensajeOriginal);
			
			string paqueteCifrado = cifrarMensajeHex(mensajeOriginal, pubB, modB);
			long long hashA = calcularHashBBP(mensajeOriginal, modA);
			string firmaDigital = aHexadecimal(expModular(hashA, privA, modA));
			
			cout << "\n================= PAQUETE PARA COPIAR =================\n";
			cout << "MENSAJE_CIFRADO (Con Ruido Inyectado):\n" << paqueteCifrado << "\n\n";
			cout << "FIRMA_DIGITAL:\n" << firmaDigital << "\n";
			cout << "=======================================================\n";
			break;
		}
		case 3: {
			string paqueteCifrado, firmaDigitalStr;
			long long privB, modB; 
			long long pubA, modA;  
			
			cout << "\n--- MODO RECEPTOR ---\n";
			cout << "1. Tus credenciales (para descifrar):\n";
			cout << "   - Tu Clave Privada (dB) : "; cin >> privB;
			cout << "   - Tu Modulo de Red (nB) : "; cin >> modB;
			
			cout << "\n2. Credenciales de quien envia (para verificar):\n";
			cout << "   - Su Clave Publica (eA) : "; cin >> pubA;
			cout << "   - Su Modulo de Red (nA) : "; cin >> modA;
			
			cout << "\n3. Pega el MENSAJE_CIFRADO: ";
			cin >> paqueteCifrado;
			cout << "4. Pega la FIRMA_DIGITAL: ";
			cin >> firmaDigitalStr;
			
			string mensajeDescifrado = descifrarMensajeHex(paqueteCifrado, privB, modB);
			long long firmaDigitalNum = desdeHexadecimal(firmaDigitalStr);
			
			long long hashB = calcularHashBBP(mensajeDescifrado, modA);
			long long hashRevelado = expModular(firmaDigitalNum, pubA, modA);
			
			cout << "\n[!] PROCESANDO FIRMA DIGITAL...\n";
			if (hashB == hashRevelado) {
				cout << "\n[EXITO] VALIDACION PERFECTA. El mensaje es autentico y limpio de ruido.\n";
				cout << "-> CONTENIDO: \"" << mensajeDescifrado << "\"\n";
			} else {
				cout << "\n[ALERTA ROJA] FIRMA INVALIDA O MENSAJE CORRUPTO.\n";
				cout << "-> CONTENIDO RECUPERADO (Lleno de ruido/Ilegible): " << mensajeDescifrado << "\n";
			}
			break;
		}
		default:
			cout << "Opcion no valida.\n";
		}
	}
	return 0;
}
