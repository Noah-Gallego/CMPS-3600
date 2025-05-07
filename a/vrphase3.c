// Noah Gallego

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <pthread.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/extensions/Xdbe.h>
#include <X11/Xatom.h> /* for intercepting X click to close */
#include <semaphore.h> // POSIX Semaphore
#include <sys/signal.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define NCARS 4

// Function Prototypes
void init();
void init_xwindows(int w, int h);
void cleanup_xwindows(void);
void check_resize(XEvent *e);
void check_mouse(XEvent *e);
void make_child_window(void);
void handle_signal(int sig);
void *child_thread(void *arg);
void moveWindow(int x, int y);
int check_keys(XEvent *e);
void physics(void);
void render(void);
int fib(int n);
void *traffic(void *arg);
void set_window_title(void);
void fillRectangle(int x, int y, int w, int h);
void drawRectangle(int x, int y, int w, int h);
void drawLine(int x0, int y0, int x1, int y1);
void drawString(int x, int y, char *str);
void clear_screen(void);
void draw_string(char s[], int y, long color);

// Global Variables
pthread_mutex_t cop;
sem_t car_semaphores[NCARS];
char **arguments, **environment;
int child, num_children = 0;
int ret;
key_t ipckey;
int mqid;
pid_t pid;

struct Global {
	Display *dpy;
	Window win;
	GC gc;
	XdbeBackBuffer backBuffer;
	XdbeSwapInfo swapInfo;
	Atom wm_delete_window;
	int xres, yres;
	int collision_flag;
	int collision[NCARS];
	int crash[2];
	int show_collisions;
	int ncollisions;
	int slow_mode;
	int passes[NCARS];
	int pause;
	int parent_pos[2];
    int parent_dim[2];
    int child_pos[2];
	int show_text;
} g;

struct Box {
	double pos[2];
	double vel[2];
	int w, h;
} intersection, cars[NCARS];

struct {
	long type;
	char text[100];
	int parent_pos[2];
    int parent_dim[2];
    int child_pos[2];
	int passes[NCARS];
	int ncollisions;
} mymsg;

int main(int argc, char *argv[], char *envp[]) {
	arguments = argv;
	environment = envp;
	if (argc > 1 && strcmp(argv[1], "-- CHILD! --") == 0) {
		child = 1;
	} 
	
	if (argc > 2) {
		mqid = atoi(argv[2]);
	}

	if (!child) {
		// Initialize Windows
		init_xwindows(460, 460);
		init();

		// Semaphore/Mutex Initialization (Traffic Control)
		pthread_t tid[NCARS];
		void *traffic(void *arg);

		pthread_mutex_init(&cop, NULL); // Unlocked

		for (int i = 0; i < NCARS; i++) {
			sem_init(&car_semaphores[i], 0, 1);
		}

		for (int i = 0; i<NCARS; i++) pthread_create(&tid[i], NULL, traffic, (void *)(long)i );

		// Create Message Queue For Text Transfer
		char pathname[128];
		getcwd(pathname, 128);
		strcat(pathname, "\foo");
		printf("%s\n", pathname);
		ipckey = ftok(pathname, 18);
		mqid = msgget(ipckey, IPC_CREAT | 0666);
	} else { // Child
		// Intialize Child Window (Smaller Window)
		init_xwindows(160, 300);

		pthread_t thread_id;
		pthread_create(&thread_id, NULL, child_thread, (void*) (long) 15);
	}

	int done = 0;
	while (!done) {
		//Handle all events in queue...
		while(XPending(g.dpy)) {
			XEvent e;
			XNextEvent(g.dpy, &e);
			check_resize(&e);
			check_mouse(&e);
			done = check_keys(&e);
		}
		//Process physics and rendering every frame
		if (!g.pause && !child) physics();
		render();

		// Message Queue Behavior for Collision/Passes (Sent To Child Window)
		if (!child && num_children > 0) {
			mymsg.type = 3;
			for (int i = 0; i < NCARS; i++) {
				mymsg.passes[i] = g.passes[i];
			}
			mymsg.ncollisions = g.ncollisions;
			msgsnd(mqid, &mymsg, sizeof(mymsg), 0);
		}

		XdbeSwapBuffers(g.dpy, &g.swapInfo, 1);
		usleep(4000);
	}
	cleanup_xwindows();
	if (!child) {
		ret = msgctl(mqid, IPC_RMID, NULL);
	}
	return 0;
}

int fib(int n) {
	return (n == 1 || n == 2) ? 1 : fib(n-1) + fib(n - 2);
}

int overlap(struct Box *c, struct Box *i) {
	if (c->pos[0] + (c->w >> 1) < i->pos[0] - (i->w >> 1)) return 0;
	if (c->pos[0] - (c->w >> 1) > i->pos[0] + (i->w >> 1)) return 0;
	if (c->pos[1] + (c->h >> 1) < i->pos[1] - (i->h >> 1)) return 0;
	if (c->pos[1] - (c->h >> 1) > i->pos[1] + (i->h >> 1)) return 0;
	return 1;
}

void *child_thread(void *arg) {
	while (1) {
		// printf("message received\n"); <-- Debugging Statement
		int received = msgrcv(mqid, &mymsg, sizeof(mymsg),0,0);
		if (received > 0) {
			switch (mymsg.type) {
				case 2:
					g.parent_pos[0] = mymsg.parent_pos[0];
					g.parent_pos[1] = mymsg.parent_pos[1];
					g.parent_dim[0] = mymsg.parent_dim[0];
					g.parent_dim[1] = mymsg.parent_dim[1];
					int x = g.parent_pos[0] + g.parent_dim[0] + 5;
					int y = g.parent_pos[1];
					moveWindow(x, y);
					break;
				case 3:
					for (int i = 0; i < NCARS; i++) {
						g.passes[i] = mymsg.passes[i];
					}
					g.ncollisions = mymsg.ncollisions;
					break;
				case 4:
					g.show_text = 1;
					break;
			}
		}
		usleep(10000);
	}
	return (void*) 0;
}

void *traffic(void *arg) {
	int carnum = (int)(long)arg;
	int current_car[NCARS] = {carnum, (carnum + 1) % NCARS};

	while (1) {
		fib(rand() % 5 + 2);
		//move the car...
		cars[carnum].pos[0] += cars[carnum].vel[0]; 
		cars[carnum].pos[1] += cars[carnum].vel[1];

		//Is car in the intersection???
		if (overlap(&cars[carnum], &intersection)) {
			/* CRITICAL SECTION START */
			pthread_mutex_lock(&cop);
			sem_wait(&car_semaphores[current_car[carnum]]); 

			//Car is in the intersection.
			while (overlap(&cars[carnum], &intersection)) {
				//Loop here until out of the intersection.
				fib(rand() % 5 + 2);
				if (g.slow_mode)
					fib(15);
				//move the car...
				if (!g.pause) {
					cars[carnum].pos[0] += cars[carnum].vel[0]; 
					cars[carnum].pos[1] += cars[carnum].vel[1];
				}
			}

			sem_post(&car_semaphores[current_car[carnum]]); 
			pthread_mutex_unlock(&cop);
			g.passes[carnum]++;
			/* CRITICAL SECTION END */
		}

		//left
		if (cars[carnum].pos[0] < -20 && cars[carnum].vel[0] < 0.0) {
			cars[carnum].pos[0] += g.xres + 40.0;
			cars[carnum].vel[0] = -(rand() % 3 + 1);
			cars[carnum].vel[0] *= 0.0002;
		}
		//top
		if (cars[carnum].pos[1] < -20 && cars[carnum].vel[1] < 0.0) {
			cars[carnum].pos[1] += g.yres + 40.0;
			cars[carnum].vel[1] = -(rand() % 3 + 1);
			cars[carnum].vel[1] *= 0.0002;
		}
		//right
		if (cars[carnum].pos[0] > g.xres + 20 && cars[carnum].vel[0] > 0.0) {
			cars[carnum].pos[0] -= (g.xres + 40.0);
			cars[carnum].vel[0] = (rand() % 3 + 1);
			cars[carnum].vel[0] *= 0.0002;
		}
		//bottom
		if (cars[carnum].pos[1] > g.yres + 20 && cars[carnum].vel[1] > 0.0) {
			cars[carnum].pos[1] -= (g.yres + 40.0);
			cars[carnum].vel[1] = (rand() % 3 + 1);
			cars[carnum].vel[1] *= 0.0002;
		}
	}
	return (void *)0;
}

void cleanup_xwindows(void) {
	//Deallocate back buffer
	if(!XdbeDeallocateBackBufferName(g.dpy, g.backBuffer)) {
		fprintf(stderr,"Error : unable to deallocate back buffer.\n");
	}
	XFreeGC(g.dpy, g.gc);
	XDestroyWindow(g.dpy, g.win);
	XCloseDisplay(g.dpy);
}

void set_window_title() {
	char ts[256];
	if (!child) {
		sprintf(ts, "3600 Intersection %ix%i", g.xres, g.yres);
	} else {
		sprintf(ts,"3600 Stats Window");
	}
	sprintf(ts, "3600 Intersection %ix%i", g.xres, g.yres);
	XStoreName(g.dpy, g.win, ts);
}

void init_xwindows(int w, int h) {
	g.xres = w;
	g.yres = h;
	XSetWindowAttributes attributes;

	// Int screen;
	int major, minor;
	XdbeBackBufferAttributes *backAttr;

	// XGCValues gcv;
	g.dpy = XOpenDisplay(NULL);

    // List of events we want to handle
	attributes.event_mask = ExposureMask | StructureNotifyMask |
							PointerMotionMask | ButtonPressMask |
							ButtonReleaseMask | KeyPressMask | KeyReleaseMask;

	// Various window attributes
	attributes.backing_store = Always;
	attributes.save_under = True;
	attributes.override_redirect = False;
	attributes.background_pixel = 0x00000000;

	// Get default root window
	Window root;
	root = DefaultRootWindow(g.dpy);

	// Create a window
	g.win = XCreateWindow(g.dpy, root, 0, 0, g.xres, g.yres, 0,
					    CopyFromParent, InputOutput, CopyFromParent,
					    CWBackingStore | CWOverrideRedirect | CWEventMask |
						CWSaveUnder | CWBackPixel, &attributes);

	// Create gc
	g.gc = XCreateGC(g.dpy, g.win, 0, NULL);

	// Get DBE version
	if (!XdbeQueryExtension(g.dpy, &major, &minor)) {
		fprintf(stderr, "Error : unable to fetch Xdbe Version.\n");
		XFreeGC(g.dpy, g.gc);
		XDestroyWindow(g.dpy, g.win);
		XCloseDisplay(g.dpy);
		exit(1);
	}
	printf("Xdbe version %d.%d\n", major, minor);

	//G et back buffer and attributes (used for swapping)
	g.backBuffer = XdbeAllocateBackBufferName(g.dpy, g.win, XdbeUndefined);
	backAttr = XdbeGetBackBufferAttributes(g.dpy, g.backBuffer);
    g.swapInfo.swap_window = backAttr->window;
    g.swapInfo.swap_action = XdbeUndefined;
	XFree(backAttr);
	
	// Map and raise window
	set_window_title();
	XMapWindow(g.dpy, g.win);
	XRaiseWindow(g.dpy, g.win);

	// To intercept user clicking x in title bar.
    g.wm_delete_window = XInternAtom(g.dpy, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(g.dpy, g.win, &g.wm_delete_window, 1);
}

void fillRectangle(int x, int y, int w, int h) {
	XFillRectangle(g.dpy, g.backBuffer, g.gc, x, y, w, h);
}

void drawRectangle(int x, int y, int w, int h) {
	XDrawRectangle(g.dpy, g.backBuffer, g.gc, x, y, w, h);
}

void drawLine(int x0, int y0, int x1, int y1) {
	XDrawLine(g.dpy, g.backBuffer, g.gc, x0, y0, x1, y1);
}

void drawString(int x, int y, char *str) {
	XDrawString(g.dpy, g.backBuffer, g.gc, x, y, str, strlen(str));
}

void init(void) {
	//Initialize cars direction, speed, etc.
	srand((unsigned)time(NULL));
	g.collision_flag = 0;
	g.show_collisions = 0;
	g.ncollisions = 0;
	g.slow_mode = 0;
	//the intersection
	intersection.w = 112;
	intersection.h = 112;
	intersection.pos[0] = g.xres / 2;
	intersection.pos[1] = g.yres / 2;
	intersection.vel[0] = 0;
	intersection.vel[1] = 0;

	for (int i = 0; i<NCARS; i++) {
		cars[i].w = 18;
		cars[i].h = 18;
		cars[i].pos[0] = intersection.pos[0];
		cars[i].pos[1] = intersection.pos[1];
		cars[i].vel[0] = 0;
		cars[i].vel[1] = 0;
	}
	int offset = 21;
	offset = 15;
	//Car heading West
	int i = 0;
	cars[i].w += rand() % 4 + 14;
	cars[i].pos[0] = g.xres + 30;
	cars[i].pos[1] -= offset;
	cars[i].vel[0] = -(rand() % 3 + 1);
	cars[i].vel[1] = 0;
	//Car heading East
	i = 1;
	cars[i].w += rand() % 4 + 14;
	cars[i].pos[0] = -40;
	cars[i].pos[1] += offset;
	cars[i].vel[0] = rand() % 3 + 1;
	cars[i].vel[1] = 0;
	//Car heading South
	i = 2;
	cars[i].h += rand() % 4 + 14;
	cars[i].pos[0] -= offset;
	cars[i].pos[1] = -30;
	cars[i].vel[0] = 0;
	cars[i].vel[1] = rand() % 3 + 1;
	//Car heading North
	i = 3;
	cars[i].h += rand() % 4 + 14;
	cars[i].pos[0] += offset;
	cars[i].pos[1] = g.yres + 30;
	cars[i].vel[0] = 0;
	cars[i].vel[1] = -(rand() % 3 + 1);

	//Scale the velocity...
	for (int i = 0; i<NCARS; i++) {
		cars[i].vel[0] *= 0.0002;
		cars[i].vel[1] *= 0.0002;
	}
}

void check_resize(XEvent *e) {
    if (e->type != ConfigureNotify)
        return;
    XConfigureEvent xce = e->xconfigure;
    g.xres = xce.width;
    g.yres = xce.height;

    if (child) {
        //store the child's position
        g.child_pos[0] = xce.x;
        g.child_pos[1] = xce.y;
    }
    if (!child) {
        Window root = DefaultRootWindow(g.dpy);
        Window chld;
        int x, y;
        XTranslateCoordinates(g.dpy, g.win, root, 0, 0, &x, &y, &chld);
        g.parent_pos[0] = x;
        g.parent_pos[1] = y;
        g.parent_dim[0] = xce.width;
        g.parent_dim[1] = xce.height;
        //parent sends its new position to the child.
        if (num_children > 0) {
            mymsg.type = 2;
            mymsg.parent_pos[0] = g.parent_pos[0];
            mymsg.parent_pos[1] = g.parent_pos[1];
            mymsg.parent_dim[0] = g.parent_dim[0];
            mymsg.parent_dim[1] = g.parent_dim[1];
            msgsnd(mqid, &mymsg, sizeof(mymsg), 0);
        }
    }
    set_window_title();
    usleep(1000);
}

void moveWindow(int x, int y) {
    //This will move the window to position x,y
    XMoveWindow(g.dpy, g.win, x, y);
}

void clear_screen(void) {
	//XClearWindow(dpy, win);
	XSetForeground(g.dpy, g.gc, 0x00050505);
	XFillRectangle(g.dpy, g.backBuffer, g.gc, 0, 0, g.xres, g.yres);
}

void check_mouse(XEvent *e) {
	static int savex = 0;
	static int savey = 0;
	static int counter = 0;

    if (e->type != ButtonPress && e->type != ButtonRelease &&
										e->type != MotionNotify) {
        return;
	}
	if (e->type == ButtonRelease)
		return;
	if (e->type == ButtonPress) {
		if (e->xbutton.button==1) { }
		if (e->xbutton.button==3) { }
	}
	if (e->type == MotionNotify) {
		if (savex != e->xbutton.x || savey != e->xbutton.y) {
			//mouse moved
			savex = e->xbutton.x;
			savey = e->xbutton.y;
			if (++counter > 20) {
				printf("p");
				counter = 0;
			}
		}
	}
}

void make_child_window() {
	pid = fork();
	if (pid == 0) { // Child Window
		signal(SIGUSR2, handle_signal);
		
		char str[32];
		sprintf(str, "%i", mqid);
		char *arg[] = {arguments[0], "-- CHILD! --", str, NULL};
		execve(arguments[0], arg, environment);

		num_children = 0;
		g.show_text = 1;
	} else { // Parent
		++num_children;
		mymsg.type = 2;
		mymsg.parent_pos[0] = g.parent_pos[0];
		mymsg.parent_pos[1] = g.parent_pos[1];
		mymsg.parent_dim[0] = g.parent_dim[0];
		mymsg.parent_dim[1] = g.parent_dim[1];
		msgsnd(mqid, &mymsg, sizeof(mymsg), 0);

		mymsg.type = 4;
		msgsnd(mqid, &mymsg, sizeof(mymsg), 0);
		g.show_text = 0;
	}
}

void handle_signal(int sig) {
	if (sig == SIGUSR2) {
		exit(0);
	}
}

int check_keys(XEvent *e) {
	if (e->type == ClientMessage) {
	    if ((Atom)e->xclient.data.l[0] == g.wm_delete_window)
    	    return 1;
	}

	if (e->type != KeyPress && e->type != KeyRelease) return 0;
	int key = XLookupKeysym(&e->xkey, 0);
	if (e->type == KeyPress) {
		switch (key) {
			case XK_c:
				g.show_collisions ^= 1;
				break;
			case XK_s:
				g.slow_mode ^= 1;
				break;
			case XK_p:
				g.pause = !g.pause;
				break;
			case XK_w:
				if (num_children == 0 && !child) {
					make_child_window();
				} else if (pid > 0) {
					kill(pid, SIGUSR2);
					num_children = 0;
				}
				break;
			case XK_Escape:
				return 1;
		}
	}
	return 0;
}

void physics() {
	// Check For Car Collisions
	g.collision_flag = 0;

	for (int i = 0; i<NCARS; i++) {
		for (int j = 0; j<NCARS; j++) {
			if (i == j)
				continue;
			if (overlap(&cars[i], &cars[j])) {
				g.collision_flag = 1;
				g.collision[0] = cars[i].pos[0];
				g.collision[1] = cars[i].pos[1];
				g.collision[2] = cars[j].pos[0];
				g.collision[3] = cars[j].pos[1];
				g.crash[0] = i;
				g.crash[1] = j;
				++g.ncollisions;
			}
		}
	}
}

void draw_string(char s[], int y, long color) {
	int x = 20;
	XSetForeground(g.dpy, g.gc, color);
	drawString(x, y, s);
}

void render(void) {
	clear_screen();

	int y = 20;

	XSetForeground(g.dpy, g.gc, 0x00ff0000);
	//draw intersection
	//XSetForeground(g.dpy, g.gc, 0x00ffff55);
	XSetForeground(g.dpy, g.gc, 0x00aaaa55);
	drawRectangle(intersection.pos[0] - (intersection.w >> 1),
					intersection.pos[1] - (intersection.h >> 1),
					intersection.w, intersection.h);
	//roadway color
	XSetForeground(g.dpy, g.gc, 0x00333333);
	//roadway north
	fillRectangle(	intersection.pos[0] - (intersection.w >> 1),
					0,
					intersection.w,
					(g.yres >> 1) - (intersection.h >> 1) - 1);
	//roadway south
	fillRectangle(	intersection.pos[0] - (intersection.w >> 1),
					(g.yres>>1) + (intersection.h >> 1) + 2,
					intersection.w,
					(g.yres >> 1) - (intersection.h >> 1));
	//roadway east
	fillRectangle(	0,
					(g.yres>>1) - (intersection.h >> 1),
					(g.xres >> 1) - (intersection.w >> 1) - 1,
					intersection.h);
	//roadway west
	fillRectangle(	(g.xres >> 1) + (intersection.w >> 1) + 2,
					(g.yres>>1) - (intersection.h >> 1),
					(g.xres >> 1) - (intersection.w >> 1) - 1,
					intersection.h);
	//Highway dashed lines
	XSetForeground(g.dpy, g.gc, 0x00666655);
	//dashed lines north
	int y1 = 0;
	int y2 = 20;
	for (int i = 0; i<5; i++) {
		fillRectangle(intersection.pos[0] - 2, y1, 4, y2);
		y1 += y2 + 9;
	}
	//dashed lines south
	y1 = 20;
	y2 = 20;
	for (int i = 0; i<5; i++) {
		fillRectangle(intersection.pos[0] - 2, g.yres - 1 - y1, 4, y2);
		y1 += 29;
	}
	//dashed lines west
	int x1 = 0;
	int x2 = 20;
	for (int i = 0; i<5; i++) {
		fillRectangle(x1, intersection.pos[1] - 2, x2, 4);
		x1 += x2 + 9;
	}
	//dashed lines east
	x1 = 20;
	x2 = 20;
	for (int i = 0; i<5; i++) {
		fillRectangle(g.xres - 1 - x1, intersection.pos[1] - 2, x2, 4);
		x1 += 29;
	}

	// Draw Cars
	unsigned int col[] = {0x00ff0000, 0x0000ff00, 0x004444ff, 0x00ff00ff, 0x00ffcc88};

	for (int i = 0; i<NCARS; i++) {
		XSetForeground(g.dpy, g.gc, col[i]);
		fillRectangle(cars[i].pos[0] - (cars[i].w >> 1),
						cars[i].pos[1] - (cars[i].h >> 1),
						cars[i].w, cars[i].h);
	}

	// Display Menu
	draw_string("'C' = See collisions", y += 16, 0x0000ff00);
	draw_string("'S' = Slow mode", y += 16, 0x0000ff00);
	draw_string("'W' = Stat Window", y += 16, 0x0000ff00);
	
	if (!g.pause) draw_string("'P' = Pause Mode", y += 16, 0x0000ff00);

	if ((child && g.show_text) || (!child && (g.show_text || num_children == 0))) {
		char buffer[100];
		sprintf(buffer, " n collisions: %i", g.ncollisions);
		draw_string(buffer, y += 16, 0x0000ff00);

		if (g.pause) {
			draw_string("-- PAUSED --", y += 16, 0x00ffffff);
		}

		for (int i = 0; i < 4; i++) {
			sprintf(buffer, "Car Number %i : %i", (i + 1), g.passes[i]);
			draw_string(buffer, y += 16, 0x00ffffff);
		}
	}
 
	if (child) {
		draw_string("STAT WINDOW", y = 20, 0x00ffffff);
	}

	if (g.show_collisions) {
		if (g.collision_flag) {
			//show collision with lines drawn fron corner.
			XSetForeground(g.dpy, g.gc, col[g.crash[0]]);
			drawLine(g.xres-1, 0, g.collision[0], g.collision[1]);
			XSetForeground(g.dpy, g.gc, col[g.crash[1]]);
			drawLine(g.xres-1, 0, g.collision[2], g.collision[3]);
		}
	}
}