#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <random>
#include <vector>
#include <GL/glut.h>

// ウィンドウ設定
#define WINDOW_X (500)
#define WINDOW_Y (500)
#define WINDOW_NAME "test3"

void init_GL(int argc, char *argv[]);
void init();
void set_callback_functions();

void glut_display();
void glut_keyboard(unsigned char key, int x, int y);
void glut_mouse(int button, int state, int x, int y);
void glut_motion(int x, int y);
void glut_idle();

void draw_raindrop();
void draw_waves();

typedef struct{
    double x;
    double z;
    double r;
    double t;
} wave;

std::vector<std::vector<double>> rains;
std::vector<wave> waves;
std::random_device rnd;

bool start_flag = false;
int rain_rate = 3;

// グローバル変数
double g_angle1 = 0.0;
double g_angle2 = 0.0;
double g_distance = 20.0;
bool g_isLeftButtonOn = false;
bool g_isRightButtonOn = false;

int main(int argc, char *argv[]){
    init_GL(argc,argv);
    init();
    set_callback_functions();
    glutMainLoop();

    return 0;
}
void init_GL(int argc, char *argv[]){
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
    glutInitWindowSize(WINDOW_X,WINDOW_Y);
    glutCreateWindow(WINDOW_NAME);
}

void init(){
    glClearColor(0.0, 0.0, 0.0, 0.0); // 背景の塗りつぶし色を指定
}

void set_callback_functions(){
    glutDisplayFunc(glut_display);
    glutKeyboardFunc(glut_keyboard);
    glutMouseFunc(glut_mouse);
    glutMotionFunc(glut_motion);
    glutPassiveMotionFunc(glut_motion);
    glutIdleFunc(glut_idle);
}

void glut_keyboard(unsigned char key, int x, int y){
    switch(key){
    // 終了操作
    case 'q':
    case 'Q':
    case '\033':
        exit(0);
        break;

    case 's':  // start
        start_flag = start_flag ? false : true;
        break;
    // 雨量変更
    case '0': rain_rate = 0; break;
    case '1': rain_rate = 1; break;
    case '2': rain_rate = 2; break;
    case '3': rain_rate = 3; break;
    case '4': rain_rate = 4; break;
    case '5': rain_rate = 5; break;
    case '6': rain_rate = 6; break;
    case '7': rain_rate = 7; break;
    case '8': rain_rate = 8; break;
    case '9': rain_rate = 9; break;
    case '-': rain_rate = 10; break;
    }

    glutPostRedisplay();
}

// マウス操作
void glut_mouse(int button, int state, int x, int y){
    if(button == GLUT_LEFT_BUTTON){
        if(state == GLUT_UP){
            g_isLeftButtonOn = false;
        }else if(state == GLUT_DOWN){
            g_isLeftButtonOn = true;
        }
    }

    if(button == GLUT_RIGHT_BUTTON){
        if(state == GLUT_UP){
            g_isRightButtonOn = false;
        }else if(state == GLUT_DOWN){
            g_isRightButtonOn = true;
        }
    }
}

// マウス移動
void glut_motion(int x, int y){
    static int px = -1, py = -1;
    if(g_isLeftButtonOn == true){
        if(px >= 0 && py >= 0){
            g_angle1 += (double)-(x - px)/20;
            g_angle2 += (double)(y - py)/20;
        }
        px = x;
        py = y;
    }else if(g_isRightButtonOn == true){
        if(px >= 0 && py >= 0){
            g_distance += (double)(y - py)/20;
        }
        px = x;
        py = y;
    }else{
        px = -1;
        py = -1;
    }
    glutPostRedisplay();
}

// 描画
void glut_display(){
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(30.0, 1.0, 0.1, 100);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    if (cos(g_angle2)>0){
        gluLookAt(g_distance * cos(g_angle2) * sin(g_angle1),
                    g_distance * sin(g_angle2),
                    g_distance * cos(g_angle2) * cos(g_angle1),
                    0.0, 0.0, 0.0, 0.0, 1.0, 0.0);}
    else{
        gluLookAt(g_distance * cos(g_angle2) * sin(g_angle1),
                    g_distance * sin(g_angle2),
                    g_distance * cos(g_angle2) * cos(g_angle1),
                    0.0, 0.0, 0.0, 0.0, -1.0, 0.0);}

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    draw_raindrop();
    draw_waves();

    glFlush();
    glDisable(GL_DEPTH_TEST);

    glutSwapBuffers();
}

// ループ処理
void glut_idle(){
    int count_rain = 0, count_wave = 0;
    for(int i=0; i<rains.size(); i++){
        rains[i][1] -= 0.1;
        if((double)rains[i][1]-0.5 < -4.0) count_rain++;
    }
    for(int i=0; i<waves.size(); i++){
        waves[i].r += 0.05; waves[i].t -= 0.03;
        if(waves[i].t < 0) count_wave++;
    }
    if(count_rain > 0){
        for(int i=0; i<count_rain; i++){
            wave new_w;
            new_w.x = rains[i][0]; new_w.z = rains[i][2];
            new_w.r = 0.01; new_w.t = 1.0;
            waves.push_back(new_w);
        }
        rains.erase(rains.begin(), rains.begin()+count_rain);
    }
    if(count_wave > 0) waves.erase(waves.begin(), waves.begin()+count_wave);
    if(start_flag){
        if(rnd()%10 < rain_rate){
            std::vector<double> new_v = {(double)(rnd()%400)/50.0-4.0, 4.0, (double)(rnd()%400)/50.0-4.0};
            rains.push_back(new_v);
        }
    }
    usleep(10000); // フレームレート調整
    glutPostRedisplay();
}

void draw_raindrop(){
    glColor3d(1.0, 1.0, 1.0);
    glLineWidth(1.0);
    for(int i=0; i<rains.size(); i++){
        glBegin(GL_LINES);
        glVertex3d(rains[i][0], rains[i][1]-0.5, rains[i][2]);
        glVertex3d(rains[i][0], rains[i][1]+0.5, rains[i][2]);
        glEnd();
    }
}

void draw_waves(){
    double x, z, r, color;
    for(int i=0; i<waves.size(); i++){
        x = waves[i].x; z = waves[i].z;
        r = waves[i].r; color = waves[i].t;

        glColor3d(color, color, color);
        glBegin(GL_LINE_LOOP);
        for(double j=0.0; j<50; j++){
            glVertex3d(r*cos(2.0*M_PI*j/50.0)+x, -4.0, r*sin(2.0*M_PI*j/50.0)+z);
        }
        glEnd();
        glBegin(GL_LINE_LOOP);
        for(double j=0.0; j<50; j++){
            glVertex3d((r+0.03)*cos(2.0*M_PI*j/50.0)+x, -4.0, (r+0.03)*sin(2.0*M_PI*j/50.0)+z);
        }
        glEnd();
    }       
}

