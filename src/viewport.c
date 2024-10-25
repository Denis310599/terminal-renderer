#include <stdint.h>
#include <stdio.h>
#include "../include/renderer.h"
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

void postProcessFrameToChar(char * buffer, Pixel * frameBuffer);
void printFrameGP();
void importStl(char * path, Vector3d scale, Vector3d pos);

int main(){
	//return 0;
	DEBUG = 0;
	//Set up the renderer
	printf("Starting...\n");
	initRenderer();
	SCREEN_WIDTH=400;
	SCREEN_HEIGHT=100;

	//Define the objects
	importStl("../assets/eevee2.stl", (Vector3d){1, 1, 1}, (Vector3d){47, 214, 0});
	//return 0;

	Object cubeObj;
	cubeObj.tipo = Cubo;
	Cube myCube = {{0, 0, 0}, {2, 2, 2}, {3.141592/4, 0, 3.141592/4}};
	cubeObj.cubo = myCube;

	Object planeObj;
	planeObj.tipo = Plano;
	Plane myPlane = {(Vector3d){0, 0, 0}, (Vector3d){0, 0, 1}};
	planeObj.plano = myPlane;
	//addObject(planeObj);
	//addObject(cubeObj);
	myCube.pos = (Vector3d){2, -2, 1};
	myCube.escala = (Vector3d){1, 1, 1};
	myCube.rotacion = (Vector3d){0, 0, 0};
	cubeObj.cubo = myCube;
	//addObject(cubeObj);

	//Define the lights
	Light light;
	light.tipo = point;
	light.pos = (Vector3d){0, 60, 60};
	light.shadow = 0;
	light.intensidad=20;
	addLight(light);
	light.tipo = point;
	light.pos = (Vector3d){25, -25, 30};
	light.shadow = 0;
	light.intensidad=20;
	addLight(light);
	light.pos = (Vector3d){-25, -30, 30};
	light.intensidad=10;
	addLight(light);


	//Define the camera
	Camera myCam;
	//myCam.pos = (Vector3d){15, 0, 8};
	myCam.pos = (Vector3d){70, 0, 50};
	myCam.dir = (Vector3d){-5, 0, -5};
	myCam.fov = 90;
	
	ACTIVE_CAMERA = myCam;
	FAST_LIGHT=0;

	//Define the frame buffer
	Pixel frameBuffer[SCREEN_WIDTH*SCREEN_HEIGHT];
  char output[((SCREEN_WIDTH + 1) * SCREEN_HEIGHT) + 10];

	//Time metrics
	clock_t tOld = clock();
	clock_t tNew = clock();
	double dt = 0;
	double fps;

	//Animation items
	Quaternion cameraRotationQuaternion;
	double angularSpeed = deg2rad(360)/10;
	while(1){
		//Calculate position of the objects in the scene (in this case the camera)
		cameraRotationQuaternion = newQuat((Vector3d) {0, 0, 1}, angularSpeed * dt);
		ACTIVE_CAMERA.pos = vect_rot(ACTIVE_CAMERA.pos, cameraRotationQuaternion);
		ACTIVE_CAMERA.dir = normalizeVector(vect_sum((Vector3d){0, 0, 30}, ACTIVE_CAMERA.pos, -1));
		printf("Active Dir: x %f y %f z %f\n", ACTIVE_CAMERA.dir.x, ACTIVE_CAMERA.dir.y, ACTIVE_CAMERA.dir.z);
		//Render the frame
		renderFrame(frameBuffer);
		//printFrameGP(frameBuffer);
		//Post processing of a frame
		//postProcessFrameToChar(output, frameBuffer);
		//char* tablaPixeles = " .`'-~+:;<tfjrxnuvczmwqpdbkhao#MW&8%B@$";
		char* tablaPixeles = " .`-':;+<tfxvcznumwpbkhj#WW8B$";
		//char* tablaPixeles = "`.-':;~+<rnuvwpd#W8$";
		//char* tablaPixeles = "`.-':;+<rnuvwpd#W8$";
		int longitudTablaPixeles = strlen(tablaPixeles);
		
		for (int y = 0; y < SCREEN_HEIGHT; y++) {
			for (int x = 0; x < SCREEN_WIDTH; x++) {
				// Indice del array
				char pixel;
				int i = (SCREEN_WIDTH) * y + x;
				int indice = (SCREEN_WIDTH +1)*y +x;
				int intensidad = frameBuffer[i].r;
				int indicePixel = intensidad *  longitudTablaPixeles / 256;
				//printf("PostProcesando pixel x: %d, y: %d\n", x, y);	
				//printf("Intensidad: %d, IndicePixel: %d\n", intensidad, indicePixel);
				if (intensidad == -1) {
					pixel = ' ';
				} else {
					pixel = indicePixel <= (longitudTablaPixeles - 1)
									? tablaPixeles[indicePixel]
									: tablaPixeles[longitudTablaPixeles - 1];
				}
					output[indice] = pixel;
					//printf("Caracter puesto: %c\n", pixel);
					//printf("Caracter puesto: %c\n", output[indice]);
					//printf("%s\n", output);

			}
			output[((SCREEN_WIDTH+1)*y)-1] = '\n';
		}
		output[((SCREEN_WIDTH + 1) * SCREEN_HEIGHT)] = '\0';
		
		//Calculate metrics
		tNew = clock();
		dt = ((double)(tNew-tOld))/CLOCKS_PER_SEC;
		fps = (double)1 /dt;

		//Print the frame
		printf("%2f FPS\n", fps);
		printf("%f dt\n", dt);
		printf("%s\n", output);
		tOld = tNew;
	}
}

void postProcessFrameToChar(char * output, Pixel * frameBuffer){
	char* tablaPixeles = " .`'-~+:;<tfjrxnuvczmwqpdbkhao#MW&8%B@$";
	int longitudTablaPixeles = strlen(tablaPixeles);
	
	for (int y = 0; y < SCREEN_HEIGHT; y++) {
	  for (int x = 0; x < SCREEN_WIDTH; x++) {
			// Indice del array
			char pixel;
			int i = (SCREEN_WIDTH) * y + x;
			int indice = (SCREEN_WIDTH +1)*y +x;
			int intensidad = frameBuffer[i].r;
			int indicePixel = intensidad *  longitudTablaPixeles / 256;
			//printf("PostProcesando pixel x: %d, y: %d\n", x, y);	
			//printf("Intensidad: %d, IndicePixel: %d\n", intensidad, indicePixel);
			if (intensidad == -1) {
				pixel = ' ';
			} else {
				pixel = indicePixel <= (longitudTablaPixeles - 1)
                ? tablaPixeles[indicePixel]
                : tablaPixeles[longitudTablaPixeles - 1];
			}
				output[indice] = pixel;
				//printf("Caracter puesto: %c\n", pixel);
				//printf("Caracter puesto: %c\n", output[indice]);
				//printf("%s\n", output);

    }
		output[((SCREEN_WIDTH+1)*y)-1] = '\n';
  }
	output[((SCREEN_WIDTH + 1) * SCREEN_HEIGHT)] = '\0';
}


void importStl(char * path, Vector3d scale, Vector3d pos){
	FILE * p_file;
  unsigned char buffer[50*4];

	//Open the file
	p_file = fopen(path, "r");

	if(p_file == NULL){return;}

	//Get the file type
	fread(buffer, sizeof(uint8_t)*80, 1, p_file);
	printf("Primera palabra: %s\n", buffer);
	if(strncmp((char *)buffer, "solid", 6) == 0){
		return;
	}
	int n_triangulos;	

	//Get polygon count
	fread(buffer, sizeof(uint32_t), 1, p_file);
	printf("0x%02x 0x%02x 0x%02x 0x%02x \n", buffer[0], buffer[1], buffer[2], buffer[3]);	
	printf("%d %d %d %d \n", buffer[0], buffer[1], buffer[2], buffer[3]);	
	//int n_triangulos = 256*256*256*buffer[0] + 256*256*buffer[1] + 256*buffer[2] + 1*buffer[3];
	memcpy(&n_triangulos, &buffer[0], sizeof(int));
	printf("Numero de triangulos: %d\n", n_triangulos);

	Polygon *polygonArr = malloc(n_triangulos*sizeof(Polygon));
	

	//Get every polygon
	float x_coord;
	float y_coord;
	float z_coord;

	Vector3d point[4];
	Object meshObject;
	meshObject.tipo = Malla;
	Vector3d meanPosition = {0, 0, 0};
	Vector3d meanPoint;
	int leido = 0;
	for (int triangle_index = 0; triangle_index<n_triangulos; triangle_index++){
		leido = fread(buffer, sizeof(unsigned char), 50, p_file);
		if(leido!=50){
			printf("Leidos %d triangulos", triangle_index);
			break;
		}
		//printf("Leido: %d\n", leido);
		//printf("Triangulo %d\n", triangle_index);

		for(int  point_index = 0; point_index<4; point_index++){
			memcpy(&x_coord, &buffer[point_index*12], sizeof(float));
			memcpy(&y_coord, &buffer[point_index*12 + 4], sizeof(float));
			memcpy(&z_coord, &buffer[point_index*12 + 8], sizeof(float));
			if(point_index == 0){
				point[point_index] = (Vector3d){x_coord, y_coord, z_coord};
			}else{
				//point[point_index] = (Vector3d){(x_coord+pos.x)*scale.x, (y_coord+pos.y)*scale.y, (z_coord+pos.z)*scale.z};
				//addObject(meshObject);
				point[point_index] = (Vector3d){x_coord, y_coord, z_coord};
			}
			//printf("Punto %d: x %f y %f z %f\n", point_index, x_coord, y_coord, z_coord);
		}

		point[0] = normalizeVector(vectProd(vect_sum(point[1], point[3], -1), vect_sum(point[2], point[3], -1)));

		//Create the polygon
		meanPoint = (Vector3d) {0, 0, 0};
		meanPoint = vect_sum(meanPoint, point[1], 1.0/3.0);
		meanPoint = vect_sum(meanPoint, point[2], 1.0/3.0);
		meanPoint = vect_sum(meanPoint, point[3], 1.0/3.0);
		meanPosition = vect_sum(meanPosition, meanPoint, 1.0/n_triangulos);
		polygonArr[triangle_index] = newPolygon(point[1], point[2], point[3], point[0]);
		//break;
	}
	printf("Center of Object: x %f y %f z %f", meanPosition.x, meanPosition.y, meanPosition.z);
	getchar();
	//create the mesh
	Mesh *myMesh = newMesh(polygonArr, n_triangulos, scale, pos);
	meshObject.p_malla = myMesh;
	addObject(meshObject);
	//Create the object
	//Return the mesh
}


void printFrameGP(){
	//Create the header
	char baseMsg[] = "\033_Gf=%d,s=%d,v%d;%s\033\\";
	char pixelData[27] = {255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255};
	//char output[2000];
	
	//printf(baseMsg, 24, 3, 3, pixelData);
	printf("\033_Gf=%d,s=%d,v%d;%s\033\\", 24, 3, 3, pixelData);

}
