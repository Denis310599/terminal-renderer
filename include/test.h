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

enum ObjectType {Plano, Cubo, Poligono};

typedef struct Object{
	enum ObjectType tipo;
	union{
		struct Plano plano;
		struct Cubo cubo;
		struct Poligono poligono;
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
} Light;

//################ List Types #################
typedef struct ObjectListItem{
	Object object;
	ObjectListItem * next;
} ObjectListNode;

typedef struct LightListItem{
	Light light;
	LightListItem * next;
} LightListNode;



//Global variables
extern double FPS;
extern double DELTA_TIME;
extern int SCREEN_WIDTH;
extern int SCREEN_HEIGHT;
extern int GPU_MODE;
extern int FAST_LIGHT;
extern double FRONT_CLIP;
extern Camera ACTIVE_CAMERA;


//Method definitions
void renderFrame();

int addLight(Light light);
Light getLight(int id);
Light * getAllLights();
int updateLight(int id, Light light);
int deleteLight(int id);

int addObject(Object object);
Object getObject(int id);
Object * getAllObjects();
int updateObject(int id, Object ibject);
int deleteObject(int id);


#endif
