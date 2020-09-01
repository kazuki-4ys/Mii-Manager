#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <3ds.h>
#include <citro2d.h>
#include <string>
#include "filemanager.hpp"
#include "mii.hpp"

#define MAX_SHOW_FILE_NUM 10
#define LINE_HEIGHT 15
#define SCREEN_GAP 8

using namespace std;

static C2D_SpriteSheet spriteSheet;
bool initErr = false;
int miiNum;

class Text{
	C2D_TextBuf g_buf;
	C2D_Text c_text;
	public:
	Text(string);
	Text(void);
	void ChangeText(string);
	void Print(float,float);
	~Text(void);
};

Text::Text(string s){
	g_buf = C2D_TextBufNew(4096);
	ChangeText(s);
}

Text::Text(void){
	g_buf = C2D_TextBufNew(4096);
	ChangeText("");
}

void Text::ChangeText(string s){
	C2D_TextBufClear(g_buf);
	C2D_TextParse(&c_text,g_buf,s.c_str());
	C2D_TextOptimize(&c_text);
}

void Text::Print(float x,float y){
	C2D_DrawText(&c_text, 0,x,y, 0.5f, 0.5f, 0.5f);
}

Text::~Text(void){
	C2D_TextBufDelete(g_buf);
}

bool is3dsmiiFile(string s){
	if(s.size() < 7)return false;
	string tmpString = s.substr(s.size() - 7);
    if(tmpString == ".3dsmii" || tmpString == ".3DSMII"){
		return true;
	}
    return false;
}

void appExit(void){
	C2D_SpriteSheetFree(spriteSheet);
	C2D_Fini();
	C3D_Fini();
	romfsExit();
	gfxExit();
	exit(0);
}

int main()
{
	string doneMsg;
	string initErrMsg;
	string defaultFooterMsg = " Install Mii /  Install as personal Mii\n Dump Mii / START Exit";
	string defaultDir = "sdmc:/3DSMIIs";
	int index = 0,page,i,j,stat;
	romfsInit();
	Text entriesString[MAX_SHOW_FILE_NUM];
	Text curPathString(defaultDir),footer(defaultFooterMsg);
	C2D_Sprite topSprite;
	C2D_Sprite allowSprite;
	C2D_Sprite entrySprites[MAX_SHOW_FILE_NUM][3];
	DIR*pdir = opendir(defaultDir.c_str());
	if(pdir){
        closedir(pdir);
	}else{
		mkdir(defaultDir.c_str(),0777);
	}
	FileManager fm(defaultDir);
	if(!fm.valid){
        initErr = true;
		initErrMsg = "Error:cannot open sdmc:/3DSMIIs";
        goto error;
	}
	error:
	if(initErr){
		initErrMsg += "\nPress START to exit";
        footer.ChangeText(initErrMsg);
	}

	// Initialize the libs
	gfxInitDefault();
	C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
	C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
	C2D_Prepare();

	// Create screen
	C3D_RenderTarget* bottom = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);
	C3D_RenderTarget* top = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
	// Load graphics
	spriteSheet = C2D_SpriteSheetLoad("romfs:/gfx/sprites.t3x");
	if (!spriteSheet) svcBreak(USERBREAK_PANIC);
    C2D_SpriteFromSheet(&topSprite, spriteSheet,0);
	C2D_SpriteFromSheet(&allowSprite, spriteSheet,1);
	allowSprite.params.pos.x = (float)SCREEN_GAP;
	for(i = 0;i < MAX_SHOW_FILE_NUM;i++){
        for(j = 0;j < 3;j++){
            C2D_SpriteFromSheet(&entrySprites[i][j], spriteSheet,2 + j);
		    (entrySprites[i][j]).params.pos.y = (float)(i * LINE_HEIGHT + SCREEN_GAP + LINE_HEIGHT * 2);
			(entrySprites[i][j]).params.pos.x = (float)(SCREEN_GAP + LINE_HEIGHT);
		}
	}

	// Main loop
	while (aptMainLoop())
	{
		C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
		C2D_TargetClear(bottom, C2D_Color32(0x68, 0xB0, 0xD8, 0xFF));
		C2D_SceneBegin(bottom);
		hidScanInput();
		// Respond to user input
		u32 kDown = hidKeysDown();
		if (kDown & KEY_START)appExit();
        if(!initErr){
		    if((kDown & KEY_A) && (fm.entries.size() > 0)){
				if(fm.entries[index].isDir){
                    if(!fm.OpenPath(fm.getFullPath(index))){
						index = 0;
						curPathString.ChangeText(fm.curPath);
					}
				}else if(is3dsmiiFile(fm.entries[index].name)){
					footer.ChangeText(installMii(fm.getFullPath(index).c_str(),false));
				}
			}else if(kDown & KEY_B){
                if(!fm.Back()){
					footer.ChangeText(defaultFooterMsg);
					curPathString.ChangeText(fm.curPath);
					index = 0;
				}
			}else if(kDown & KEY_Y){
				stat = dumpMii(fm.curPath,&doneMsg);
                footer.ChangeText(defaultFooterMsg);
				if(stat == 0){
					index = 0;
					fm.Reload();
				}
				if(stat != 1){
					footer.ChangeText(doneMsg);
				}
			}else if((kDown & KEY_X) && (fm.entries.size() > 0)){
				if(is3dsmiiFile(fm.entries[index].name)){
					footer.ChangeText(installMii(fm.getFullPath(index).c_str(),true));
				}
			}else if(kDown & KEY_UP){
                if(index > 0){
					index--;
					footer.ChangeText(defaultFooterMsg);
				}
			}else if(kDown & KEY_DOWN){
				if(index < fm.entries.size() - 1){
					index++;
					footer.ChangeText(defaultFooterMsg);
				}
			}
			page = index / MAX_SHOW_FILE_NUM;
			for(i = 0;i < MAX_SHOW_FILE_NUM;i++){
			    if(page * MAX_SHOW_FILE_NUM + i < fm.entries.size()){
                    entriesString[i].ChangeText(fm.entries[page * MAX_SHOW_FILE_NUM + i].name);
					if((fm.entries[page * MAX_SHOW_FILE_NUM + i]).isDir){
                        C2D_DrawSprite(&entrySprites[i][0]);
					}else if(is3dsmiiFile(fm.entries[page * MAX_SHOW_FILE_NUM + i].name)){
                        C2D_DrawSprite(&entrySprites[i][1]);
					}else{
						C2D_DrawSprite(&entrySprites[i][2]);
					}
			    }else{
			    	entriesString[i].ChangeText("");
			    }
		        entriesString[i].Print((float)(LINE_HEIGHT * 2 + SCREEN_GAP),(float)((i + 2) * LINE_HEIGHT + SCREEN_GAP));
		    }
			if(fm.entries.size() > 0){
				allowSprite.params.pos.y = (float)((index - page * MAX_SHOW_FILE_NUM) * LINE_HEIGHT + LINE_HEIGHT * 2 + SCREEN_GAP);
				C2D_DrawSprite(&allowSprite);
			}
			curPathString.Print((float)SCREEN_GAP,(float)SCREEN_GAP);
			footer.Print((float)SCREEN_GAP,202.0f);
		}else{
			footer.Print((float)SCREEN_GAP,100.0f);
		}
		C2D_TargetClear(top, C2D_Color32(0x68, 0xB0, 0xD8, 0xFF));
		C2D_SceneBegin(top);
		C2D_DrawSprite(&topSprite);

		C3D_FrameEnd(0);
	}
	return 0;
}
