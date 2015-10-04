/*    Copyright (c) 2015, Professional Zelda Hackers Std.

Permission to use, copy, modify, and/or distribute this software for any purpose
 with or without fee is hereby granted, provided that the above copyright notice
  and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF
THIS SOFTWARE.
*/



#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <vector>
#include <stack>

#include "endian.h"

#include <SDL2/SDL.h>
#undef SDL_main
#include <SDL2/SDL_opengl.h>
#include <GL/glu.h>

float degtorad = 3.141592653589793 / 180.0;

float sens = 1.0/32;



char * currentzmap;

uint8_t mem8(uint32_t addr)
{
    return *(currentzmap+addr);
}
uint32_t mem32(uint32_t addr)
{
    return swap32(*(uint32_t*)(currentzmap+addr));
}

struct vertex
{
    int16_t x = 0;
    int16_t y = 0;
    int16_t z = 0;
    //uint16_t nothing = 0;
    
    int16_t u = 0;
    int16_t v = 0;
    
    int8_t i = 0;
    int8_t j = 0;
    int8_t k = 0;
    int8_t l = 0;
    vertex() { }
    vertex(uint32_t addr, bool print)
    {
        uint32_t rawdata = mem32(addr);
        x = (rawdata & 0xFFFF0000)
                     / 0x00010000;
        y = (rawdata & 0x0000FFFF)
                     / 0x00000001;
        rawdata = mem32(addr+4);
        z = (rawdata & 0xFFFF0000)
                     / 0x00010000;
        rawdata = mem32(addr+8);
        u = (rawdata & 0xFFFF0000)
                     / 0x00010000;
        v = (rawdata & 0x0000FFFF)
                     / 0x00000001;
        rawdata = mem32(addr+12);
        i = (rawdata & 0xFF000000)
                     / 0x01000000;
        j = (rawdata & 0x00FF0000)
                     / 0x00010000;
        k = (rawdata & 0x0000FF00)
                     / 0x00000100;
        l = (rawdata & 0x000000FF)
                     / 0x00000001;
        if(print)
            {}//printf("made vertex %d %d %d : %d %d %d\n", x, y, z, i, j, k);
    }
};

struct dlistpointer
{
    char * buffer;
    uint32_t offset;
};

int main(int argc, char ** argv)
{
    if(argc<2)
    {
        puts("Usage: zev2 mymap.zmap <others>");
        return 0;
    }
    
    std::vector<char*> files;
    
    for(auto i = 1; i < argc; i++)
        files.push_back(argv[i]);
    
    std::vector<dlistpointer> opaque_dlists;
    std::vector<dlistpointer> glassy_dlists;
    
    unsigned index = 0;
    for(auto filename : files)
    {
        index = 0;
        auto file = fopen(filename, "rb");
        if (file == NULL)
        {
            puts("Could not open file.");
            return 0;
        }
        printf("Loading map %s", filename);
        
        fseek(file, 0, SEEK_END);
        auto filesize = ftell(file);
        fseek(file, 0, SEEK_SET);
        
        currentzmap = (char*)malloc(filesize);
        
        fread(currentzmap, 1, filesize, file);
        
        fclose(file);
        
        
        
        bool breakout = false;
        
        unsigned meshaddress = 0;
        
        while(breakout == false)
        {
            switch (mem8(index))
            {
            case 0x00:
                puts("Start positions"); break;
            case 0x01:
                puts("Actor list"); break;
            case 0x02:
                puts("Cameras"); break;
            case 0x03:
                puts("Collision"); break;
            case 0x04:
                puts("Maplist"); break;
            case 0x05:
                puts("Wind info"); break;
            case 0x06:
                puts("Entrance list"); break;
            case 0x07:
                puts("Special objects"); break;
            case 0x08:
                puts("Room behavior"); break;
            case 0x09:
                puts("Unused?"); break;
            case 0x0A:
                meshaddress = mem32(index+4);
                printf("Mesh address %08X\n", meshaddress); break;
            case 0x0B:
                puts("Object list"); break;
            case 0x0C:
                puts("Unused env settings"); break;
            case 0x0D:
                puts("Paths"); break;
            case 0x0E:
                puts("Transition actor list"); break;
            case 0x0F:
                puts("Env settings"); break;
            case 0x10:
                puts("Time settings"); break;
            case 0x11:
                puts("Skybox settings"); break;
            case 0x12:
                puts("Skybox modifier"); break;
            case 0x13:
                puts("Exit List"); break;
            case 0x14:
                puts("End of header"); breakout = true; break;
            case 0x15:
                puts("Sound settings (scene)"); break;
            case 0x16:
                puts("Sound settings (room)"); break;
            case 0x17:
                puts("Cutscenes"); break;
            case 0x18:
                puts("Extra headers"); break;
            case 0x19:
                puts("Camera, world map"); break;
            default:
                puts("Unknown");
            }
            index += 8;
        }
        
        if(meshaddress>>24 != 0x03)
        {   printf("Unsupported mesh header bank. %02X",meshaddress>>24); return 0; }
        
        
        
        uint8_t count;
        uint32_t start;
        uint32_t end;
        
        index = meshaddress&0x00FFFFFF;
        
        int meshtype = -1;
        
        switch (mem8(index))
        {
        case 0x00:
        case 0x02:
            count = mem8(index+1);
            start = mem32(index+4);
            end = mem32(index+8);
            puts("Found the meshes");
            meshtype = mem8(index);
            break;
        default:
            puts("Unsupported mesh type in mesh header."); printf("%08X\n",index); return 0;
        }
        
        if(start>>24 != 0x03)
        {   printf("Unsupported mesh data bank. %02X",start>>24); return 0; }
        printf("%d\n", count);
        
        index = start&0x00FFFFFF;
        
        if(meshtype == 0)
        {
            for(auto i = 0; i < count; i++)
            {
                if(mem8(index) == 0x03)
                    opaque_dlists.push_back({currentzmap, mem32(index)&0x00FFFFFF});
                if(mem8(index+4) == 0x03)
                    glassy_dlists.push_back({currentzmap, mem32(index+4)&0x00FFFFFF});
                index += 8;
            }
            puts("Installed dlists");
        }
        if(meshtype == 2)
        {
            for(auto i = 0; i <= count; i++)
            {
                if(mem8(index+8) == 0x03)
                    opaque_dlists.push_back({currentzmap, mem32(index+8)&0x00FFFFFF});
                if(mem8(index+12) == 0x03)
                    glassy_dlists.push_back({currentzmap, mem32(index+12)&0x00FFFFFF});
                index += 16;
            }
            puts("Installed dlists");
        }
        
    }
    
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_RendererInfo info;
    
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 8);
    
    SDL_CreateWindowAndRenderer(800, 600, SDL_WINDOW_OPENGL, &window, &renderer);
    SDL_GetRendererInfo(renderer, &info);
    
    glViewport(0, 0, 800, 600);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
	gluPerspective(80.0f, 800.0/600.0, 1.0f, 65536.0f*2);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
    
    glShadeModel(GL_SMOOTH);
    glClearColor(0.4f, 0.6f, 0.8f, 1.0f);
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glEnable(GL_NORMALIZE);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    GLfloat asdasgasfbasd[] = {0.0f, 0.0f, 0.0f, 1.0f};
    glLightfv(GL_LIGHT0, GL_DIFFUSE, asdasgasfbasd);
    glEnable(GL_LIGHT1);
    
    float xpos = 0;
    float ypos = 100;
    float zpos = 0;
    float yaw = 0; // horizontal
    float pitch = 0; // vertical
    
    auto corestate = SDL_GetKeyboardState(NULL);
    
    SDL_SetRelativeMouseMode(SDL_TRUE);
    int xdelta = 0;
    int ydelta = 0;
    
    SDL_Event event;
    bool firstrun = true;
    
    uint32_t oldtime = SDL_GetTicks();
    uint32_t newtime = SDL_GetTicks()+100;
    while(opaque_dlists.size() > 0)
    {
        while(SDL_PollEvent( &event ))
            if(event.type == SDL_QUIT) goto quit;
        
        // handle inputs
        SDL_GetRelativeMouseState(&xdelta,&ydelta);
        if(SDL_GetWindowFlags(window)&SDL_WINDOW_INPUT_FOCUS)
        {
            yaw += xdelta*sens;
            pitch += ydelta*sens;
        }
        if(pitch > 90) pitch = 90;
        if(pitch < -90) pitch = -90;
        
        float camspeed = newtime - oldtime;
        oldtime = newtime;
        newtime = SDL_GetTicks();
        
        
        float multiplier = 1.0f;
        
        if( (corestate[SDL_SCANCODE_E] or corestate[SDL_SCANCODE_D])
        and (corestate[SDL_SCANCODE_F] or corestate[SDL_SCANCODE_W])
        )   multiplier = sqrt(0.5f);
        
        if(corestate[SDL_SCANCODE_E])
        {
		    xpos += sin(yaw*degtorad) * cos(pitch*degtorad) * camspeed * multiplier;
		    ypos -= cos(yaw*degtorad) * cos(pitch*degtorad) * camspeed * multiplier;
		    zpos -= sin(pitch*degtorad) * camspeed * multiplier;
        }
        if(corestate[SDL_SCANCODE_D])
        {
		    xpos -= sin(yaw*degtorad) * cos(pitch*degtorad) * camspeed * multiplier;
		    ypos += cos(yaw*degtorad) * cos(pitch*degtorad) * camspeed * multiplier;
		    zpos += sin(pitch*degtorad) * camspeed * multiplier;
        }
        if(corestate[SDL_SCANCODE_F])
        {
		    xpos += sin((yaw+90)*degtorad) * camspeed * multiplier;
		    ypos -= cos((yaw+90)*degtorad) * camspeed * multiplier;
        }
        if(corestate[SDL_SCANCODE_W])
        {
		    xpos += sin((yaw-90)*degtorad) * camspeed * multiplier;
		    ypos -= cos((yaw-90)*degtorad) * camspeed * multiplier;
        }
        
        
        // reset screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glLoadIdentity();
        
        // handle modal state
        glRotatef(pitch, 1.0, 0, 0);
        glRotatef(yaw, 0, 1.0, 0);
        
        glTranslatef(-xpos, -zpos, -ypos);
        
        // fun stuff
        GLfloat ambientColor[] = {1.4f, 1.5f, 1.6f, 4.0f};
        glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientColor);
        
        GLfloat color[] = {1.0f, 0.9f, 0.8f, 1.0f};
        glLightfv(GL_LIGHT1, GL_DIFFUSE, color);
        GLfloat pos[] = {1.0f, 1.0f, 0.2f, 0.0f};
        glLightfv(GL_LIGHT1, GL_POSITION, pos);
        
        //origin
        
        glDisable(GL_LIGHTING);
        glBegin(GL_QUADS);
        
        glColor3f(1,0,0);
        
        glVertex3f(-9,-1, 0);
        glVertex3f(-9, 1, 0);
        glVertex3f( 9,-1, 0);
        glVertex3f( 9, 1, 0);
        glVertex3f(-9, 0,-1);
        glVertex3f(-9, 0,-1);
        glVertex3f( 9, 0,-1);
        glVertex3f( 9, 0,-1);
        
        glColor3f(0,1,0);
        glVertex3f( 0,-9,-1);
        glVertex3f( 0,-9, 1);
        glVertex3f( 0, 9,-1);
        glVertex3f( 0, 9, 1);
        glVertex3f(-1,-9, 0);
        glVertex3f( 1,-9, 0);
        glVertex3f(-1, 9, 0);
        glVertex3f( 1, 9, 0);
        
        glColor3f(0,1,1);
        glVertex3f( 1, 0,-9);
        glVertex3f(-1, 0,-9);
        glVertex3f( 1, 0, 9);
        glVertex3f(-1, 0, 9);
        glVertex3f( 0,-1,-9);
        glVertex3f( 0, 1,-9);
        glVertex3f( 0,-1, 9);
        glVertex3f( 0, 1, 9);
        
        glEnd();
        glEnable(GL_LIGHTING);
        
        
        for(auto list : opaque_dlists)
        {
            //info
            //printf("dlist: %08X\n", list);
            
            currentzmap = list.buffer;
            
            //state
            std::vector<vertex> verts;
            std::stack<uint32_t> stack;
            
            bool normal = true;
            bool filter = true;
            bool normalize = true;
            bool unsupported = false;
            index = list.offset;
            // interpret dlist
            while(1)
            {
                skippc:
                switch(mem8(index))
                {
                case 0x01:
                    //puts("verts");
                    {
                        unsigned int count = (mem32(index)&0xFFF000)/0x1000;
                        int where = (mem32(index)&0x000FFF)/2;
                        where -= count;
                        
                        int addr = (mem32(index+4)&0x00FFFFFF);
                        unsigned int max = where+count;
                        if(mem8(index+4) != 03)
                        {
                            unsupported = true;
                            break;
                        }
                        unsupported = false;
                        if(firstrun)
                            {}/*printf("vertex command %03X->%03X %03X->%03X %06X\n"
                                , (mem32(index)&0xFFF000)/0x1000, count
                                , mem32(index)&0x000FFF, where
                                , addr);*/
                        for(unsigned i = 0; i < count; i++)
                        {
                            if(where+i >= verts.size())
                                verts.push_back(vertex(addr + i*16, firstrun));
                            else
                                verts[where+i] = vertex(addr + i*16, firstrun);
                        }
                    }
                    break;
                
                
                #define autovertex(index) \
                    verts[(index)].x, verts[(index)].y, verts[(index)].z
                #define autoijk(index) \
                    verts[(index)].i, verts[(index)].j, verts[(index)].k
                #define autorgb(index) \
                    (uint8_t)(verts[(index)].i)/256.0, (uint8_t)(verts[(index)].j)/256.0, (uint8_t)(verts[(index)].k)/256.0
                    
                    
                case 0x05:
                    //puts("tri1");
                    if(unsupported)
                        break;
                    else
                    {
                        glBegin(GL_TRIANGLES);
                        glNormal3f(0.0, 0.0, 1.0);
                        uint8_t vert1 = mem8(index+1)/2;
                        uint8_t vert2 = mem8(index+2)/2;
                        uint8_t vert3 = mem8(index+3)/2;
                        
                        if(normalize)
                        {
                            glColor3f(1.0f, 1.0f, 1.0f);
                            
                            glNormal3f(autoijk(vert1));
                            glVertex3f(autovertex(vert1));
                            glNormal3f(autoijk(vert2));
                            glVertex3f(autovertex(vert2));
                            glNormal3f(autoijk(vert3));
                            glVertex3f(autovertex(vert3));
                        }
                        else
                        {
                            glColor3f(autorgb(vert1));
                            glVertex3f(autovertex(vert1));
                            glColor3f(autorgb(vert2));
                            glVertex3f(autovertex(vert2));
                            glColor3f(autorgb(vert3));
                            glVertex3f(autovertex(vert3));
                        }
                        
                        
                        glEnd();
                    }
                    break;
                    
                    
                case 0x06:
                    //puts("tri2");
                    if(unsupported)
                        break;
                    else
                    {
                        glBegin(GL_TRIANGLES);
                        uint8_t vert1 = mem8(index+1)/2;
                        uint8_t vert2 = mem8(index+2)/2;
                        uint8_t vert3 = mem8(index+3)/2;
                        uint8_t vert4 = mem8(index+4+1)/2;
                        uint8_t vert5 = mem8(index+4+2)/2;
                        uint8_t vert6 = mem8(index+4+3)/2;
                        
                        if(normalize)
                        {
                            glColor3f(1.0f, 1.0f, 1.0f);
                            glNormal3f(autoijk(vert1));
                            glVertex3f(autovertex(vert1));
                            glNormal3f(autoijk(vert2));
                            glVertex3f(autovertex(vert2));
                            glNormal3f(autoijk(vert3));
                            glVertex3f(autovertex(vert3));
                            glNormal3f(autoijk(vert4));
                            glVertex3f(autovertex(vert4));
                            glNormal3f(autoijk(vert5));
                            glVertex3f(autovertex(vert5));
                            glNormal3f(autoijk(vert6));
                            glVertex3f(autovertex(vert6));
                        }
                        else
                        {
                            glColor3f(autorgb(vert1));
                            glVertex3f(autovertex(vert1));
                            glColor3f(autorgb(vert2));
                            glVertex3f(autovertex(vert2));
                            glColor3f(autorgb(vert3));
                            glVertex3f(autovertex(vert3));
                            glColor3f(autorgb(vert4));
                            glVertex3f(autovertex(vert4));
                            glColor3f(autorgb(vert5));
                            glVertex3f(autovertex(vert5));
                            glColor3f(autorgb(vert6));
                            glVertex3f(autovertex(vert6));
                        }
                        glEnd();
                    }
                    break;
                case 0x07:
                    //puts("quad");
                    // TODO
                    break;
                case 0xD9:
                    //puts("mode");
                    {
                        bool filterclear = ((0x00200000&mem32(index  )) != 0);
                        bool normalclear = ((0x00020000&mem32(index  )) != 0);
                        bool filterenset = ((0x00200000&mem32(index+4)) != 0);
                        bool normalenset = ((0x00020000&mem32(index+4)) != 0);
                        
                        filter = filterenset|(filter&filterclear);
                        normal = normalenset|(normal&normalclear);
                        
                        if(normal)
                        {
                            normalize = true;
                            glEnable(GL_LIGHTING);
                        }
                        else if(filter)
                        {
                            normalize = false;
                            glDisable(GL_LIGHTING);
                        }
                    }
                    // TODO
                    break;
                case 0xDE:
                    //puts("subdl");
                    stack.push(index+8);
                    index=mem32(index+4)&0x00FFFFFF;
                    goto skippc;
                    break;
                case 0xDF:
                    //puts("return");
                    if(stack.size() > 0)
                    {
                        index = stack.top();
                        stack.pop();
                        goto skippc;
                    }
                    else
                        goto finished;
                    break;
                }
                index += 8;
            }
            finished:
            //printf("%08X: %08X\n",index,mem32(index));
            ;
        }
        
        glFlush();
        
        SDL_GL_SwapWindow(window); 
        
        SDL_Delay(5);
        firstrun = false;
    }
    
    quit:
    
    SDL_Quit();
}




