#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "rip_snds_txt_levls.h"
#include "utils.h"

/* local functions declarations */
static void rip_texts_rpl(const char path[], FILE *rplFile_fp);


void rip_snds_txt_levls(enum RPLtype_e RPLtype){
    extern char *dirs[];

    static const char *rplFiles[5] = {  "BRIEF.RPL",
                                        "SOUNDS.RPL",
                                        "MISSION.RPL",
                                        "TEXTS.RPL",
                                        "LEVELS.RPL"
    };

    static const char *extensions[5] = {    ".wav",
                                            ".wav",
                                            ".txt",
                                            ".txt",
                                            ".map"
    };


    FILE *rplFile_fp, *rplEntry_fp;
    rpl_header_t rpl_header;
    char path[32];
    char *namePtr;
    BYTE *entry_buf;

    unsigned int i;

    sprintf(path, "%s%s/", dirs[BASE_DIR], dirs[RPLtype + 1]);

    if((rplFile_fp = fopen(rplFiles[RPLtype], "rb")) == NULL){
        perror("Couldn't open RPL file");
        exit(EXIT_FAILURE);
    }

    namePtr = path + strlen(path);

    /* TEXTS.RPL is a mere txt file without any internal structure whatsoever; treat it separately */
    if(RPLtype == TEXTS_RPL){
        strcpy(namePtr, "TEXTS.txt");
        rip_texts_rpl(path, rplFile_fp);
        fclose(rplFile_fp);
        return;
    }

    entry_buf = RPL_header_init(&rpl_header, rplFile_fp);

    /* save the entries */
    for(i = 0; i < rpl_header.entry_count; ++i){
        strncpy(namePtr, rpl_header.rpl_entryrecord[i].entry_name, sizeof(rpl_header.rpl_entryrecord[0].entry_name));
        namePtr[8] = '\0';
        strcat(namePtr, extensions[RPLtype]);

        if((rplEntry_fp = fopen(path, "wb")) == NULL){
            fprintf(stderr, "Couldn't create %s", path);
            continue;
        }

        fseek(rplFile_fp, rpl_header.rpl_entryrecord[i].entry_offset, SEEK_SET);
        fread(entry_buf, 1, rpl_header.rpl_entryrecord[i].entry_size, rplFile_fp);
        fwrite(entry_buf, 1, rpl_header.rpl_entryrecord[i].entry_size, rplEntry_fp);
        fclose(rplEntry_fp);
    }

    fclose(rplFile_fp);
    free(entry_buf);
    free(rpl_header.rpl_entryrecord);
}



/* local functions definitions */
static void rip_texts_rpl(const char path[], FILE *rplFile_fp){
    BYTE buf[512];
    FILE *texts_fp;
    size_t bytes_read;

    if((texts_fp = fopen(path, "wb")) == NULL){
            fprintf(stderr, "Couldn't create %s", path);
            return;
        }

        while((bytes_read = fread(buf, 1, sizeof(buf), rplFile_fp) ))
            fwrite(buf, 1, bytes_read, texts_fp);

        fclose(texts_fp);
}


