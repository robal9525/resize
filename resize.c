#include <stdio.h>
#include <stdlib.h>

#include "bmp.h"

int main(int argc, char *argv[])
{
    // asegurar el uso adecuado
    if (argc != 4)
    {
        fprintf(stderr, "Usage: copy infile outfile\n");
        return 1;
    }
    // primer argumento en linea de comando, sera el factor de cresimiento de mi imagen.
    int n = atoi(argv[1]);
    // recordar nombres de archivos
    char *infile = argv[2];
    char *outfile = argv[3];

    if ((n < 1) || (n > 100))
    {
        fprintf(stderr, "El primer argumento tiene que ser un numero de 1 a 100");
        return 2;
    }

    // abrir archivo de entrada
    FILE *inptr = fopen(infile, "r");
    if (inptr == NULL)
    {
        fprintf(stderr, "Could not open %s.\n", infile);
        return 3;
    }

    // abrir archivo de salida
    FILE *outptr = fopen(outfile, "w");
    if (outptr == NULL)
    {
        fclose(inptr);
        fprintf(stderr, "Could not create %s.\n", outfile);
        return 4;
    }

    // read infile's BITMAPFILEHEADER,  leer el BITMAPFILEHEADER de infile
    BITMAPFILEHEADER bf;
    fread(&bf, sizeof(BITMAPFILEHEADER), 1, inptr);

    // read infile's BITMAPINFOHEADER , leer BITMAPINFOHEADER de infile
    BITMAPINFOHEADER bi;
    fread(&bi, sizeof(BITMAPINFOHEADER), 1, inptr);

    // ensure infile is (likely) a 24-bit uncompressed BMP 4.0
    // asegúrese de que el archivo sea (probablemente) un BMP 4.0 sin comprimir de 24 bits
    if (bf.bfType != 0x4d42 || bf.bfOffBits != 54 || bi.biSize != 40 ||
        bi.biBitCount != 24 || bi.biCompression != 0)
    {
        fclose(outptr);
        fclose(inptr);
        fprintf(stderr, "Unsupported file format.\n");
        return 5;
    }

    int altura_original = bi.biHeight;
    int anchura_original = bi.biWidth;
    int altura_nueva = altura_original * n;
    int anchura_nueva = anchura_original * n;

    /* calculo del relleno en caso que el ancho de la imagen
       es decir el numero de pixeles a lo horizontal, no sea
       multiplo de 4. se multiplica el numero de pixeles que
       hay a lo ancho con su tamaño en byte que es de 3 por pixel(24 )
       bits por pixel) se extrae el residuo de su division entrs 4
       y se le resta al primer 4...*/

    int relleno_original = (4 - (anchura_original * sizeof(RGBTRIPLE)) % 4) % 4;
    int relleno_nuevo = (4 - (anchura_nueva * sizeof(RGBTRIPLE)) % 4) % 4;

    /*se reconfigurara los archivos de cabecera, para indicarle las nuevas
    dimensiones del archivo*/

    bi.biHeight = altura_nueva;
    bi.biWidth = anchura_nueva;

    /* se le asigna su nuevo tamaño al archivo de cabecera que contiene
    la informacion del tamaño de la imagen de  mapa de bits bmp*/
    bi.biSizeImage = ((sizeof(RGBTRIPLE) * anchura_nueva) + relleno_nuevo) * abs(altura_nueva);

    /* se calcula el tamaño total del archivo que seria, el tamaño de la imagen(lo calculado
    anteriormente), mas el tamaño del archivo de cabecera(tamaño total), mas el tamaño del
    de informacion de cabecera*/
    bf.bfSize = bi.biSizeImage + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);


    // write outfile's BITMAPFILEHEADER, escribir el BITMAPFILEHEADER del archivo
    fwrite(&bf, sizeof(BITMAPFILEHEADER), 1, outptr);

    // write outfile's BITMAPINFOHEADER, escribir BITMAPINFOHEADER del archivo
    fwrite(&bi, sizeof(BITMAPINFOHEADER), 1, outptr);


  /* primer for, lo que hace es iterar entre todas las filas, osea de arriba hacia abajo,
     usando para eso una variable(y) cuyo  ciclo finalizara hasta que y = a la altura original
     de la imagen,por ejemplo: small.bmp mide 3 pixeles de alto por 3 de ancho (sin incluir el relleno)
     el ciclo del primer for terminara cuano y = 2, (0,1,2 tres valores poibles para y, y los 3 pixeles de alto de small.bmp)*/
  for (int y = 0, bi_altura = abs(altura_original); y < bi_altura; y++)
  {
      /*se declara una variable de tipo arreglo, que tendra un ancho de 3 pixeles en el caso de small.bmp
        esta se ocupara para guardar la fila entera nueva mas adelante*/
      RGBTRIPLE fila[anchura_nueva];
      /*se declara una variable de tipo entero llamada pixel, la cual su unico proposito es
      servir de contador*/
      int pixel = 0;
      /*el segundo for se ocupa para navegar entre las posiciones de pixeles horizontales(el ancho) de la imagen,
      se declara la variable (x) la cual finalizara su ciclo de iteracion cuando alcance  el valor del ancho de la imagen
      en el caso de small.bmp es 3 pixeles*/
      for (int x = 0; x < anchura_original; x++)
      {
          /*se declara un variable temporal llamada triple, la cual copiara el valor que este guardado en la en la posicion
          (x,y de los for 1 y 2) de la matriz o mapa de bits, por ejemplo: en small.bmp la posicion y = 2, x = 2 contiene
          el unico pixel blanco, entonces la variable triple = (ffffff). el  valor de triple ira variando conforme la los valores
          de (x , y) varien*/
          RGBTRIPLE triple;

          /* simplemente leemos lo que esta en la posicion y , x,  y para eso usamos 'fread', y accedemos al valor que tiene en
          ese instate la vartiable triple, especificamos su tama;o que sera de 3 byte lo que pesa un pixel en bmp, tambien
          especicamos que solo leera un valor es decir el de triple, y por ultimo el nombre del fichero de entrada o donde se almacena
          el mapa de bit original*/
          fread(&triple, sizeof(RGBTRIPLE), 1, inptr);

          /* el tercer for usado, esta dentro del ciclo del segundo for, y se usa para iterar valores hasta que la variable declarada
          (pixel) es igual a el numero del factor(n) por el cual se agranadara la imagen*/
          for (int p = 0; p < n; p++, pixel++)
          {
              /* esta parte es para igualar, cada una de las posiciones de elementos contenidos en nuestro arreglo declarado (fila),
              a el valoer que contiene nuestra variable auxiliar (triple), por ejemplo:en small.bmp, con un factor de agrandamiento = 3,
              en la pocicion (y = 2, x = 2), si triple = (ffffff), entonces fila[0] = (ffffff), fila[1] = (ffffff), fila[2] = (ffffff)
              se debe recordar que el arreglo triple en vez de guardar tipicos caracteres, guardar pixeles*/
              fila[pixel] = triple;
          }
      }
       /*esta funcio , es para saltarece y no leer los bloquees que vendrian conteniendo el relleno, una vez que culmine el segundo
        ciclo for, o que se hallan leido los datos que especifica el parametro de anchura original. se especifica el nombre del ficheo que contiene
        la imagen original, luego se designa cuanto espacio de lectura es el que se saltara, y este sera igual al relleno original, y por ultimo
        se designa, desde donde se realizara el salto, en este caso de la posicion actual en la que esta x , y*/
       fseek(inptr, relleno_original, SEEK_CUR);

      /* el cuarto for, contenido en el primer for, se encaraga simplemente de realizar la funcion (fwrite) n veces,
      esto quiere decir que, es la forma con la cual se multiplicaran las filas o el arreleglo contenidos en la variable (fila)
      esta tendra el tama;o ya modificado en el tercer for*/
      for (int x1 = 0; x1 < n; x1++)
      {
          /* se ejecuta la funcion fwrite , la cual escribe , o en este caso con mas presicion contruye nuestra nueva imagen ya modificada..
          se accede a el contenido de variable (fila), que tendra un tama;o (n) veces el de (triple), se le especifica el tama;o que tendra
          es decir el de la misma variable (fila) en este caso, se designa que solo se lea el contenido en especifico de fila con el (1),
          y por ultimo el nobre del fichero en donde estara guardada nuestra nueva imagen(nuestro archivo de salida outptr)*/
          fwrite(&fila, sizeof(fila), 1, outptr);

          /* el quinto for , contenido en el cuarto for, iterara el valor de la variable (relleno) hasta que esta sea igual al valor
          a la variable relleno nuevo*/
          for (int relleno = 0; relleno < relleno_nuevo; relleno++)
          {
              /* en cada iteracion del quinto for, si es que la hay, se insertara el valor de 0x00 en cada una de las iteaciones
              del cuarto for,  esto quiere decir que esta es la funcion encargada de escribir el relleno una vez, se halla contruido
              la fila*/
              fputc(0x00, outptr);
          }
      }


  }
    // close infile
    fclose(inptr);

    // close outfile
    fclose(outptr);

    // success
    return 0;
}
