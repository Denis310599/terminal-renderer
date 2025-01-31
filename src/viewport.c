#include <GL/gl.h>
#include <stdint.h>
#include <stdio.h>
#include "../include/renderer.h"
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
//#include "./include/glad/glad.h"
#include <GLFW/glfw3.h>

#include "../lib/lpng1644/png.h"

void postProcessFrameToChar(Pixel * frameBuffer, char * output);
void printFrameGP(Pixel * buffer, unsigned char * pixel_data_in, ViewportSettings * viewport_settins);
void importStl(char * path, Vector3d scale, Vector3d pos);
void encode_png(unsigned char* raw_data, int width, int height);
char * base64_encode(const unsigned char *src, size_t len, size_t * outputLen);

int FRAME_BUFFER_INDEX = 0;

#define SHM_NAME "/png_shared_memory"  // Shared memory name
#define SHM_SIZE (1024 * 1024)         // Shared memory size (1MB)

char * payload; 
size_t output_length;

/*Function that creates a new viewport object*/
void create_viewport(ViewportSettings * viewportSettings){
	RenderSettings * renderSettings = malloc(sizeof(RenderSettings));
	renderSettings->gpu_mode = 1;
	renderSettings->front_clip = 0.1;
	renderSettings->color_mode = 1;
	renderSettings->screen_width = 600;
	renderSettings->screen_height = 400;

	viewportSettings->x = 0;
	viewportSettings->y = 0;
	viewportSettings->screen_width = 600;
	viewportSettings->screen_height = 400;
	viewportSettings->is_viewport = 1;
	viewportSettings->render_settings = renderSettings;
	viewportSettings->is_char_printed = 0;
}

void init_viewport(ViewportSettings * viewportSettings){
	//Initiates openGL
	if(viewportSettings->render_settings->gpu_mode){
		viewportSettings->window = setUpOpenGL(viewportSettings);
	}


	Pixel * frameBuffer = malloc(viewportSettings->screen_width*viewportSettings->screen_height*sizeof(Pixel));//[SCREEN_WIDTH*SCREEN_HEIGHT];
	unsigned char *pixelDataBuffer = (unsigned char *) malloc(viewportSettings->screen_width*viewportSettings->screen_height*3*sizeof(char));

	Pixel ** frameBufferPtr = malloc(sizeof(Pixel *));
	*frameBufferPtr = frameBuffer;

	unsigned char ** pixelDataBufferPtr = malloc(sizeof(unsigned char *));
	*pixelDataBufferPtr = pixelDataBuffer;

	//I dont know if this will fail, don't know if this address won't be accessed by other part of the program
	//spoiler: it failed, but i fixed it
	viewportSettings->pixel_data_buffer = frameBufferPtr;
	viewportSettings->gpu_frame_buffer = pixelDataBufferPtr;

	
}

/*Function that renders a frame in a viewport*/
void render_viewport(ViewportSettings * viewportSettings){
	//Orders to render a new frame
	if(viewportSettings->render_settings->gpu_mode){
		//printf("%s", *(viewportSettings->gpu_frame_buffer));
		calculateFrameGPU(viewportSettings, *(viewportSettings->gpu_frame_buffer));
	}else{
		renderFrame(*(viewportSettings->pixel_data_buffer));
	}

	//Move cursor to location and prints
	//printf("Moving cursor\n");
	//printf("\033[0;0H");
	fflush(stdout);
	char moveCursor[] = "123[200;200H";
	//printf("%d\n",viewportSettings->x);
	//printf("%d\n",viewportSettings->y);
	fflush(stdout);
	sprintf((char *) moveCursor, "\033[%d;%dH", viewportSettings->x, viewportSettings->y);
	printf("%s",moveCursor);
	fflush(stdout);

	//printf("Printing viewport\n");
	//fflush(stdout);

	//Shows in the viewport
	if(viewportSettings->is_char_printed == 1){
		char * output = malloc(sizeof(char)*(((viewportSettings->screen_width+1)*viewportSettings->screen_height)+ 10));
		postProcessFrameToChar(*(viewportSettings->pixel_data_buffer),output);
		printf("%s\n", output);
	}else{
		printFrameGP(*(viewportSettings->pixel_data_buffer),*(viewportSettings->gpu_frame_buffer), viewportSettings);
	}
	//In case we want to render this to char
  //char * output = malloc(sizeof(char)*(((SCREEN_WIDTH + 1) * SCREEN_HEIGHT) + 10));//;[((SCREEN_WIDTH + 1) * SCREEN_HEIGHT) + 10];
	//postProcessFrameToChar(frameBuffer,output);
	//printf("%s\n", output);


}


int main(){
	//return 0;
	DEBUG = 0;
	//Set up the renderer
	printf("Starting...\n");
	payload = base64_encode(SHM_NAME, strlen(	SHM_NAME), &output_length);
	initRenderer();
	SCREEN_WIDTH=1280;
	SCREEN_HEIGHT=720;
	PIXEL_RESOL = 1;
	GPU_MODE = 1;

	//Define the objects
	RenderSettings renderSettings;
	renderSettings.gpu_mode = 1;
	renderSettings.front_clip = 0.1;
	renderSettings.color_mode = 1;
	renderSettings.screen_width = 1280;
	renderSettings.screen_height = 720;
	ViewportSettings viewport;
	viewport.screen_width = 1280;
	viewport.screen_height = 720;
	viewport.render_settings = &renderSettings;
	viewport.y = 10;
	viewport.x = 10;

	printf("Creating viewport\n");
	create_viewport(&viewport);
	printf("Initializing viewport\n");
	init_viewport(&viewport);
	viewport.x = 5;
	viewport.y = 5;
	//GLFWwindow * window = setUpOpenGL(&viewport);
	//viewport.window = window;
	//getchar();
	
	importStl("../assets/eevee2.stl", (Vector3d){0.03f, 0.03f, 0.03f}, (Vector3d){-130, 214, 0});
	importStl("../assets/teapot.stl", (Vector3d){0.3, 0.3, 0.3}, (Vector3d){0, 0, 0});
	importStl("../assets/teapot.stl", (Vector3d){0.3, 0.3, 0.3}, (Vector3d){0, 10, 0});
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
	addObject(cubeObj);
	myCube.pos = (Vector3d){0, 6, 0};
	myCube.escala = (Vector3d){0.5, 0.5, 0.5};
	myCube.rotacion = (Vector3d){0, 0, 0};
	cubeObj.cubo = myCube;
	addObject(cubeObj);

	//Define the lights
	Light light;
	light.tipo = point;
	light.pos = (Vector3d){0, 60, 60};
	light.shadow = 0;
	light.intensidad=40;
	addLight(light);
	light.tipo = point;
	light.pos = (Vector3d){25, -25, 30};
	light.shadow = 0;
	light.intensidad=30;
	addLight(light);
	light.pos = (Vector3d){-25, -30, 30};
	light.intensidad=20;
	addLight(light);


	//Define the camera
	Camera myCam;
	//myCam.pos = (Vector3d){15, 0, 8};
	//myCam.pos = (Vector3d){70, 0, 50};
	myCam.pos = (Vector3d){10, 0, 3};
	myCam.dir = (Vector3d){2, 0, 3};
	myCam.fov = 45;
	
	ACTIVE_CAMERA = myCam;
	renderSettings.active_camera = myCam;
	FAST_LIGHT=0;

	//Define the frame buffer
	Pixel * frameBuffer = malloc(SCREEN_WIDTH*SCREEN_HEIGHT*sizeof(Pixel));//[SCREEN_WIDTH*SCREEN_HEIGHT];
	unsigned char *pixelDataBuffer = (unsigned char *) malloc(SCREEN_WIDTH*SCREEN_HEIGHT*3*sizeof(char));
  char * output = malloc(sizeof(char)*(((SCREEN_WIDTH + 1) * SCREEN_HEIGHT) + 10));//;[((SCREEN_WIDTH + 1) * SCREEN_HEIGHT) + 10];

	//pixelDataBuffer = NULL;
	//viewport.gpu_frame_buffer = &pixelDataBuffer;
	//viewport.pixel_data_buffer = &frameBuffer;
	viewport.is_char_printed = 0;

	//Time metrics
	clock_t tOld = clock();
	clock_t tNew = clock();
	double dt = 0;
	double fps;

	//Animation items
	Quaternion cameraRotationQuaternion;
	double angularSpeed = deg2rad(360)/10;
	//renderFrame(frameBuffer);
	
	printf("\033[2J");
	while(1){
	//while(!glfwWindowShouldClose(window)){
	//	if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS){
	//		glfwSetWindowShouldClose(window, 1);
	//	}


		//Calculate position of the objects in the scene (in this case the camera)
		cameraRotationQuaternion = newQuat((Vector3d) {0, 0, 1}, angularSpeed * dt);
		ACTIVE_CAMERA.pos = vect_rot(ACTIVE_CAMERA.pos, cameraRotationQuaternion);
		ACTIVE_CAMERA.dir = normalizeVector(vect_sum((Vector3d){0, 0, 1}, ACTIVE_CAMERA.pos, -1));
		viewport.render_settings->active_camera = ACTIVE_CAMERA;
		//printf("Active Dir: x %f y %f z %f\n", ACTIVE_CAMERA.dir.x, ACTIVE_CAMERA.dir.y, ACTIVE_CAMERA.dir.z);
		//Render the frame
		//calculateFrameGPU(&viewport ,pixelDataBuffer);
		//renderFrame(frameBuffer);
		//printFrameGP(frameBuffer, pixelDataBuffer, &viewport);
		render_viewport(&viewport);
		//printFrameGP(frameBuffer, NULL);
		//postProcessFrameToChar(frameBuffer,output);
		//printf("%s\n", output);
		
		//Calculate metrics
		printf("\033[0;0H");
		fflush(stdout);
		printf("%2f FPS\n", fps);
		tNew = clock();
		dt = ((double)(tNew-tOld))/CLOCKS_PER_SEC;
		fps = (double)1.0 /dt;
		tOld = tNew;
	}
}


void postProcessFrameToChar(Pixel * frameBuffer, char * output){
	//char* tablaPixeles = " .`'-~+:;<tfjrxnuvczmwqpdbkhao#MW&8%B@$";
	//char* tablaPixeles = " .`-':;+<tfxvcznumwpbkhj#WW8B$";
	//char* tablaPixeles = "`.-':;~+<rnuvwpd#W8$";
	//char* tablaPixeles = "`.-':;+<rnuvwpd#W8$";

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
	//getchar();
	//create the mesh
	Mesh *myMesh = newMesh(polygonArr, n_triangulos, scale, pos);
	meshObject.p_malla = myMesh;
	addObject(meshObject);
	//Create the object
	//Return the mesh
}

/*This function renders an image in the terminal using the graphic protocol*/
void printFrameGP(Pixel * buffer, unsigned char * pixel_data_in, ViewportSettings * viewport_settins){
	//Create the header
	//char headerMsg[] = "\033_Ga=T,i=1,m=%d,x=1234,y=1234,c=1,f=24,s=12345,v=12345,q=1;\033\\";
	//char headerMsg2[] = "\033_Gm=12345;\033\\";
	long int pixelCount = viewport_settins->screen_width*viewport_settins->screen_height;
	unsigned char * pixelData;// = malloc(sizeof(unsigned char)*pixelCount*3);
	size_t payloadToRead = 0;
	//char * payload; 
//size_t output_length;

	//char output[2000]
	int m = 1;
	int offset = 0;
	//unsigned char *printBuffer = malloc((strlen(headerMsg) + strlen(headerMsg2)*(1+(pixelCount*4/4096))+pixelCount*4)*sizeof(char));
	//unsigned char *auxBuffer = malloc(5100 * sizeof(char)); 

	if(pixel_data_in == NULL){
		//Creates the pixel data
		pixelData = malloc(sizeof(unsigned char)*pixelCount*3); 
		for(long int pixel_index = 0; pixel_index<pixelCount; pixel_index++){
			pixelData[pixel_index*3] = (unsigned char) buffer[pixel_index].r;
			pixelData[pixel_index*3+1] = (unsigned char) buffer[pixel_index].r;
			pixelData[pixel_index*3+2] = (unsigned char) buffer[pixel_index].r;
		}
	}else{
		pixelData = pixel_data_in;
	}
	//Creates the protocol message to be printed that kitty protocol understands.
	//payload = base64_encode(pixelData, pixelCount*3, &output_length);
	//const char * imagePath = "/home/denis/cosasMias/Proyectos/terminal-renderer/build/output.png";
	//unsigned char imagePath2[] = "/home/denis/cosasMias/Proyectos/terminal-renderer/build/output.png";
	encode_png(pixelData, viewport_settins->screen_width,viewport_settins->screen_height);
	//payload = base64_encode(SHM_NAME, strlen(	SHM_NAME), &output_length);
	//Montamos las cabeceras
	FRAME_BUFFER_INDEX = !FRAME_BUFFER_INDEX;
	m = (output_length>4096)? 1 : 0;
	//m=1;
	//sprintf((char *) headerMsg, "\033_Ga=T,i=%d,m=%d,f=24,s=%d,v=%d,q=2;\033\\",FRAME_BUFFER_INDEX+1, m, viewport_settins->screen_width, SCREEN_HEIGHT);
	//sprintf((char *) headerMsg2, "\033_Gm=1;\033\\");
	//printf("\x1b[%d;%dH", 10, 20);
	//sprintf((char *) printBuffer, "\033_Ga=T,i=%d,m=%d,f=24,s=%d,v=%d,q=2;\033\\",FRAME_BUFFER_INDEX+1, m, viewport_settins->screen_width, SCREEN_HEIGHT);
	//puts(headerMsg);
	
	//printf("%d\n", (int) output_length);
	printf("\033_Ga=T,t=s,i=%d,f=100,q=2;",FRAME_BUFFER_INDEX+1);
	fflush(stdout);	
	fwrite(&payload[0], sizeof(char), (int) output_length, stdout);
	printf("\033\\");
	fflush(stdout);	
	//printf("\033_Ga=T,i=%d,m=%d,f=24,s=%d,v=%d,q=2;\033\\",FRAME_BUFFER_INDEX+1, m, SCREEN_WIDTH, SCREEN_HEIGHT);
	//char * test = "1234";
	/*while(m==1){
		m = (output_length>4096)? 1 : 0;
		payloadToRead= (output_length>4096)?4096:output_length;
		//sprintf((char *) auxBuffer,"\033_Gm=%d;%.*s\033\\",m, (int) payloadToRead, payload+offset);
		//sprintf((char *) auxBuffer,"%.*s", (int) payloadToRead, payload+offset);
		//strncat((char *) printBuffer, (char *) auxBuffer, strlen((char *)auxBuffer));
		if(m==1) fwrite("\033_Gm=1;", 1, 7, stdout);
		else     fwrite("\033_Gm=0;", 1, 7, stdout);

		//printf("Buffer con fwrite: %d, %d\n", offset, (int) payloadToRead);
		fwrite(&payload[offset], sizeof(char), (int) payloadToRead, stdout);
		//printf("%s",auxBuffer);
		//printf("\n");
		//fwrite(&test[1], sizeof(char), 2, stdout);
		//puts("\033\\");
		//
		fwrite("\033\\", 1, 2, stdout);
		offset +=payloadToRead;
		output_length-=payloadToRead;
	}*/
		//getchar();
	
	//Prints the actual message
	//printf("%s", printBuffer);
	//puts(printBuffer);
	//puts(payload);
	//fflush(stdout);
	//free(pixelData);
	printf("\033_Ga=d,d=i,i=%d\033\\",!FRAME_BUFFER_INDEX+1);
	//fflush(stdout);
	//printf("\x1b[%d;%dH", 8, 20);
	//getchar();
}

static const unsigned char base64_table[65] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";


// Custom write function to write PNG data into a memory buffer
typedef struct {
    unsigned char *buffer;
    size_t size;
    size_t capacity;
} memory_writer;

//function that writes an image to the shared memory
void png_memory_write(png_structp png_ptr, png_bytep data, png_size_t length) {
    memory_writer *mem = (memory_writer *)png_get_io_ptr(png_ptr);
    if (mem->size + length > mem->capacity) {
        fprintf(stderr, "Buffer overflow while writing PNG data\n");
        longjmp(png_jmpbuf(png_ptr), 1);
    }
    memcpy(mem->buffer + mem->size, data, length);
    mem->size += length;
}

//Function that encodes an image as PNG and sotores it in shared memory
void encode_png(unsigned char* raw_data, int width, int height) {
    //FILE *fp = fopen(filename, "wb");
    //if (!fp) {
    //    printf("File opening failed");
    //    return;
    //}

		// Allocate memory for PNG encoding
		size_t buffer_size = SHM_SIZE;
    unsigned char *png_buffer = (unsigned char *)malloc(buffer_size);

	memory_writer mem = {png_buffer, 0, buffer_size};
    
	// Create png_struct and png_info structures
    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png) {
        printf("png_create_write_struct failed");
        //fclose(fp);
        return;
    }

    png_infop info = png_create_info_struct(png);
    if (!info) {
        printf("png_create_info_struct failed");
        png_destroy_write_struct(&png, (png_infopp)NULL);
        //fclose(fp);
        return;
    }

    // Set error handling (if needed)
    if (setjmp(png_jmpbuf(png))) {
        printf("Error during PNG creation");
        png_destroy_write_struct(&png, &info);
        //fclose(fp);
        return;
    }
	
	png_set_write_fn(png, &mem, png_memory_write, NULL);
	png_set_compression_level(png, 1); // Fastest compression
	png_set_filter(png, 0, PNG_FILTER_NONE);

    // Initialize the IO for writing the image to the file
    //png_init_io(png, fp);

    // Write the PNG header (signature)
    png_set_IHDR(
        png, info,
        width, height,
        8, PNG_COLOR_TYPE_RGB, // 8 bits per channel, RGBA format
        PNG_INTERLACE_NONE,     // No interlacing
        PNG_COMPRESSION_TYPE_DEFAULT, // Default compression
        PNG_FILTER_TYPE_DEFAULT // Default filter
    );

    png_write_info(png, info);

    // Prepare row pointers for each row of the image (raw_data is an array of pixel data)
    png_bytep rows[height];
    for (int i = 0; i < height; i++) {
        rows[i] = raw_data + i * width * 3;  // 4 bytes per pixel (RGBA)
    }

    // Write the image data to the PNG file
    png_write_image(png, rows);

    // Finish the PNG creation
    png_write_end(png, NULL);

    // Clean up and close the file
    png_destroy_write_struct(&png, &info);
    //fclose(fp);
		
    // Create shared memory object
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("Failed to create shared memory object");
        free(png_buffer);
        return;
    }

    // Resize shared memory to fit the PNG data
    if (ftruncate(shm_fd, mem.size) == -1) {
        perror("Failed to resize shared memory object");
        close(shm_fd);
        shm_unlink(SHM_NAME);
        free(png_buffer);
        return;
    }

    // Map shared memory into address space
    void *shm_ptr = mmap(NULL, mem.size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED) {
        perror("Failed to map shared memory");
        close(shm_fd);
        shm_unlink(SHM_NAME);
        free(png_buffer);
        return;
    }

	// Copy PNG data into shared memory
    memcpy(shm_ptr, png_buffer, mem.size);
    //printf("PNG data written to shared memory: %s\n", SHM_NAME);

    // Cleanup
    munmap(shm_ptr, mem.size);
    close(shm_fd);
    free(png_buffer);
}



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
