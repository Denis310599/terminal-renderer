#include <stdlib.h>
#include <float.h>
#include <stdarg.h>
#include <math.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include "../include/renderer.h"

	int SCREEN_WIDTH = 200;
	int SCREEN_HEIGHT = 50;
	int GPU_MODE = 0;
	int FAST_LIGHT = 0;
	double FRONT_CLIP = 0.1;
	double UMBRAL_LUZ = 0.1;
	int COLOR_MODE = 0;
	Camera ACTIVE_CAMERA = (Camera){};
	double FPS = 0;
	double DELTA_TIME = 0;
//Funcion que dibuja el borde en el mapa que se le pasa como parametro
int dibujaBorde(int x, int y, int indice, char* mapa);

ObjectListNode * preProcessObjectPixelBuffer( ObjectListNode ** buffer );
void preprocessPolygonFromCube(ObjectListNode ** List, struct Cubo cubo);
void preProcessPolygon(ObjectListNode ** buffer, struct Poligono polygon);

//Funcion que dibuja el frame rendereizado en el array que se le especifica
void procesaPixel(int x, int y, int indice, Pixel* mapa, ObjectListNode ** buffer, ObjectListNode * commonObjects);

//Funcion que devuelve el valor de la luminosidad del pixel dentre 0 a 255.
int calculaLuminosidadPixel(int x, int y, struct Camara cam, ObjectListNode ** buffer, ObjectListNode * commonObjects);

int calculaEfectoLuz(struct Vector3d puntoColision, struct Luz light, struct Vector3d normalSuperficie);

//Funcion que realiza operaciones matematicas con vectores
//Defined in header file
/*double vect_mul(struct Vector3d v1, struct Vector3d v2);
struct Vector3d vect_sum(struct Vector3d v1, struct Vector3d v2, double factor);
struct Vector3d vect_translation(struct Vector3d point, struct Vector3d translation, int inverse);
struct Quaternion quat_mul(struct Quaternion q1, struct Quaternion q2);
struct Quaternion quat_inv(struct Quaternion);
struct Vector3d vect_rot(struct Vector3d v, struct Quaternion rot);
struct Quaternion newQuat(struct Vector3d v, double angle);
struct Vector3d quat2vec(struct Quaternion q);
double anguloEntreVectores(struct Vector3d v1, struct Vector3d v2);
double moduloVector(struct Vector3d v);
double deg2rad(double deg);
double rad2deg(double rad);
struct Vector3d normalizeVector(struct Vector3d v);
struct Vector3d vectProd(struct Vector3d v1, struct Vector3d v2);
struct Vector3d vectProjectionIntoPlane(struct Vector3d point, struct Vector3d ogPlane, struct Vector3d normPlane);
*/

int DEBUG =0;
void debug(char *  msg, ...){

	va_list argPtr;
	va_start(argPtr, msg);
	if(DEBUG){
		printf("[DEBUG]");
		vprintf(msg, argPtr);
		printf("\n");
	}
	va_end(argPtr);
}

//Funciones correspondiente con el objeto a renderizar
double getDistanciaColision(struct Vector3d origen, struct Vector3d rayo, struct Object objComprobar,struct Object* objColisionado);
double getDistanciaColisionEsfera(struct Vector3d origen ,struct Vector3d rayo, struct Esfera esfera);
double getDistanciaColisionCubo(struct Vector3d origen ,struct Vector3d rayo, struct Cubo cubo, struct Object * Poligono);
struct Vector3d getPuntoColision(struct Vector3d origen ,struct Vector3d rayo, struct Object* objColisionado, ObjectListNode * listaObjects, ObjectListNode * commonObjectList, int calculaTodos);
double getDistanciaColisionPlano(struct Vector3d origen ,struct Vector3d rayo, struct Plano plano);
double getDistanciaColisionPoligono(struct Vector3d origen, struct Vector3d rayo, struct Vector3d p1, struct Vector3d p2, struct Vector3d p3);
struct Vector3d getNormalPunto(struct Vector3d punto, struct Object object);

//Functions related with linked lists
//Creates a new ObjectListNode
ObjectListNode * newObjectList();
void objectListPush(ObjectListNode * head, struct Object dato);
void objectListPushFirst(ObjectListNode ** head, struct Object data);
void lightListPushFirst(LightListNode ** head, Light data);
void freeList(ObjectListNode * head);
void freeLightList(LightListNode * head);

//We configure the renderer via its global variables.
//DELTA_TIME
//
//FPS
//SCREEN_WIDTH
//SCREEN_HEIGHT
//GPU/CPU
//FAST_ILLUM
//LIGHT_THRESHOLD
//FRONT_CLIP
//ACTIVE_CAMERA
//

//********************************************************
//************* Variable Deffinitions ++++++++++++++++++++
//********************************************************
ObjectListNode * listaObjetos = NULL;
LightListNode * listaLuces = NULL;

//********************************************************
//************* Public accessible functions
//********************************************************
//Function that renders the frame
void renderFrame(Pixel * frameBuffer){
	//Sets up the camera
	debug("Camera direction: x %f, y %f, z %f", ACTIVE_CAMERA.dir.x, ACTIVE_CAMERA.dir.y, ACTIVE_CAMERA.dir.z);
	ACTIVE_CAMERA.dir = normalizeVector(ACTIVE_CAMERA.dir);
	debug("Camera direction: x %f, y %f, z %f", ACTIVE_CAMERA.dir.x, ACTIVE_CAMERA.dir.y, ACTIVE_CAMERA.dir.z);
	debug("Rendering Frame. GPU: %d", GPU_MODE);
	//getchar();
	if(GPU_MODE == 0){
		debug("Rendering with CPU");
		//Render in CPU mode
		//PreProcess the polygons (Pseudo Fragment Shader)
		ObjectListNode * bufferObjectPixel [SCREEN_WIDTH * SCREEN_HEIGHT];
		ObjectListNode * commonObjectsBuffer = preProcessObjectPixelBuffer(bufferObjectPixel);
		//getchar();
		//Process every single Pixel
		debug("Processing each individual Pixel");
		for (int y = 0; y<SCREEN_HEIGHT; y++){
			for (int x = 0; x<SCREEN_WIDTH; x++){  
				debug("============\nPixel %d, %d\n", x, y);
				int i = (SCREEN_WIDTH)*y + x;
				procesaPixel(x, y, i, frameBuffer, bufferObjectPixel, commonObjectsBuffer);
				if(x == 20 && y == 20){
					//getchar();
				}
			}
		}
	}
}


//Function that sets up the variables
void initRenderer(){
	debug("Initialazing renderer...");
	SCREEN_WIDTH = 200;
	SCREEN_HEIGHT = 50;
	GPU_MODE = 0;
	FAST_LIGHT = 0;
	FRONT_CLIP = 0.1;
	UMBRAL_LUZ = 0.1;
	COLOR_MODE = 0;
}

//Function that manages object adition in the scene
int addObject(Object object){
	debug("Adding Objects...");
	//we get the last Id on the queue, and becouse its stored incrementally,
	//if the Id to insert is smaller, it for sure will be inserted, otherwise not.
	int ultimoId = listaObjetos != NULL ? listaObjetos->object.id : 0;
	

	objectListPushFirst(&listaObjetos, object);
	return ++ultimoId;
}

//Function that get an specific Object from the scene
int getObject(int id, Object * object){
	ObjectListNode * current = listaObjetos;
	int encontrado = 0;
	//Searchs for the object in the list with matching id
	while(current != NULL && !encontrado){
		if(current->object.id == id){
			encontrado = 1;
		}else{
			current = current->next;
		}
	}
	//If finded return 0, if not returns -1
	if(encontrado){
		*object = current->object;
		return  0;
	}else{
		return -1;
	}
}

ObjectListNode * getAllObjects(){
	return listaObjetos;
}

int updateObject(int id, Object object){
	ObjectListNode * current = listaObjetos;
	int encontrado = 0;
	//Searchs for the object in the list with matching id
	while(current != NULL && !encontrado){
		if(current->object.id == id){
			encontrado = 1;
		}else{
			current = current->next;
		}
	}
	//If node finded, update it
	if(encontrado){
		current->object = object;
		return 0;
	}else{
		return -1;
	}
}

int deleteObject(int id){
	ObjectListNode * current = listaObjetos;
	ObjectListNode * previous = NULL;

	int encontrado = 0;
	//Searchs for the object in the list with matching id
	while(current != NULL && !encontrado){
		if(current->object.id == id){
			encontrado = 1;
		}else{
			previous = current;
			current = current->next;
		}
	}
	//If node finded, update it
	if(encontrado){
		if(previous != NULL){
			previous->next = current->next;
		}else{
			listaObjetos = current->next;
		}
		return 0;
	}else{
		return -1;
	}
}

//Manage Lights from the scene
//Function that manages object adition in the scene
int addLight(Light light){
	debug("Adding Light");
	int ultimoId = listaLuces != NULL ? listaLuces->light.id : 0;
	lightListPushFirst(&listaLuces, light);
	return ++ultimoId;
}

//Function that get an specific Object from the scene
int getLight(int id, Light* light){
	LightListNode * current = listaLuces;
	int encontrado = 0;
	//Searchs for the light in the list with matching id
	while(current != NULL && !encontrado){
		if(current->light.id == id){
			encontrado = 1;
		}else{
			current = current->next;
		}
	}
	//If finded return 0, if not returns -1
	if(encontrado){
		*light= current->light;
		return  0;
	}else{
		return -1;
	}
}

LightListNode * getAllLights(){
	return listaLuces;
}

int updateLight(int id, Light light){
	LightListNode * current = listaLuces;
	int encontrado = 0;
	//Searchs for the light in the list with matching id
	while(current != NULL && !encontrado){
		if(current->light.id == id){
			encontrado = 1;
		}else{
			current = current->next;
		}
	}
	//If node finded, update it
	if(encontrado){
		current->light=light;
		return 0;
	}else{
		return -1;
	}
}

int deleteLight(int id){
	LightListNode * current = listaLuces;
	LightListNode * previous = NULL;

	int encontrado = 0;
	//Searchs for the light in the list with matching id
	while(current != NULL && !encontrado){
		if(current->light.id == id){
			encontrado = 1;
		}else{
			previous = current;
			current = current->next;
		}
	}
	//If node finded, update it
	if(encontrado){
		if(previous != NULL){
			previous->next = current->next;
		}else{
			listaLuces = current->next;
		}
		return 0;
	}else{
		return -1;
	}
}

//********************************************************
//************* Private Functions ++++++++++++++++++++
//********************************************************

void clearScreen()
{
  //printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
}



//Funcion que calcula el buffer de objeto-pixel, utilizado en el procesado de cada pixel
ObjectListNode * preProcessObjectPixelBuffer( ObjectListNode ** buffer){
	debug("Pre Processing Objects...");
	ObjectListNode * auxList = NULL;	//Objects computed in all pixels
	//First we free the memory allocated in last frame
	for(int i = 0; i<(SCREEN_WIDTH * SCREEN_HEIGHT); i++){
		if(buffer[i] != NULL){
			//freeList(buffer[i]);
		}
		buffer[i] = NULL;
	}
	//Camera plane: cam.pos, cam.dir
	//Loop over each object in the scene and compute the pseudo fragment shader
	
	ObjectListNode * current = listaObjetos;
	while(current != NULL){
		switch(current->object.tipo){
			case Plano:
				debug("preprocessPixelBuffer(): Preprocesando Plano");
				objectListPushFirst(&auxList, current->object);
				break;
			case Cubo:
				debug("preprocessPixelBuffer(): Preprocesando Cubo");
				preprocessPolygonFromCube(buffer, current->object.cubo);
				break;
			case Poligono:
				debug("preprocessPixelBuffer(): Preprocesando poligono");
				preProcessPolygon(buffer, current->object.poligono);
				break;
			case Malla:
				debug("preProcessPixelBuffer(): Preprocesando Malla");
				for(int i = 0; i<current->object.p_malla->n_polygon; i++){
					preProcessPolygon(buffer, current->object.p_malla->polygons[i]);
				}
				break;

		}
		current = current->next;
	}
	debug("preProcessPixelBuffer(): AuxList: %d", auxList);
	return auxList;
}

//Funcion que procesa cada uno de los pixeles
void procesaPixel(int x, int y, int indice, Pixel * mapa, ObjectListNode ** buffer, ObjectListNode * commonObjects){
	//Calculate pixel brigtness checking only the necessary objects
	int intensidad = calculaLuminosidadPixel(x, y,ACTIVE_CAMERA, buffer, commonObjects);	
	//Build the returned Pixel
	
	Pixel auxPixel;
	if(COLOR_MODE){
		auxPixel = (Pixel){intensidad, 0, 0};
	}else{
		auxPixel = (Pixel){intensidad, intensidad, intensidad};
		}
	debug("Pixel Calculado: %f, %f, %f", auxPixel.r, auxPixel.g, auxPixel.b );
	mapa[indice] = auxPixel;
	return;

	}

//Funcion que dibuja el borde
int dibujaBorde(int x, int y, int indice, char* mapa){
	char * output = mapa;
	//Bordes del frame
	if (x == 0 || y == 0 ){
		output[indice] = '#';
		//printf("#");
		if(x == SCREEN_WIDTH-1){
			output[indice+1] = '\n';
			//printf("\n");
		}
	}else if(x == SCREEN_WIDTH-1){
		//printf("#\n");
		output[indice] = '#';
		output[indice+1] = '\n';
	}else if(y == SCREEN_HEIGHT-1){
		//printf("#");
		output[indice] = '#';
	}else{
		//printf(" ");
		//output[indice] = ' ';
		return -1;
	}
	
	mapa = output;
	return 0;
}




//Funcion que calcula la luminosdidad de un pixel
int calculaLuminosidadPixel(int x, int y, struct Camara cam, ObjectListNode ** buffer, ObjectListNode * commonObjects){
	debug("Calculando intensidad");
	//Obten el rayo en coordenadas locales
	//double anchoReal = 2.0*sin(deg2rad((double)cam.fov)/2.0);
	double anchoReal = 2.0*tan(deg2rad(cam.fov/2.0))*FRONT_CLIP;
	double altoReal = (anchoReal*2*SCREEN_HEIGHT)/SCREEN_WIDTH;//El *2 es por el aspect ratio de los pixeles (1:2)
	
	debug("Ancho Real: %f, Alto Real: %f, Fov: %f", anchoReal, altoReal, deg2rad(cam.fov));
	//La camara apunta hacia y, y el plano de pixeles es perpendicular al plano y,x.
	struct Vector3d pixelLocal;
	pixelLocal.x = ((-1.0*anchoReal)/2.0) + (double)x*anchoReal/(double)SCREEN_WIDTH;
	pixelLocal.y = FRONT_CLIP;
	pixelLocal.z = ((altoReal)/2.0) - (double)y*altoReal/(double)SCREEN_HEIGHT;
	
	debug("Pixel local: %f, %f, %f", pixelLocal.x, pixelLocal.y, pixelLocal.z);
	//Paso el rayo a coordenadas globales
	//Para ello roto en el plano xy y despues haci aabajo
	
	//Proyecto el vector de camara en el plano xy
	struct Vector3d camVectProy = vectProjectionIntoPlane(cam.dir, (struct Vector3d){0, 0, 0}, (struct Vector3d){0, 0, 1});
	debug("Projected Cam Vect: x %f, y %f, z %f", camVectProy.x, camVectProy.y, camVectProy.z);
	//Rotacion 1
	double anguloGiroXY = anguloEntreVectores((struct Vector3d){0, 1, 0}, camVectProy);
	anguloGiroXY = cam.dir.x >= 0 ? -anguloGiroXY : anguloGiroXY;
	struct Vector3d pixelGlobal = vect_rot(pixelLocal, newQuat((struct Vector3d){0, 0, 1}, anguloGiroXY));

	debug("Pixel global: %f, %f, %f", pixelGlobal.x, pixelGlobal.y, pixelGlobal.z);	
	//Rotacion 2
	if(camVectProy.x == cam.dir.x && camVectProy.y == cam.dir.y && camVectProy.z == cam.dir.z){
		pixelGlobal = pixelGlobal;
	}else{
		pixelGlobal = vect_rot(pixelGlobal, newQuat(normalizeVector(vectProd(camVectProy, cam.dir)), anguloEntreVectores(camVectProy, cam.dir)));
	}
	debug("Pixel global: %f, %f, %f", pixelGlobal.x, pixelGlobal.y, pixelGlobal.z);	


	//printf("Angulo Rotar Cam: %f, (%f, %f, %f)", rad2deg(anguloGiro2), vectProy.x, vectProy.y, vectProy.z);
	//Calcula el punto de colision
	struct Object objetoColisionado;
	struct Object * ptrObjColisionado = &objetoColisionado;
		struct Vector3d puntoColision = getPuntoColision(cam.pos, pixelGlobal, &objetoColisionado, buffer[x+SCREEN_WIDTH*y], commonObjects, 0);
	debug("Punto colision: %f, %f, %f", puntoColision.x, puntoColision.y, puntoColision.z); 	
	//Si no hay punto de colision no hace falta calcular el resto
	
	if(isnan(puntoColision.x)){
		return -1;
	}
	//Calcula la normal al plano que pasa por ese punto
	struct Vector3d normalSuperficie = getNormalPunto(puntoColision, objetoColisionado);
	debug("NormalSuperficice: %f %f %f", normalSuperficie.x, normalSuperficie.y, normalSuperficie.z);
	//Suma el efecto de todas las luces de la escena en ese nivel de brillo
	int luminosidadTotal = 0;
	if (FAST_LIGHT){
		//Directamente se pone como el angulo entre las normales
		//double anguloIncidenciaLuz = anguloEntreVectores((struct Vector3d) {-cam.dir.x, -cam.dir.y, -cam.dir.z}, normalSuperficie);
		//luminosidadTotal = 255*(1-sin(anguloIncidenciaLuz));
		luminosidadTotal = 255*vect_mul(normalSuperficie, (struct Vector3d) {-cam.dir.x, -cam.dir.y, -cam.dir.z});//(1-anguloIncidenciaLuz/deg2rad(90));
		debug("Luminosidad Total: %d", luminosidadTotal /*rad2deg(anguloIncidenciaLuz)*/);
	}else{
		LightListNode * currentLight = listaLuces;
		while(currentLight != NULL){	
			luminosidadTotal += calculaEfectoLuz(puntoColision, currentLight->light,normalSuperficie);
			debug("Luminosidad Total: %d", luminosidadTotal);
			currentLight = currentLight->next;
		}
	}

	if(luminosidadTotal > 255){
		luminosidadTotal = 255;
	}else if(luminosidadTotal < 10){
		luminosidadTotal = 10;
	}

	return (luminosidadTotal);
}


int calculaEfectoLuz(struct Vector3d puntoColision, struct Luz luz, struct Vector3d normalSuperficie){
	debug("**\nProcesando nueva Luz");
	//Calcula el vector de procedencia de la luz
	//Despues calcula si ha colisionado con algo
	struct Vector3d vectLuz;
	struct Object objetoColisionado;
	struct Vector3d puntoColisionLuz;
	double dLuz = 0;
	switch(luz.tipo){
		case point:
			vectLuz = vect_sum(puntoColision, luz.pos, -1);

			dLuz = moduloVector(vectLuz);
			if((luz.intensidad/(dLuz*dLuz))<UMBRAL_LUZ){
				//return 0;
			}
			if(luz.shadow == 1){
				puntoColisionLuz = getPuntoColision(luz.pos, vectLuz, &objetoColisionado, NULL, NULL, 1);
			}
			break;
		case inf:
			vectLuz = normalizeVector(luz.pos);
			//Si es luz infinita el rayo se tira al reves, porque no tiene un origen como tal.
			if(luz.shadow == 1){
				puntoColisionLuz = getPuntoColision(vect_sum(puntoColision, vectLuz, -10000), vectLuz, &objetoColisionado, NULL, NULL, 1);
			}
			break;

	}
	
	if(luz.shadow == 1){
		debug("Punto colision luz: %f, %f, %f", puntoColisionLuz.x, puntoColisionLuz.y, puntoColisionLuz.z);
		double distanciaEntrePuntos = moduloVector(vect_sum(puntoColisionLuz, puntoColision, -1.0));
		debug("Distancia entre punto de luz y de colision pixel: %f", distanciaEntrePuntos);
		if(distanciaEntrePuntos > 0.01){
			//El rayo de luz ha chocado en otro lado
			//Esta zona está oscura
			return 0;
		}
	}

	//Obtengo el angulo entre los vectores
	vectLuz.x = -vectLuz.x;
	vectLuz.y = -vectLuz.y;
	vectLuz.z = -vectLuz.z;
	//double anguloIncidenciaLuz = anguloEntreVectores(vectLuz, normalSuperficie);
	debug("vectLuz: %f, %f, %f", vectLuz.x, vectLuz.y, vectLuz.z);
	//debug("Angulo incidencia luz: %f", rad2deg(anguloIncidenciaLuz));
	//if(-90 > rad2deg(anguloIncidenciaLuz) && rad2deg(anguloIncidenciaLuz)>90){
	//	return 0;
	//}

	double intensidadLuz = 0;
	double d;
	switch(luz.tipo){
		case inf:
			intensidadLuz = luz.intensidad;
			break;
		case point:
			d = moduloVector(vectLuz);
			//intensidadLuz = luz.intensidad/(d*d);
			double atenuation = 1.0/(1+0.09*d+0.032*d*d);
			intensidadLuz = luz.intensidad*atenuation;
			break;
	}

	vectLuz = normalizeVector(vectLuz);

	//Diffuse Light
	double diffuse = 255*intensidadLuz*vect_mul(normalSuperficie, vectLuz);//intensidadLuz*(1-sin(anguloIncidenciaLuz));
	if(diffuse<0){ diffuse = 0;}
	
	//Base Light
	double base = 0;//1*255.0/longitudTablaPixeles;
	//Gets calculated after all this

	//Specular Light
	double specular;
	if(luz.tipo == inf){
		specular = 0;
	}else{
		struct Vector3d v = {-vectLuz.x, -vectLuz.y, -vectLuz.z};
		struct Vector3d reflectedVector = vect_sum(v, normalSuperficie, -2*vect_mul(v, normalSuperficie));
		specular = vect_mul((struct Vector3d){-ACTIVE_CAMERA.dir.x, -ACTIVE_CAMERA.dir.y, -ACTIVE_CAMERA.dir.z}, reflectedVector);
		if(specular<0){ specular = 0;}
		specular = 0.2*255*intensidadLuz*pow(specular, 32);
	}

	double efectoLuz = diffuse + base + specular;
	if(efectoLuz<0){ efectoLuz = 0;}
	debug("Efecto Luz = %d", (int) round(efectoLuz));
	//El valor de la luminosidad es el angulo de incidencia de la luz con factor senoidal.
	return (int) round(efectoLuz);
}



struct Vector3d getNormalPunto(struct Vector3d punto, struct Object objeto){
	//Actuo diferente segun el tipo de objeto que sea
	struct Vector3d ret = {0.0, 0.0, 0.0};
	//debug("Puntero a objeto: %f", objeto.poligono.normal.y);
	switch(objeto.tipo){
		//case Esfera:
		//	debug("Normal de esfera");
		//	//Calculo la normal si se trata de una esfera
		//	ret = normalizeVector(vect_sum( punto, objeto.esfera.pos, -1.0));
		//	break;
		case Plano:
			debug("Normal de plano");
			ret = objeto.plano.normal ;
			break;
		case Poligono:
			debug("Normal de poligono");
			ret = objeto.poligono.normal;
			break;
	}
	debug("Vector Normal Calculado: %f, %f, %f", ret.x, ret.y, ret.z);
	return ret;
}


//***************************************************************************
//********** Calculo de punto de colisión
//***************************************************************************

//Funcion que recorre todos los onjetos y obtiene el punto de colision mas cercano
struct Vector3d getPuntoColision(struct Vector3d origen ,struct Vector3d rayo, struct Object* objColisionado, ObjectListNode * listaObjects, ObjectListNode * commonObjectList, int calculaTodos){
	debug("Obteniendo punto de colision del rayo. %d", listaObjects);
	debug("Origin: x %f, y %f, z %f.\n Ray: x %f, y %f, z %f", origen.x, origen.y, origen.z, rayo.x, rayo.y, rayo.z);
	//Recorro todos los objetos calculando el punto de colision
	double minD = -1.0;

	//When i want to check all the objects of the scene (ie light processing).
	if(calculaTodos){
		debug("  Comprobando todos los objetos");
		ObjectListNode * current = listaObjetos;
		int i = 0;
		while(current != NULL){
			i++;
			double computedDistance = getDistanciaColision(origen, rayo, current->object,objColisionado);
			//Updates the min distance if its the nearest.
			minD = minD < 0 ? computedDistance: ((computedDistance >= 0 && computedDistance < minD) ? computedDistance : minD);
			current = current->next;
			debug("  ·%d. Dist: %f", i, computedDistance);
		}

	}else{
		//When i want only a few objects to process (ie vertex shader)
		debug("  Comprobando algunos objetos (buffer)");
		ObjectListNode * currentNode = listaObjects;
		struct Object auxObject;
		int i = 0;
		while(currentNode != NULL){
			i++;
			double computedDistance = getDistanciaColision(origen, rayo, currentNode->object, &auxObject);
			//update if new closer valid distance
			if((0 <= computedDistance && computedDistance < minD) || (computedDistance >= 0 && minD <= 0)){
				*objColisionado = auxObject;
				minD = computedDistance;
			}
			currentNode = currentNode->next;
			debug("  ·%d. Dist: %f", i, computedDistance);
		}

		debug("  Objetos comunes");
		i = 0;
		currentNode = commonObjectList;
		while(currentNode != NULL){
			i++;
			double computedDistance = getDistanciaColision(origen, rayo, currentNode->object, &auxObject);
			//update if new closer valid distance
			if((0 <= computedDistance && computedDistance < minD) || (computedDistance >= 0 && minD <= 0)){
				*objColisionado = auxObject;
				minD = computedDistance;
			}
			currentNode = currentNode->next;
			debug("  ·%d. Dist: %f", i, computedDistance);
		}
	}

	debug("Min Dist: %f", minD);
	//Return the computed distance
	if(minD < 0){
		struct Vector3d aux = {NAN, 0.0, 0.0};
		//*objColisionado = NULL;
		return aux;
	}
	return vect_sum(origen, normalizeVector(rayo), minD);
}

//Function that computes the collission distance from an object
double getDistanciaColision(struct Vector3d origen, struct Vector3d rayo, struct Object objComprobar,struct Object* objColisionado){
	double dAux = -1;
	struct Object objetoAux;
	switch(objComprobar.tipo){
			//case Esfera: 
				//dAux = getDistanciaColisionEsfera(origen, rayo, objComprobar.esfera);
				//objetoAux = objComprobar;
				//break;
			case Plano:
				dAux = getDistanciaColisionPlano(origen, rayo, objComprobar.plano);
				objetoAux = objComprobar;
				break;
			case Poligono:
				dAux = getDistanciaColisionPoligono(origen, rayo, objComprobar.poligono.p1, objComprobar.poligono.p2, objComprobar.poligono.p3);
				debug("Normal: %f, %f, %f", objComprobar.poligono.normal.x, objComprobar.poligono.normal.y, objComprobar.poligono.normal.z);
				objetoAux = objComprobar;
				break;
				
			case Cubo:
				dAux = getDistanciaColisionCubo(origen, rayo, objComprobar.cubo, &objetoAux); 
				break;
	}
		*objColisionado = objetoAux;
	debug("Distancia al objeto: %f", dAux);
	return dAux;
}


double getDistanciaColisionPlano(struct Vector3d origen ,struct Vector3d rayo, struct Plano plano){
	struct Vector3d l = normalizeVector(rayo);
	struct Vector3d n = normalizeVector(plano.normal);
	struct Vector3d p0 = plano.pos;
	struct Vector3d l0 = origen;
	double d = vect_mul(vect_sum(p0, l0, -1.0), n)/vect_mul(l, n);
	debug("Distancia hasta el plano: %f", d);
	if (d>=0){
		return d;
	}else{
		return -1.0;
	}
}

void preprocessPolygonFromCube(ObjectListNode ** buffer, struct Cubo cubo){
	debug("Preprocesando cubo");
	//Obtengo los poligonos que forman el cubo en coordenadas locales
	//Cara superior
	struct Poligono up1 = {{cubo.escala.x/2, cubo.escala.y/2, cubo.escala.z/2},{cubo.escala.x/2, -cubo.escala.y/2, cubo.escala.z/2},{-cubo.escala.x/2, cubo.escala.y/2, cubo.escala.z/2}, (struct Vector3d){0, 0, 1}};
	struct Poligono up2 = {{-cubo.escala.x/2, -cubo.escala.y/2, cubo.escala.z/2},{cubo.escala.x/2, -cubo.escala.y/2, cubo.escala.z/2},{-cubo.escala.x/2, cubo.escala.y/2, cubo.escala.z/2}, (struct Vector3d){0, 0, 1}};
	struct Poligono down1 = {{cubo.escala.x/2, cubo.escala.y/2, -cubo.escala.z/2},{cubo.escala.x/2, -cubo.escala.y/2, -cubo.escala.z/2},{-cubo.escala.x/2, cubo.escala.y/2, -cubo.escala.z/2}, (struct Vector3d){0, 0, -1}};
	struct Poligono down2 = {{-cubo.escala.x/2, -cubo.escala.y/2, -cubo.escala.z/2},{cubo.escala.x/2, -cubo.escala.y/2, -cubo.escala.z/2},{-cubo.escala.x/2, cubo.escala.y/2, -cubo.escala.z/2}, (struct Vector3d){0, 0, -1}};
	struct Poligono right1 = {{cubo.escala.x/2, cubo.escala.y/2, cubo.escala.z/2},{cubo.escala.x/2, -cubo.escala.y/2, cubo.escala.z/2},{cubo.escala.x/2, cubo.escala.y/2, -cubo.escala.z/2}, (struct Vector3d){1, 0, 0}};
	struct Poligono right2 = {{cubo.escala.x/2, cubo.escala.y/2, -cubo.escala.z/2},{cubo.escala.x/2, -cubo.escala.y/2, -cubo.escala.z/2},{cubo.escala.x/2, -cubo.escala.y/2, cubo.escala.z/2}, (struct Vector3d){1, 0, 0}};
	struct Poligono left1 = {{-cubo.escala.x/2, cubo.escala.y/2, cubo.escala.z/2},{-cubo.escala.x/2, -cubo.escala.y/2, cubo.escala.z/2},{-cubo.escala.x/2, cubo.escala.y/2, -cubo.escala.z/2}, (struct Vector3d){-1, 0, 0}};
	struct Poligono left2 = {{-cubo.escala.x/2, cubo.escala.y/2, -cubo.escala.z/2},{-cubo.escala.x/2, -cubo.escala.y/2, -cubo.escala.z/2},{-cubo.escala.x/2, -cubo.escala.y/2, cubo.escala.z/2}, (struct Vector3d){-1, 0, 0}};
	struct Poligono front1 = {{cubo.escala.x/2, cubo.escala.y/2, cubo.escala.z/2},{cubo.escala.x/2, cubo.escala.y/2, -cubo.escala.z/2},{-cubo.escala.x/2, cubo.escala.y/2, cubo.escala.z/2}, (struct Vector3d){0, 1, 0}};
	struct Poligono front2 = {{cubo.escala.x/2, cubo.escala.y/2, -cubo.escala.z/2},{-cubo.escala.x/2, cubo.escala.y/2, cubo.escala.z/2},{-cubo.escala.x/2, cubo.escala.y/2, -cubo.escala.z/2}, (struct Vector3d){0, 1, 0}};
	struct Poligono back1 = {{cubo.escala.x/2, -cubo.escala.y/2, cubo.escala.z/2},{cubo.escala.x/2, -cubo.escala.y/2, -cubo.escala.z/2},{-cubo.escala.x/2, -cubo.escala.y/2, cubo.escala.z/2}, (struct Vector3d){0, -1, 0}};
	struct Poligono back2 = {{cubo.escala.x/2, -cubo.escala.y/2, -cubo.escala.z/2},{-cubo.escala.x/2, -cubo.escala.y/2, cubo.escala.z/2},{-cubo.escala.x/2, -cubo.escala.y/2, -cubo.escala.z/2}, (struct Vector3d){0, -1, 0}};
	struct Poligono arrPoligonos[] = {up1, up2, down1, down2, left1, left2, right1, right2, front1, front2, back1, back2};
	//Paso el poligono a globales y calculo la distancia
	for(int i = 0; i<12; i++){
		//Roto los poligonos
		struct Quaternion rot1;
		struct Quaternion rot2;
		struct Quaternion rot3;
		rot1 = newQuat((struct Vector3d){0, 0, 1}, cubo.rotacion.z);
		rot2 = newQuat(vect_rot((struct Vector3d){1, 0, 0}, rot1), cubo.rotacion.x);
		rot3 = newQuat(vect_rot((struct Vector3d){0, 1, 0}, rot2), cubo.rotacion.y);

		arrPoligonos[i].p1 = vect_rot(arrPoligonos[i].p1, rot1); 
		arrPoligonos[i].p2 = vect_rot(arrPoligonos[i].p2, rot1); 
		arrPoligonos[i].p3 = vect_rot(arrPoligonos[i].p3, rot1); 
		arrPoligonos[i].normal = vect_rot(arrPoligonos[i].normal, rot1); 
		
		arrPoligonos[i].p1 = vect_rot(arrPoligonos[i].p1, rot2); 
		arrPoligonos[i].p2 = vect_rot(arrPoligonos[i].p2, rot2); 
		arrPoligonos[i].p3 = vect_rot(arrPoligonos[i].p3, rot2); 
		arrPoligonos[i].normal = vect_rot(arrPoligonos[i].normal, rot2); 
		
		arrPoligonos[i].p1 = vect_rot(arrPoligonos[i].p1, rot3); 
		arrPoligonos[i].p2 = vect_rot(arrPoligonos[i].p2, rot3); 
		arrPoligonos[i].p3 = vect_rot(arrPoligonos[i].p3, rot3); 
		arrPoligonos[i].normal = vect_rot(arrPoligonos[i].normal, rot3); 


		//Desplazo los poligonos
		arrPoligonos[i].p1 = vect_translation(arrPoligonos[i].p1, cubo.pos, 0); 
		arrPoligonos[i].p2 = vect_translation(arrPoligonos[i].p2, cubo.pos, 0); 
		arrPoligonos[i].p3 = vect_translation(arrPoligonos[i].p3, cubo.pos, 0); 

		//Now we process the polygon
		preProcessPolygon(buffer, arrPoligonos[i]);
	}
}

void preProcessPolygon(ObjectListNode ** buffer, struct Poligono polygon){
	debug("\nPreprocesando Poligono");
	//Project polygon points into plane
	struct Vector3d p1 = polygon.p1;
	struct Vector3d p2 = polygon.p2;
	struct Vector3d p3 = polygon.p3;
	struct Plano auxPlane;
	auxPlane.pos = vect_translation( ACTIVE_CAMERA.pos, vect_sum((struct Vector3d){0, 0, 0}, ACTIVE_CAMERA.dir, FRONT_CLIP), 0);
	auxPlane.normal = normalizeVector(ACTIVE_CAMERA.dir);
	debug("Pos plano: x %f, y %f, z %f", auxPlane.pos.x, auxPlane.pos.y, auxPlane.pos.z);
	struct Vector3d pointCamVector = vect_sum(p1, ACTIVE_CAMERA.pos, -1);	
	struct Vector3d auxRay = normalizeVector(pointCamVector);
	//double d1 = moduloVector(pointCamVector);
	p1 = vect_sum( (Vector3d){0, 0, 0}, auxRay, getDistanciaColisionPlano( ACTIVE_CAMERA.pos, auxRay, auxPlane));
	debug("P1: x %f, y %f, z %f", p1.x, p1.y, p1.z);

	pointCamVector = vect_sum(p2, ACTIVE_CAMERA.pos, -1);	
	auxRay = normalizeVector(pointCamVector);
	//double d2 = moduloVector(pointCamVector);
	p2 = vect_sum( (Vector3d){0, 0, 0}, auxRay, getDistanciaColisionPlano( ACTIVE_CAMERA.pos, auxRay, auxPlane));

	pointCamVector = vect_sum(p3, ACTIVE_CAMERA.pos, -1);	
	auxRay = normalizeVector(pointCamVector);
	//double d3 = moduloVector(pointCamVector);
	p3 = vect_sum( (Vector3d){0, 0, 0}, auxRay, getDistanciaColisionPlano( ACTIVE_CAMERA.pos, auxRay, auxPlane));

	//p1 = vectProjectionIntoPlane(p1, ACTIVE_CAMERA.pos, ACTIVE_CAMERA.dir);	
	//p2 = vectProjectionIntoPlane(p2, ACTIVE_CAMERA.pos, ACTIVE_CAMERA.dir);	
	//p3 = vectProjectionIntoPlane(p3, ACTIVE_CAMERA.pos, ACTIVE_CAMERA.dir);	
	
	//Move to local coords.
	struct Vector3d camVectProy = normalizeVector(vectProjectionIntoPlane( ACTIVE_CAMERA.dir, (struct Vector3d){0, 0, 0}, (struct Vector3d){0, 0, 1}));
	debug("Vector Projection: x %f, y %f, z %f", camVectProy.x, camVectProy.y, camVectProy.z);
	debug("Camera direction: x %f, y %f, z %f", ACTIVE_CAMERA.dir.x, ACTIVE_CAMERA.dir.y, ACTIVE_CAMERA.dir.z);
	double anguloGiro1 =  -anguloEntreVectores(camVectProy, ACTIVE_CAMERA.dir /*normalizeVector(vect_sum((Vector3d){0, 0, 0}, ACTIVE_CAMERA.pos, -1))*/);
	anguloGiro1 = ACTIVE_CAMERA.dir.z >= 0 ? anguloGiro1 : -anguloGiro1;
	//Rotation
	if(fabs(ACTIVE_CAMERA.dir.x) <= 0.0001 && fabs(ACTIVE_CAMERA.dir.y) <= 0.0001){
		//If camera lookin up
		debug("Cam looking up or down\n");
		camVectProy = (Vector3d){1, 0, 0};	
	}

	if(fabs(ACTIVE_CAMERA.dir.z) <= 0.0001){
		//If camera looking into plane xy
		debug("Cam looking at horizont\n");
		camVectProy = ACTIVE_CAMERA.dir;
		anguloGiro1 = 0;
	}

	double anguloGiroXY = fabs(anguloEntreVectores((struct Vector3d){0, 1, 0}, camVectProy));
	anguloGiroXY = ACTIVE_CAMERA.dir.x >= 0 ? anguloGiroXY : -anguloGiroXY;
	Quaternion quatGiro1 = newQuat(normalizeVector(vectProd(camVectProy, (Vector3d){0, 0, 1})), anguloGiro1);


	p1 = vect_rot(p1, quatGiro1);
	debug("P1 local: x %f, y %f, z %f", p1.x, p1.y, p1.z);
	p1 = vect_rot(p1, newQuat((struct Vector3d){0, 0, 1}, anguloGiroXY));
	debug("P1 local: x %f, y %f, z %f", p1.x, p1.y, p1.z);
	p2 = vect_rot(p2,  quatGiro1);
	p2 = vect_rot(p2, newQuat((struct Vector3d){0, 0, 1}, anguloGiroXY));
	p3 = vect_rot(p3, quatGiro1);
	p3 = vect_rot(p3, newQuat((struct Vector3d){0, 0, 1}, anguloGiroXY));

	double xBoundBoxMax = p1.x > p2.x ? (p1.x > p3.x ? p1.x: p3.x) : (p2.x > p3.x ? p2.x: p3.x);
	double xBoundBoxMin = p1.x < p2.x ? (p1.x < p3.x ? p1.x: p3.x) : (p2.x < p3.x ? p2.x: p3.x);
	double zBoundBoxMax = p1.z > p2.z ? (p1.z > p3.z ? p1.z: p3.z) : (p2.z > p3.z ? p2.z: p3.z);
	double zBoundBoxMin = p1.z < p2.z ? (p1.z < p3.z ? p1.z: p3.z) : (p2.z < p3.z ? p2.z: p3.z);
	//double dBoundBoxMax = d1 > d2 ? (d1 > d3 ? d1: d3) : (d2 > d3 ? d2: d3);
	//double dBoundBoxMin = d1 < d2 ? (d1 < d3 ? d1: d3) : (d2 < d3 ? d2: d3);

	//double anchoReal = 2.0*sin(deg2rad((double)myCam.fov)/2.0);
	double anchoReal = 2.0*tan(deg2rad(ACTIVE_CAMERA.fov/2.0))*FRONT_CLIP;
	double altoReal = (anchoReal*2*SCREEN_HEIGHT)/SCREEN_WIDTH;//El *2 es por el aspect ratio de los pixeles (1:2)
	int xMin = trunc((xBoundBoxMin + anchoReal/2.0) * SCREEN_WIDTH / anchoReal);
	int xMax = trunc((xBoundBoxMax + anchoReal/2.0) * SCREEN_WIDTH / anchoReal);
	xMin = xMin < 0 ? 0 : xMin;
	xMax = xMax > (SCREEN_WIDTH-1) ? (SCREEN_WIDTH-1) : xMax;
	
	//Gets inverted couse y pixel index is inverted
	int zMax = trunc((-zBoundBoxMin + altoReal/2.0) * SCREEN_HEIGHT / altoReal);
	int zMin = trunc((-zBoundBoxMax + altoReal/2.0) * SCREEN_HEIGHT / altoReal);
	zMin = zMin < 0 ? 0 : zMin;
	zMax = zMax > (SCREEN_HEIGHT-1) ? (SCREEN_HEIGHT-1) : zMax;
	debug("Bounding box: x %f %f, z %f %f", xBoundBoxMin, xBoundBoxMax, zBoundBoxMin, zBoundBoxMax);
	debug("Bounding box pix: x %d %d, z %d %d", xMin, xMax, zMin, zMax);
	//Fill in the list associated with the pixels inside the bounding box
	ObjectListNode * current;
	struct Object objAux;
	objAux.tipo = Poligono;
	for(int y = zMin; y <= zMax; y++){
		for(int x = xMin; x <= xMax; x++){
			objAux.poligono = polygon;
			//objAux.poligono.dMax = dBoundBoxMax;
			//objAux.poligono.dMin = dBoundBoxMin;
			debug("Poligono dentro de screen: x %d, y %d", x, y);
			//debug("Current: dmin %f, dmax %f", objAux.poligono.dMin, objAux.poligono.dMax);

			//Don't add if polygon is behind another polygon. Add it if is close.
			current = buffer[x+SCREEN_WIDTH*y];
			/*if(current != NULL){
				debug("Actual: dmin %f, dmax %f", current->object.poligono.dMin, current->object.poligono.dMax);
				if(current->object.poligono.dMax < objAux.poligono.dMin){
					debug("No hago nada");
					//farder, do nothing
				}else if(current->object.poligono.dMin > objAux.poligono.dMax){
					debug("Insertando Poligono unico");
					//closer, insert first and remove all
					freeList(current);
					//current = newObjectList();
					//current->object = objAux;
					buffer[x+SCREEN_WIDTH*y] = NULL;
					objectListPushFirst(&buffer[x + SCREEN_WIDTH*y], objAux);
					//buffer[x+SCREEN_WIDTH*y] = current;
				}else{
					debug("Add Poligono");
					//In the middle, insert it
					//objectListPush(current, objAux);
					objectListPushFirst(&buffer[x + SCREEN_WIDTH*y], objAux);
				}
			}else{
				objectListPushFirst(&buffer[x + SCREEN_WIDTH*y], objAux);
			}*/
			objectListPushFirst(&buffer[x + SCREEN_WIDTH*y], objAux);
		}
	}
	//getchar();
}

double getDistanciaColisionCubo(struct Vector3d origen ,struct Vector3d rayo, struct Cubo cubo, struct Object * poligono){
	debug("Calculando colision con cubo");
	//Obtengo los poligonos que forman el cubo en coordenadas locales
	//Cara superior
	struct Poligono up1 = {{cubo.escala.x/2, cubo.escala.y/2, cubo.escala.z/2},{cubo.escala.x/2, -cubo.escala.y/2, cubo.escala.z/2},{-cubo.escala.x/2, cubo.escala.y/2, cubo.escala.z/2}, (struct Vector3d){0, 0, 1}};
	struct Poligono up2 = {{-cubo.escala.x/2, -cubo.escala.y/2, cubo.escala.z/2},{cubo.escala.x/2, -cubo.escala.y/2, cubo.escala.z/2},{-cubo.escala.x/2, cubo.escala.y/2, cubo.escala.z/2}, (struct Vector3d){0, 0, 1}};
	struct Poligono down1 = {{cubo.escala.x/2, cubo.escala.y/2, -cubo.escala.z/2},{cubo.escala.x/2, -cubo.escala.y/2, -cubo.escala.z/2},{-cubo.escala.x/2, cubo.escala.y/2, -cubo.escala.z/2}, (struct Vector3d){0, 0, -1}};
	struct Poligono down2 = {{-cubo.escala.x/2, -cubo.escala.y/2, -cubo.escala.z/2},{cubo.escala.x/2, -cubo.escala.y/2, -cubo.escala.z/2},{-cubo.escala.x/2, cubo.escala.y/2, -cubo.escala.z/2}, (struct Vector3d){0, 0, -1}};
	struct Poligono right1 = {{cubo.escala.x/2, cubo.escala.y/2, cubo.escala.z/2},{cubo.escala.x/2, -cubo.escala.y/2, cubo.escala.z/2},{cubo.escala.x/2, cubo.escala.y/2, -cubo.escala.z/2}, (struct Vector3d){1, 0, 0}};
	struct Poligono right2 = {{cubo.escala.x/2, cubo.escala.y/2, -cubo.escala.z/2},{cubo.escala.x/2, -cubo.escala.y/2, -cubo.escala.z/2},{cubo.escala.x/2, -cubo.escala.y/2, cubo.escala.z/2}, (struct Vector3d){1, 0, 0}};
	struct Poligono left1 = {{-cubo.escala.x/2, cubo.escala.y/2, cubo.escala.z/2},{-cubo.escala.x/2, -cubo.escala.y/2, cubo.escala.z/2},{-cubo.escala.x/2, cubo.escala.y/2, -cubo.escala.z/2}, (struct Vector3d){-1, 0, 0}};
	struct Poligono left2 = {{-cubo.escala.x/2, cubo.escala.y/2, -cubo.escala.z/2},{-cubo.escala.x/2, -cubo.escala.y/2, -cubo.escala.z/2},{-cubo.escala.x/2, -cubo.escala.y/2, cubo.escala.z/2}, (struct Vector3d){-1, 0, 0}};
	struct Poligono front1 = {{cubo.escala.x/2, cubo.escala.y/2, cubo.escala.z/2},{cubo.escala.x/2, cubo.escala.y/2, -cubo.escala.z/2},{-cubo.escala.x/2, cubo.escala.y/2, cubo.escala.z/2}, (struct Vector3d){0, 1, 0}};
	struct Poligono front2 = {{cubo.escala.x/2, cubo.escala.y/2, -cubo.escala.z/2},{-cubo.escala.x/2, cubo.escala.y/2, cubo.escala.z/2},{-cubo.escala.x/2, cubo.escala.y/2, -cubo.escala.z/2}, (struct Vector3d){0, 1, 0}};
	struct Poligono back1 = {{cubo.escala.x/2, -cubo.escala.y/2, cubo.escala.z/2},{cubo.escala.x/2, -cubo.escala.y/2, -cubo.escala.z/2},{-cubo.escala.x/2, -cubo.escala.y/2, cubo.escala.z/2}, (struct Vector3d){0, -1, 0}};
	struct Poligono back2 = {{cubo.escala.x/2, -cubo.escala.y/2, -cubo.escala.z/2},{-cubo.escala.x/2, -cubo.escala.y/2, cubo.escala.z/2},{-cubo.escala.x/2, -cubo.escala.y/2, -cubo.escala.z/2}, (struct Vector3d){0, -1, 0}};
	struct Poligono arrPoligonos[] = {up1, up2, down1, down2, left1, left2, right1, right2, front1, front2, back1, back2};
	//Paso el poligono a globales y calculo la distancia
	double dMin = -1.0;
	double dAux = -1.0;
	for(int i = 0; i<12; i++){
		//Roto los poligonos
		struct Quaternion rot1;
		struct Quaternion rot2;
		struct Quaternion rot3;
		rot1 = newQuat((struct Vector3d){0, 0, 1}, cubo.rotacion.z);
		rot2 = newQuat(vect_rot((struct Vector3d){1, 0, 0}, rot1), cubo.rotacion.x);
		rot3 = newQuat(vect_rot((struct Vector3d){0, 1, 0}, rot2), cubo.rotacion.y);

		arrPoligonos[i].p1 = vect_rot(arrPoligonos[i].p1, rot1); 
		arrPoligonos[i].p2 = vect_rot(arrPoligonos[i].p2, rot1); 
		arrPoligonos[i].p3 = vect_rot(arrPoligonos[i].p3, rot1); 
		arrPoligonos[i].normal = vect_rot(arrPoligonos[i].normal, rot1); 
		
		arrPoligonos[i].p1 = vect_rot(arrPoligonos[i].p1, rot2); 
		arrPoligonos[i].p2 = vect_rot(arrPoligonos[i].p2, rot2); 
		arrPoligonos[i].p3 = vect_rot(arrPoligonos[i].p3, rot2); 
		arrPoligonos[i].normal = vect_rot(arrPoligonos[i].normal, rot2); 
		
		arrPoligonos[i].p1 = vect_rot(arrPoligonos[i].p1, rot3); 
		arrPoligonos[i].p2 = vect_rot(arrPoligonos[i].p2, rot3); 
		arrPoligonos[i].p3 = vect_rot(arrPoligonos[i].p3, rot3); 
		arrPoligonos[i].normal = vect_rot(arrPoligonos[i].normal, rot3); 


		//Desplazo los poligonos
		arrPoligonos[i].p1 = vect_translation(arrPoligonos[i].p1, cubo.pos, 0); 
		arrPoligonos[i].p2 = vect_translation(arrPoligonos[i].p2, cubo.pos, 0); 
		arrPoligonos[i].p3 = vect_translation(arrPoligonos[i].p3, cubo.pos, 0); 
		//Ahora calculo la distancia hasta ese poligono si es que hay
		dAux = getDistanciaColisionPoligono(origen, rayo, arrPoligonos[i].p1, arrPoligonos[i].p2, arrPoligonos[i].p3);
		if(dAux >= 0 && (dAux < dMin || dMin < 0)){
			dMin = dAux;
			poligono->tipo = Poligono;
			poligono->poligono = arrPoligonos[i];
		}
	}
	debug("Distancia al cubo: %f", dMin);	
	debug("Normal: %f, %f", poligono->poligono.normal.x, poligono->poligono.normal.y);
	return dMin;
}


double getDistanciaColisionPoligono(struct Vector3d origen, struct Vector3d rayo, struct Vector3d p1, struct Vector3d p2, struct Vector3d p3){
	debug("Calculando colision con poligono:\n - p1: %f, %f, %f\n - p2: %f, %f, %f\n - p3: %f, %f, %f\n - N: %f, %f, %f", p1.x, p1.y, p1.z, p2.x, p2.y, p2.z, p3.x, p3.y, p3.z);
	//Sigo el algoritmo Möler and Tumbore
	//https://stackoverflow.com/questions/42740765/intersection-between-line-and-triangle-in-3d
	
	struct Vector3d dir = normalizeVector(rayo);
	struct Vector3d E1 = vect_sum(p2, p1, -1);
	struct Vector3d E2 = vect_sum(p3, p1, -1);
	struct Vector3d cross_e2 = vectProd(dir, E2);
	double det = vect_mul(E1, cross_e2);
	if(det < DBL_EPSILON && det > -DBL_EPSILON){
		debug("Det = 0");
		return -1.0;
	}
	double invDet = 1.0/det;
	struct Vector3d s = vect_sum(origen, p1, -1);
	float u = invDet * vect_mul(s, cross_e2);

	if(u<0 || u > 1){
		return -1.0;
	}

	struct Vector3d cross_e1 = vectProd(s, E1);
	float v = invDet * vect_mul(dir, cross_e1);

	if(v<0 || u+v > 1){
		return -1.0;
	}

	float t = invDet * vect_mul(E2, cross_e1);
	if(t>DBL_EPSILON){
		return t;
	}else{
		return -1.0;
	}
}

Polygon newPolygon(Vector3d p1, Vector3d p2, Vector3d p3, Vector3d normal){
	Polygon p;
	p.p1 = p1;
	p.p2 = p2;
	p.p3 = p3;
	p.normal = normal;

	return p;
}

Mesh * newMesh(Polygon * polygonArray, int n_polygon, Vector3d scale, Vector3d position){
	Mesh * m = malloc(sizeof(int) + sizeof(Polygon) * n_polygon);
	for (int i = 0; i < n_polygon; i++){
		m->polygons[i] = polygonArray[i];
		m->polygons[i].p1 = vect_sum(vect_element_product(m->polygons[i].p1, scale), position, 1);
		m->polygons[i].p2 = vect_sum(vect_element_product(m->polygons[i].p2, scale), position, 1);
		m->polygons[i].p3 = vect_sum(vect_element_product(m->polygons[i].p3, scale), position, 1);
	}
	m->n_polygon = n_polygon;
	return m;
}

//***************************************************************************
//********** Operations with lists
//***************************************************************************
ObjectListNode * newObjectList(){
	ObjectListNode * head = NULL;
	head = (ObjectListNode *) malloc(sizeof(ObjectListNode));
	head -> next = NULL;
	return head;
}

void objectListPush(ObjectListNode * head, struct Object dato){
	ObjectListNode * current = head;
	while(current -> next != NULL ){
		current = current ->next;
	}

	//Set the last item
	current->next = (ObjectListNode *) malloc(sizeof(ObjectListNode));
	current->next->object = dato;
	current-> next = NULL;
}

void objectListPushFirst(ObjectListNode ** head, struct Object data){
	ObjectListNode * auxNode;
	auxNode = (ObjectListNode *) malloc(sizeof(ObjectListNode));

	auxNode->object = data;
	auxNode->next = *head;
	*head = auxNode;
}

//Function that frees the memory of an entire list
void freeList(ObjectListNode * head){
	ObjectListNode * current = head;
	ObjectListNode * next;
	while(current->next != NULL){
		next = current -> next;
		free(current);
		current = next;
	}

	free(current);
}

//Light Lists
void lightListPushFirst(LightListNode ** head, Light data){
	LightListNode * auxNode;
	auxNode = (LightListNode *) malloc(sizeof(LightListNode));

	auxNode->light = data;
	auxNode->next = *head;
	*head = auxNode;
}

void freeLightList(LightListNode * head){
	LightListNode * current = head;
	LightListNode * next;
	while(current->next != NULL){
		next = current -> next;
		free(current);
		current = next;
	}

	free(current);
}


//***************************:w
//************************************************
//********** Operaciones Matematicas
//***************************************************************************


struct Vector3d vect_sum(struct Vector3d v1, struct Vector3d v2, double factor){
	struct Vector3d res = {0.0, 0.0, 0.0};
	//debug("v1 %f, %f, %f", v1.x, v1.y, v1.z);
	//debug("v2 %f, %f, %f", v2.x, v2.y, v2.z);
	res.x = v1.x + factor * v2.x;	
	res.y = v1.y + factor * v2.y;	
	res.z = v1.z + factor * v2.z;	
	return res;
}

//Function that calculates the dot product of two vectors
double vect_mul(struct Vector3d v1, struct Vector3d v2){
	double res = 0.0;
	res= v1.x*v2.x + v1.y*v2.y + v1.z*v2.z;
	return res; 
}

Vector3d vect_element_product(struct Vector3d v1, struct Vector3d v2){
	Vector3d res= (Vector3d) {v1.x*v2.x, v1.y*v2.y, v1.z*v2.z};
	return res; 
}
struct Vector3d vect_translation(struct Vector3d point, struct Vector3d translation, int inverse){
	//Sumamos cada una de las componentes y listo.
	if(inverse){
		return vect_sum(point, translation, -1);
	}else{
		return vect_sum(point, translation, 1);
	}
}

//Funcion que realiza la multiplicacion de 2 quaternion
struct Quaternion quat_mul(struct Quaternion q1, struct Quaternion q2){
	double t0 = q1.q1*q2.q1 - q1.q2*q2.q2 - q1.q3*q2.q3 - q1.q4*q2.q4;
	double t1 = q1.q1*q2.q2 + q1.q2*q2.q1 - q1.q3*q2.q4 + q1.q4*q2.q3;
	double t2 = q1.q1*q2.q3 + q1.q2*q2.q4 + q1.q3*q2.q1 - q1.q4*q2.q2;
	double t3 = q1.q1*q2.q4 - q1.q2*q2.q3 + q1.q3*q2.q2 + q1.q4*q2.q1;
	struct Quaternion ret =  {t0, t1, t2, t3};
	return ret;
}

//Funcion que realiza la inversa de un quaternion
struct Quaternion quat_inv(struct Quaternion q){
	struct Quaternion qNew = {q.q1, -1*q.q2, -1*q.q3, -1*q.q4};
	return qNew;
}

double deg2rad(double deg){
	return 0.01745329252*deg;
}

double rad2deg(double rad){
	return rad*57.29577951;
}

//Funcion que pasa de vector + angulo (rads) sentido horario a quaternion
struct Quaternion newQuat(struct Vector3d v, double angle){
	debug("NewQuat(). V: %f, %f, %f. Angle: %f", v.x, v.y, v.z, rad2deg(angle));
	struct Vector3d vAux = normalizeVector(v);
	double theta = angle/2.0;
	double seno = sin(theta);
	double coseno = cos(theta);
	struct Quaternion ret = {coseno, vAux.x*seno, vAux.y*seno, vAux.z*seno};
 
	return ret;
}

//Funcion que realiza una rotación de un vector
struct Vector3d vect_rot(struct Vector3d v, struct Quaternion rot){
	//Normalizo el vector de rotacion
	struct Vector3d auxVect = {rot.q2, rot.q3, rot.q4};
	struct Vector3d vectNorm = normalizeVector(auxVect);
	
	//Creo el Quaternion auxiliar con el vector a rotar
	struct Quaternion qAux = {0.0, v.x, v.y, v.z};

	//Roto el vector y saco la informacion del quaternion
	struct Quaternion qAuxRotado = quat_mul(quat_mul(quat_inv(rot),qAux), rot);
	struct Vector3d ret = {qAuxRotado.q2, qAuxRotado.q3, qAuxRotado.q4};
	return ret;
}

//Funcion que calcula el angulo entre dos vectores
double anguloEntreVectores(struct Vector3d v1, struct Vector3d v2){
	//Se calcula el dot product
	double dotProd = vect_mul(v1, v2);
	//Se despeja de la formula el angulo a·b = |a|·|b|·cos(theta)
	double theta = acos(dotProd / (moduloVector(v1) * moduloVector(v2)));
	return theta;
}

double moduloVector(struct Vector3d v){
	return sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
}

struct Vector3d normalizeVector(struct Vector3d v){
	double modulo = moduloVector(v);
	struct Vector3d ret = {v.x/modulo, v.y/modulo, v.z/modulo};
	return ret;
}

//Funcion que realiza el producto vectorial entre dos vectores
struct Vector3d vectProd(struct Vector3d v1, struct Vector3d v2){
	struct Vector3d ret = {(v1.y*v2.z) - (v1.z*v2.y), (v1.z*v2.x)-(v1.x*v2.z), (v1.x*v2.y)-(v1.y*v2.x)};
	return ret;
}



struct Vector3d vectProjectionIntoPlane(struct Vector3d point, struct Vector3d ogPlane, struct Vector3d normPlane){
	struct Vector3d ret;
	struct Vector3d vAux = vect_sum(point, ogPlane, -1);
	double d = vect_mul(vAux, normalizeVector(normPlane));

	ret = vect_sum(point, normalizeVector(normPlane), -d);
	return ret;
}
