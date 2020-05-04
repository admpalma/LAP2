/*
largura maxima = 100 colunas
tab = 4 espaços
0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789

	Linguagens e Ambientes de Programação (B) - Projeto de 2019/20

	Cartography.c

	Este ficheiro constitui apenas um ponto de partida para o
	seu trabalho. Todo este ficheiro pode e deve ser alterado
	à vontade, a começar por este comentário. É preciso inventar
	muitas funções novas.

COMPILAÇÃO

  gcc -std=c11 -o Main Cartography.c Main.c -lm

IDENTIFICAÇÃO DOS AUTORES

  Aluno 1: 55415 Andre Palma
  Aluno 2: 54953 Tiago Teles

COMENTÁRIO

 Coloque aqui a identificação do grupo, mais os seus comentários, como
 se pede no enunciado.

*/

#define USE_PTS		true
#include "Cartography.h"

/* STRING -------------------------------------- */

static void showStringVector(StringVector sv, int n) {
	int i;
	for( i = 0 ; i < n ; i++ ) {
		printf("%s\n", sv[i]);
	}
}

/* UTIL */

static void error(String message)
{
	fprintf(stderr, "%s.\n", message);
	exit(1);	// Termina imediatamente a execução do programa
}

static void readLine(String line, FILE *f)	// lê uma linha que existe obrigatoriamente
{
	if( fgets(line, MAX_STRING, f) == NULL )
		error("Ficheiro invalido");
	line[strlen(line) - 1] = '\0';	// elimina o '\n'
}

static int readInt(FILE *f)
{
	int i;
	String line;
	readLine(line, f);
	sscanf(line, "%d", &i);
	return i;
}


/* IDENTIFICATION -------------------------------------- */

static Identification readIdentification(FILE *f)
{
	Identification id;
	String line;
	readLine(line, f);
	sscanf(line, "%s %s %s", id.freguesia, id.concelho, id.distrito);
	return id;
}

static void showIdentification(int pos, Identification id, int z)
{
	if( pos >= 0 ) // pas zero interpretado como não mostrar
		printf("%4d ", pos);
	else
		printf("%4s ", "");
	if( z == 3 )
		printf("%-25s %-13s %-22s", id.freguesia, id.concelho, id.distrito);
	else if( z == 2 )
		printf("%-25s %-13s %-22s", "", id.concelho, id.distrito);
	else
		printf("%-25s %-13s %-22s", "", "", id.distrito);
}

static void showValue(int value)
{
	if( value < 0 ) // value negativo interpretado como char
		printf(" [%c]\n", -value);
	else
		printf(" [%3d]\n", value);
}

static bool sameIdentification(Identification id1, Identification id2, int z)
{
	if( z == 3 )
		return strcmp(id1.freguesia, id2.freguesia) == 0
			&& strcmp(id1.concelho, id2.concelho) == 0
			&& strcmp(id1.distrito, id2.distrito) == 0;
	else if( z == 2 )
		return strcmp(id1.concelho, id2.concelho) == 0
			&& strcmp(id1.distrito, id2.distrito) == 0;
	else
		return strcmp(id1.distrito, id2.distrito) == 0;
}


/* COORDINATES -------------------------------------- */

Coordinates coord(double lat, double lon)
{
	Coordinates c = {lat, lon};
	return c;
}

static Coordinates readCoordinates(FILE *f)
{
	double lat, lon;
	String line;
	readLine(line, f);
	sscanf(line, "%lf %lf", &lat, &lon);
	return coord(lat, lon);
}

bool sameCoordinates(Coordinates c1, Coordinates c2)
{
	return c1.lat == c2.lat && c1.lon == c2.lon;
}

static double toRadians(double deg)
{
	return deg * PI / 180.0;
}

// https://en.wikipedia.org/wiki/Haversine_formula
double haversine(Coordinates c1, Coordinates c2)
{
	double dLat = toRadians(c2.lat - c1.lat);
	double dLon = toRadians(c2.lon - c1.lon);

	double sa = sin(dLat / 2.0);
	double so = sin(dLon / 2.0);

	double a = sa * sa + so * so * cos(toRadians(c1.lat)) * cos(toRadians(c2.lat));
	return EARTH_RADIUS * (2 * asin(sqrt(a)));
}


/* RECTANGLE -------------------------------------- */

Rectangle rect(Coordinates tl, Coordinates br)
{
	Rectangle rect = {tl, br};
	return rect;
}

static void showRectangle(Rectangle r)
{
	printf("{%lf, %lf, %lf, %lf}",
			r.topLeft.lat, r.topLeft.lon,
			r.bottomRight.lat, r.bottomRight.lon);
}

static Rectangle calculateBoundingBox(Coordinates vs[], int n)
{
	//TODO Done?
	if(!n) error("n = 0 at calculateBoundingBox");
	double maxLat=vs[n-1].lat,
		   maxLon=vs[n-1].lon,
		   minLat=vs[n-1].lat,
		   minLon=vs[n-1].lon;
	while(n-->0){
		if(maxLat<vs[n].lat){
			maxLat = vs[n].lat;
		}
		if(maxLon<vs[n].lon){
			maxLon = vs[n].lon;
		}
		if(minLat>vs[n].lat){
			minLat = vs[n].lat;
		}
		if(minLon>vs[n].lon){
			minLon = vs[n].lon;
		}
	}
	/* printf("[%f,%f,%f,%f]\n",maxLat,maxLon,minLat,minLon); */
	return rect(coord(maxLat,minLon), coord(minLat,maxLon));
}

bool insideRectangle(Coordinates c, Rectangle r)
{
	//TODO Done?
	return !(r.topLeft.lat < c.lat ||r.topLeft.lon < c.lon ||
			r.bottomRight.lat > c.lat || r.bottomRight.lon > c.lon);
}


/* RING -------------------------------------- */

static Ring readRing(FILE *f)
{
	Ring r;
	int i, n = readInt(f);
	r.nVertexes = n;
	r.vertexes = malloc(sizeof(Coordinates)*n);
	for( i = 0 ; i < n ; i++ ) {
		r.vertexes[i] = readCoordinates(f);
	}
	r.boundingBox =
		calculateBoundingBox(r.vertexes, r.nVertexes);
	/* printf("[%f,%f,%f,%f]\n",r.boundingBox.topLeft.lat,r.boundingBox.topLeft.lon,r.boundingBox.bottomRight.lat,r.boundingBox.bottomRight.lon); */
	return r;
}


// http://alienryderflex.com/polygon/
bool insideRing(Coordinates c, Ring r)
{
	if( !insideRectangle(c, r.boundingBox) )	// otimização
		return false;
	double x = c.lon, y = c.lat;
	int i, j;
	bool oddNodes = false;
	for( i = 0, j = r.nVertexes - 1 ; i < r.nVertexes ; j = i++ ) {
		double xi = r.vertexes[i].lon, yi = r.vertexes[i].lat;
		double xj = r.vertexes[j].lon, yj = r.vertexes[j].lat;
		if( ((yi < y && y <= yj) || (yj < y && y <= yi))
								&& (xi <= x || xj <= x) ) {
			oddNodes ^= (xi + (y-yi)/(yj-yi) * (xj-xi)) < x;
		}
	}
	return oddNodes;
}

bool adjacentRings(Ring a, Ring b)
{
//TODO
	return false;
}


/* PARCEL -------------------------------------- */

static Parcel readParcel(FILE *f)
{
	Parcel p;
	p.identification = readIdentification(f);
	int i, n = readInt(f);
	p.edge = readRing(f);
	p.nHoles = n;
	p.holes = malloc(sizeof(Ring)*n);
	for( i = 0 ; i < n ; i++ ) {
		p.holes[i] = readRing(f);
	}
	return p;
}

static void showHeader(Identification id)
{
	showIdentification(-1, id, 3);
	printf("\n");
}

static void showParcel(int pos, Parcel p, int lenght)
{
	showIdentification(pos, p.identification, 3);
	showValue(lenght);
}

bool insideParcel(Coordinates c, Parcel p)
{
//TODO
	return false;
}

bool adjacentParcels(Parcel a, Parcel b)
{
	//TODO
		return false;
}


/* CARTOGRAPHY -------------------------------------- */

int loadCartography(String fileName, Cartography *cartography)
{
	FILE *f;
	int i;
	f = fopen(fileName, "r");
	if( f == NULL )
		error("Impossivel abrir ficheiro");
	int nParcels = readInt(f);
	*cartography = malloc(sizeof(Parcel)*nParcels);
	for( i = 0 ; i < nParcels ; i++ ) {
		(*cartography)[i] = readParcel(f);
	}

	fclose(f);
	return nParcels;
}

static int findLast(Cartography cartography, int n, int j, Identification id)
{
	for(  ; j < n ; j++ ) {
		if( !sameIdentification(cartography[j].identification, id, 3) )
			return j-1;
	}
	return n-1;
}

void showCartography(Cartography cartography, int nParcels)
{
	int last;
	Identification header = {"___FREGUESIA___", "___CONCELHO___", "___DISTRITO___"};
	showHeader(header);
	for( int i = 0 ; i < nParcels ; i = last + 1 ) {
		last = findLast(cartography, nParcels, i, cartography[i].identification);
		showParcel(i, cartography[i], last - i + 1);
	}
}


/* INTERPRETER -------------------------------------- */

static bool checkArgs(int arg)
{
	if( arg != -1 )
		return true;
	else {
		printf("ERRO: FALTAM ARGUMENTOS!\n");
		return false;
	}
}

static bool checkPos(int pos, int nParcels)
{
	if( 0 <= pos && pos < nParcels )
		return true;
	else {
		printf("ERRO: POSICAO INEXISTENTE!\n");
		return false;
	}
}

// L
static void commandListCartography(Cartography cartography, int nParcels)
{
	showCartography(cartography, nParcels);
}


// M pos
static int countVertices(Parcel p){
	int totalEdginess = p.edge.nVertexes;
	for(int i = 0;i<p.nHoles;i++){
		totalEdginess+=p.holes[i].nVertexes;
	}
	return totalEdginess;
}

static void commandMaximum(int pos, Cartography cartography, int nParcels)
{
	if( !checkArgs(pos) || !checkPos(pos, nParcels) )
		return;
	//TODO
	int edgeIndex = pos++;
	int maxEdginess = countVertices(cartography[pos]);
	Identification id = cartography[pos].identification;
	/* Put here for less dynamic memory management */
	int currentEdginess = 0;
	for(;pos<nParcels;pos++){
		if(sameIdentification(cartography[pos].identification, id, 3) ){
			currentEdginess = countVertices(cartography[pos]);
			if(currentEdginess>maxEdginess){
				maxEdginess = currentEdginess;
				edgeIndex = pos;
			}
		}else{
			break;
		}
	}
	showIdentification(edgeIndex, cartography[edgeIndex].identification, 3);
	showValue(maxEdginess);

}

//X
static void commandExtreme(Cartography cartography,int nParcels){
	nParcels--;
	int north=nParcels,
		east=nParcels,
		south=nParcels,
		west=nParcels;

	while(nParcels-->0){
		if(cartography[north].edge.boundingBox.topLeft.lat<
				cartography[nParcels].edge.boundingBox.topLeft.lat){
			north = nParcels;
		}
		if(cartography[east].edge.boundingBox.bottomRight.lon<
				cartography[nParcels].edge.boundingBox.topLeft.lon){
			east = nParcels;
		}
		if(cartography[south].edge.boundingBox.bottomRight.lat>
				cartography[nParcels].edge.boundingBox.bottomRight.lat){
			south = nParcels;
		}
		if(cartography[west].edge.boundingBox.topLeft.lon>
				cartography[nParcels].edge.boundingBox.bottomRight.lon){
			west = nParcels;
		}
	}
	showIdentification(north, cartography[north].identification, 3);
	printf("[N]\n");

	showIdentification(east, cartography[east].identification, 3);
	printf("[E]\n");

	showIdentification(south, cartography[south].identification, 3);
	printf("[S]\n");

	showIdentification(west, cartography[west].identification, 3);
	printf("[W]\n");
}

static void commandSummary(int pos, Cartography cartography, int nParcels)
{
	if (!checkArgs(pos) || !checkPos(pos, nParcels)) {
		return;
	}

	showIdentification(pos, cartography[pos].identification, 3);
	printf("\n");

	printf("     %d ", cartography[pos].edge.nVertexes);

	for (int i = 0; i < cartography[pos].nHoles; i++)
	{
		printf("%d ", cartography[pos].holes->nVertexes);
	}

	showRectangle(cartography[pos].edge.boundingBox);
	printf("\n");
}

static void commandTravel(double lat, double lon, int pos, Cartography cartography, int nParcels)
{
	if (!checkArgs(pos) || !checkPos(pos, nParcels)) {
		return;
	}

	Coordinates vertex = { lat, lon };
	double minDistance = haversine(vertex, cartography[pos].edge.vertexes[0]);
	for (int i = 1; i < cartography[pos].edge.nVertexes; i++)
	{
		double distance = haversine(vertex, cartography[pos].edge.vertexes[i]);
		if (distance < minDistance)
		{
			minDistance = distance;
		}
	}

	printf("%f\n", minDistance);
}

static void commandHowMany(int pos, Cartography cartography, int nParcels)
{
	if (!checkArgs(pos) || !checkPos(pos, nParcels)) {
		return;
	}
	#define parish cartography[pos].identification.freguesia
	#define county cartography[pos].identification.concelho
	#define district cartography[pos].identification.distrito

	const int DISTRICT = 0, COUNTY = 1, PARISH = 2;
	int howMany[3] = { 0, 0, 0 };
	for (int i = 0; i < nParcels; i++)
	{
		if (strcmp(district, cartography[i].identification.distrito) == 0) {
			howMany[DISTRICT]++;
			if (strcmp(county, cartography[i].identification.concelho) == 0) {
				howMany[COUNTY]++;
				if (strcmp(parish, cartography[i].identification.freguesia) == 0) {
					// TODO Can be optimized
					howMany[PARISH]++;
				}
			}
		}
	}
	#undef parish
	#undef county
	#undef district
	int i = 3;
	do {
		showIdentification(pos, cartography[pos].identification, i);
		showValue(howMany[--i]);
	} while (i > 0);
}

void interpreter(Cartography cartography, int nParcels)
{
	String commandLine;
	for(;;) {	// ciclo infinito
		printf("> ");
		readLine(commandLine, stdin);
		char command = ' ';
		double arg1 = -1.0, arg2 = -1.0, arg3 = -1.0;
		sscanf(commandLine, "%c %lf %lf %lf", &command, &arg1, &arg2, &arg3);
		// printf("%c %lf %lf %lf\n", command, arg1, arg2, arg3);
		switch( commandLine[0] ) {
			case 'L': case 'l':	// listar
				commandListCartography(cartography, nParcels);
				break;

			case 'M': case 'm':	// maximo
				commandMaximum((int)arg1, cartography, nParcels);
				break;

			case 'X': case 'x':	// eXtremo
				commandExtreme(cartography, nParcels);
				break;

			case 'R': case 'r':	// Resumo
				commandSummary((int)arg1, cartography, nParcels);
				break;

			case 'V': case 'v':	// Viagem
				commandTravel(arg1, arg2, (int)arg3, cartography, nParcels);
				break;

			case 'Q': case 'q':	// Quantos
				commandHowMany((int)arg1, cartography, nParcels);
				break;

			case 'Z': case 'z':	// terminar
				printf("Fim de execucao! Volte sempre.\n");
				return;

			default:
				printf("Comando desconhecido: \"%s\"\n", commandLine);
		}
	}
}
