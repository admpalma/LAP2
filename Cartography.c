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

static void showStringVector(String** sv, int n) {
	int i;
	for( i = 0 ; i < n ; i++ ) {
		printf("%s\n", *sv[i]);
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
	r.vertexes = malloc(sizeof(Coordinates)*(unsigned int)n);
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
		if( insideRectangle(a.vertexes[i], b.boundingBox) )
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
	p.holes = malloc(sizeof(Ring)*(unsigned int)n);
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
		return (adjacentRings(a.edge,b.edge))||
			(isAdjacentRingsList(a.edge, b.holes, b.nHoles) ||
			 isAdjacentRingsList(b.edge, a.holes, a.nHoles));
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
	*cartography = malloc(sizeof(Parcel)*(unsigned int)nParcels);
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

static void searchMaxEdgeWithDir(int pos, Cartography cartography, int nParcels, int dir,Identification id,int* maxEdginess, int* edgeIndex){
	int currentEdginess = 0;

	for(;(((pos<nParcels) && dir == 1) || ((pos>=nParcels) && dir == -1)) && sameIdentification(cartography[pos].identification, id, 3) ;pos+= dir){
		currentEdginess = countVertices(cartography[pos]);
		if(currentEdginess>*maxEdginess){
			*maxEdginess = currentEdginess;
			*edgeIndex = pos;
		}
	}
}

static void commandMaximum(int pos, Cartography cartography, int nParcels)
{
	if( !checkArgs(pos) || !checkPos(pos, nParcels) )
		return;
	//TODO
	int maxEdginess = countVertices(cartography[pos]);
	int edgeIndex = pos;

	searchMaxEdgeWithDir(pos-1,cartography,0,-1,
			cartography[pos].identification, &maxEdginess,&edgeIndex);

	searchMaxEdgeWithDir(pos+1,cartography,nParcels, 1,
			cartography[pos].identification, &maxEdginess,&edgeIndex);

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
int removeDuplicatesSV(String** sv, int length)
{
	if (length == 0 || length == 1)
	{
		return length;
	}

	int newLength = 0;
	sv[newLength++] = sv[0];
	for (int i = 1; i < length; i++)
	{
		if (strcmp((char*)*sv[i - 1], (char*)*sv[i]) != 0)
		{
			sv[newLength++] = sv[i];
		}
	}
	return newLength;
}

static int strcmpPoint(const void* vec1, const void* vec2) {
	return strcmp(*(char**)vec1, *(char**)vec2);
}

static void sortedDisplayNames(String** names, int nParcels)
{
	qsort(names, (unsigned int)nParcels, sizeof(String*), strcmpPoint);
	int length = removeDuplicatesSV(names, nParcels);
	showStringVector(names, length);
}

//C
static void commandCounties(Cartography cartography, int nParcels)
{
	String* counties[nParcels];
	for (int i = 0; i < nParcels; i++)
	{
		counties[i] = &cartography[i].identification.concelho;
	}
	sortedDisplayNames(counties, nParcels);
}

static void commandDistricts(Cartography cartography, int nParcels)
{
	String* districts[nParcels];
	for (int i = 0; i < nParcels; i++)
	{
		districts[i] = &cartography[i].identification.distrito;
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

	if (adjacentTo(pos, cartography, nParcels, adjacent) > 0)
	{
		for (int i = 0; adjacent[i] != -1; i++) {
			showIdentification(adjacent[i], cartography[adjacent[i]].identification, 3);
			printf("\n");
		}
	}
	else
	{
		printf("NAO HA ADJACENCIAS\n");
	}
}

bool inArray(int* array, int size, int elem)
{
	for (int i = 0; i < size; i++)
	{
		if (array[i] == elem)
		{
			return true;
		}
	}
	return false;
}

// Returns new length of sv
int removeDuplicatesIntArr(int* arr, int length)
{
	if (length == 0 || length == 1)
	{
		return length;
	}

	int newLength = 0;
	arr[newLength++] = arr[0];
	for (int i = 1; i < length; i++)
	{
		if (arr[i - 1] != arr[i])
		{
			arr[newLength++] = arr[i];
		}
	}
	return newLength;
}

static int compareInt(const void* a, const void* b) {
	const int* pa = (const int*)a;
	const int* pb = (const int*)b;
	return *pa - *pb;
}

static int sortedRemoveDuplicatesIntArr(int* arr, int length)
{
	qsort(arr, (unsigned int)length, sizeof(int), compareInt);
	return removeDuplicatesIntArr(arr, length);
}


static int removeFromIntArr(int* toRemove, int toRemoveLen, int* arr, int arrLen)
{
	int newLen = 0;
	for (int i = 0; i < arrLen; i++)
	{
		if (!inArray(toRemove, toRemoveLen, arr[i]))
		{
			arr[newLen++] = arr[i];
		}
	}

	return newLen;
}

static int crossingsBetween(int pos1, int pos2, Cartography cartography, int nParcels)
{
	if (pos1 == pos2) {
		return 0;
	}
	else
	{
		int crossings = 1;
		int visited[nParcels];
		visited[0] = pos1;
		int totalVisited = 1;
		int* visiting = malloc(sizeof(int) * (unsigned int)nParcels);
		int visitingInUse = adjacentTo(pos1, cartography, nParcels, visiting);
		int* adjacencies[nParcels];
		int adjacenciesSizes[nParcels];
		int allocatedAdjacencies = 0;
		while (visitingInUse > 0 && !inArray(visiting, visitingInUse, pos2))
		{
			// Update visited
			crossings++;
			memcpy(&visited[totalVisited], visiting, sizeof(int) * (unsigned int) visitingInUse);
			totalVisited += visitingInUse;

			// Grab new adjacencies
			int tempVisitingSize = 0;
			for (int i = 0; i < visitingInUse; i++)
			{
				// Allocate more memory if necessary
				if (i >= allocatedAdjacencies)
				{
					adjacencies[i] = malloc(sizeof(int) * (unsigned int)nParcels);
					allocatedAdjacencies++;
				}
				adjacenciesSizes[i] = adjacentTo(visiting[i], cartography, nParcels, adjacencies[i]);
				tempVisitingSize += adjacenciesSizes[i];
			}

			// Merge adjacencies for the next visiting level
			visiting = realloc(visiting, sizeof(int) * (unsigned int)tempVisitingSize);
			for (int i = 0, j = 0; i < tempVisitingSize; i += adjacenciesSizes[j++])
			{
				memcpy(&visiting[i], adjacencies[j], sizeof(int) * (unsigned int)adjacenciesSizes[j]);
			}

			// Clean next visiting level
			tempVisitingSize = sortedRemoveDuplicatesIntArr(visiting, tempVisitingSize);
			visitingInUse = removeFromIntArr(visited, totalVisited, visiting, tempVisitingSize);

			// Free up unnecessary memory
			if (visitingInUse != 0)
			{
				visiting = realloc(visiting, sizeof(int) * (unsigned int)visitingInUse);
			}
		}

		free(visiting);
		for (int i = 0; i < allocatedAdjacencies; i++)
		{
			free(adjacencies[i]);
		}

		if (visitingInUse > 0) {
			return crossings;
		} else {
			return -1;
		}
	}
}

static void commandBorders(int pos1, int pos2, Cartography cartography, int nParcels)
{
	if (!checkArgs(pos1) || !checkPos(pos1, nParcels)
		|| !checkArgs(pos2) || !checkPos(pos2, nParcels)) {
		return;
	}

	int crossings = crossingsBetween(pos1, pos2, cartography, nParcels);
	if (crossings < 0) {
		printf("NAO HA CAMINHO\n");
	}
	else {
		printf("%d\n", crossings);
	}
}

/* static void joinParts(int* part1,int* part2){ */
/* 	for(int i = 0;i<part2.numParts;i++){ */
/* 		part1.parts[part1.numParts++] = part2.parts[i]; */
/* 	} */
/* } */

static void commandPartitions(double maxDistance,Cartography cartography ,int nParcels){
	if (!checkArgs(maxDistance))
		return;
	int* allParts[nParcels];
	int numPerPart[nParcels];
	int numParts = 0;
	int remaining = nParcels;
	for(int i = 0;i<nParcels;i++){//Cartography parcel
		/* printf("Parcel %d\n",i); */
		bool gotIn = false;
		for(int j = 0;j<numParts;j++){//
			/* printf("Part %d\n",j); */
			for(int k = 0;k<numPerPart[j];k++){
				/* printf("Part inside Part %d\n",k); */
				double distance = haversine(cartography[i].edge.vertexes[0], cartography[allParts[j][k]].edge.vertexes[0]);
				if(distance<maxDistance){
					allParts[j][numPerPart[j]++] = i;
					gotIn = true;
					break;
				}
			}
		}
		if(!gotIn){
			numPerPart[numParts] = 1;
			allParts[numParts] = malloc(sizeof(int)*remaining);
			allParts[numParts++][0] = i;
		}
	}
	for(int i = 0;i<numParts;i++){
		for(int j = 0;j<numParts;j++){
			if(i == j) j++;
			for(int k = 0;k<numPerPart[i];k++){
				for(int m = 0;m<numPerPart[j];m++){
				if (allParts[i][k] == allParts[j][m]){
					int temp[numPerPart[i]+numPerPart[j]];
					memcpy(&allParts[i][numPerPart[i]], allParts[j], sizeof(int)*numPerPart[j]);
					numPerPart[i] = sortedRemoveDuplicatesIntArr(allParts[i], numPerPart[i]+numPerPart[j]);
					numPerPart[j] = 0;
				}
				}
			}
		}
	}
	for(int i = 0;i<numParts;i++){
		printf("Num of elements: %d\n",numPerPart[i]);
		int last= allParts[i][0];
		for(int j = 0;j<numPerPart[i];j++){
			if(!((last+1)==allParts[i][j])){
				printf("%d ",allParts[i][j]);
			}else{
				last = allParts[i][j];
			}
		}
		printf("\n");
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
		//printf("%c %lf %lf %lf\n", command, arg1, arg2, arg3);
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

			case 'F': case 'f':	// Fronteiras
				commandBorders((int)arg1, (int)arg2, cartography, nParcels);
				break;
			case 'T': case 't':
				commandPartitions((double)arg1,cartography,nParcels);
				break;
			case 'Z': case 'z':	// terminar
				printf("Fim de execucao! Volte sempre.\n");
				return;

			default:
				printf("Comando desconhecido: \"%s\"\n", commandLine);
		}
	}
}
