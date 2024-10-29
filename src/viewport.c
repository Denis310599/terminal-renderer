#include <stdint.h>
#include <stdio.h>
#include "../include/renderer.h"
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../lib/lpng1644/png.h"

void postProcessFrameToChar(char * buffer, Pixel * frameBuffer);
void printFrameGP(Pixel * buffer);
void importStl(char * path, Vector3d scale, Vector3d pos);
char * base64_encode(const unsigned char *src, size_t len, size_t * outputLen);

int FRAME_BUFFER_INDEX = 0;

int main(){
	//return 0;
	DEBUG = 0;
	//Set up the renderer
	printf("Starting...\n");
	initRenderer();
	SCREEN_WIDTH=1280;
	SCREEN_HEIGHT=720;

	//Define the objects
	importStl("../assets/teapot.stl", (Vector3d){0.3, 0.3, 0.3}, (Vector3d){0, 0, 0});
	//importStl("../assets/eevee2.stl", (Vector3d){0.1, 0.1, 0.1}, (Vector3d){8, 0, 0});
	//return 0;

	Object cubeObj;
	cubeObj.tipo = Cubo;
	Cube myCube = {{0, 0, 0}, {2, 2, 2}, {3.141592/4, 0, 3.141592/4}};
	cubeObj.cubo = myCube;

	Object planeObj;
	planeObj.tipo = Plano;
	Plane myPlane = {(Vector3d){0, 0, 0}, (Vector3d){0, 0, 1}};
	planeObj.plano = myPlane;
	addObject(planeObj);
	//addObject(cubeObj);
	myCube.pos = (Vector3d){0, 0, 0};
	myCube.escala = (Vector3d){0.5, 0.5, 0.5};
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
	//myCam.pos = (Vector3d){70, 0, 50};
	myCam.pos = (Vector3d){5, 0, 3.5};
	myCam.dir = (Vector3d){-5, 0, -5};
	myCam.fov = 90;
	
	ACTIVE_CAMERA = myCam;
	FAST_LIGHT=0;

	//Define the frame buffer
	Pixel * frameBuffer = malloc(SCREEN_WIDTH*SCREEN_HEIGHT*sizeof(Pixel));//[SCREEN_WIDTH*SCREEN_HEIGHT];
  char * output = malloc(sizeof(char)*(((SCREEN_WIDTH + 1) * SCREEN_HEIGHT) + 10));//;[((SCREEN_WIDTH + 1) * SCREEN_HEIGHT) + 10];

	//Time metrics
	clock_t tOld = clock();
	clock_t tNew = clock();
	double dt = 0;
	double fps;

	//Animation items
	Quaternion cameraRotationQuaternion;
	double angularSpeed = deg2rad(360)/10;
	renderFrame(frameBuffer);
	
	printf("\033[2J");
	while(1){
		//Calculate position of the objects in the scene (in this case the camera)
		cameraRotationQuaternion = newQuat((Vector3d) {0, 0, 1}, angularSpeed * dt);
		ACTIVE_CAMERA.pos = vect_rot(ACTIVE_CAMERA.pos, cameraRotationQuaternion);
		ACTIVE_CAMERA.dir = normalizeVector(vect_sum((Vector3d){0, 0, 1}, ACTIVE_CAMERA.pos, -1));
		//printf("Active Dir: x %f y %f z %f\n", ACTIVE_CAMERA.dir.x, ACTIVE_CAMERA.dir.y, ACTIVE_CAMERA.dir.z);
		//Render the frame
		renderFrame(frameBuffer);
		printFrameGP(frameBuffer);
		//return 0;
		//Post processing of a frame
		//postProcessFrameToChar(output, frameBuffer);
		//char* tablaPixeles = " .`'-~+:;<tfjrxnuvczmwqpdbkhao#MW&8%B@$";
		/*char* tablaPixeles = " .`-':;+<tfxvcznumwpbkhj#WW8B$";
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
		output[((SCREEN_WIDTH + 1) * SCREEN_HEIGHT)] = '\0';*/
		
		//Calculate metrics
		//printf("\033[2J");
		printf("%2f FPS\n", fps);
		tNew = clock();
		dt = ((double)(tNew-tOld))/CLOCKS_PER_SEC;
		fps = (double)1 /dt;

		//Print the frame
		//printf("%f dt\n", dt);
		//printf("%s\n", output);
		//printFrameGP(frameBuffer);
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


void printFrameGP(Pixel * buffer){
	//Create the header
	char headerMsg[] = "\033_Ga=T,i=1,m=%d,x=1234,y=1234,c=1,f=24,s=12345,v=12345,q=1;\033\\";
	char headerMsg2[] = "\033_Gm=12345;\033\\";
	long int pixelCount = SCREEN_WIDTH*SCREEN_HEIGHT;
	unsigned char * pixelData = malloc(sizeof(unsigned char)*pixelCount*3);
	size_t output_length;
	char * payload; 
	size_t payloadToRead = 0;
	//char output[2000]
	int m = 1;
	int offset = 0;
	unsigned char *printBuffer = malloc((strlen(headerMsg) + strlen(headerMsg2)*(1+(pixelCount*4/4096))+pixelCount*4)*sizeof(char));
	unsigned char *auxBuffer = malloc(5100 * sizeof(char)); 
	//PNG Stuff
	/*png_structp png_ptr = NULL;
	png_infop info_ptr = NULL;
	png_byte ** row_pointers = NULL;
	
	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	info_ptr = png_create_info_struct(png_ptr);
	
	if(!png_ptr || !info_ptr){
		return;
	}

	png_set_IHDR(png_ptr,
							info_ptr,
							SCREEN_WIDTH,
							SCREEN_HEIGHT,
							8,
							PNG_COLOR_TYPE_RGB,
							PNG_INTERLACE_NONE,
							PNG_COMPRESSION_TYPE_DEFAULT,
							PNG_FILTER_TYPE_DEFAULT);
	
	FILE *fp = fopen("/tmp/frame.png", "wb");
  if (!fp) {
      perror("Failed to open file for writing");
      //free(pixelData);
      return;
  }
  

	//Write the data in png format
	row_pointers = png_malloc(png_ptr, SCREEN_HEIGHT*sizeof(png_byte *));
	for(int y = 0; y<SCREEN_HEIGHT; y++){
		png_byte *row = png_malloc(png_ptr, sizeof(unsigned char) * SCREEN_WIDTH*pixelCount);
		row_pointers[y] = row;
		for(int x=0; x<SCREEN_WIDTH; x++){
			*row++ =buffer[(SCREEN_WIDTH*y)+x].r;
			*row++ =buffer[(SCREEN_WIDTH*y)+x].g;
			*row++ =buffer[(SCREEN_WIDTH*y)+x].b;
		}
	}

	png_init_io(png_ptr, fp);
	png_set_rows(png_ptr, info_ptr,row_pointers);
	png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

	for(int y = 0; y<SCREEN_HEIGHT; y++){
		png_free(png_ptr, row_pointers[y]);
	}
	png_free(png_ptr, row_pointers);
	fclose(fp);
	*/
	


	//Creates the pixel data
	for(long int pixel_index = 0; pixel_index<pixelCount; pixel_index++){
		pixelData[pixel_index*3] = (unsigned char) buffer[pixel_index].r;
		pixelData[pixel_index*3+1] = (unsigned char) buffer[pixel_index].r;
		pixelData[pixel_index*3+2] = (unsigned char) buffer[pixel_index].r;
	}

	// Write the image data to a temporary file
  /*FILE *fp = fopen("/tmp/frame.rgb", "wb");
  if (!fp) {
      perror("Failed to open file for writing");
      free(pixelData);
      return;
  }
  //fwrite(pixelData, 1, SCREEN_WIDTH* SCREEN_HEIGHT * 3, fp);

	fclose(fp);*/

	//char * dir = "/tmp/frame.png";
	//payload = base64_encode(dir, strlen(dir), &output_length);
	//printf("\033_Ga=T,t=f,i=2,f=100,q=1;%.*s\033\\", (int) output_length, payload);
	
	//fflush(stdout);
	//free(pixelData);
	
	payload = base64_encode(pixelData, pixelCount*3, &output_length);
	m = (output_length>4096)? 1 : 0;
	//printf("Payload: %s\n", payload);
	//printf("Output length: %d\n", output_length);
	//printf("\033[2J");
	//printf("\033_Ga=T,i=1,m=%d,f=24,s=%d,v=%d,q=1;\033\\", m, SCREEN_WIDTH, SCREEN_HEIGHT);
	//printf("%ld\n", output_length);
	FRAME_BUFFER_INDEX = !FRAME_BUFFER_INDEX;
	printf("\x1b[%d;%dH", 10, 20);
	sprintf((char *) printBuffer, "\033_Ga=T,i=%d,m=%d,f=24,s=%d,v=%d,q=2;\033\\",FRAME_BUFFER_INDEX+1, m, SCREEN_WIDTH, SCREEN_HEIGHT);
	while(m==1){
		m = (output_length>4096)? 1 : 0;
		payloadToRead= (output_length>4096)?4096:output_length;
		//printf("Offset: %d\n", offset);
		//printf("Output Length left: %d\n", output_length);
		//printf("_Gm=%d;%.*s\n", m, (int) payloadToRead, payload + offset);
		//printf("a");
		//printf("\033_Gm=%d;%.*s\033\\", m, (int) payloadToRead, payload + offset);
		sprintf((char *) auxBuffer,"\033_Gm=%d;%.*s\033\\",m, (int) payloadToRead, payload+offset);
		strncat((char *) printBuffer, (char *) auxBuffer, strlen((char *)auxBuffer));
		//fflush(stdout);
		//printf("_Ga=T,m=%d,f=32,s=%d,v=%d;%.*s\n", m, SCREEN_WIDTH, SCREEN_HEIGHT, (int) payloadToRead,payload+offset);
		//printf("\033_Ga=T,i=1,m=%d,f=32,s=%d,v=%d;%.*s\033\\", m, SCREEN_WIDTH, SCREEN_HEIGHT, (int) payloadToRead,payload+offset);
		offset +=payloadToRead;
		output_length-=payloadToRead;
		//printf("%d\n",(int) output_length);
		//printf("%d\n", offset);
		//printf("%d\n", payloadToRead);
	}

	printf("%s", printBuffer);
	//printf("\033_Ga=p,i=%d,c=40,r=10\033\\", FRAME_BUFFER_INDEX+1);
	fflush(stdout);
	printf("\033_Ga=d,d=i,i=%d\033\\",!FRAME_BUFFER_INDEX+1);
	fflush(stdout);
	printf("\x1b[%d;%dH", 8, 20);
	//printf("a");
	//free(pixelData);
	//free(printBuffer);
	//free(auxBuffer);
	//printf("a");
	//fflush(stdout);
	//printf(baseMsg, 24, 3, 3, pixelData);
	//printf("\e_Ga=T,f=%d,s=%d,v=%d;%s\e\\", 24, SCREEN_HEIGHT, SCREEN_WIDTH, pixelData);


}
static const unsigned char base64_table[65] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/**
* base64_encode - Base64 encode
* @src: Data to be encoded
* @len: Length of the data to be encoded
* @out_len: Pointer to output length variable, or %NULL if not used
* Returns: Allocated buffer of out_len bytes of encoded data,
* or empty string on failure
*/
char * base64_encode(const unsigned char *src, size_t len, size_t * outputLen)
{
    unsigned char *out, *pos;
    const unsigned char *end, *in;

    size_t olen;

    olen = 4*((len + 2) / 3); /* 3-byte blocks to 4-byte */

    if (olen < len)
        return NULL; /* integer overflow */
		*outputLen = olen;
    char * outStr = malloc(sizeof(char)*olen);
    out = (unsigned char*)&outStr[0];

    end = src + len;
    in = src;
    pos = out;
    while (end - in >= 3) {
        *pos++ = base64_table[in[0] >> 2];
        *pos++ = base64_table[((in[0] & 0x03) << 4) | (in[1] >> 4)];
        *pos++ = base64_table[((in[1] & 0x0f) << 2) | (in[2] >> 6)];
        *pos++ = base64_table[in[2] & 0x3f];
        in += 3;
    }

    if (end - in) {
        *pos++ = base64_table[in[0] >> 2];
        if (end - in == 1) {
            *pos++ = base64_table[(in[0] & 0x03) << 4];
            *pos++ = '=';
        }
        else {
            *pos++ = base64_table[((in[0] & 0x03) << 4) |
                (in[1] >> 4)];
            *pos++ = base64_table[(in[1] & 0x0f) << 2];
        }
        *pos++ = '=';
    }

    return outStr;
}
