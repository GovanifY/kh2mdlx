#include <stdio.h>

/*
 * Here is an high-level overview of the MDLX file format
 * 
 *          BAR   
 * |-------------------|
 * |        0x04       |
 * | |---------------| |
 * | |               | |
 * | |     MDL_H     | |
 * | |   |-------|   | |
 * | |   |MDL_P_H|   | |
 * | |   |MDL_P_H|   | |
 * | |   | ...   |   | |
 * | |   |-------|   | |
 * | |     ?????     | |
 * | |     MDL_P     | |
 * | | |-----------| | |
 * | | |   BONES   | | |
 * | | | |-------| | | |
 * | | | |  BONE | | | |
 * | | | |  BONE | | | |
 * | | | |  ...  | | | |
 * | | | |-------| | | |
 * | | |    SUBP   | | |
 * | | | |-------| | | |
 * | | | | VIFPKT| | | |
 * | | | | VIFPKT| | | |
 * | | | |  ...  | | | |
 * | | | |-------| | | |
 * | | |    DMA    | | |
 * | | | |-------| | | |
 * | | | |DMA_VIF| | | |
 * | | | | MAT_I | | | |
 * | | | | MAT_I | | | |
 * | | | |  ...  | | | |
 * | | | |DMA_VIF| | | |
 * | | | |  ...  | | | |
 * | | | --------- | | |
 * | | |    MAT    | | |
 * | | | |-------| | | |
 * | | | | MAT_I | | | |
 * | | | | MAT_I | | | |
 * | | | |  ...  | | | |
 * | | | |-------| | | |
 * | | |-----------| | |
 * | |     MDL_P     | |
 * | | |-----------| | |
 * | | |   BONES   | | |
 * | | | |-------| | | |
 * | | | |  BONE | | | |
 * | | | |  BONE | | | |
 * | | | |  ...  | | | |
 * | | | |-------| | | |
 * | | |    SUBP   | | |
 * | | | |-------| | | |
 * | | | | VIFPKT| | | |
 * | | | | VIFPKT| | | |
 * | | | |  ...  | | | |
 * | | | |-------| | | |
 * | | |    DMA    | | |
 * | | | |-------| | | |
 * | | | |DMA_VIF| | | |
 * | | | | MAT_I | | | |
 * | | | | MAT_I | | | |
 * | | | |  ...  | | | |
 * | | | |DMA_VIF| | | |
 * | | | |  ...  | | | |
 * | | | --------- | | |
 * | | |    MAT    | | |
 * | | | |-------| | | |
 * | | | | MAT_I | | | |
 * | | | | MAT_I | | | |
 * | | | |  ...  | | | |
 * | | | |-------| | | |
 * | | |-----------| | |
   | |      ...      | |
 * | |---------------| |
 * |                   |
 * |-------------------|
 * |        0x07       |
 * |    |---------|    |
 * |    |  TIM_0  |    |
 * |    |  TIM_1  |    |
 * |    |  .....  |    |
 * |    |---------|    |
 * |                   |
 * |-------------------|
 * |       0x17        |
 * |-------------------|
 * 
 *
 * 0x04 is the model. It contains a model header, followed by a model part per
 * texture, each including their list of bones, subpart to render by the VU1,
 * DMA tags to refer to the subparts and matrices. For more informations refer
 * to obj2kh2v.
 *
 * 0x07 is the texture container: it contains several textures under the TIM2
 * format. For more information refer to the tool building it.
 *
 * 0x17 is the object definition: contains collision, which bone lock-on target
 * is on, etc */

struct mdl_header {
    unsigned int nmb;
    unsigned int res1;
    unsigned int res2;
    unsigned int next_mdl_header; 
    unsigned short bone_cnt;
    unsigned short unk1;
    unsigned int bone_off;
    unsigned int unk2;
    unsigned short mdl_part_cnt; 
    unsigned short unk3; 
};

struct mdl_part_header {
    unsigned int unk1;
    unsigned int texture_idx;
    unsigned int unk2;
    unsigned int unk3;
    unsigned int DMA_off;
    unsigned int mat_off;
    unsigned int unk4;
    unsigned int unk5;
};

struct bone_entry {
    unsigned short idx;
    unsigned short res1;
    short parent;
    unsigned short res2;
    float unk1;
    float unk2;
    float sca_x;
    float sca_y;
    float sca_z;
    float sca_w;
    float rot_x;
    float rot_y;
    float rot_z;
    float rot_w;
    float trans_x;
    float trans_y;
    float trans_z;
    float trans_w;
};

struct DMA {
    unsigned short vif_len;
    unsigned short res1;
    unsigned int vif_off;
    unsigned int vif_inst1;
    unsigned int vif_inst2;
};

int main(int argc, char* argv[]){
	printf("dae2mdlx\n--- Early rev, don't blame me if it eats your cat\n\n");
	if(argc<2){printf("usage: dae2mdlx model.dae"); return -1;}

		FILE *mdl;
        char empty[] = {0x00};

		mdl=fopen("test.kh2m","wb");

        // write kh2 dma in-game header
		for (int i=0; i<0x90;i++){fwrite(empty , 1 , sizeof(empty) , mdl);}

        fwrite(&(int 3),sizeof(int),1, mdl);


		fclose(mdl);
}


