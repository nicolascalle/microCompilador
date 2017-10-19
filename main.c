/* =================COMPILADOR DE MICRO====================*/
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


/* ------ Declaramos las estructuras  ------- */
    #define FilasTS 15
	#define ColumnasTS 13
	#define TamanioNombreLexema 32+1
typedef enum
	{
	    INICIO, FIN, LEER, ESCRIBIR, ID, CONSTANTE, PARENIZQUIERDO, PARENDERECHO, PUNTOYCOMA, COMA, ASIGNACION, SUMA, RESTA, FDT, ERRORLEXICO
	} TOKEN;

	typedef struct
	{
	    char identificadorDeLexema[TamanioNombreLexema];
	    TOKEN t;
	} RegTS;


	typedef struct
	{
	    TOKEN clase;
	    char nombre[TamanioNombreLexema];
	    int valor;
	} REG_EXPRESION;


/* ------ Declaramos las funciones  ------- */
//Scanner
	TOKEN scanner();
	int columnaTS(int c);
	int estadoFinal(int e);

//Parser
	void Objetivo(void);
	void Programa(void);
	void ListaSentencias(void);
	void Sentencia(void);
	void ListaIdentificadores(void);
	void Identificador(REG_EXPRESION * resultado);
	void ListaExpresiones(void);
	void Expresion(REG_EXPRESION * resultado);
	void Primaria(REG_EXPRESION * resultado);
	void OperadorAditivo(char * resultado);

//Rutinas semanticas
	REG_EXPRESION ProcesarCte(void);
	REG_EXPRESION ProcesarId(void);
	char * ProcesarOp(void);
	void Leer(REG_EXPRESION in);
	void Escribir(REG_EXPRESION out);
	REG_EXPRESION GenInfijo(REG_EXPRESION e1, char * op, REG_EXPRESION e2);

//Funciones auxiliares
	void Match(TOKEN t);
	TOKEN ProximoToken();
	void ErrorLexico();
	void ErrorSintactico();
	void Generar(char * co, char * a, char * b, char * c);
	char * Extraer(REG_EXPRESION * preg);
	int Buscar(char * id, TOKEN * t);
	void Colocar(char * id);
	void Chequear(char * s);
	void Comenzar(void);
	void Terminar(void);
	void Asignar(REG_EXPRESION izq, REG_EXPRESION der);


/* ------  Declaramos las variables globales  ------- */

	FILE * in;
	FILE * out;
	RegTS TS[1000] = { {"inicio", INICIO}, {"fin", FIN}, {"leer", LEER}, {"escribir", ESCRIBIR}, {"$", 99} };
	char buffer[TamanioNombreLexema];
	TOKEN tokenActual;
	int flagToken = 0;


/* ---------  Creamos el programa comando  --------- */
int main(int argc, char * argv[])
{
    printf("------------------------------------------");
	printf("\n       COMPILADOR DEL LENGUAJE MICRO\n");
	printf("------------------------------------------\n\n");
	int l = strlen(argv[1]);

    /* --------------  Hacemos las validaciones correspondientes:  -------------- */
	    if ( argc == 1 )
	    {
	        printf("Error. Ingresar el nombre del archivo fuente y el nombre del archivo de salida\n");
	        return -1;
	    }
	    if ( argc == 2 )
	    {
	        printf("Error. Falta ingresar el nombre del archivo de salida.\n");
	        return -1;
	    }
	    if ( argv[1][l-1] != 'm' || argv[1][l-2] != '.' )
	    {
	        printf("Error. El archivo fuente debe finalizar con \".m\"\n");
	        return -1;
	    }
	    if ( (in = fopen(argv[1], "r") ) == NULL )
	    {
	        printf("No se pudo abrir archivo fuente\n");
	        return -1;
	    }
		    if ( (out = fopen(argv[2], "w") ) == NULL)
	    {
	        printf("No se pudo abrir archivo de salida\n");
	        return -1;
	    }

    /* --------------  Iniciamos el proceso de compilacion:  -------------- */
    	Objetivo();

    /* --------------  Terminado el proceso, cerramos los archivos:  -------------- */
    	fclose(in);
    	fclose(out);

    return 0;
}



/* -------------------  SCANNER ------------------- */
TOKEN scanner()
{
    int tabla[FilasTS][ColumnasTS] = { {  1,  3,  5,  6,  7,  8,  9, 10, 11, 14, 13,  0, 14 },
                                       {  1,  1,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2 },
                                       { 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14 },
                                       {  4,  3,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4 },
                                       { 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14 },
                                       { 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14 },
                                       { 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14 },
                                       { 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14 },
                                       { 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14 },
                                       { 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14 },
                                       { 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14 },
                                       { 14, 14, 14, 14, 14, 14, 14, 14, 14, 12, 14, 14, 14 },
                                       { 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14 },
                                       { 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14 },
                                       { 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14 } };
    int caracter;
    int columna;
    int estado = 0;
    int i = 0;
    do
    {
        caracter = fgetc(in);
        columna = columnaTS(caracter);
        estado = tabla[estado][columna];
        if ( columna != 11 )
        {
            buffer[i] = caracter;
            i++;
        }
    }while ( !estadoFinal(estado) && !(estado == 14) );

    buffer[i] = '\0';
    switch ( estado )
    {
        case 2 :
            if ( columna != 11 )
            {
            ungetc(caracter,in);
            buffer[i-1] = '\0';
            }
            return ID;
        case 4 :
            if ( columna != 11 )
            {
            ungetc(caracter, in);
            buffer[i-1] = '\0';
            }
            return CONSTANTE;
        case 5 : return SUMA;
        case 6 : return RESTA;
        case 7 : return PARENIZQUIERDO;
        case 8 : return PARENDERECHO;
        case 9 : return COMA;
        case 10 : return PUNTOYCOMA;
        case 12 : return ASIGNACION;
        case 13 : return FDT;
        case 14 : return ERRORLEXICO;
    }
    return 0;
}

int estadoFinal(int e)
{
    if ( e == 0 || e == 1 || e == 3 || e == 11 || e == 14 ) return 0;
    return 1;
}

int columnaTS(int c)
{
    if ( isalpha(c) ) return 0;
    if ( isdigit(c) ) return 1;
    if ( c == '+' ) return 2;
    if ( c == '-' ) return 3;
    if ( c == '(' ) return 4;
    if ( c == ')' ) return 5;
    if ( c == ',' ) return 6;
    if ( c == ';' ) return 7;
    if ( c == ':' ) return 8;
    if ( c == '=' ) return 9;
    if ( c == EOF ) return 10;
    if ( isspace(c) ) return 11;
    return 12;
}


/* -------------- PROCEDIMIENTOS DE ANALISIS SINTACTICO (PAS) -------------- */
void Objetivo(void)
{
    /* <objetivo> -> <programa> FDT #terminar */
    Programa();
    Match(FDT);
    Terminar();
}

void Programa(void)
{
    /* <programa> -> #comenzar INICIO <listaSentencias> FIN */
    Comenzar();
    Match(INICIO);
    ListaSentencias();
    Match(FIN);
}

void ListaSentencias(void)
{
    /* <listaSentencias> -> <sentencia> {<sentencia>} */
    Sentencia();
    while ( 1 )
    {
        switch ( ProximoToken() )
        {
        case ID : case LEER : case ESCRIBIR :
            Sentencia();
            break;
        default : return;
        }
    }
}

void Sentencia(void)
{
    TOKEN tok = ProximoToken();
    REG_EXPRESION izq, der;
    switch ( tok )
    {
        case ID :
            /* <sentencia>-> ID := <expresion> #asignar ; */
            Identificador(&izq);
            Match(ASIGNACION);
            Expresion(&der);
            Asignar(izq, der);
            Match(PUNTOYCOMA);
            break;
        case LEER :
            /* <sentencia> -> LEER ( <listaIdentificadores> ) */
            Match(LEER);
            Match(PARENIZQUIERDO);
            ListaIdentificadores();
            Match(PARENDERECHO);
            Match(PUNTOYCOMA);
            break;
        case ESCRIBIR :
            /* <sentencia> -> ESCRIBIR ( <listaExpresiones> ) */
            Match(ESCRIBIR);
            Match(PARENIZQUIERDO);
            ListaExpresiones();
            Match(PARENDERECHO);
            Match(PUNTOYCOMA);
            break;
        default :
            return;
    }
}

void ListaIdentificadores(void)
{
    /* <listaIdentificadores> -> <identificador> #leer_id {COMA <identificador> #leer_id} */
    TOKEN t;
    REG_EXPRESION reg;
    Identificador(&reg);
    Leer(reg);
    for ( t = ProximoToken(); t == COMA; t = ProximoToken() )
    {
        Match(COMA);
        Identificador(&reg);
        Leer(reg);
    }

}

void Identificador(REG_EXPRESION * resultado)
{
    /* <identificador> -> ID #procesar_id */
    Match(ID);
    *resultado = ProcesarId();
}

void ListaExpresiones(void)
{
    /* <listaExpresiones> -> <expresion> #escribir_exp {COMA <expresion> #escribir_exp} */
    TOKEN t;
    REG_EXPRESION reg;
    Expresion(&reg);
    Escribir(reg);
    for ( t = ProximoToken(); t == COMA; t = ProximoToken() )
    {
        Match(COMA);
        Expresion(&reg);
        Escribir(reg);
    }
}

void Expresion(REG_EXPRESION * resultado)
{
    /* <expresion> -> <primaria> { <operadorAditivo> <primaria> #gen_infijo } */
    REG_EXPRESION operandoIzq, operandoDer;
    char op[TamanioNombreLexema];
    TOKEN t;
    Primaria(&operandoIzq);
    for ( t = ProximoToken(); t == SUMA || t == RESTA; t = ProximoToken() )
    {
        OperadorAditivo(op);
        Primaria(&operandoDer);
        operandoIzq = GenInfijo(operandoIzq, op, operandoDer);
    }
    *resultado = operandoIzq;
}

void Primaria(REG_EXPRESION * resultado)
{
    TOKEN tok = ProximoToken();
    switch ( tok )
    {
        case ID :
            /* <primaria> -> <identificador> */
            Identificador(resultado);
            break;
        case CONSTANTE :
            /* <primaria> -> CONSTANTE #procesar_cte */
            Match(CONSTANTE);
            *resultado = ProcesarCte();
            break;
        case PARENIZQUIERDO :
            /* <primaria> -> PARENIZQUIERDO <expresion> PARENDERECHO */
            Match(PARENIZQUIERDO);
            Expresion(resultado);
            Match(PARENDERECHO);
            break;
        default :
            return;
    }
}

void OperadorAditivo(char * resultado)
{
    /* <operadorAditivo> -> SUMA #procesar_op | RESTA #procesar_op */
    TOKEN t = ProximoToken();
    if ( t == SUMA || t == RESTA )
    {
        Match(t);
        strcpy(resultado, ProcesarOp());
    }
    else
        ErrorSintactico();
}


/* -------------------  RUTINAS SEMANTICAS  ------------------- */
REG_EXPRESION ProcesarCte(void)
{
    /* Convierte cadena que representa numero a numero entero y construye un registro semantico */
    REG_EXPRESION reg;
    reg.clase = CONSTANTE;
    strcpy(reg.nombre, buffer);
    sscanf(buffer, "%d", &reg.valor);
    return reg;
}

REG_EXPRESION ProcesarId(void)
{
    /* Declara ID y construye el correspondiente registro semantico */
    REG_EXPRESION reg;
    Chequear(buffer);
    reg.clase = ID;
    strcpy(reg.nombre, buffer);
    return reg;
}

char * ProcesarOp(void)
{
    /* Declara OP y construye el correspondiente registro semantico */
    return buffer;
}

void Leer(REG_EXPRESION in)
{
    /* Genera la instruccion para leer */
    Generar("Read", in.nombre, "Entera", "");
}

void Escribir(REG_EXPRESION out)
{
    /* Genera la instruccion para escribir */
    Generar("Write", Extraer(&out), "Entera", "");
}

REG_EXPRESION GenInfijo(REG_EXPRESION e1, char * op, REG_EXPRESION e2)
{
/* Genera la instruccion para una operacion infija y construye un registro semantico con el resultado */
    REG_EXPRESION reg;
    static unsigned int numTemp = 1;
    char cadTemp[TamanioNombreLexema] ="Temp&";
    char cadNum[TamanioNombreLexema];
    char cadOp[TamanioNombreLexema];
    if ( op[0] == '-' ) strcpy(cadOp, "Restar");
    if ( op[0] == '+' ) strcpy(cadOp, "Sumar");
    sprintf(cadNum, "%d", numTemp);
    numTemp++;
    strcat(cadTemp, cadNum);
    if ( e1.clase == ID) Chequear(Extraer(&e1));
    if ( e2.clase == ID) Chequear(Extraer(&e2));
    Chequear(cadTemp);
    Generar(cadOp, Extraer(&e1), Extraer(&e2), cadTemp);
    strcpy(reg.nombre, cadTemp);
    return reg;
}


/* -------------------  FUNCIONES AUXILIARES  ------------------- */
void Match(TOKEN t)
{
    if ( !(t == ProximoToken()) ) ErrorSintactico();
    flagToken = 0;
}

TOKEN ProximoToken()
{
    if ( !flagToken )
    {
        tokenActual = scanner();
        if ( tokenActual == ERRORLEXICO ) ErrorLexico();
        flagToken = 1;
        if ( tokenActual == ID ) Buscar(buffer, &tokenActual);
    }
    return tokenActual;
}

void ErrorLexico()
{
    printf("--Error Lexico--\n");
    fprintf(out, "Error Lexico\n");

}

void ErrorSintactico()
{
    printf("--Error Sintactico--\n");
    fprintf(out, "Error Sintactico");
}

void Generar(char * accion, char * a, char * b, char * c)
{
    /* Produce la salida de la instruccion para la MV por stdout */
    printf("%s %s%c%s%s\n", accion, a, ',', b, c);
    fprintf(out,"%s %s%c%s%s\n", accion, a, ',', b, c );
}

char * Extraer(REG_EXPRESION * preg)
{
    /* Retorna la cadena del registro semantico */
    return preg->nombre;
}

int Buscar(char * id, TOKEN * t)
{
    /* Determina si un identificador esta en la TS */
    int i = 0;
    while ( strcmp("$", TS[i].identificadorDeLexema) )
    {
        if ( !strcmp(id, TS[i].identificadorDeLexema) )
        {
            *t = TS[i].t;
            return 1;
        }
        i++;
    }
    return 0;
}

void Colocar(char * id)
{
    /* Agrega un identificador a la TS */
    int i = 4;
    while ( strcmp("$", TS[i].identificadorDeLexema) ) i++;
    {
        if ( i < 999 )
        {
            strcpy(TS[i].identificadorDeLexema, id );
            TS[i].t = ID;
            strcpy(TS[++i].identificadorDeLexema, "$" );
        }
    }
}

void Chequear(char * s)
{
    /* Si la cadena No esta en la Tabla de Simbolos la agrega,y si es el nombre de una variable genera la instruccion */
    TOKEN t;
    if ( !Buscar(s, &t) )
    {
        Colocar(s);
        Generar("Declara", s, "Entera", "");
    }
}

void Comenzar(void)
{
    /* Inicializaciones Semanticas */
    printf("-------Inicio de la compilacion-------\n");
    fprintf(out,"Inicio de  la compilacion\n");
}

void Terminar(void)
{
    /* Genera la instruccion para terminar la ejecucion del programa */
    printf("-------Fin de la compilacion-------\n");
    fprintf(out,"Fin de la compilacion\n");
}

void Asignar(REG_EXPRESION izq, REG_EXPRESION der)
{
    /* Genera la instruccion para la asignacion */
    Generar("Almacena", Extraer(&der), izq.nombre, "");
}

