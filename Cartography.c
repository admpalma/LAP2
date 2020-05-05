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
	return c.lat >= r.bottomRight.lat &&
		c.lat <= r.topLeft.lat &&
		c.lon >= r.topLeft.lon &&
		c.lon <= r.bottomRight.lon;
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
	for (int i = 0; i < a.nVertexes; i++)
	{
		for (int j = 0; j < b.nVertexes; j++)
		{
			if (sameCoordinates(a.vertexes[i], b.vertexes[j]))
			{
				return true;
			}
		}
	}
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
	if (!insideRing(c, p.edge))
	{
		return false;
	}
	else
	{
		for (int i = 0; i < p.nHoles; i++)
		{
			if (insideRing(c, p.holes[i]))
			{
				return false;
			}
		}
		return true;
	}
}

bool isAdjacentRingsList(Ring a, Ring* list, int num)
{

	while (num > 0) {
		if (adjacentRings(a, list[--num]))
		{
			return true;
		}
	}
	return false;
}

bool adjacentParcels(Parcel a, Parcel b)
{
	if (adjacentRings(a.edge, b.edge))
	{
		return true;
	}
	else {
		return isAdjacentRingsList(a.edge, b.holes, b.nHoles) || isAdjacentRingsList(b.edge, a.holes, a.nHoles);
	}
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

static int searchMaxEdgeWithDir(int pos, Cartography cartography, int nParcels, int dir,Identification id,int* maxEdginess){
	int edgeIndex = pos;
	*maxEdginess = countVertices(cartography[pos]);
	int currentEdginess = 0;

	for(;((pos<nParcels)&& dir == 1 || (pos>=nParcels)&& dir == -1) && (sameIdentification(cartography[pos].identification, id, 3)) ;pos+= dir){
		currentEdginess = countVertices(cartography[pos]);
		if(currentEdginess>*maxEdginess){
			*maxEdginess = currentEdginess;
			edgeIndex = pos;
		}
	}
	return edgeIndex;
}

static void commandMaximum(int pos, Cartography cartography, int nParcels)
{
	if( !checkArgs(pos) || !checkPos(pos, nParcels) )
		return;
	//TODO
	int maxEdginess;
	int lowerVert = searchMaxEdgeWithDir(pos,cartography,0,-1,cartography[pos].identification, &maxEdginess);
	int higherVer = searchMaxEdgeWithDir(pos,cartography,nParcels, 1,cartography[pos].identification, &maxEdginess);
	int edgeIndex = lowerVert < higherVer? higherVer:lowerVert;

	/* Put here for less dynamic memory management */
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

// Returns new length of sv
int removeDuplicatesSV(StringVector sv, int length)
{
	if (length == 0 || length == 1)
	{
		return length;
	}

	int newLength = 0;
	for (int i = 1; i < length; i++)
	{
		if (strcmp((char*)sv[i - 1], (char*)sv[i]))
		{
			strcpy((char*)sv[newLength++], (char*)sv[i - 1]);
		}
	}
	return newLength;
}

static void sortedDisplayNames(StringVector names, int nParcels)
{
	qsort(names, (unsigned int)nParcels, sizeof(String), strcmp);
	int length = removeDuplicatesSV(names, nParcels);
	showStringVector(names, length);
}

static void commandCounties(Cartography cartography, int nParcels)
{
	StringVector counties; //TODO memory hog
	for (int i = 0; i < nParcels; i++)
	{
		strcpy(counties[i], cartography[i].identification.concelho);
	}
	sortedDisplayNames(counties, nParcels);
}

static void commandDistricts(Cartography cartography, int nParcels)
{
	StringVector districts; //TODO memory hog
	for (int i = 0; i < nParcels; i++)
	{
		strcpy(districts[i], cartography[i].identification.distrito);
	}
	sortedDisplayNames(districts, nParcels);
}

static void commandParcels(double lat, double lon, Cartography cartography, int nParcels)
{
	Coordinates vertex = { lat, lon };
	for (int i = 0; i < nParcels; i++)
	{
		if (insideParcel(vertex, cartography[i]))
		{
			showIdentification(i, cartography[i].identification, 3);
			printf("\n");
			return;
		}
	}
	printf("FORA DO MAPA\n");
}

// Writes the indexes of Parcels adjacent to pos in adjancent, leaving -1 in it's last position
// Returns the number of adjacent Parcels found
static int adjacentTo(int pos, Cartography cartography, int nParcels, int* adjacent)
{
	int counter = 0;
	for (int i = 0; i < nParcels; i++)
	{
		if (i != pos)
		{
			if (adjacentParcels(cartography[pos], cartography[i]))
			{
				adjacent[counter++] = i;
			}
		}
	}
	adjacent[counter] = -1;
	return counter;
}

static void commandAdjacencies(int pos, Cartography cartography, int nParcels)
{
	if (!checkArgs(pos) || !checkPos(pos, nParcels)) {
		return;
	}

	int adjacent[nParcels];

	if (adjacentTo(pos, cartography, nParcels, adjacent))
	{
		for (int i = 0; adjacent[i] != -1; i++) {
			showIdentification(pos, cartography[adjacent[i]].identification, 3);
			printf("\n");
		}
	}
	else
	{
		printf("NAO HA ADJACENCIAS\n");
	}
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

			case 'C': case 'c':	// Concelhos
				commandCounties(cartography, nParcels);
				break;

			case 'D': case 'd':	// Distritos
				commandDistricts(cartography, nParcels);
				break;

			case 'P': case 'p':	// Parcelas
				commandParcels(arg1, arg2, cartography, nParcels);
				break;

			case 'A': case 'a':	// Adjacencias
				commandAdjacencies((int)arg1, cartography, nParcels);
				break;

			case 'Z': case 'z':	// terminar
				printf("Fim de execucao! Volte sempre.\n");
				return;

			default:
				printf("Comando desconhecido: \"%s\"\n", commandLine);
		}
	}
}
