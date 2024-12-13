#include <GLFW/glfw3.h>
#ifndef RENDERER

//################ Math Types #################
typedef struct Vector3d{
	double x;
	double y;
	double z;
} Vector3d;

typedef struct Quaternion{
	double q1;
	double q2;
	double q3;
	double q4;
} Quaternion;

//################ Camera #################
typedef struct Camara{
	Vector3d pos;
	Vector3d dir;
	int fov;
} Camera;

//################ Object Types#################
typedef struct Cubo{
	Vector3d pos;
	Vector3d escala;
	Vector3d rotacion;
} Cube;

typedef struct Plano{
	Vector3d pos;
	Vector3d normal;
} Plane;

typedef struct Poligono{
	Vector3d p1;
	Vector3d p2;
	Vector3d p3;
	Vector3d normal;
	double dMax;
	double dMin;
} Polygon;

typedef struct Mesh{
	int n_polygon;
	Polygon polygons[];
} Mesh;

enum ObjectType {Plano, Cubo, Poligono, Malla};

typedef struct Object{
	enum ObjectType tipo;
	union{
		struct Plano plano;
		struct Cubo cubo;
		struct Poligono poligono;
		Mesh * p_malla;
	};
	int id;
} Object;


//################ Light Types #################
enum LightType {inf, point};

typedef struct Luz{
	enum LightType tipo;
	struct Vector3d pos;
	double intensidad;
	int shadow; //0 no, 1 hard, 2 soft
	int id;
} Light;

//################ List Types #################
typedef struct ObjectListItem{
	Object object;
	struct ObjectListItem * next;
} ObjectListNode;

typedef struct LightListItem{
	Light light;
	struct LightListItem * next;
} LightListNode;


typedef struct Pixel{
	int r;
	int g;
	int b;
} Pixel;



//Global variables
extern double FPS;
extern double DELTA_TIME;
extern int SCREEN_WIDTH;
extern int SCREEN_HEIGHT;
extern int GPU_MODE;
extern int FAST_LIGHT;
extern double FRONT_CLIP;
extern Camera ACTIVE_CAMERA;
extern double UMBRAL_LUZ;
extern int COLOR_MODE;
extern int DEBUG;
extern double PIXEL_RESOL; 

//Method definitions
void renderFrame(Pixel * frameBuffer);
void calculateFrameGPU(unsigned char * pixelData);
void initRenderer();
GLFWwindow * setUpOpenGL();

int addLight(Light light);
int getLight(int id, Light * light);
LightListNode * getAllLights();
int updateLight(int id, Light light);
int deleteLight(int id);

int addObject(Object object);
int getObject(int id, Object * object);
ObjectListNode * getAllObjects();
int updateObject(int id, Object object);
int deleteObject(int id);

Polygon newPolygon(Vector3d p1, Vector3d p2, Vector3d p3, Vector3d normal);
Mesh * newMesh(Polygon * polygonArray, int m_polygon, Vector3d scale, Vector3d position);


//Math operations

double vect_mul(Vector3d v1, Vector3d v2);
//Calculates element wise product of 2 vectors.
Vector3d vect_element_product(struct Vector3d v1, struct Vector3d v2);
Vector3d vect_sum(Vector3d v1, Vector3d v2, double factor);
Vector3d vect_translation(Vector3d point, Vector3d translation, int inverse);
Quaternion quat_mul(Quaternion q1, Quaternion q2);
Quaternion quat_inv(Quaternion);
Vector3d vect_rot(Vector3d v, Quaternion rot);
Quaternion newQuat(Vector3d v, double angle);
Vector3d quat2vec(Quaternion q);
double anguloEntreVectores(Vector3d v1, Vector3d v2);
double moduloVector(Vector3d v);
double deg2rad(double deg);
double rad2deg(double rad);
Vector3d normalizeVector(Vector3d v);
Vector3d vectProd(Vector3d v1, Vector3d v2);
Vector3d vectProjectionIntoPlane(Vector3d point, Vector3d ogPlane, Vector3d normPlane);


#endif
