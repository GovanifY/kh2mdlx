#include <stdio.h>
#include <stdlib.h>

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
 * | |     ?????     | | <- unused in KH2, used in KH1
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
    unsigned int unk_off;
    unsigned short mdl_subpart_cnt; 
    unsigned short unk3; 
};

struct mdl_subpart_header {
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
    unsigned int parent;
    unsigned int res2;
    unsigned int unk1;
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

        struct mdl_header *head=malloc(sizeof(struct mdl_header));
        struct mdl_subpart_header *subh=malloc(sizeof(struct mdl_subpart_header));
        head->nmb=3;
        head->bone_cnt=1;
        head->mdl_subpart_cnt=1;

        // write here dummy headers, will need to modify their offsets later
        int off_head = ftell(mdl);
        fwrite(head , 1 , sizeof(struct mdl_header) , mdl);
        int off_subh = ftell(mdl);
        fwrite(subh , 1 , sizeof(struct mdl_subpart_header) , mdl);


        int off_bone = ftell(mdl);
        struct bone_entry *bone=malloc(sizeof(struct bone_entry));
        bone->idx=0;
        bone->parent=-1;
        bone->sca_x=1.0;
        bone->sca_y=1.0;
        bone->sca_z=1.0;
        bone->rot_x=1.0;
        bone->rot_y=1.0;
        bone->rot_z=1.0;
        bone->trans_x=1.0;
        bone->trans_y=1.0;
        bone->trans_z=1.0;
        fwrite(bone , 1 , sizeof(struct bone_entry) , mdl);

        //FILE * dummy_vif = fopen("fish.kh2v", "rb");
        FILE * dummy_vif = fopen("triangle.kh2v", "rb");
        //FILE * dummy_vif = fopen("geosphere.kh2v", "rb");


        int off_vif = ftell(mdl) - 0x90;

        size_t n, m;
        unsigned char buff[8192];
        do {
            n = fread(buff, 1, sizeof buff, dummy_vif);
            if (n) m = fwrite(buff, 1, n, mdl);
            else   m = 0;
        } while ((n > 0) && (n == m));
        

        int off_dma = ftell(mdl);
        // 910
        char end_dma[] = {0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x17, 0x00, 0x00, 0x00, 0x00};
        unsigned short qwc_len=15;
        unsigned short qwc_mat_len=4;
        unsigned short res_unk = 0x3000;
        unsigned int vif_off=0x0;
        fwrite(&qwc_len , 1 , sizeof(qwc_len) , mdl);
        fwrite(&res_unk , 1 , sizeof(res_unk) , mdl);
        fwrite(&off_vif , 1 , sizeof(off_vif) , mdl);
		for (int i=0; i<8;i++){fwrite(empty , 1 , sizeof(empty) , mdl);}
        fwrite(&qwc_mat_len , 1 , sizeof(qwc_len) , mdl);
        fwrite(&res_unk , 1 , sizeof(res_unk) , mdl);
		for (int i=0; i<4;i++){fwrite(empty , 1 , sizeof(empty) , mdl);}
        char stcycl[] = {0x01, 0x01, 0x00, 0x01}; // stcycl 1,1
        fwrite(stcycl , 1 , sizeof(stcycl) , mdl);
        // dma writes after vif, in that case 307
        char qwc_vif_len[] = {11};
        char unpack[] = {0x80, 0x04, 0x6c}; // unpack V4_32
        unsigned short unpack_end=0x6c04;
        fwrite(qwc_vif_len , 1 , sizeof(qwc_vif_len) , mdl);
        fwrite(unpack , 1 , sizeof(unpack) , mdl);
        fwrite(end_dma , 1 , sizeof(end_dma) , mdl);

        

        int off_mat = ftell(mdl);
        unsigned int mat_dummy[] = { 0x01, 0x0, 0x0, 0x0 };
        fwrite(mat_dummy , 1 , sizeof(mat_dummy) , mdl);
        
        // fixing header offsets
        fseek(mdl, off_head, SEEK_SET);
        head->bone_off=off_bone - 0x90;
        fwrite(head , 1 , sizeof(struct mdl_header) , mdl);
        fseek(mdl, off_subh, SEEK_SET);
        subh->DMA_off=off_dma - 0x90;
        subh->mat_off=off_mat - 0x90;
        fwrite(subh , 1 , sizeof(struct mdl_subpart_header) , mdl);

		fclose(mdl);
}


