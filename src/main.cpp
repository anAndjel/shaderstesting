#include <SDL2/SDL.h>
#include <glad/gl.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include <cstdlib>

// ===== VERTEX SHADER =====
const char* vertSrc = R"(
#version 330 core
layout(location=0) in vec2 pos;
void main(){ gl_Position=vec4(pos,0.0,1.0); }
)";

// ===== AUDIO =====
const int SAMPLE_RATE=44100;
const int AMPLITUDE=28000;

struct Voice{ double phase=0.0; double env=0.0; };
Voice kick,snare,hat,sub,synth;

double bpm=128.0;
double beatTime=0.0;
int lastStep=-1;

// 16-step patterns
int patternKick[16]={1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0};
int patternSnare[16]={0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0};
int patternHat[16]={1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
int patternClap[16]={0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0};

void audioCallback(void* userdata, Uint8* stream, int len){
    Sint16* buf=(Sint16*)stream;
    int samples=len/2;
    double secPerStep=60.0/bpm/4.0;

    for(int i=0;i<samples;i++){
        double out=0.0;
        double time=beatTime+i/(double)SAMPLE_RATE;
        int step=(int)(time/secPerStep)%16;

        // Trigger only on step change
        if(step!=lastStep){
            if(patternKick[step]) kick.env=1.0;
            if(patternSnare[step]) snare.env=1.0;
            if(patternHat[step]) hat.env=1.0;
            if(patternClap[step]) synth.env=0.5;
            lastStep=step;
        }

        // Kick
        out += sin(kick.phase*2.0)*kick.env*0.8;
        kick.phase += 2.0*M_PI*80.0/SAMPLE_RATE; kick.env*=0.995;
        if(kick.phase>2*M_PI) kick.phase-=2*M_PI;

        // Snare
        out += ((rand()%20000)/10000.0-1.0)*snare.env*0.5;
        snare.env *=0.992;

        // Hi-hat
        out += ((rand()%20000)/10000.0-1.0)*hat.env*0.3;
        hat.env *=0.985;

        // Sub-bass
        sub.phase += 2.0*M_PI*40.0/SAMPLE_RATE; if(sub.phase>2*M_PI) sub.phase-=2*M_PI;
        out += sin(sub.phase)*0.3;

        // Synth stab
        out += sin(synth.phase*2.0)*synth.env*0.2;
        synth.phase += 2.0*M_PI*200.0/SAMPLE_RATE; synth.env*=0.998;
        if(synth.phase>2*M_PI) synth.phase-=2*M_PI;

        buf[i]=(Sint16)(AMPLITUDE*out);
    }

    beatTime += samples/(double)SAMPLE_RATE;
}

// ===== HELPERS =====
std::string loadFile(const char* path){
    std::ifstream in(path); if(!in){ std::cerr<<"Failed "<<path<<"\n"; return ""; }
    std::stringstream ss; ss<<in.rdbuf(); return ss.str();
}

GLuint compileShader(GLenum type,const char* src){
    GLuint s=glCreateShader(type); glShaderSource(s,1,&src,nullptr); glCompileShader(s);
    GLint ok; glGetShaderiv(s,GL_COMPILE_STATUS,&ok);
    if(!ok){ char buf[1024]; glGetShaderInfoLog(s,1024,nullptr,buf); std::cerr<<"Shader error:\n"<<buf<<"\n"; }
    return s;
}

GLuint linkProgram(GLuint vs,GLuint fs){
    GLuint prog=glCreateProgram(); glAttachShader(prog,vs); glAttachShader(prog,fs); glLinkProgram(prog);
    GLint ok; glGetProgramiv(prog,GL_LINK_STATUS,&ok);
    if(!ok){ char buf[1024]; glGetProgramInfoLog(prog,1024,nullptr,buf); std::cerr<<"Link error:\n"<<buf<<"\n"; }
    glDeleteShader(vs); glDeleteShader(fs); return prog;
}

void reloadShader(GLuint &shaderProgram,const char* fragPath,const char* vertSrc){
    std::string src=loadFile(fragPath);
    GLuint fs=compileShader(GL_FRAGMENT_SHADER,src.c_str());
    GLint compiled; glGetShaderiv(fs,GL_COMPILE_STATUS,&compiled);
    if(!compiled){ std::cerr<<"Shader not compiled\n"; glDeleteShader(fs); return; }
    GLuint vs=compileShader(GL_VERTEX_SHADER,vertSrc);
    GLuint newProg=linkProgram(vs,fs);
    if(newProg){ glDeleteProgram(shaderProgram); shaderProgram=newProg; std::cout<<"Shader reloaded!\n"; }
}

// ===== MAIN =====
int main(){
    if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO)!=0){ std::cerr<<"SDL_Init failed\n"; return -1; }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION,3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION,3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_Window* win=SDL_CreateWindow("kdhfksjdhf",SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,1280,720,SDL_WINDOW_OPENGL);
    SDL_GLContext ctx=SDL_GL_CreateContext(win);
    if(!gladLoaderLoadGL()){ std::cerr<<"GLAD failed\n"; return -1; }

    std::cout<<"GL "<<glGetString(GL_VERSION)<<"\n";

    SDL_AudioSpec want,have; SDL_zero(want);
    want.freq=SAMPLE_RATE; want.format=AUDIO_S16SYS; want.channels=1; want.samples=1024; want.callback=audioCallback;
    if(SDL_OpenAudio(&want,&have)<0){ std::cerr<<"Audio failed: "<<SDL_GetError()<<"\n"; }
    SDL_PauseAudio(0);

    float quad[]={-1,-1,1,-1,1,1,-1,-1,1,1,-1,1};
    GLuint vao,vbo; glGenVertexArrays(1,&vao); glGenBuffers(1,&vbo);
    glBindVertexArray(vao); glBindBuffer(GL_ARRAY_BUFFER,vbo);
    glBufferData(GL_ARRAY_BUFFER,sizeof(quad),quad,GL_STATIC_DRAW);
    glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,2*sizeof(float),0); glEnableVertexAttribArray(0);

    const char* fragPath="shaders/effect.frag";
    std::string src=loadFile(fragPath);
    GLuint vs=compileShader(GL_VERTEX_SHADER,vertSrc);
    GLuint fs=compileShader(GL_FRAGMENT_SHADER,src.c_str());
    GLuint shaderProgram=linkProgram(vs,fs);

    bool running=true; SDL_Event e;
    while(running){
        while(SDL_PollEvent(&e)){
            if(e.type==SDL_QUIT) running=false;
            if(e.type==SDL_KEYDOWN && e.key.keysym.scancode==SDL_SCANCODE_R) reloadShader(shaderProgram,fragPath,vertSrc);
        }

        int w,h; SDL_GetWindowSize(win,&w,&h);
        glViewport(0,0,w,h); glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);
        float ftime=SDL_GetTicks()*0.001f;

        int step=(int)(ftime/(60.0/bpm/4.0))%16;
        float bass=(patternKick[step]*1.0 + 0.4); if(bass>1.0) bass=1.0;

        glUniform1f(glGetUniformLocation(shaderProgram,"iTime"),ftime);
        glUniform2f(glGetUniformLocation(shaderProgram,"iResolution"),(float)w,(float)h);
        glUniform1f(glGetUniformLocation(shaderProgram,"uBass"),bass);

        glBindVertexArray(vao); glDrawArrays(GL_TRIANGLES,0,6);
        SDL_GL_SwapWindow(win);
    }

    SDL_CloseAudio(); SDL_Quit();
    return 0;
}

