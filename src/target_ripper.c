#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "makedir.h"
#include "utils.h"
#include "rip_snds_txt_levls.h"
#include "rip_data.h"
#include "rip_post.h"
#include "rip_md.h"
#include "rip_tt.h"

#define clear_stdin() while(getchar() != '\n')

/* local functions declarations */
static int isInDataFolderCheck(void);
static mipmapRipMode_t mipmapRipModeSelect(void);
static void make_RPL_dirs(void);

int main(void){
    mipmapRipMode_t mipmapRipMode;

    puts("\t\tTarget resources ripper by Yagotzirck\n");

    /* Ask the user if the exe is placed in Target's data directory */
    if(!isInDataFolderCheck())
        return 1;

    clear_stdin();

    mipmapRipMode = mipmapRipModeSelect();

    clear_stdin();

    /* Create directories */
    puts("\nCreating directories");
    make_RPL_dirs();

    /* BRIEF.RPL: polish speech played when choosing missions */
    puts("Ripping BRIEF.RPL");
    rip_snds_txt_levls(BRIEF_RPL);

    /* SOUNDS.RPL: do I really need to explain what the contents of this are? */
    puts("Ripping SOUNDS.RPL");
    rip_snds_txt_levls(SOUNDS_RPL);

    /* MISSION.RPL: Text files, mostly english subtitles */
    puts("Ripping MISSION.RPL");
    rip_snds_txt_levls(MISSION_RPL);

    /* TEXTS.RPL: file that assigns strings to variables (maybe for internationalization?) */
    puts("Ripping TEXTS.RPL");
    rip_snds_txt_levls(TEXTS_RPL);

    /* LEVELS.RPL: supposedly game's map data */
    puts("Ripping LEVELS.RPL");
    rip_snds_txt_levls(LEVELS_RPL);

    /* DATA.RPL: fonts, images, sprites */
    puts("Ripping DATA.RPL");
    rip_data();

    /* POST.RPL: NPC/enemies' sprites */
    puts("Ripping POST.RPL");
    rip_post();

    /* MD.RPL: flats and textures */
    puts("Ripping MD.RPL");
    rip_md(mipmapRipMode);

    /* TT.RPL: doors' textures, mostly */
    puts("Ripping TT.RPL");
    rip_tt();



    puts("\nTarget has been successfully ripped.");
    return 0;
}



/* local functions definitions */
static int isInDataFolderCheck(void){

    for(;;){
        printf("Is this exe placed on Target \"\\data\" directory? (Y/n) ");
        switch(toupper( getchar() ) ){
            case 'Y':
                return 1;

            case 'N':
                puts("Place this exe on data directory then launch it from there.");
                return 0;

            default:
                puts("Invalid answer");
                clear_stdin();
        }
    }
}


static mipmapRipMode_t mipmapRipModeSelect(void){

    for(;;){
        printf("Rip (A)ll mipmap textures/flats, or only the (B)iggest versions? ");
        switch(toupper( getchar() ) ){
            case 'A':
                return MIP_RIP_ALL;

            case 'B':
                return MIP_RIP_BIGGEST;

            default:
                puts("Invalid answer");
                clear_stdin();
        }
    }
}

static void make_RPL_dirs(void){
    extern char *dirs[];
    char path[21];
    char *basedir_ptr;

    strcpy(path, dirs[BASE_DIR]);
    basedir_ptr = path + strlen(path);

    makeDir(path);

    strcpy(basedir_ptr, dirs[BRIEF_DIR]);
    makeDir(path);

    strcpy(basedir_ptr, dirs[DATA_DIR]);
    makeDir(path);

    strcpy(basedir_ptr, dirs[LEVELS_DIR]);
    makeDir(path);

    strcpy(basedir_ptr, dirs[MD_DIR]);
    makeDir(path);

    strcpy(basedir_ptr, dirs[MISSION_DIR]);
    makeDir(path);

    strcpy(basedir_ptr, dirs[POST_DIR]);
    makeDir(path);

    strcpy(basedir_ptr, dirs[SOUNDS_DIR]);
    makeDir(path);

    strcpy(basedir_ptr, dirs[TEXTS_DIR]);
    makeDir(path);

    strcpy(basedir_ptr, dirs[TT_DIR]);
    makeDir(path);
}
