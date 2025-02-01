#include <fcntl.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include "../include/renderer.h"
#include "../include/viewport.h"


/*Structures related with components*/

/*Definition of the components*/
typedef struct Container{
  int isFocus;
} Container;

typedef struct Viewport{
  ViewportSettings * vp_settings;
} Viewport;

enum ComponentType {container_t, viewport_t};
typedef struct Component{
  enum ComponentType component_type;
  int x;
  int y;
  int height;
  int width;
  int isUpdated;
  union{
    Container properties;
  };
  int childCount;
  struct Component ** children;
} Component;

/*Function declaration*/
void initUI();
void drawUI();
void drawContainer(Component * component);
void drawBox(int x, int y, int height, int width);
void drawViewport();

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

int main(){
  initUI();
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
    vp_render_viewport(&viewport);
    
  }
}

void initUI(){
  //Prepares the terminal
  prepareTerminal();

  //Calculates component table
  parentComponent.x = 0;
  parentComponent.y = 1;

  int rows, cols;
  get_terminal_size(&rows, &cols);
  parentComponent.height = rows;
  parentComponent.width = cols-1;
  parentComponent.isUpdated = 1;
  parentComponent.childCount = 0;
}

void prepareTerminal(){
  //Swaps buffer
  printf("\033[?1049h");
  fflush(stdout);

  //Enable raw input mode
  enable_raw_mode();

  //Set not blocking input
  set_nonblocking_mode();


}

void restoreTerminal(){
  //signal(SIGWINCH, handle_resize);
  disable_raw_mode();
  
  //Restores buffer
  printf("\033[?1049l");
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

void drawComponent(Component * component){
  if(component->component_type == container_t) drawContainer(component);
}

void drawContainer(Component * component){
  if(component->isUpdated== 0) return;
  component->isUpdated = 0;
  //Draws the border
  drawBox(component->x,
          component->y,
          component->height,
          component->width);
  
  //Iterates over every child component
}

void drawBox(int x, int y, int height, int width){
  int true_x = x;
  int true_y = y;
  int true_end_x = x+width;
  int true_end_y = y+height;

  int max_x = parentComponent.x + parentComponent.width;
  int max_y = parentComponent.y + parentComponent.height;


  if(x<0) true_x = 0;
  if(true_end_x > max_x) true_end_x = max_x;
  if(y<0) true_y = 0;
  if(true_end_y > max_y) true_end_y = max_y;
  
  //Position cursor to start
  printf("\033[%d;%dH", true_y, true_x);
  fflush(stdout);
  for (int y_iter = true_y; y_iter <=true_end_y; y_iter++){
    for(int  x_iter = true_x; x_iter <= true_end_x; x_iter++){
      //if(y_iter == true_y) printf("%s", ".");
      if(x_iter == true_x && y_iter == true_y+1){ 
        printf("%s", "╭");
      }else if(x_iter == true_x && y_iter == true_end_y){ 
        printf("%s", "╰");
      }else if(x_iter == true_end_x && y_iter == true_y+1){
        printf("%s", "╮");
      }else if(x_iter == true_end_x && y_iter == true_end_y){
        printf("%s", "╯");
      }else if(y_iter == true_y+1 || y_iter == true_end_y){ 
        printf("%s", "─");
      }else if(x_iter == true_x || x_iter == true_end_x){
        printf("%s", "│");
      }else{
        printf(" ");  
      }
    }
    printf("\033[%d;%dH", y_iter, true_x);
    fflush(stdout);
  }
}
void drawUI(){
  drawComponent(&parentComponent);
}
