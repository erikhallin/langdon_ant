/*TODO

full random by default
-number of colonies
-move pattern
-individual colors

*/


#include <windows.h>
#include <vector>
#include <ctime>
#include <stdlib.h>
#include <gl/gl.h>

using namespace std;

struct st_ant
{
    st_ant()
    {
        posx=posy=dir=0;
    }
    st_ant(int x,int y)
    {
        posx=x;
        posy=y;
        dir=rand()%4;
        //step=0;
    }

    int posx,posy;
    int dir;//0 up, cw
    //int step;
};

struct st_col
{
    st_col()
    {
        r=g=b=0;
    }
    st_col(float _r,float _g, float _b)
    {
        r=_r;
        g=_g;
        b=_b;
    }
    float r,g,b;
};

int _w_width=600;
int _w_height=400;
vector< vector<int> > g_vec_board;
//vector< vector<int> > g_vec_board_next;
vector<st_ant> g_vec_ants;
vector<int>    g_v_walk_seq;
vector<st_col> g_colors;
bool have_sequence=false;
bool init_done=false;
int ant_counter=0;
bool set_ant_route=false;

LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);
void EnableOpenGL(HWND hwnd, HDC*, HGLRC*);
void DisableOpenGL(HWND, HDC, HGLRC);
bool init(void);
bool update(void);
bool draw(void);

int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine,
                   int nCmdShow)
{
    WNDCLASSEX wcex;
    HWND hwnd;
    HDC hDC;
    HGLRC hRC;
    MSG msg;
    BOOL bQuit = FALSE;

    if(lpCmdLine=="makeroute")
    {
        set_ant_route=true;
    }
    else have_sequence=true;


    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_OWNDC;
    wcex.lpfnWndProc = WindowProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = "LangtonsAnt";
    wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);;


    if (!RegisterClassEx(&wcex))
        return 0;

    if(true)//if fullscreen
    {
        //Detect screen resolution
        RECT desktop;
        // Get a handle to the desktop window
        const HWND hDesktop = GetDesktopWindow();
        // Get the size of screen to the variable desktop
        GetWindowRect(hDesktop, &desktop);
        // The top left corner will have coordinates (0,0)
        // and the bottom right corner will have coordinates
        // (horizontal, vertical)
        _w_width  = desktop.right;
        _w_height = desktop.bottom;
    }

    hwnd = CreateWindowEx(0,
                          "LangtonsAnt",
                          "LangtonsAnt",
                          WS_VISIBLE | WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
                          CW_USEDEFAULT,
                          CW_USEDEFAULT,
                          _w_width,
                          _w_height,
                          NULL,
                          NULL,
                          hInstance,
                          NULL);

    ShowWindow(hwnd, nCmdShow);

    EnableOpenGL(hwnd, &hDC, &hRC);

    while (!bQuit)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                bQuit = TRUE;
            }
            else
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        else
        {
            if(have_sequence)
            {
                if(!init_done)
                {
                    //startup
                    if(!init())
                    {
                        //cout<<"ERROR: Could not init\n";
                        return 1;
                    }
                    init_done=true;
                }
                update();
                draw();
                SwapBuffers(hDC);
            }
        }
    }

    DisableOpenGL(hwnd, hDC, hRC);
    DestroyWindow(hwnd);

    return msg.wParam;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_CLOSE:
            PostQuitMessage(0);
        break;

        case WM_DESTROY:
            return 0;

        case WM_KEYDOWN:
        {
            switch (wParam)
            {
                case VK_ESCAPE:
                {
                    PostQuitMessage(0);
                }break;

                case VK_SPACE:
                {
                    //reset board
                    g_vec_board.clear();
                    g_vec_ants.clear();
                    g_v_walk_seq.clear();
                    g_colors.clear();
                    init();
                }break;
            }
        }break;

        case WM_KEYUP:
        {
            switch (wParam)
            {
                case VK_UP:
                {
                    if(!have_sequence)
                    {
                        g_v_walk_seq.push_back(0);
                        Beep(900,200);
                    }
                }break;

                case VK_RIGHT:
                {
                    if(!have_sequence)
                    {
                        g_v_walk_seq.push_back(1);
                        Beep(700,200);
                    }
                }break;

                case VK_DOWN:
                {
                    if(!have_sequence)
                    {
                        g_v_walk_seq.push_back(2);
                        Beep(500,200);
                    }
                }break;

                case VK_LEFT:
                {
                    if(!have_sequence)
                    {
                        g_v_walk_seq.push_back(3);
                        Beep(400,200);
                    }
                }break;

                case VK_RETURN:
                {
                    if((int)g_v_walk_seq.size()>0 && !have_sequence)
                    {
                        have_sequence=true;
                        Beep(800,100);
                        Beep(0,30);
                        Beep(900,100);
                    }
                }break;

                break;
            }
        }break;

        case WM_LBUTTONUP:
        {
            if(!have_sequence)
            {
                ant_counter++;
                Beep(1000,100);
            }
        }break;

        case WM_RBUTTONUP:
        {
            if(!have_sequence)
            {
                ant_counter--;
                if(ant_counter<1)
                {
                    ant_counter=1;
                    Beep(50,30);
                }
                else Beep(500,50);
            }
        }break;

        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return 0;
}

void EnableOpenGL(HWND hwnd, HDC* hDC, HGLRC* hRC)
{
    PIXELFORMATDESCRIPTOR pfd;

    int iFormat;

    *hDC = GetDC(hwnd);

    ZeroMemory(&pfd, sizeof(pfd));

    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW |
                  PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 24;
    pfd.cDepthBits = 16;
    pfd.iLayerType = PFD_MAIN_PLANE;

    iFormat = ChoosePixelFormat(*hDC, &pfd);

    SetPixelFormat(*hDC, iFormat, &pfd);

    *hRC = wglCreateContext(*hDC);

    wglMakeCurrent(*hDC, *hRC);

    //set 2D mode
    glClearColor(0.0,0.0,0.0,0.0);  //Set the cleared screen colour to black
    glViewport(0,0,_w_width,_w_height);   //This sets up the viewport so that the coordinates (0, 0) are at the top left of the window

    //Set up the orthographic projection so that coordinates (0, 0) are in the top left
    //and the minimum and maximum depth is -10 and 10. To enable depth just put in
    //glEnable(GL_DEPTH_TEST)
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0,_w_width,_w_height,0,-1,1);

    //Back to the modelview so we can draw stuff
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    /*//Enable antialiasing
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POLYGON_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT,GL_NICEST);
    glHint(GL_POLYGON_SMOOTH_HINT,GL_NICEST);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearStencil(0);*/
}

void DisableOpenGL (HWND hwnd, HDC hDC, HGLRC hRC)
{
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(hRC);
    ReleaseDC(hwnd, hDC);
}

bool init()
{
    //Beep(1440, 500);

    srand(time(0));
    //build board
    for(int x=0;x<_w_width;x++)
    {
        g_vec_board.push_back(vector<int>());
        //g_vec_board_next.push_back(vector<int>());
        for(int y=0;y<_w_height;y++)
        {
            g_vec_board[x].push_back(-1);
            //g_vec_board_next[x].push_back(0);
        }
    }

    //spawn ants
    if(!set_ant_route) ant_counter=rand()%50+10;
    int numof_ants=ant_counter;
    if(numof_ants<1) numof_ants=10;
    for(int i=0;i<numof_ants;i++)
    {
        int posx=rand()%_w_width;
        int posy=rand()%_w_height;
        g_vec_ants.push_back(st_ant(posx,posy));
    }

    //generate walk sequence
    /*g_v_walk_seq.push_back(1);
    g_v_walk_seq.push_back(3);
    g_v_walk_seq.push_back(1);
    g_v_walk_seq.push_back(3);*/
    if(!set_ant_route)
    {
        int step_size=rand()%10+3;
        for(int i=0;i<step_size;i++)
        {
            g_v_walk_seq.push_back(rand()%4);
        }

    }

    //generate colors
    for(int i=0;i<(int)g_v_walk_seq.size();i++)
    {
        g_colors.push_back(st_col(float((int)rand()%100)/100.0,
                                  float((int)rand()%100)/100.0,
                                  float((int)rand()%100)/100.0));
    }

    return true;
}

bool update()
{
    //ant walk
    for(int i=0;i<(int)g_vec_ants.size();i++)
    {
        if(g_vec_board[g_vec_ants[i].posx][g_vec_ants[i].posy]<0) g_vec_board[g_vec_ants[i].posx][g_vec_ants[i].posy]=0;

        //rotate
        g_vec_ants[i].dir+=g_v_walk_seq[ g_vec_board[g_vec_ants[i].posx][g_vec_ants[i].posy] ];
        while(g_vec_ants[i].dir>3) g_vec_ants[i].dir-=4;

        //move
        switch(g_vec_ants[i].dir)
        {
            case 0: g_vec_ants[i].posy-=1; break;
            case 1: g_vec_ants[i].posx+=1; break;
            case 2: g_vec_ants[i].posy+=1; break;
            case 3: g_vec_ants[i].posx-=1; break;
        }

        //if ant is outside the board, remove
        if(g_vec_ants[i].posx<0 || g_vec_ants[i].posx>=_w_width ||
           g_vec_ants[i].posy<0 || g_vec_ants[i].posy>=_w_height)
        {
            g_vec_ants.erase(g_vec_ants.begin()+i);
            i--;
            continue;
        }

        //update board
        g_vec_board[g_vec_ants[i].posx][g_vec_ants[i].posy]++;
        if(g_vec_board[g_vec_ants[i].posx][g_vec_ants[i].posy]>=(int)g_v_walk_seq.size()) g_vec_board[g_vec_ants[i].posx][g_vec_ants[i].posy]=0;
    }

    return true;
}

bool draw()
{
    //glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
    glLoadIdentity();

    //draw board
    glPushMatrix();
    glColor3f(1,1,1);
    glBegin(GL_POINTS);
    for(int x=0;x<_w_width;x++)
    {
        for(int y=0;y<_w_height;y++)
        {
            if(g_vec_board[x][y]>=0)
            {
                glColor3f(g_colors[g_vec_board[x][y]].r,
                          g_colors[g_vec_board[x][y]].g,
                          g_colors[g_vec_board[x][y]].b);
                glVertex2f(x,y);
            }
        }
    }
    glEnd();

    //draw ants

    glPopMatrix();

    return true;
}



