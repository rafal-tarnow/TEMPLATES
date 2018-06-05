#include <iostream>
#include <stdlib.h>
#include <GL/glut.h>
#include <math.h>
#include <vector>

using namespace std;


//Point class for taking the points
class Point {
public:
    float x, y;
    void setxy(float x2, float y2)
    {
        x = x2; y = y2;
    }
    //operator overloading for '=' sign
    const Point & operator=(const Point &rPoint)
    {
        x = rPoint.x;
        y = rPoint.y;
        return *this;
    }

};

int factorial(int n)
{
    if (n<=1)
        return(1);
    else
        n=n*factorial(n-1);
    return n;
}

float binomial_coff(float n,float k)
{
    float ans;
    ans = factorial(n) / (factorial(k)*factorial(n-k));
    return ans;
}





int SCREEN_HEIGHT = 500;
int point_index = 0;

const int CONTROL_POINTS_LIMIT = 4;
Point abc[CONTROL_POINTS_LIMIT];
vector<Point> punkty;


void myInit() {
    glClearColor(1.0,1.0,1.0,0.0);
    glColor3f(0.0,0.0,0.0);
    glPointSize(3);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.0,640.0,0.0,500.0);

}

void drawDot(int x, int y) {
    glBegin(GL_POINTS);
    glVertex2i(x,y);
    glEnd();
    glFlush();
}

void drawDot(Point p1) {
    glBegin(GL_POINTS);
    glVertex2f(p1.x,p1.y);
    glEnd();
    glFlush();
}

void drawLine(Point p1, Point p2) {
    glBegin(GL_LINES);
    glVertex2f(p1.x, p1.y);
    glVertex2f(p2.x, p2.y);
    glEnd();
    glFlush();
}


//Calculate the bezier point
Point drawBezier(Point PT[], double t) {
    Point P;
    P.x = pow((1 - t), 3) * PT[0].x + 3 * t * pow((1 -t), 2) * PT[1].x + 3 * (1-t) * pow(t, 2)* PT[2].x + pow (t, 3)* PT[3].x;
    P.y = pow((1 - t), 3) * PT[0].y + 3 * t * pow((1 -t), 2) * PT[1].y + 3 * (1-t) * pow(t, 2)* PT[2].y + pow (t, 3)* PT[3].y;

    return P;
}


//Calculate the bezier point [generalized]


Point generateBezierPoint(Point PT[], double t) {
    Point P;
    P.x = 0; P.y = 0;
    for (int i = 0; i<CONTROL_POINTS_LIMIT; i++)
    {
        P.x = P.x + binomial_coff((float)(CONTROL_POINTS_LIMIT - 1), (float)i) * pow(t, (double)i) * pow((1 - t), (CONTROL_POINTS_LIMIT - 1 - i)) * PT[i].x;
        P.y = P.y + binomial_coff((float)(CONTROL_POINTS_LIMIT - 1), (float)i) * pow(t, (double)i) * pow((1 - t), (CONTROL_POINTS_LIMIT - 1 - i)) * PT[i].y;
    }
    //cout<<P.x<<endl<<P.y;
    //cout<<endl<<endl;
    return P;
}


void myMouse(int button, int state, int x, int y) {
    // If left button was clicked
    if(button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        // Store where mouse was clicked, Y is backwards.
        abc[point_index].setxy((float)x,(float)(SCREEN_HEIGHT - y));
        punkty.push_back(abc[point_index]);
        drawDot(abc[point_index]);

        point_index++;

        // Draw the red  dot.


        // If (click-amout) points are drawn do the curve.
        if(point_index == CONTROL_POINTS_LIMIT)
        {
            glColor3f(0.2,1.0,0.0);
            // Drawing the control lines
            for(int k=0;k<CONTROL_POINTS_LIMIT-1;k++)
                drawLine(abc[k], abc[k+1]);

            Point p1 = abc[0];
            /* Draw each segment of the curve.Make t increment in smaller amounts for a more detailed curve.*/
            for(double t = 0.0;t <= 1.0; t += 0.02)
            {
                Point p2 = generateBezierPoint(abc,t);
                drawLine(p1, p2);
                p1 = p2;
            }
            glColor3f(0.0,0.0,0.0);

            point_index = 0;
        }
    }
}


void myDisplay() {
    cout << "myDisplay()" << endl;
    glClear(GL_COLOR_BUFFER_BIT);

    for(unsigned int i = 0; i < punkty.size(); i++)
    {
        drawDot(punkty.at(i));
    }


    glColor3f(0.2,1.0,0.0);
    // Drawing the control lines

    unsigned int control_index = 0;

    for(unsigned int i = 0;  i < punkty.size(); i = i + 4)
    {

        for(int k= 0 + i; k <CONTROL_POINTS_LIMIT-1 + i ;k++)
        {
            if(k + 1 < punkty.size())
            {
                drawLine(punkty.at(k), punkty.at(k+1));
            }
        }

    }


    glFlush();
    glutSwapBuffers();
    glutPostRedisplay();
}

int main(int argc, char *argv[]) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGB);
    glutInitWindowSize(640,500);
    glutInitWindowPosition(100,150);
    glutCreateWindow("Bezier Curve");
    glutMouseFunc(myMouse);
    glutDisplayFunc(myDisplay);
    myInit();
    glutMainLoop();

    return 0;
}
