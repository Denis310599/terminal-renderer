#include <asm-generic/ioctls.h>
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

/*Data structures*/
typedef struct StringTable{
	char ** table;
	int length;
} StringTable;

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

typedef struct TabView{
	StringTable tabTitles;
	int tabCount;
	int selectedTab;
	int focusTab;
	Color bgColor;
	Color bgSelectedColor;
	Color bgFocusColor;
	Color txtColor;
	Color txtSelectedColor;
	Color txtFocusColor;
	int tabPosition; //0 up, 1 down, 2 right, 3 left
	char *iconEnd;
	char *iconStart;
	int tabHeight;
	int offset;
} TabView;

typedef struct Style{
	char ** barckgroundColor;
	char ** focusBackgroundColor;
	int borderWidth;
	int borderRadius;
	char ** focusBorderColor;
	char ** borderColor;
} Style;

enum ComponentType {container_t, viewport_t, tabview_t};
typedef struct Component{
  //Basic config
	enum ComponentType component_type;
  int x;
  int y;
	int global_x;
	int global_y;
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

	//Function handlers
	void (* onKeyPress)(struct Component *, char keypress);

	//Appearance
	int border; //0 no border, 1 normal border, 2 big border
	Color backgroundColor;
	Color borderColor;



	//Custom settings
  union{
    Container container_properties;
		Viewport viewport_properties;
		TabView tabview_properties;
  };
  int childCount;
	struct Component * parent;
  struct Component ** children;
} Component;

/*Function declaration*/
void initUI();
void drawUI();
void drawContainer(Component * component, Component * parent);
void drawComponent(Component * component, Component * parent);
void drawBox(int x, int y, int height, int width, int drawBorder, Color bgColor, Color borderColor);
void drawViewport(Component * component, Component * parent);
void drawTabView(Component * component, Component * parent);
void printText(int x, int y, char * text, Color bg, Color fg);
void mark_component_as_updated(Component * component);
void handle_resize();

void calculateComponentDimensions(Component * component, Component * parent);
void calculateComponentDimensionsWidth(Component * component, Component * parent);
void calculateComponentDimensionsHeight(Component * component, Component * parent);
void preCalculateComponent(Component * component, Component * parent);

Component * newContainer();
Component * newViewport();
Component * newTabView();
Color * newColor(uint8_t r, uint8_t g, uint8_t b);

int handleInput();
void closeProgram();

void enable_raw_mode();
void set_nonblocking_mode();
void disable_raw_mode();
void prepareTerminal();
void restoreTerminal();
void get_terminal_size(int *rows, int *cols);


void addStringToTable(char * string, StringTable * table);
StringTable newStringTable(char * string);
/*File accessible variables*/
Component parentComponent;

/*Custom function declaration(aka handlers)*/
void handleObjectManajerKeyPress(Component * component, char keypress);

/*Color deffinitions*/
const Color BG_COLOR = (Color){7, 18, 36};
const Color BG_2_COLOR= (Color){29, 42, 64};
const Color BG_3_COLOR= (Color){42, 59, 89};
const Color BG_DISABLE_COLOR = (Color){67, 75, 89};
const Color FONT_COLOR= (Color){173, 173, 173};
const Color FONT_2_COLOR= (Color){130, 130, 130};
const Color FONT_3_COLOR= (Color){99, 99, 99};
const Color BG_VP_COLOR= (Color){60, 60, 60};

/*Variable deffinitions*/
Component * focusComponent;


/*Handler deffinition*/


void handleObjectManajerKeyPress(Component * component, char keypress){
	debug("Handling keypress in ObjectManager");
	switch(keypress){
		case '3':
			component->tabview_properties.selectedTab == (component->tabview_properties.tabCount-1) ? : component->tabview_properties.selectedTab++;
			break;
		case '1':
			component->tabview_properties.selectedTab == 0 ? : component->tabview_properties.selectedTab--;
			break;
	}
	component->isUpdated = 1;

	if(component->tabview_properties.selectedTab >= component->childCount) return;
	component->children[component->tabview_properties.selectedTab]->isUpdated = 1;

}

/*Here starts the Library related functions*/
int main(){
	DEBUG=1;
  initUI();
	//getchar();
  //ViewportSettings viewport;
  //vp_create_viewport(&viewport);
  //vp_init_viewport(&viewport);
  //viewport.x = 5;
  //viewport.y = 5;

  //Define the camera
	importStl("../assets/eevee2.stl", (Vector3d){0.03f, 0.03f, 0.03f}, (Vector3d){-130, 214, 0});
	importStl("../assets/teapot.stl", (Vector3d){0.3, 0.3, 0.3}, (Vector3d){0, 0, 0});
	importStl("../assets/teapot.stl", (Vector3d){0.3, 0.3, 0.3}, (Vector3d){0, 10, 0});

  printf("\033[2J");
  while(1){
    if(handleInput()) break;
		//viewport.render_settings->active_camera = myCam;

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
  parentComponent.width = cols;
  parentComponent.real_width= cols;
  parentComponent.isUpdated = 3;
  parentComponent.childCount = 6;
	parentComponent.component_type = container_t;
	parentComponent.backgroundColor = BG_COLOR;
	parentComponent.border = 0;
	parentComponent.padding = 0;

	/*Commands bar container*/
	Component * child = newContainer();
	child->bottomToBottomOf= &parentComponent;
	child->marginTop = 0;
	child->startToStartOf = &parentComponent;
	child->endToEndOf= &parentComponent;
	child->height = 2;
	//child->real_width = 3;
	child->autoWidth = 2;
	child->width = 10;
	child->backgroundColor = BG_COLOR;
	child->border = 0;

	parentComponent.children = malloc(6*sizeof(Component *));
	parentComponent.children[0] = child;

	/*Status bar container*/
	child = newContainer();
	child->bottomToTopOf= parentComponent.children[0];
	child->startToStartOf = &parentComponent;
	child->endToEndOf= &parentComponent;
	child->height = 1;
	//child->real_width = 3;
	child->autoWidth = 2;
	child->backgroundColor = BG_2_COLOR;
	child->border = 0;

	parentComponent.children[1] = child;

	/*Top Menu container*/
	child = newContainer();
	child->topToTopOf= &parentComponent;
	child->startToStartOf = &parentComponent;
	child->endToEndOf= &parentComponent;
	child->height = 3;
	//child->real_width = 3;
	child->autoWidth = 2;
	child->backgroundColor = BG_COLOR;
	child->border = 1;

	parentComponent.children[2] = child;

	/*Object container*/
	child = newContainer();
	child->topToBottomOf= parentComponent.children[2];
	child->bottomToTopOf= parentComponent.children[1];
	child->startToStartOf = &parentComponent;
	child->endToEndOf= &parentComponent;
	//child->height = 3;
	//child->real_width = 3;
	child->autoWidth = 2;
	child->autoHeight = 2;
	child->widthBias = 0.3;
	child->xBias = 1;
	child->backgroundColor = BG_COLOR;
	child->border = 1;

	parentComponent.children[3] = child;

	/*Main buttons container*/
	child = newContainer();
	child->topToBottomOf= parentComponent.children[2];
	child->startToStartOf = &parentComponent;
	child->endToStartOf= parentComponent.children[3];
	child->height = 5;
	//child->real_width = 3;
	child->autoWidth = 2;
	child->autoHeight = 0;
	child->backgroundColor = BG_2_COLOR;
	child->border = 0;

	parentComponent.children[4] = child;


	/*Viewport container*/
	child = newContainer();
	child->topToBottomOf= parentComponent.children[4];
	child->startToStartOf = &parentComponent;
	child->endToStartOf= parentComponent.children[3];
	child->bottomToTopOf = parentComponent.children[1];
	//child->height = 5;
	//child->real_width = 3;
	child->autoWidth = 2;
	child->autoHeight = 2;
	child->padding = 1;
	child->backgroundColor = BG_COLOR;
	child->border = 1;
	child->childCount = 1;
	child->children = malloc(sizeof(Component *));
	parentComponent.children[5] = child;
	

	/*viewport*/
	child = newViewport();
	child->topToTopOf= parentComponent.children[5];
	child->startToStartOf = parentComponent.children[5];
	child->endToEndOf= parentComponent.children[5];
	child->bottomToBottomOf = parentComponent.children[5];
	//child->height = 5;
	//child->real_width = 3;
	child->autoWidth = 2;
	child->autoHeight = 2;
	child->backgroundColor = BG_COLOR;
	child->isUpdated = 1;
	
	Camera myCam;
	//myCam.pos = (Vector3d){15, 0, 8};
	//myCam.pos = (Vector3d){70, 0, 50};
	myCam.pos = (Vector3d){10, 0, 3};
	myCam.dir = (Vector3d){2, 0, 3};
	myCam.fov = 45;
	myCam.dir = normalizeVector(vect_sum((Vector3d){0, 0, 1}, myCam.pos, -1));
	child->viewport_properties.vp_settings->render_settings->active_camera = myCam;

	parentComponent.children[5]->children[0] = child;


	/*Object Manager*/
	Component * objectContainer = parentComponent.children[3];
	objectContainer->padding = 1;
	objectContainer->children = malloc(sizeof(Component*));
	objectContainer->childCount = 1;
	objectContainer->children[0] = newTabView();
	objectContainer->children[0]->topToTopOf = objectContainer;
	objectContainer->children[0]->bottomToBottomOf = objectContainer;
	objectContainer->children[0]->startToStartOf = objectContainer;
	objectContainer->children[0]->endToEndOf = objectContainer;
	objectContainer->children[0]->autoHeight = 2;
	objectContainer->children[0]->autoWidth = 2;
	objectContainer->children[0]->tabview_properties.selectedTab = 0;
	objectContainer->children[0]->onKeyPress = handleObjectManajerKeyPress;

	focusComponent = objectContainer->children[0];
	


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
  parentComponent.width = cols;
  parentComponent.real_width= cols;

}

void mark_component_as_updated(Component * component){
	component->real_height = -1;
	component->real_width = -1;
	component->isUpdated = 3;
	if(component->component_type == viewport_t){
		component->isUpdated = 1;
	}

	for(int i = 0; i<component->childCount; i++){
		mark_component_as_updated(component->children[i]);
	}
}

void restoreTerminal(){
//glfwDestroyWindow(window);
  glfwTerminate();
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
		debug("Char presser: %c", ch);
    if(ch == 'q'){
      //Process closing
      closeProgram();
      return 1;
    }
		//Pass the event to the focused component
		focusComponent->onKeyPress(focusComponent, ch);
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
  switch (component->component_type){
		case container_t: drawContainer(component, parent); break;
		case viewport_t: drawViewport(component, parent); break;
		case tabview_t: drawTabView(component, parent); break;
	}
}

void drawContainer(Component * component, Component * parent){
	debug("***Drawing container %d.\n X: %d, Y: %d,\n height: %d,\n width: %d", component, component->x, component->y, component->real_height, component->real_width);
  if(component->isUpdated != 0){
		component->isUpdated--;
		//Draws the border
		if(component == &parentComponent) {
			component->global_x = component->x;
			component->global_y = component->y;
			drawBox(component->x,
						component->y,
						component->real_height,
						component->real_width,
						component->border,
						component->backgroundColor,
						(Color){});
		}else if(parent != NULL){
			component->global_x = component->x + parent->global_x;
			component->global_y = component->y + parent->global_y;
			drawBox(component->global_x,
						component->global_y,
						component->real_height,
						component->real_width,
						component->border,
						component->backgroundColor,
						(Color){});

		}
	}
  
  //Iterates over every child component
	for (int i = 0; i<component->childCount; i++){
		drawComponent(component->children[i], component);
	}
}

void drawBox(int x, int y, int height, int width, int drawBorder, Color bgColor, Color borderColor){
  int true_x = x+1;
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

/*Function that prints some text with color at a desired position*/
void printText(int x, int y, char * text, Color bg, Color fg){
  printf("\033[%d;%dH", y, x);
	printf("\e[38;2;%d;%d;%d;48;2;%d;%d;%dm%s", fg.r, fg.g, fg.b, bg.r, bg.g, bg.b, text);
  fflush(stdout);
}

void drawViewport(Component * component, Component * parent){

	if(component->isUpdated != 0){
		component->isUpdated--;
		//Get the terminal cell size
		debug("Calculating viewport size");
		struct winsize ws;
		ioctl(0, TIOCGWINSZ, &ws);
		debug("Window Size: %d cols %d rows %dx %dy", ws.ws_col, ws.ws_row, ws.ws_xpixel, ws.ws_ypixel);
		debug("Component Size: %d cols %d rows", component->real_width, component->real_height);
		component->viewport_properties.vp_settings->y = component->y + parent->global_y;
		component->viewport_properties.vp_settings->x = component->x + parent->global_x;
		component->viewport_properties.vp_settings->screen_width = component->real_width * ws.ws_xpixel / ws.ws_col;
		component->viewport_properties.vp_settings->screen_height= component->real_height * ws.ws_ypixel / ws.ws_row;
		component->viewport_properties.vp_settings->render_settings->screen_height = component->viewport_properties.vp_settings->screen_height;
		component->viewport_properties.vp_settings->render_settings->screen_width = component->viewport_properties.vp_settings->screen_width;

		if(component->viewport_properties.vp_settings->window != NULL){
			resizeWindow(component->viewport_properties.vp_settings);
		}

		debug("Viewport Size: %d x; %d y", component->viewport_properties.vp_settings->screen_width, component->viewport_properties.vp_settings->screen_height);
	}
	if(component->viewport_properties.vp_settings->window == NULL){
		debug("Initializing viewport");
		vp_init_viewport(component->viewport_properties.vp_settings);
	}

	debug("Drawing viewport");
	debug("%dx, %dy", component->viewport_properties.vp_settings->x, component->viewport_properties.vp_settings->y);
	vp_render_viewport(component->viewport_properties.vp_settings);
}

/*Function that handles the drawing of a tabview*/
void drawTabView(Component * component, Component * parent){
	debug("***Drawing tabview***");
	if(component->isUpdated != 0){
		component->isUpdated--;
		/*Draws the actual tabs*/
		
		int start_x = component->x + parent->global_x +1;
		int start_y = component->y + parent->global_y +1;

		int aux_x = start_x;
		int aux_y = start_y;
		int arrowLengthStart = strlen(component->tabview_properties.iconStart);
		int arrowLengthEnd = strlen(component->tabview_properties.iconEnd);
		int offset = component->tabview_properties.offset;
		//Draw the first right icon
		debug("Drawing first icon at %d %d", aux_x, aux_y);
		printText(aux_x, aux_y, component->tabview_properties.iconStart, component->tabview_properties.bgColor, component->tabview_properties.txtColor);
		aux_x += strlen(component->tabview_properties.iconStart);

		//Draw the actual tabs
		debug("Drawing tabs");
		for(int i = 0; i<component->tabview_properties.tabCount; i++){
			debug("tab %d", i+1);
			Color bgColor;
			Color fwColor;
			debug(".. Title: %s", component->tabview_properties.tabTitles.table[i]);
			int titleLength = strlen(component->tabview_properties.tabTitles.table[i]);
			debug(".. Title of length %d", titleLength);


			int max_titleLength = start_x + component->real_width - aux_x -arrowLengthEnd;
			titleLength = max_titleLength >= titleLength ? titleLength : max_titleLength;
			char * title = malloc(sizeof(char) * titleLength);
			strncpy(title, component->tabview_properties.tabTitles.table[i], titleLength);
			debug(".. titleLength: %d", titleLength);
			if(titleLength > 0) {
				title[titleLength] = '\0';
			}

			if(i == component->tabview_properties.selectedTab){
				 bgColor= component->tabview_properties.bgSelectedColor;
				fwColor = component->tabview_properties.txtSelectedColor;
			}else if(i == component->tabview_properties.focusTab){
				 bgColor= component->tabview_properties.bgFocusColor;
				fwColor = component->tabview_properties.txtFocusColor;
			}else{
				 bgColor= component->tabview_properties.bgColor;
				fwColor = component->tabview_properties.txtColor;
			}
			debug(".. Drawing tab %s at %d %d", component->tabview_properties.tabTitles.table[i], aux_x, aux_y);
			printText(aux_x, aux_y, title, bgColor, fwColor);
			aux_x += titleLength;
			debug("Aux_x: %d", aux_x);
			
			//Update the global coordinates
			component->global_x = component->x + parent->global_x;
			component->global_y = component->y + parent->global_y;

			if(aux_x == (start_x + component->real_width - arrowLengthEnd)) break;
		}

		int leftTabToDraw = start_x + component->real_width - arrowLengthEnd - aux_x;
		for(int i = 0; i< leftTabToDraw; i++){
			printText(aux_x, start_y, " ", component->tabview_properties.bgColor, component->tabview_properties.txtColor);
			aux_x++;
		}

		//Draw the final arrow
		aux_x = start_x+component->real_width - arrowLengthEnd;
		printText(aux_x, aux_y, component->tabview_properties.iconEnd, component->tabview_properties.bgColor, component->tabview_properties.txtColor);
	}
	//Draws the component of the selected tab
	if(component->tabview_properties.selectedTab >= component->childCount) return;
	drawComponent(component->children[component->tabview_properties.selectedTab], component);
}

void drawUI(){
	calculateComponentDimensions(&parentComponent, &parentComponent);
  drawComponent(&parentComponent, NULL);
}

/*Function that pre-process a component, updating properties for further calculation*/
void preCalculateComponent(Component * component, Component * parent){
	if(component->real_height != -1 && component->real_width != -1) return;
	component->parent = parent;

	if(component->component_type ==tabview_t){
		component->paddingTop = component->tabview_properties.tabHeight;
	}
}

/*Function that calculates component dimensions and populates its properties*/
void calculateComponentDimensions(Component * component, Component * parent){
	debug("Calculando componente con id %d", component);
	preCalculateComponent(component, parent);

	calculateComponentDimensionsWidth(component, parent);
	calculateComponentDimensionsHeight(component, parent);
	
	//Process the children that are not processed yet
	debug("Numero hijos: %d", component->childCount);
	//debug("Hijos: %d", 1);
	for(int i = 0; i<component->childCount; i++){
		debug("Calculando hijo %d", i);
		if(component->children[i]->real_height == -1 || component->children[i]->real_width == -1) calculateComponentDimensions(component->children[i], component);
	}
}

/*Function that calculates and populates the dimensions and positions of a component y coordinate*/
void calculateComponentDimensionsHeight(Component * component, Component * parent){

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
				if(child->real_height == -1)	calculateComponentDimensionsHeight(child, component);
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
			calculateComponentDimensionsHeight(component->topToTopOf, parent);
		}
		min_y = component->topToTopOf->y + componentMarginTop;
	}else if(component->topToBottomOf != NULL){
		if(component->topToBottomOf->real_height == -1){
			calculateComponentDimensionsHeight(component->topToBottomOf, parent);
		}
		if(component->topToBottomOf == parent){
			min_y = component->topToBottomOf->real_height;
		}else{
			min_y = component->topToBottomOf->y+ component->topToBottomOf->real_height;
		}
		min_y += componentMarginTop;
	}else if((component->bottomToBottomOf == NULL && component->bottomToTopOf == NULL) || component->topToTopOf == parent){
		//Default: topToTopOf: Parent
		int parentPadding = parent->paddingTop != -1 ? parent->paddingTop : parent->padding;
		min_y = componentMarginTop + parentPadding;
	}
	debug("min_y = %d", min_y);

	//Bot of component
	if(component->bottomToTopOf!= NULL){
		if(component->bottomToTopOf->real_height == -1){
			calculateComponentDimensionsHeight(component->bottomToTopOf, parent);
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
			calculateComponentDimensionsHeight(component->bottomToBottomOf, parent);
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

	
}

/*Function that calculates and populates the dimensions and positions of a component x coordinate*/
void calculateComponentDimensionsWidth(Component * component, Component * parent){

	debug("Calculando componente con id %d", parent);
	//Skips this one if it's position depends on his parent height and it's not yet been calculated
	if(parent->real_width== 0 && (component->startToEndOf== parent || component->endToEndOf== parent)) return;
	//Calculates the dimensions
	int componentMarginStart= component->marginStart== -1 ? component->margin : component->marginStart;
	int componentMarginEnd= component->marginEnd== -1 ? component->margin : component->marginEnd;
	int componentPaddingStart= parent->paddingStart== -1 ? parent->padding: parent->paddingStart;
	int componentPaddingEnd= parent->paddingEnd== -1 ? parent->padding: parent->paddingEnd;
	int min_x = -1;
	int max_x = -1;

	if(component == &parentComponent){
		debug("Component is parent");
		component->real_width= component->width;
	}else if(component->autoWidth== 1){
		debug("Height dependant on childs");
			//Height dependant on children height
			component->real_width= 0;
			int auxHeight= 0;
			int maxHeight= 0;
			for(int i = component->childCount - 1; i>=0; i--){
				Component * child = component->children[i];
				if(child->real_height == -1)	calculateComponentDimensionsWidth(child, component);
				int auxMargin= 0;
				auxMargin += child->marginBot == -1 ? child->margin : child->marginBot;	
				auxHeight = child->y + child->real_height + auxMargin;
				maxHeight = maxHeight<auxHeight ? auxHeight : maxHeight;
				maxHeight += componentPaddingEnd;
			}
			component->real_width= maxHeight;
		debug("  Height: %d", maxHeight);
	}else{
		component->real_width= component->width;
	}

	//Calculate the max and min coords of location
	//Top of component
	if(component->startToStartOf!= NULL && component->startToStartOf!= parent){
		if(component->startToStartOf->real_width== -1){
			calculateComponentDimensionsWidth(component->startToStartOf, parent);
		}
		min_x= component->startToStartOf->y + componentMarginStart;
	}else if(component->startToEndOf!= NULL){
		if(component->startToEndOf->real_width== -1){
			calculateComponentDimensionsWidth(component->startToEndOf, parent);
		}
		if(component->startToEndOf== parent){
			min_x= component->startToEndOf->real_height;
		}else{
			min_x = component->startToEndOf->x+ component->startToEndOf->real_height+1;
		}
		min_x += componentMarginStart;
	}else if(component->endToEndOf== NULL && component->endToEndOf== NULL || component->startToStartOf== parent){
		//Default: topToTopOf: Parent
		int parentPadding = parent->paddingStart!= -1 ? parent->paddingStart: parent->padding;
		min_x = componentMarginStart + parentPadding;
	}
	debug("min_x = %d", min_x);

	//Bot of component
	if(component->endToStartOf!= NULL){
		if(component->endToStartOf->real_width== -1){
			calculateComponentDimensionsWidth(component->endToStartOf, parent);
		}
		int auxHeight;
		if(component->endToStartOf== parent){
			auxHeight = 0;
		}else{
			auxHeight=component->endToStartOf->x-1;
		}
		auxHeight -= componentMarginStart;
		max_x = auxHeight < 0 ? 0 : auxHeight;
	}else if(component->endToEndOf!= NULL){
		if(component->endToEndOf->real_width== -1){
			calculateComponentDimensionsWidth(component->endToEndOf, parent);
		}
		
		int auxHeight;
		if(component->endToEndOf== parent){
			auxHeight = component->endToEndOf->real_width;
		}else{
			auxHeight = component->endToEndOf->x + component->endToEndOf->real_width;
		}
		auxHeight -= componentMarginEnd;
		if(component->endToEndOf== parent){
			int paddingParent = parent->paddingEnd == -1 ? parent->padding : parent->paddingEnd;
			auxHeight -= paddingParent;
		}
		max_x= auxHeight < 0 ? 0 : auxHeight;
	}else if(min_x!= -1){
		//max_y = component->real_height + min_y;
	}
	debug("max_x = %d", max_x);

	//Top not deffined but buttom is -> stick to bottom (Need to know the height)
	//Bottom not deffinned but top is -> Stick to top (Need to know height)
	//None of them deffined: Stick to top //Already did at top
	
	//Depending on the mode, we calculate the height and position of the object
	if(component->autoWidth== 0 || component->autoWidth== 1){
		//Top not deffined -> stick to bottom
		if(min_x == -1){ 
			//max_y -= componentMarginTop;
			min_x = max_x - component->real_width;
		}
		
		//Bot is not deffined -> stick to top
		if(max_x == -1){ 
			//min_y += componentMarginTop;
			max_x = min_x + component->real_width;
		}

		//Both are deffined, we get the middle point (or biased)
		float auxBias = component->xBias != -1 ? component->xBias*2 : 1;
		if (component != &parentComponent) component->x = min_x + auxBias * (max_x-min_x-component->real_width)/2.0f;

	}else if(component->autoWidth== 2){
		//We set the top at the maximun, and also the max size
		component->x = min_x;
		//If some of them is not deffined, we assume it 0
		if(max_x == -1 || min_x == -1) component->real_width= 0;
		else component->real_width= max_x - min_x;

		//Adjust size according to bias
		
		component->real_width*= component->widthBias!= -1 ? component->widthBias: 1; 
		
		//Both are deffined, we get the middle point (or biased)
		float auxBias = component->xBias != -1 ? component->xBias*2 : 1;
		if (component != &parentComponent) component->x = min_x + auxBias * (max_x-min_x-component->real_width)/2.0f;
		//if (component != &parentComponent) component->y = middlePoint + component->real_height/2;


	}
	debug("Final width component %d: %d", parent, component->real_width);
	debug("Final x component %d: %d", parent, component->x);
}



/*Creates a new container*/
Component * newContainer(){
	Component * cont = malloc(sizeof(Component));
	cont->x = 0;
	cont->y = 0;
	cont->height = 5;
	cont->width= 5;
	cont->autoHeight = 0;
	cont->autoWidth = 0;
	cont->margin = 0;
	cont->marginBot = -1;
	cont->marginStart = -1;
	cont->marginEnd = -1;
	cont->marginTop = -1;
	cont->paddingTop = -1;
	cont->paddingBot = -1;
	cont->paddingStart = -1;
	cont->paddingEnd = -1;
	cont->padding = 0;
	cont->component_type = container_t;
	cont->childCount = 0;
	cont->real_height = -1;
	cont->real_width = -1;
	cont->isUpdated = 3;
	cont->backgroundColor = BG_COLOR;
	cont->border = 1;
	cont->xBias = -1;
	cont->yBias = -1;
	cont->heightBias = -1;
	cont->widthBias = -1;

	return cont;
}


Component * newViewport(){
	debug("Creating viewport");
	Component * cont = newContainer();
	cont->component_type = viewport_t;
	cont->viewport_properties.vp_settings = malloc(sizeof(ViewportSettings));
	
	debug("Create Viewport");	
	vp_create_viewport(cont->viewport_properties.vp_settings);

	//cont->viewport_properties.vp_settings->window = NULL;
  //debug("Initializing viewport");
	//vp_init_viewport(cont->viewport_properties.vp_settings);

	return cont;

}

Component * newTabView(){
	debug("\n\n***Creating tabView***");
	Component * cont = newContainer();
	cont->component_type = tabview_t;

	//cont->tabview_properties.iconStart = malloc(sizeof(char)*2);
	cont->tabview_properties.iconStart = " < ";
	//cont->tabview_properties.iconEnd = malloc(sizeof(char)*2);
	cont->tabview_properties.iconEnd = " > ";

	cont->tabview_properties.tabPosition = 0;
	cont->tabview_properties.tabHeight = 1;
	cont->tabview_properties.bgColor = BG_2_COLOR;
	cont->tabview_properties.bgFocusColor = BG_COLOR;
	cont->tabview_properties.bgSelectedColor = BG_3_COLOR;
	cont->tabview_properties.txtColor = FONT_2_COLOR;
	cont->tabview_properties.txtFocusColor = FONT_2_COLOR;
	cont->tabview_properties.txtSelectedColor = FONT_COLOR;

	Component * contSon1 = newContainer();
	contSon1->backgroundColor = (Color){60, 60, 60};
	contSon1->topToTopOf = cont;
	contSon1->bottomToBottomOf= cont;
	contSon1->startToStartOf= cont;
	contSon1->endToEndOf= cont;
	contSon1->autoHeight = 2;
	contSon1->autoWidth = 2;
	contSon1->border = 1;

	Component * contSon2 = newContainer();
	contSon2->backgroundColor = (Color){120, 20, 20};
	contSon2->topToTopOf = cont;
	contSon2->bottomToBottomOf= cont;
	contSon2->startToStartOf= cont;
	contSon2->endToEndOf= cont;
	contSon2->autoHeight = 2;
	contSon2->autoWidth = 2;
	contSon2->border = 1;

	cont->tabview_properties.tabCount = 6;
	cont->children = malloc(sizeof(Component *)*2);
	cont->children[0] = contSon1;
	cont->children[1] = contSon2;
	cont->childCount = 2;
	
	cont->tabview_properties.tabTitles = newStringTable(" Hello ");
	StringTable * tablaAux = &(cont->tabview_properties.tabTitles);
	addStringToTable(" GoodBye ", tablaAux);
	addStringToTable(" GoodBye ", tablaAux);
	addStringToTable(" GoodBye ", tablaAux);
	addStringToTable(" GoodBye ", tablaAux);
	addStringToTable(" GoodBye ", tablaAux);





	cont->isUpdated = 3;
	cont->tabview_properties.focusTab = -1;
	return cont;

}


/*Creates new color based on an Hex string*/
Color * newColor(uint8_t r, uint8_t g, uint8_t b){
	Color * retPtr = malloc(sizeof(Color));
	return retPtr;
}

/*Function that creates a new table of strings*/
StringTable newStringTable(char * string){
	StringTable retTable;
	retTable.table = malloc(sizeof(char *));
	retTable.table[0] = malloc(sizeof(char)*strlen(string) +1);
	retTable.length = 1;
	strcpy(retTable.table[0], string);
	debug("Nueva tabla con elemento: %s", retTable.table[0]);
	return retTable;
}

/*Function that adds a new string to a table of strings*/
void addStringToTable(char * string, StringTable * table){
	char ** auxTable = malloc(sizeof(char *) * (table->length+1));
	memcpy(auxTable, table->table, table->length * sizeof(char*));
	free(table->table);
	table->table = auxTable;
	table->table[table->length] = malloc(sizeof(char)*strlen(string) +1);
	strcpy(table->table[table->length], string);
	debug("Elemento añadido a tabla en %d: %s", table->length, table->table[table->length]);
	table->length++;
}
