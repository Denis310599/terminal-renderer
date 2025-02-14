#include <fcntl.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include "../include/renderer.h"
#include "../include/viewport.h"

/*Appearance Structures*/
typedef struct Color{
	uint8_t r;
	uint8_t g;
	uint8_t b;
} Color;

/*Structures related with components*/

/*Definition of the components*/
typedef struct Container{
  int isFocus;
} Container;

typedef struct Viewport{
  ViewportSettings * vp_settings;
} Viewport;

typedef struct Style{
	char ** barckgroundColor;
	char ** focusBackgroundColor;
	int borderWidth;
	int borderRadius;
	char ** focusBorderColor;
	char ** borderColor;
} Style;

enum ComponentType {container_t, viewport_t};
typedef struct Component{
  //Basic config
	enum ComponentType component_type;
  int x;
  int y;
  int height;
  int width;
	int real_height;
	int real_width;
	int maxHeight;
	int minHeight;
	int maxWidth;
	int minWidth;
  int isUpdated;

	//Dynamic positioning
	struct Component * topToTopOf;
	struct Component * topToBottomOf;
	struct Component * bottomToBottomOf;
	struct Component * bottomToTopOf;
	struct Component * startToStartOf;
	struct Component * startToEndOf;
	struct Component * endToEndOf;
	struct Component * endToStartOf;
	int marginTop;
	int marginBot;
	int marginStart;
	int marginEnd;
	int margin;
	int paddingTop;
	int paddingBot;
	int paddingStart;
	int paddingEnd;
	int padding;
	float xBias;
	float yBias;

	//Dynamic sizing
	int autoHeight; //0 noAuto, 1 fitContent, 2 fitSpace, 3 percentaje
	float heightBias;
	int autoWidth;
	float widthBias;

	//Appearance
	int border; //0 no border, 1 normal border, 2 big border
	Color backgroundColor;
	Color borderColor;



	//Custom settings
  union{
    Container properties;
  };
  int childCount;
  struct Component ** children;
} Component;

/*Function declaration*/
void initUI();
void drawUI();
void drawContainer(Component * component, Component * parent);
void drawComponent(Component * component, Component * parent);
void drawBox(int x, int y, int height, int width, int drawBorder, Color bgColor, Color borderColor);
void drawViewport();
void mark_component_as_updated(Component * component);
void handle_resize();

void calculateComponentDimensions(Component * component, Component * parent, int mode);

Component * newContainer();
Color * newColor(uint8_t r, uint8_t g, uint8_t b);

int handleInput();
void closeProgram();

void enable_raw_mode();
void set_nonblocking_mode();
void disable_raw_mode();
void prepareTerminal();
void restoreTerminal();
void get_terminal_size(int *rows, int *cols);

/*File accessible variables*/
Component parentComponent;

/*Color deffinitions*/
const Color BG_COLOR = (Color){7, 18, 36};
const Color FONT_COLOR= (Color){173, 173, 173};
const Color BG_2_COLOR= (Color){29, 42, 64};
int main(){
	DEBUG=1;
  initUI();
	//getchar();
  ViewportSettings viewport;
  vp_create_viewport(&viewport);
  vp_init_viewport(&viewport);
  viewport.x = 5;
  viewport.y = 5;

  //Define the camera
	Camera myCam;
	//myCam.pos = (Vector3d){15, 0, 8};
	//myCam.pos = (Vector3d){70, 0, 50};
	myCam.pos = (Vector3d){10, 0, 3};
	myCam.dir = (Vector3d){2, 0, 3};
	myCam.fov = 45;
	viewport.render_settings->active_camera = myCam;
  importStl("../assets/eevee2.stl", (Vector3d){0.03f, 0.03f, 0.03f}, (Vector3d){-130, 214, 0});
	importStl("../assets/teapot.stl", (Vector3d){0.3, 0.3, 0.3}, (Vector3d){0, 0, 0});
	importStl("../assets/teapot.stl", (Vector3d){0.3, 0.3, 0.3}, (Vector3d){0, 10, 0});

  printf("\033[2J");
  while(1){
    if(handleInput()) break;
		myCam.dir = normalizeVector(vect_sum((Vector3d){0, 0, 1}, myCam.pos, -1));
		viewport.render_settings->active_camera = myCam;

    drawUI();
		//getchar();
    //int aux = 0/0;
		//vp_render_viewport(&viewport);
    
  }
}

void initUI(){
  //Prepares the terminal
  prepareTerminal();

	parentComponent = *newContainer();
  //Calculates component table
  parentComponent.x = 0;
  parentComponent.y = 0;

  int rows, cols;
  get_terminal_size(&rows, &cols);
  parentComponent.height = rows;
  parentComponent.width = cols-1;
  parentComponent.real_width= cols-1;
  parentComponent.isUpdated = 3;
  parentComponent.childCount = 4;
	parentComponent.component_type = container_t;
	parentComponent.backgroundColor = BG_COLOR;
	parentComponent.border = 1;
	parentComponent.padding = 0;

	parentComponent.children = malloc(2*sizeof(Component *));
	parentComponent.children[0] = newContainer();
	parentComponent.children[0]->topToTopOf = &parentComponent;
	//parentComponent.children[0]->bottomToBottomOf = &parentComponent;
	parentComponent.children[0]->height = 10;
	parentComponent.children[0]->real_width = 20;
	parentComponent.children[0]->x= 5;
	parentComponent.children[0]->autoHeight = 0;
	
	parentComponent.children[2] = newContainer();
	//parentComponent.children[2]->topToBottomOf=parentComponent.children[0];
	parentComponent.children[2]->bottomToBottomOf = &parentComponent;
	parentComponent.children[2]->height = 10;
	parentComponent.children[2]->real_width = 10;
	parentComponent.children[2]->x= 5;
	parentComponent.children[2]->autoHeight = 0;

	//parentComponent.children = malloc(1*sizeof(Component *));
	parentComponent.children[1] = newContainer();
	parentComponent.children[1]->topToBottomOf=parentComponent.children[0];
	parentComponent.children[1]->bottomToTopOf = parentComponent.children[2];
	parentComponent.children[1]->height = 10;
	parentComponent.children[1]->real_width = 10;
	parentComponent.children[1]->x= 16;
	parentComponent.children[1]->autoHeight = 2;


  Component * contChild = newContainer();
	contChild->childCount = 1;
	contChild->x = 50;
	contChild->autoHeight = 2;
	contChild->height = 10;
	contChild->real_width = 10;
	contChild->backgroundColor = BG_2_COLOR;
	contChild->border = 0;
	//contChild->marginTop = 5;
	contChild->topToTopOf = &parentComponent;//parentComponent.children[0];
	contChild->bottomToBottomOf = &parentComponent;// parentComponent.children[2];
	contChild->yBias = 0.3f;
	contChild->heightBias = 0.5f;
	parentComponent.children[3] = contChild;

	contChild->children = malloc(1*sizeof(Component *));
	Component * contChild2 = newContainer();
	contChild2->childCount = 0;
	contChild2->x = 2;
	contChild2->autoHeight = 0;
	contChild2->real_width = 3;
	contChild2->marginTop=2;
	contChild2->marginBot=3;
	contChild->children[0] = contChild2;
	//contChild2->to



	/*parentComponent.children[1] = newContainer();
	parentComponent.children[1]->bottomToBottomOf= &parentComponent;
	parentComponent.children[1]->height = 3;
	parentComponent.children[1]->real_width = 5;
	parentComponent.children[1]->x= 5;

	parentComponent.children[2] = newContainer();
	parentComponent.children[2]->topToBottomOf= parentComponent.children[0];
	parentComponent.children[2]->bottomToTopOf= parentComponent.children[1];
	parentComponent.children[2]->autoHeight = 0;
	parentComponent.children[2]->height = 3;
	parentComponent.children[2]->real_width = 5;
	parentComponent.children[2]->x= 5;
	*/

}

void prepareTerminal(){
  //Swaps buffer
  printf("\033[?1049h");
  fflush(stdout);
	printf("\033[?25l");
  fflush(stdout);

  //Enable raw input mode
  enable_raw_mode();

  //Set not blocking input
  set_nonblocking_mode();
  
	//Resize window handler
	signal(SIGWINCH, handle_resize);


}

void handle_resize(){
	debug("Resizing window");
  printf("\033[2J");
	//marco elementos como por updatear
	mark_component_as_updated(&parentComponent);
	
	int rows, cols;
  get_terminal_size(&rows, &cols);
  parentComponent.height = rows;
  parentComponent.width = cols-1;
  parentComponent.real_width= cols-1;

}

void mark_component_as_updated(Component * component){
	component->real_height = -1;
	component->isUpdated = 3;

	for(int i = 0; i<component->childCount; i++){
		mark_component_as_updated(component->children[i]);
	}
}

void restoreTerminal(){
  //signal(SIGWINCH, handle_resize);
  disable_raw_mode();
  
  //Restores buffer
  printf("\033[?1049l");
  fflush(stdout);
  printf("\033[?25h");
  fflush(stdout);

}
void enable_raw_mode() {
    struct termios term;
    tcgetattr(STDIN_FILENO, &term);  // Get terminal settings
    term.c_lflag &= ~(ICANON | ECHO); // Disable canonical mode & echo
    tcsetattr(STDIN_FILENO, TCSANOW, &term);  // Apply changes
}

void disable_raw_mode() {
    struct termios term;
    tcgetattr(STDIN_FILENO, &term);
    term.c_lflag |= (ICANON | ECHO); // Restore canonical mode & echo
    tcsetattr(STDIN_FILENO, TCSANOW, &term);
}

void set_nonblocking_mode() {
    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
}


int handleInput(){
  char ch;
  if(read(STDIN_FILENO, &ch, 1)>0){
    if(ch == 'q'){
      //Process closing
      closeProgram();
      return 1;
    }
  }
  return 0;
}

void closeProgram(){
  restoreTerminal(); 
}


void get_terminal_size(int *rows, int *cols) {
  struct winsize w;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
  *rows = w.ws_row;
  *cols = w.ws_col;
}

void drawComponent(Component * component, Component * parent){
	debug("Drawing component %d.", component);
  if(component->component_type == container_t) drawContainer(component, parent);
}

void drawContainer(Component * component, Component * parent){
	debug("Drawing container %d.\n X: %d, Y: %d,\n height: %d,\n width: %d", component, component->x, component->y, component->real_height, component->real_width);
  if(component->isUpdated== 0) return;
  component->isUpdated--;

  //Draws the border
  if(component == &parentComponent) {
		drawBox(component->x,
          component->y,
          component->real_height,
          component->real_width,
					component->border,
					component->backgroundColor,
					(Color){});
	}else if(parent != NULL){
		drawBox(component->x + parent->x,
          component->y+ parent->y,
          component->real_height,
          component->real_width,
					component->border,
					component->backgroundColor,
					(Color){});

	}
  
  //Iterates over every child component
	for (int i = 0; i<component->childCount; i++){
		drawComponent(component->children[i], component);
	}
}

void drawBox(int x, int y, int height, int width, int drawBorder, Color bgColor, Color borderColor){
  int true_x = x;
  int true_y = y+1;
  int true_end_x = x+width;
  int true_end_y = y+height;

  int max_x = parentComponent.x + parentComponent.width;
  int max_y = parentComponent.y + parentComponent.height;


  if(true_x<0) true_x = 0;
  if(true_end_x > max_x) true_end_x = max_x;
  if(true_y<0) true_y = 0;
  if(true_end_y > max_y) true_end_y = max_y;
  
  //Position cursor to start
  printf("\033[%d;%dH", true_y, true_x);
  fflush(stdout);
  for (int y_iter = true_y; y_iter <=true_end_y; y_iter++){
    for(int  x_iter = true_x; x_iter <= true_end_x; x_iter++){
      //if(y_iter == true_y) printf("%s", ".");
			if(drawBorder == 1){
				if(x_iter == true_x && y_iter == true_y){ 
					printf("\e[48;2;%d;%d;%dm%s", bgColor.r, bgColor.g, bgColor.b, "╭");
					fflush(stdout);
				}else if(x_iter == true_x && y_iter == true_end_y){ 
					printf("\e[48;2;%d;%d;%dm%s", bgColor.r, bgColor.g, bgColor.b, "╰");
					fflush(stdout);
				}else if(x_iter == true_end_x && y_iter == true_y){
					printf("\e[48;2;%d;%d;%dm%s", bgColor.r, bgColor.g, bgColor.b, "╮");
					fflush(stdout);
				}else if(x_iter == true_end_x && y_iter == true_end_y){
					printf("\e[48;2;%d;%d;%dm%s", bgColor.r, bgColor.g, bgColor.b, "╯");
					fflush(stdout);
				}else if(y_iter == true_y || y_iter == true_end_y){ 
					printf("\e[48;2;%d;%d;%dm%s", bgColor.r, bgColor.g, bgColor.b, "─");
					fflush(stdout);
				}else if(x_iter == true_x || x_iter == true_end_x){
					printf("\e[48;2;%d;%d;%dm%s", bgColor.r, bgColor.g, bgColor.b, "│");
					fflush(stdout);
				}else{
					printf("\e[48;2;%d;%d;%dm%s", bgColor.r, bgColor.g, bgColor.b, " ");
					fflush(stdout);
				}
			}else{
					printf("\e[48;2;%d;%d;%dm%s", bgColor.r, bgColor.g, bgColor.b, " ");
					fflush(stdout);
			}
    }
    printf("\033[%d;%dH", y_iter+1, true_x);
    fflush(stdout);
  }
}
void drawUI(){
	calculateComponentDimensions(&parentComponent, &parentComponent, 0);
  drawComponent(&parentComponent, NULL);
}

/*Function that calculates and populates the dimensions and positions of a component*/
/*Mode: 0 height, 1 width*/
void calculateComponentDimensions(Component * component, Component * parent, int mode){
	debug("Calculando componente con id %d", parent);
	//Skips this one if it's position depends on his parent height and it's not yet been calculated
	if(parent->real_height == 0 && (component->topToBottomOf == parent || component->bottomToBottomOf == parent)) return;
	//Calculates the dimensions
	int componentMarginTop = component->marginTop == -1 ? component->margin : component->marginTop;
	int componentMarginBot = component->marginBot == -1 ? component->margin : component->marginBot;
	int componentPaddingTop = parent->paddingTop== -1 ? parent->padding: parent->paddingTop;
	int componentPaddingBot = parent->paddingBot== -1 ? parent->padding: parent->paddingBot;
	int min_y = -1;
	int max_y = -1;

	if(component == &parentComponent){
		debug("Component is parent");
		component->real_height = component->height;
	}else if(component->autoHeight == 1){
		debug("Height dependant on childs");
			//Height dependant on children height
			component->real_height = 0;
			int auxHeight= 0;
			int maxHeight= 0;
			for(int i = component->childCount - 1; i>=0; i--){
				Component * child = component->children[i];
				if(child->real_height == -1)	calculateComponentDimensions(child, component, 0);
				int auxMargin= 0;
				auxMargin += child->marginBot == -1 ? child->margin : child->marginBot;	
				auxHeight = child->y + child->real_height + auxMargin;
				maxHeight = maxHeight<auxHeight ? auxHeight : maxHeight;
				maxHeight += componentPaddingBot;
			}
			component->real_height = maxHeight;
		debug("  Height: %d", maxHeight);
	}else{
		component->real_height = component->height;
	}

	//Calculate the max and min coords of location
	//Top of component
	if(component->topToTopOf != NULL && component->topToTopOf != parent){
		if(component->topToTopOf->real_height == -1){
			calculateComponentDimensions(component->topToTopOf, parent, 0);
		}
		min_y = component->topToTopOf->y + componentMarginTop;
	}else if(component->topToBottomOf != NULL){
		if(component->topToBottomOf->real_height == -1){
			calculateComponentDimensions(component->topToBottomOf, parent, 0);
		}
		if(component->topToBottomOf == parent){
			min_y = component->topToBottomOf->real_height;
		}else{
			min_y = component->topToBottomOf->y+ component->topToBottomOf->real_height;
		}
		min_y += componentMarginTop;
	}else if(component->bottomToBottomOf == NULL && component->bottomToTopOf == NULL || component->topToTopOf == parent){
		//Default: topToTopOf: Parent
		int parentPadding = parent->paddingTop != -1 ? parent->paddingTop : parent->padding;
		min_y = componentMarginTop + parentPadding;
	}
	debug("min_y = %d", min_y);

	//Bot of component
	if(component->bottomToTopOf!= NULL){
		if(component->bottomToTopOf->real_height == -1){
			calculateComponentDimensions(component->bottomToTopOf, parent, 0);
		}
		int auxHeight;
		if(component->bottomToTopOf == parent){
			auxHeight = 0;
		}else{
			auxHeight=component->bottomToTopOf->y;
		}
		auxHeight -= componentMarginTop;
		max_y = auxHeight < 0 ? 0 : auxHeight;
	}else if(component->bottomToBottomOf != NULL){
		if(component->bottomToBottomOf->real_height == -1){
			calculateComponentDimensions(component->bottomToBottomOf, parent, 0);
		}
		
		int auxHeight;
		if(component->bottomToBottomOf == parent){
			auxHeight = component->bottomToBottomOf->real_height;
		}else{
			auxHeight = component->bottomToBottomOf->y + component->bottomToBottomOf->real_height;
		}
		auxHeight -= componentMarginBot;
		if(component->bottomToBottomOf == parent){
			int paddingParent = parent->paddingBot == -1 ? parent->padding : parent->paddingBot;
			auxHeight -= paddingParent;
		}
		max_y= auxHeight < 0 ? 0 : auxHeight;
	}else if(min_y != -1){
		//max_y = component->real_height + min_y;
	}
	debug("max_y = %d", max_y);

	//Top not deffined but buttom is -> stick to bottom (Need to know the height)
	//Bottom not deffinned but top is -> Stick to top (Need to know height)
	//None of them deffined: Stick to top //Already did at top
	
	//Depending on the mode, we calculate the height and position of the object
	if(component->autoHeight == 0 || component->autoHeight == 1){
		//Top not deffined -> stick to bottom
		if(min_y == -1){ 
			//max_y -= componentMarginTop;
			min_y = max_y - component->real_height;
		}
		
		//Bot is not deffined -> stick to top
		if(max_y == -1){ 
			//min_y += componentMarginTop;
			max_y = min_y + component->real_height;
		}

		//Both are deffined, we get the middle point (or biased)
		float auxBias = component->yBias != -1 ? component->yBias*2 : 1;
		if (component != &parentComponent) component->y = min_y + auxBias * (max_y-min_y-component->real_height)/2;

	}else if(component->autoHeight == 2){
		//We set the top at the maximun, and also the max size
		component->y = min_y;
		//If some of them is not deffined, we assume it 0
		if(max_y == -1 || min_y == -1) component->real_height = 0;
		else component->real_height = max_y - min_y;

		//Adjust size according to bias
		
		component->real_height *= component->heightBias != -1 ? component->heightBias : 1; 
		
		//Both are deffined, we get the middle point (or biased)
		float auxBias = component->yBias != -1 ? component->yBias*2 : 1;
		if (component != &parentComponent) component->y = min_y + auxBias * (max_y-min_y-component->real_height)/2;
		//if (component != &parentComponent) component->y = middlePoint + component->real_height/2;


	}
	debug("Final height component %d: %d", parent, component->real_height);
	debug("Final y component %d: %d", parent, component->y);

	//Process the children that are not processed yet
	debug("Numero hijos: %d", component->childCount);
	//debug("Hijos: %d", 1);
	for(int i = 0; i<component->childCount; i++){
		debug("Calculando hijo %d al final", i);
		if(component->children[i]->real_height == -1) calculateComponentDimensions(component->children[i], component, 0);
	}
}

/*Creates a new container*/
Component * newContainer(){
	Component * cont = malloc(sizeof(Component));
	cont->x = 0;
	cont->y = 0;
	cont->height = 5;
	cont->height = 5;
	cont->autoHeight = 0;
	cont->margin = 0;
	cont->marginBot = -1;
	cont->marginTop = -1;
	cont->paddingTop = -1;
	cont->paddingBot = -1;
	cont->padding = 0;
	cont->component_type = container_t;
	cont->childCount = 0;
	cont->real_height = -1;
	cont->real_width = -1;
	cont->isUpdated = 3;
	cont->backgroundColor = BG_COLOR;
	cont->border = 1;
	cont->xBias = 1;
	cont->yBias = 1;
	cont->heightBias = 1;
	cont->widthBias = 1;

	return cont;
}

/*Creates new color based on an Hex string*/
Color * newColor(uint8_t r, uint8_t g, uint8_t b){
	Color * retPtr = malloc(sizeof(Color));
	return retPtr;
}
