#include <stdio.h>
#include <stdlib.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>         
#include <assimp/postprocess.h>   

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
	if(argc<3){printf("usage: dae2mdlx test.kh2v model.dae"); return -1;}

		FILE *mdl;
        char empty[] = {0x00};

		mdl=fopen("test.kh2m","wb");

        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile( argv[2],
                                             aiProcess_Triangulate            |
                                             aiProcess_JoinIdenticalVertices  |
                                             aiProcess_SortByPType);
        if( !scene)
        {
            printf("error loading model!: %s", importer.GetErrorString());
            return -1;
        }

        unsigned int mesh_nmb= scene->mNumMeshes;
        printf("Number of meshes: %d\n", mesh_nmb);
        for(int i=0; i<mesh_nmb;i++){
            int vifpkt=0;
            int vertcount=0;
            const aiMesh& mesh = *scene->mMeshes[i];
            int vert_new_order[mesh.mNumVertices];
            // should be enough chars for a lifetime
            char *filename = (char*)malloc(1024*sizeof(char));
            strcat(filename, argv[2]);
            strcat(filename, "_");
            sprintf(filename, "%d", vifpkt);
            
            // we are writing a custom interlaced, bone-supporting obj here,
            // don't assume everything is following the obj standard! 
            FILE *pkt=fopen(filename, "w");
            while(vertcount<mesh.mNumVertices){


                for(int y=0; y<mesh.mNumBones; y++){
                    printf("  Bone: %d, Affecting %d vertices\n", y+1, mesh.mBones[y]->mNumWeights); 
                    fprintf(pkt, "vb %d\n", mesh.mBones[y]->mNumWeights);
                    for(int z=0; z<mesh.mBones[y]->mNumWeights;z++){
                        printf("    Vertex %d\n", mesh.mBones[y]->mWeights[z].mVertexId);
                        vert_new_order[vertcount]=mesh.mBones[y]->mWeights[z].mVertexId+1;
                        vertcount++;
                        fprintf(pkt, "v %f %f %f\n", mesh.mVertices[mesh.mBones[y]->mWeights[z].mVertexId].x,mesh.mVertices[mesh.mBones[y]->mWeights[z].mVertexId].y,mesh.mVertices[mesh.mBones[y]->mWeights[z].mVertexId].z);
                        fprintf(pkt, "vt %f %f\n", mesh.mTextureCoords[0][mesh.mBones[y]->mWeights[z].mVertexId].x, mesh.mTextureCoords[0][mesh.mBones[y]->mWeights[z].mVertexId].y);
                    }
                    
                  }
                printf("~~~~~~~~~~\n");
                for(int y=0; y<mesh.mNumFaces; y++){
                    printf("  Face: %d, 1: %d, 2: %d, 3: %d\n", y+1, mesh.mFaces[y].mIndices[0], mesh.mFaces[y].mIndices[1], mesh.mFaces[y].mIndices[2]); 
                    // if we have all the vertices necessary for this face
                    if(vert_new_order[mesh.mFaces[y].mIndices[0]]!=0 && vert_new_order[mesh.mFaces[y].mIndices[1]]!=0 && vert_new_order[mesh.mFaces[y].mIndices[2]]!=0){
                        fprintf(pkt, "f %d %d %d\n", vert_new_order[mesh.mFaces[y].mIndices[0]], vert_new_order[mesh.mFaces[y].mIndices[1]], vert_new_order[mesh.mFaces[y].mIndices[2]]);
                    }

            }
            /*printf("Mesh: %d, number of vertices: %d, number of bones: %d\n", i+1, mesh.mNumVertices, mesh.mNumBones);
            for(int y=0; y<mesh.mNumVertices;y++){
                printf("  Vertex: %d, x: %f, y: %f, z: %f\n", y+1, mesh.mVertices[y].x, mesh.mVertices[y].y, mesh.mVertices[y].z); 
                printf("  UV: %d, x: %f, y: %f\n", y+1, mesh.mTextureCoords[0][y].x, mesh.mTextureCoords[0][y].y); 
            }
            
            printf("~~~~~~~~~~\n");*/

            }
        }

        // write kh2 dma in-game header
		for (int i=0; i<0x90;i++){fwrite(empty , 1 , sizeof(empty) , mdl);}

        struct mdl_header *head=(mdl_header *)malloc(sizeof(struct mdl_header));
        struct mdl_subpart_header *subh=(mdl_subpart_header *)malloc(sizeof(struct mdl_subpart_header));
        head->nmb=3;
        head->bone_cnt=1;
        head->mdl_subpart_cnt=1;

        // write here dummy headers, will need to modify their offsets later
        int off_head = ftell(mdl);
        fwrite(head , 1 , sizeof(struct mdl_header) , mdl);
        int off_subh = ftell(mdl);
        fwrite(subh , 1 , sizeof(struct mdl_subpart_header) , mdl);


        int off_bone = ftell(mdl);
        struct bone_entry *bone=(bone_entry *)malloc(sizeof(struct bone_entry));
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

        // we do not have a dae parser yet so we take vif packets directly
        FILE * dummy_vif = fopen(argv[1], "rb");

        fseek(dummy_vif, 0x24, SEEK_SET);
        char mat_vif_off;
        fread(&mat_vif_off, 4, 1, dummy_vif);
        fseek(dummy_vif, 0x0, SEEK_END);
        unsigned short vifpkt_len = ftell(dummy_vif)/16;
        fseek(dummy_vif, 0x0, SEEK_SET);

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
        unsigned short qwc_mat_len=4;
        unsigned short res_unk = 0x3000;
        unsigned int mat_idx = 0x0;
        unsigned int vif_off=0x0;
        fwrite(&vifpkt_len , 1 , sizeof(vifpkt_len) , mdl);
        fwrite(&res_unk , 1 , sizeof(res_unk) , mdl);
        fwrite(&off_vif , 1 , sizeof(off_vif) , mdl);
		for (int i=0; i<8;i++){fwrite(empty , 1 , sizeof(empty) , mdl);}
        fwrite(&qwc_mat_len , 1 , sizeof(qwc_mat_len) , mdl);
        fwrite(&res_unk , 1 , sizeof(res_unk) , mdl);
        fwrite(&mat_idx , 1 , sizeof(mat_idx) , mdl);
        unsigned char stcycl[] = {0x01, 0x01, 0x00, 0x01}; // stcycl 1,1
        fwrite(stcycl , 1 , sizeof(stcycl) , mdl);
        
        unsigned char unpack[] = {0x80, 0x04, 0x6c}; // unpack V4_32
        unsigned short unpack_end=0x6c04;
        fwrite(&mat_vif_off , 1 , sizeof(mat_vif_off) , mdl);
        fwrite(unpack , 1 , sizeof(unpack) , mdl);
        fwrite(end_dma , 1 , sizeof(end_dma) , mdl);

        

        int off_mat = ftell(mdl);
        unsigned int mat_dummy[] = { 0x01, 0x00, 0x0, 0x0 };
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
        fclose(dummy_vif);
}


