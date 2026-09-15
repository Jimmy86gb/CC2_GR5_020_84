import java.math.BigInteger;
import java.util.Scanner;

public class Mezclador256Bits {
    private static final BigInteger MODULO_256_BITS = BigInteger.ONE.shiftLeft(256);
    private static final BigInteger MULTIPLICADOR_ANTERIOR = BigInteger.valueOf(12);
    private static final BigInteger MULTIPLICADOR_PENULTIMO = BigInteger.valueOf(5);
    private static final int LONGITUD_MINIMA = 32;

    // Constantes para la difusión y mezcla de los 256 bits
    private static final BigInteger CONSTANTE_DIFUSION = 
            new BigInteger("9E3779B97F4A7C15F39CC0605CEDC8341082276BF3A27251F86C6A11D0C18E95", 16);
    private static final BigInteger PERTURBACION = 
            new BigInteger("A5A5A5A5A5A5A5A5A5A5A5A5A5A5A5A5A5A5A5A5A5A5A5A5A5A5A5A5A5A5A5A5", 16);

    public static BigInteger calcularHash(String texto) {
        int longitudOriginal = texto.length();
        BigInteger penultimo = BigInteger.valueOf(3);
        BigInteger anterior = BigInteger.valueOf(7).xor(BigInteger.valueOf(longitudOriginal));

        String textoRellenado = rellenarHasta32Caracteres(texto);

        for (int indice = 0; indice < textoRellenado.length(); indice++) {
            char caracter = textoRellenado.charAt(indice);
            int posicion = indice + 1;
            int valorCaracter = (int) caracter;

            BigInteger mezcla = BigInteger.valueOf(valorCaracter)
                    .xor(BigInteger.valueOf(posicion * 9L));

            BigInteger nuevo = anterior.multiply(MULTIPLICADOR_ANTERIOR)
                    .xor(penultimo.multiply(MULTIPLICADOR_PENULTIMO))
                    .xor(mezcla)
                    .mod(MODULO_256_BITS);

            penultimo = anterior;
            anterior = nuevo;
        }

        // 1. Difundir los bits a lo largo de todo el espectro de 256 bits
        BigInteger hashMezclado = difundirBits(anterior);

        // 2. Asegurar que no exista el patrón de ceros en ninguna parte del resultado
        return eliminarSecuenciaCeros(hashMezclado);
    }

    // Distribuye la entropía uniformemente en los 256 bits
    private static BigInteger difundirBits(BigInteger hash) {
        BigInteger h = hash.xor(hash.shiftRight(64));
        h = h.multiply(CONSTANTE_DIFUSION).mod(MODULO_256_BITS);
        h = h.xor(h.shiftRight(128));
        return h;
    }

    // Bucle dinámico que elimina cualquier patrón de 6 ceros consecutivos en hexadecimal
    private static BigInteger eliminarSecuenciaCeros(BigInteger hash) {
        BigInteger resultado = hash;
        while (convertirAHexadecimal(resultado).contains("000000")) {
            resultado = resultado.xor(PERTURBACION).mod(MODULO_256_BITS);
        }
        return resultado;
    }

    private static String rellenarHasta32Caracteres(String texto) {
        StringBuilder resultado = new StringBuilder(texto);
        while (resultado.length() < LONGITUD_MINIMA) {
            resultado.append(' ');
        }
        return resultado.toString();
    }

    public static String convertirAHexadecimal(BigInteger hash) {
        return String.format("%064X", hash);
    }

    public static void main(String[] args) {
        try (Scanner scanner = new Scanner(System.in)) {
            System.out.print("Ingrese el texto: ");
            String texto = scanner.nextLine();

            BigInteger hash = calcularHash(texto);

            System.out.println();
            System.out.println("Hash decimal: " + hash);
            System.out.println("Hash hexadecimal: " + convertirAHexadecimal(hash));
        }
    }
}