#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <3ds.h>
#include <citro2d.h>
#include "filemanager.hpp"
#include "mii.hpp"

#define MAX_SHOW_FILE_NUM 10

using namespace std;

C2D_TextBuf g_dynamicBuf,g_footer;
u8 *top_fb;
static C2D_SpriteSheet spriteSheet;
bool initErr = false;
char doneMsg[128];
int miiNum;

bool is3dsmiiFile(string s){
	if(s.size() < 7)return false;
	string tmpString = s.substr(s.size() - 7);
    if(tmpString == ".3dsmii" || tmpString == ".3DSMII"){
		return true;
	}
    return false;
}

static void sceneInit(void)
{
	g_dynamicBuf = C2D_TextBufNew(4096);
	g_footer = C2D_TextBufNew(4096);
}

static void sceneExit(void)
{
	// Delete the text buffers
	C2D_TextBufDelete(g_dynamicBuf);
	C2D_TextBufDelete(g_footer);
}

int main()
{
	char *msg,*defaultFooterMsg = " Install Mii /  Install as personal Mii\n Dump Mii / START Exit";
	string defaultDir = "sdmc:/3DSMIIs";
	int index = 0,page,i,j,stat;
	romfsInit();
	C2D_Text entriesString[MAX_SHOW_FILE_NUM];
	C2D_Text curPathString,footer;
	char doneMsg[128];
	memset(doneMsg,0,128);
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
		msg = "Error cannot open sdmc:/3DSMIIs";
		memcpy(doneMsg,msg,strlen(msg));
        goto error;
	}
	error:
	if(!initErr){
        memcpy(doneMsg,defaultFooterMsg,strlen(defaultFooterMsg));
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
	allowSprite.params.pos.x = 8.0f;
	for(i = 0;i < MAX_SHOW_FILE_NUM;i++){
        for(j = 0;j < 3;j++){
            C2D_SpriteFromSheet(&entrySprites[i][j], spriteSheet,2 + j);
		    (entrySprites[i][j]).params.pos.y = (float)(i * 15 + 38);
			(entrySprites[i][j]).params.pos.x = 23.0f;
		}
	}
	// Initialize the scene
	sceneInit();

	// Main loop
	while (aptMainLoop())
	{
		C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
		C2D_TargetClear(bottom, C2D_Color32(0x68, 0xB0, 0xD8, 0xFF));
		C2D_SceneBegin(bottom);
		hidScanInput();
		// Respond to user input
		u32 kDown = hidKeysDown();
		if (kDown & KEY_START)
			break; // break in order to return to hbmenu
        if(!initErr){
		    if(kDown & KEY_A){
				if(fm.entries[index].isDir){
                    if(!fm.OpenPath(fm.getFullPath(index)))index = 0;
				}else if(is3dsmiiFile(fm.entries[index].name)){
                    memset(doneMsg,0,128);
					installMii(fm.getFullPath(index).c_str(),false,doneMsg);
				}
			}else if(kDown & KEY_B){
                if(!fm.Back()){
					index = 0;
					memset(doneMsg,0,128);
		            memcpy(doneMsg,defaultFooterMsg,strlen(defaultFooterMsg));
				}
			}else if(kDown & KEY_Y){
				memset(doneMsg,0,128);
				stat = dumpMii(fm.curPath,doneMsg);
				if(stat == 1){
                    memcpy(doneMsg,defaultFooterMsg,strlen(defaultFooterMsg));
				}else if(stat == 0){
					index = 0;
					fm.Reload();
				}
			}else if(kDown & KEY_X){
				if(is3dsmiiFile(fm.entries[index].name)){
                    memset(doneMsg,0,128);
					installMii(fm.getFullPath(index).c_str(),true,doneMsg);
				}
			}else if(kDown & KEY_UP){
                if(index > 0){
					index--;
					memset(doneMsg,0,128);
		            memcpy(doneMsg,defaultFooterMsg,strlen(defaultFooterMsg));
				}
			}else if(kDown & KEY_DOWN){
				if(index < fm.entries.size() - 1){
					index++;
					memset(doneMsg,0,128);
					memcpy(doneMsg,defaultFooterMsg,strlen(defaultFooterMsg));
				}
			}
			page = index / MAX_SHOW_FILE_NUM;
			C2D_TextBufClear(g_dynamicBuf);
			C2D_TextBufClear(g_footer);
			for(i = 0;i < MAX_SHOW_FILE_NUM;i++){
			    if(page * MAX_SHOW_FILE_NUM + i < fm.entries.size()){
                    C2D_TextParse(&entriesString[i], g_dynamicBuf,fm.entries[page * MAX_SHOW_FILE_NUM + i].name.c_str());
					if((fm.entries[page * MAX_SHOW_FILE_NUM + i]).isDir){
                        C2D_DrawSprite(&entrySprites[i][0]);
					}else if(is3dsmiiFile(fm.entries[page * MAX_SHOW_FILE_NUM + i].name)){
                        C2D_DrawSprite(&entrySprites[i][1]);
					}else{
						C2D_DrawSprite(&entrySprites[i][2]);
					}
			    }else{
			    	C2D_TextParse(&entriesString[i], g_dynamicBuf,"");
			    }
		        C2D_TextOptimize(&entriesString[i]);
			    C2D_DrawText(&entriesString[i], 0,38.0f,(float)((i + 2) * 15 + 8), 0.5f, 0.5f, 0.5f);
		    }
			C2D_TextParse(&curPathString, g_dynamicBuf,fm.curPath.c_str());
			C2D_TextOptimize(&curPathString);
            C2D_DrawText(&curPathString, 0, 8.0f,8.0f, 0.5f, 0.5f, 0.5f);
			if(fm.entries.size() > 0){
				allowSprite.params.pos.y = (float)((index - page * MAX_SHOW_FILE_NUM) * 15 + 38);
				C2D_DrawSprite(&allowSprite);
			}
		}
		C2D_TextParse(&footer, g_footer,doneMsg);
		C2D_TextOptimize(&footer);
		C2D_DrawText(&footer,0,8.0f,202.0f,0.5f, 0.5f, 0.5f);
		C2D_TargetClear(top, C2D_Color32(0x68, 0xB0, 0xD8, 0xFF));
		C2D_SceneBegin(top);
		C2D_DrawSprite(&topSprite);

		C3D_FrameEnd(0);
	}

	// Deinitialize the scene
	sceneExit();
    //free(raw.data);
	C2D_SpriteSheetFree(spriteSheet);
	// Deinitialize the libs
	C2D_Fini();
	C3D_Fini();
	romfsExit();
	gfxExit();
	return 0;
}
