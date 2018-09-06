#ifndef RIP_SOUNDS_H
#define RUP_SOUNDS_H

enum RPLtype_e{ BRIEF_RPL,
                SOUNDS_RPL,
                MISSION_RPL,
                TEXTS_RPL,
                LEVELS_RPL
};

void rip_snds_txt_levls(enum RPLtype_e RPLtype);

#endif /* RIP_SOUNDS_H */
