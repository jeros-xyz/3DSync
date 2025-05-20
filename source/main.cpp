#include <stdio.h>
#include <malloc.h>
#include <dirent.h>
#include <string>
#include <vector>
#include <iostream>

#include <3ds.h>

#include <curl/curl.h>

#include "libs/inih/INIReader/INIReader.h"
#include "modules/dropbox.h"
#include "features/uploader.h"
#include "features/downloader.h"

bool componentsInit(){
    bool result = true;
    gfxInitDefault();

    consoleInit(GFX_BOTTOM, NULL);
    printf(CONSOLE_RED "\n 3DSync " VERSION_STRING " by Kyraminol feat. Jeros" CONSOLE_RESET);
    printf("\n\n\n\n\n\n  Sync your saves with another 3DS,\n   a PC or even a cloud.");
    printf("\n\n\n\n\n\n Commit: " CONSOLE_BLUE REVISION_STRING CONSOLE_RESET);

    consoleInit(GFX_TOP, NULL);
    printf("Initializing components...\n\n");
    
    APT_SetAppCpuTimeLimit(30);
    cfguInit();
    romfsInit();
    pxiDevInit();
    amInit();
    acInit();

    u32* socketBuffer = (u32*)memalign(0x1000, 0x100000);
    if(socketBuffer == NULL){printf("Failed to create socket buffer.\n\n"); result = false;}
    if(socInit(socketBuffer, 0x100000)){printf("socInit failed.\n\n"); result = false;}

    httpcInit(0);
    sslcInit(0);
    return result;
}

void componentsExit(){
    sslcExit();
    httpcExit();
    socExit();
    acExit();
    pxiDevExit();
    romfsExit();
    cfguExit();
    gfxExit();
}


int main(int argc, char** argv){
    if(!componentsInit()) componentsExit();

    try {
        //Init inputs values
        INIReader reader("/3ds/3DSync/3DSync.ini");
        if (reader.ParseError() < 0) {
            throw "Can't load configuration\n";
        } else {
            std::string authorizationCode = reader.Get("Dropbox", "AuthorizationCode", "false");
            std::string refreshToken = reader.Get("Dropbox", "RefreshToken", "false");
            std::map<std::string, std::string> values = reader.GetValues();

            if (!(((authorizationCode.length()) && (refreshToken.length()) && (values.size() > 0)))) {
                throw "Configuration Missing\n";
            }
            
            Dropbox * dropbox = new Dropbox(authorizationCode, refreshToken);            
            dropbox->initDropbox();

            Uploader * uploader = new Uploader(dropbox, values);
            Downloader * downloader = new Downloader(dropbox);

            printf("\nPress START to exit...\n");
            printf("\n  Push A button to upload\n");
            printf("\n  Push B button to Download\n\n");

            while (aptMainLoop()){
                hidScanInput();
                u32 kDown = hidKeysDown();
                if (kDown & KEY_START) break;
                if (kDown & KEY_A) uploader->makeUpload();
                if (kDown & KEY_B) downloader->makeDownload("", "/");
                gfxFlushBuffers();
                gfxSwapBuffers();
                gspWaitForVBlank();
            }
        }
    } catch (std::exception& e) {
        printf(e.what());
    }

    componentsExit();
    return 0;
}
