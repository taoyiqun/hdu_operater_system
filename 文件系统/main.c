#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#define END             0xffff  //fat����ռ��
#define FREE            0x0000  //fat����δռ��
#define MAX_OPENFILE    10  //�����ļ�������
#define PATH_MAX_LENGTH 100  //·����󳤶�
#define NAME_MAX_LENGTH 15  //�ļ�����󳤶�
#define PATH            "./sys"  //�ļ�ϵͳд���ļ�.sys��
#define MODIFIED '1'     //���ļ�����޸�
#define UNMODEFIED '0'  //���ļ�����δ�޸�
#define ASSIGNED '1'   //���ļ����ռ��
#define UNASSIGNED '0' //���ļ�����ͷ�
#define DICTORY '0'  //fcbΪ�ļ���
#define DATA '1'   //fcbΪ�ļ�
#define EMPTY '0'  //�ļ����ļ���Ϊ��
#define UNEMPTY '1'//�ļ����ļ��в�Ϊ��
typedef struct FCB {
    char filename[8];//�ļ���
    char exname[4];//��չ��
    unsigned char attribute;//��ʶλ DATA('1')Ϊ�����ļ� DICTORY('0')Ϊ�ļ���
    struct tm time_info;//����ʱ��
    unsigned short first;//��ʼ��
    unsigned long length;//ռ�ô�С(��BLOCK_SIZE����)
    char free;//��ʶλ EMPTY('0')��ʾ�ļ�Ϊ�� UNEMPTY('1')��ʾ�ļ���Ϊ��
    int len;//�ļ�ʵ�ʳ���
} fcb;

typedef struct FAT {
    unsigned short id;
} fat;

typedef struct USEROPEN {
    fcb open_fcb;//�򿪵��ļ���Ŀ����Ϣ
    char dir[80];//����·��
    int count;//��дָ��
    char fcb_state;//��ʶ�Ƿ��޸� MODIFIED('1')��ʾ���޸� UNMODIFIED('0')��ʾδ���޸�
    char topenfile;//��ʶ�Ƿ�ռ�� ASSIGNED('1')��ʾ��ռ�� UNASSIGNED('0')��ʾδ��ռ��
} useropen;

typedef struct BLOCK0 {
    int BLOCK_SIZE;//�������̿��С
    int BLOCK_NUM;//���̿����
    int SIZE;//�ļ�ϵͳ�ܴ�С
    int FAT_NUM;//����fat����Ҫ�Ĵ��̿���
    unsigned short root_start;//��Ŀ¼��ʼ����
    unsigned char *start_block;//��������ʼָ��
} block0;

unsigned char *myvhard;//�ļ�ϵͳ��ʼλ��ָ��
useropen openfile_list[MAX_OPENFILE];//���ļ���Ŀ��
int curdir;//��ǰ���ļ���Ŀ��
char current_dir[80];//��ǰ·��
unsigned char *startp;//��������ʼָ��
int BLOCK_SIZE;//�������̿��С
int BLOCK_NUM;//���̿����
int SIZE;//�ļ�ϵͳ�ܴ�С
int FAT_NUM;//����һ��fat����Ҫ�Ĵ��̿���
int ROOT_START;//��Ŀ¼��ʼ����

void openfile_set(int setdir,fcb *open_fcb, char dir[],int count, char fcbstate, char topenfile);
void fcb_set(fcb *f, const char *filename, const char *exname, unsigned char attribute, unsigned short first,
             unsigned long length, char ffree, int len);
void fcb_copy(fcb *dest, fcb *src);
int do_read(int des_fd,int len,char *text);
void format();
void fat_format();
int get_free();
int fat_allocate(unsigned short first, unsigned short length);
//ȡ��ʵ�ʵ�ַ
void *get_fact_path(char *fact_path, const char *path);
fcb *find_fcb_r(char *token, int start,int *cnt);
fcb *find_fcb(const char *path, int *cnt);
void init_folder(int first, int second);
void reclaim_space(int first);
void reclaim_space_fat(int cnt);
void do_write(int des_fd,char *str, int len,int mode);
void startsys() {
    FILE *fp;
    int i;
    if ((fp = fopen(PATH, "r")) != NULL) {
        //���ȶ�ȡ�����飬ȷ�����̿��С
        block0 *init_block = (block0 *) malloc(sizeof(block0));
        memset(init_block, 0, sizeof(block0));
        fread(init_block, sizeof(block0), 1, fp);
        BLOCK_SIZE = init_block->BLOCK_SIZE;
        SIZE = init_block->SIZE;
        BLOCK_NUM = init_block->BLOCK_NUM;
        FAT_NUM = init_block->FAT_NUM;
        ROOT_START = init_block->root_start;
        fclose(fp);
        fp = fopen(PATH, "r");
        myvhard = (unsigned char *) malloc(SIZE);
        memset(myvhard, 0, SIZE);
        fread(myvhard, SIZE, 1, fp);
        fclose(fp);
    } else {
        printf("�ļ�ϵͳδ��װ������format������װ�ļ�ϵͳ\n");
        format();
    }
    //��ʼ���û��򿪱�
    //��һ���ʼ��Ϊ��Ŀ¼
    openfile_set(0,((fcb *) (myvhard + (1+FAT_NUM*2) * BLOCK_SIZE)),"\\",0,UNMODEFIED,ASSIGNED);
    curdir = 0;//��ǰĿ¼
    //��������Ϊ��
    fcb *empty = (fcb *) malloc(sizeof(fcb));//����һ���յ�FCB
    fcb_set(empty, "\0", "\0", DICTORY, 0, 0, EMPTY,0);
    for (i = 1; i < MAX_OPENFILE; i++) {
        openfile_set(i,empty,"\0",0,UNMODEFIED,UNASSIGNED);
    }
    //���õ�ǰĿ¼
    strcpy(current_dir, openfile_list[curdir].dir);
    startp = ((block0 *) myvhard)->start_block;
    free(empty);
}
void format() {
    int i;
    int first;
    FILE *fp;
    block0 *init_block = (block0 *) malloc(sizeof(block0));
    unsigned char *ptr;
    printf("������ÿ����̿��С\n");
    scanf("%d",&init_block->BLOCK_SIZE);
    printf("��������̿����\n");
    scanf("%d",&init_block->BLOCK_NUM);
    getchar();
    init_block->SIZE = init_block->BLOCK_NUM * init_block->BLOCK_SIZE;
    SIZE = init_block->SIZE;
    BLOCK_SIZE = init_block->BLOCK_SIZE;
    BLOCK_NUM = init_block->BLOCK_NUM;
    init_block->FAT_NUM = (int) (((init_block->BLOCK_NUM)* sizeof(fat))/init_block->BLOCK_SIZE)+1;
    FAT_NUM = init_block->FAT_NUM;
    init_block->root_start = 1+2*FAT_NUM;
    ROOT_START = init_block->root_start;
    init_block = realloc(init_block,init_block->SIZE);
    ptr = (unsigned char *) init_block;
    myvhard = (unsigned char *) init_block;
    init_block->start_block = (unsigned char *) (init_block + BLOCK_SIZE * (FAT_NUM * 2 + 1 +2));
    ptr = ptr + BLOCK_SIZE;
    //��ʼ��FAT
    fat_format();
    //����ռ���������FAT1 FAT2��
    //��һ��get_freeδ���
    fat_allocate(get_free(),1);
    fat_allocate(get_free(), FAT_NUM);
    fat_allocate(get_free(), FAT_NUM);
    ptr += BLOCK_SIZE * 2*FAT_NUM;
    //����ռ����Ŀ¼
    fcb *root = (fcb *) ptr;
    first = get_free();
    if (first == -1){
        fprintf(stderr, "ϵͳ���޿��д��̿�\nformatʧ��\n");
        exit(-1);
    }
    if(fat_allocate(first, 2) == -1){
        fprintf(stderr, "ϵͳ���޿��д��̿�\nformatʧ��\n");
        exit(-1);
    }
    fcb_set(root, ".", "dic", DICTORY, first, BLOCK_SIZE , UNEMPTY,0);
    root++;
    fcb_set(root, "..", "dic", DICTORY, first, BLOCK_SIZE, UNEMPTY,0);
    root++;
    // ��ʼ����Ŀ¼��ʣ��ı���
    for (i = 2; i < BLOCK_SIZE / sizeof(fcb); i++, root++) {
        root->free = EMPTY;
    }
    root = (fcb *)(myvhard + BLOCK_SIZE *(2+2*FAT_NUM));
    for (i = 0; i < BLOCK_SIZE / sizeof(fcb); i++, root++) {
        root->free = EMPTY;
    }
    //д���ļ�
    fp = fopen(PATH, "w");
    fwrite(myvhard, SIZE, 1, fp);
    fclose(fp);
}

//fcb����
void fcb_set(fcb *f, const char *filename, const char *exname, unsigned char attribute, unsigned short first,
             unsigned long length, char ffree, int len) {
    time_t *now = (time_t *) malloc(sizeof(time_t));
    struct tm *time_info;
    time(now);
    time_info = localtime(now);
    memset(f->filename, 0, 8);
    memset(f->exname, 0, 4);
    strncpy(f->filename, filename, 7);
    strncpy(f->exname, exname, 3);
    f->attribute = attribute;
    memcpy(&f->time_info,time_info, sizeof(struct tm));
    f->first = first;
    f->length = length;
    f->free = ffree;
    f->len = len;
    free(now);
}
//��������
void openfile_set(int setdir,fcb *open_fcb, char dir[],int count, char fcbstate, char topenfile){
    fcb_copy(&openfile_list[setdir].open_fcb, open_fcb);
    strcpy(openfile_list[setdir].dir, dir);
    openfile_list[setdir].count = count;//��дָ��Ϊ0
    openfile_list[setdir].fcb_state = fcbstate;
    openfile_list[setdir].topenfile = topenfile;
}
//fcb����
void fcb_copy(fcb *dest, fcb *src) {
    memset(dest->filename, '\0', 8);
    memset(dest->exname, '\0', 3);
    strcpy(dest->filename, src->filename);
    strcpy(dest->exname, src->exname);
    memcpy(&dest->time_info, &src->time_info, sizeof(struct tm));
    dest->attribute = src->attribute;
    dest->first = src->first;
    dest->length = src->length;
    dest->free = src->free;
    dest->len = src->len;
}
//FAT��ʼ��
void fat_format(){
    fat *fat0 = (fat *) (myvhard + BLOCK_SIZE);
    fat *fat1 = (fat *) (myvhard + BLOCK_SIZE * (FAT_NUM + 1));
    int i;
    for (i = 0; i < BLOCK_NUM; i++, fat0++, fat1++) {
        fat0->id = FREE;
        fat1->id = FREE;
    }
}
//��ȡ���п� ���򷵻�-1
int get_free() {
    unsigned char *ptr = myvhard;
    fat *fat0 = (fat *) (ptr + BLOCK_SIZE);
    fat *fat1 = (fat *) (ptr + BLOCK_SIZE * (1+FAT_NUM));
    int i ;
    for (i = 0; i < BLOCK_NUM; i++, fat0++,fat1++) {
        if (fat0->id == FREE){
            fat0->id = END;
            return i;
        }
    }
    return -1;
}
//fat�����ռ�
int fat_allocate(unsigned short first, unsigned short length){
    fat *fat0 = (fat *) (myvhard + BLOCK_SIZE);
    fat *fat1 = (fat *) (myvhard + BLOCK_SIZE * FAT_NUM + BLOCK_SIZE);
    int i,j;
    int allocated;
    //������ʼλ��
    fat0 = fat0 + first;
    for (i = 0; i < length-1; i++) {
        allocated = get_free();
        if(allocated != -1){
            fat0->id = allocated;
        } else{
            fprintf(stderr, "ϵͳ���޿��д��̿�\n");
            //���˲���
            fat0 = (fat *) (myvhard + BLOCK_SIZE);
            fat1 = (fat *) (myvhard + BLOCK_SIZE * (FAT_NUM + 1));
            for (j = 0; j < BLOCK_NUM; j++, fat0++, fat1++) {
                fat0->id = fat1->id;
            }
            return -1;
        }
        fat0 = (fat *) (myvhard + BLOCK_SIZE);
        fat0 = fat0 + allocated;
    }
    fat0->id = END;
    fat0 = (fat *) (myvhard + BLOCK_SIZE);
    fat1 = (fat *) (myvhard + BLOCK_SIZE * (FAT_NUM + 1));
    for (j = 0; j < BLOCK_NUM; j++, fat0++, fat1++) {
        fat1->id = fat0->id;
    }
    return 0;
}
void get_fullname(char *fullname, fcb *fcb) {
    memset(fullname, '\0', NAME_MAX_LENGTH);
    strcat(fullname, fcb->filename);
    if (fcb->attribute == DATA) {
        strncat(fullname, ".", 2);
        strncat(fullname, fcb->exname, 4);
    }
}
int get_useropenfd(){
    int i;
    for (i = 0; i < MAX_OPENFILE; i++) {
        if (openfile_list[i].topenfile == UNASSIGNED) {
            return i;
        }
    }
    return -1;
}
//ȡ��ʵ�ʵ�ַ
void *get_fact_path(char *fact_path, const char *path) {
    //�����ͷΪ\���ǴӸ�Ŀ¼��ʼ,������ʵ�ʵ�ַ
    if (path[0] == '\\') {
        strcpy(fact_path, path);
        return 0;
    }
    char str[PATH_MAX_LENGTH];
    char *token,*final;
    //����ǰĿ¼
    memset(fact_path, '\0', PATH_MAX_LENGTH);
    strcpy(fact_path, current_dir);
    //����
    strcpy(str, path);
    token = strtok(str, "/");
    do {
        //��ǰĿ¼
        if (!strcmp(token, ".")) {
            continue;
        }
        if (!strcmp(token, "~")){
            continue;
        }
        //�ϲ�Ŀ¼
        if (!strcmp(token, "..")) {
            //��Ŀ¼���ϲ�Ŀ¼��Ϊ��Ŀ¼
            if (!strcmp(fact_path, "\\")) {
                continue;
            } else {
                //�ҵ����һ���ָ������λ��
                final = strrchr(fact_path,'/');
                //����λ���ó�\0,���ϲ�Ŀ¼
                memset(final, '\0', 1);
                continue;
            }
        }
        strcat(fact_path, "/");
        strcat(fact_path, token);
    } while ((token = strtok(NULL, "/")) != NULL);
    return fact_path;
}
//Ѱ���ļ���Ŀ¼�ڿ���λ��  �Ӹ�Ŀ¼��ʼ��
fcb *find_fcb(const char *path, int *cnt) {
    char fact_path[PATH_MAX_LENGTH];
    get_fact_path(fact_path, path);
    if (strcmp(fact_path,"\\")==0) {
        *cnt = ROOT_START;
        return (fcb *) (myvhard + BLOCK_SIZE * ROOT_START);
    }
    char *token = strtok(fact_path, "/");
    token = strtok(NULL, "/");
    return find_fcb_r(token, ROOT_START,cnt);
}
//Ѱ���ļ���Ŀ¼�ڿ���λ��  ��ʼ�ݹ�
fcb *find_fcb_r(char *token, int start,int *cnt) {
    int i, length = BLOCK_SIZE;
    char fullname[NAME_MAX_LENGTH] = "\0";
    fcb *root = (fcb *) (BLOCK_SIZE * start + myvhard);
    fcb *dir;
    block0 *init_block = (block0 *) myvhard;
    for (i = 0, dir = root; i < length / sizeof(fcb); i++, dir++) {
        if (dir->free == EMPTY) {
            continue;
        }
        //ȡ�������ļ���
        get_fullname(fullname, dir);
        if (!strcmp(token, fullname)) {
            token = strtok(NULL, "/");
            if (token == NULL) {
                *cnt = start;
                return dir;
            }
            return find_fcb_r(token, dir->first,cnt);
        }
    }
    fat *fat0 = (fat *) (myvhard + BLOCK_SIZE);
    fat0 = fat0 + start;
    if(fat0->id == END){
        *cnt = 0;
        return NULL;
    } else{
        return find_fcb_r(token, fat0->id,cnt);
    }
}
void reclaim_space_fat(int cnt){
    fcb * dir = (fcb *)(myvhard + BLOCK_SIZE * cnt);
    int i;
    int length = BLOCK_SIZE;
    int flag = 0;
    fat *fat0 = (fat *) (myvhard + BLOCK_SIZE);
    fat *fat1 = (fat *) (myvhard + BLOCK_SIZE * FAT_NUM + BLOCK_SIZE);
    int flag2 = 0;
    for (i = 0 ; i < length / sizeof(fcb); i++, dir++) {
        if (dir->free == UNEMPTY) {
            flag = 1;
            break;
        }
    }
    if (flag == 0){
        return;
    }
    for (int j = 0; j < BLOCK_NUM; ++j,fat0++,fat1++) {
        if(fat0->id == cnt){
            flag2 =1;
            break;
        }
    }
    if(flag2 == 0){
        return;
    }
    fat* fat0_copy = (fat *) (myvhard + BLOCK_SIZE);
    fat* fat1_copy = (fat *) (myvhard + BLOCK_SIZE * FAT_NUM + BLOCK_SIZE);
    fat0_copy = fat0_copy + cnt;
    fat1_copy = fat1_copy + cnt;
    fat0->id = fat0_copy->id;
    fat1->id = fat1_copy->id;
    fat0_copy->id = fat1_copy->id = FREE;

}
void cd_open(char *args,int mode) {
    int fd;
    char fact_path[PATH_MAX_LENGTH];
    fcb *dir;
    int i;
    int cnt;
    memset(fact_path, '\0', PATH_MAX_LENGTH);
    get_fact_path(fact_path, args);
    dir = find_fcb(fact_path,&cnt);
    if ( dir == NULL) {
        fprintf(stderr, "�Ҳ�������Ŀ\n");
        return;
    }
    if (mode == DICTORY && dir->attribute == DATA){
        fprintf(stderr, "�벻Ҫ�����ļ���\n");
        return;
    }
    if (mode == DATA && dir->attribute == DICTORY){
        fprintf(stderr, "�벻Ҫ�����ļ�����\n");
        return;
    }
    //���cd�Ƿ��Ѿ���openfile_list��
    for (i = 0; i < MAX_OPENFILE; i++) {
        if (openfile_list[i].topenfile == UNASSIGNED) {
            continue;
        }
        if (!strcmp(dir->filename, openfile_list[i].open_fcb.filename) &&
            dir->first == openfile_list[i].open_fcb.first) {
            //���ô��ļ�λ��
            curdir = i;
            memset(current_dir, '\0', sizeof(current_dir));
            strcpy(current_dir, openfile_list[i].dir);
            printf("��Ŀ��Ϊ��%s��չ��Ϊ:%s·��Ϊ:%s�޸�ʱ��Ϊ��%d��%d��%d��%dʱ%d��%d��\n",  openfile_list[curdir].open_fcb.filename,openfile_list[curdir].open_fcb.exname,openfile_list[curdir].dir
                    ,openfile_list[curdir].open_fcb.time_info.tm_year+1900,openfile_list[curdir].open_fcb.time_info.tm_mon+1,openfile_list[curdir].open_fcb.time_info.tm_mday,openfile_list[curdir].open_fcb.time_info.tm_hour,openfile_list[curdir].open_fcb.time_info.tm_min,openfile_list[curdir].open_fcb.time_info.tm_sec);
            return;
        }
    }
    //�ļ�δ��
    fd = get_useropenfd();
    if (fd == -1) {
        fprintf(stderr, "û�ж���Ĵ򿪴���,���ȹر�ĳ������\n");
        return;
    }
    openfile_set(fd,dir,fact_path,0,UNMODEFIED,ASSIGNED);
    curdir = fd;
    memset(current_dir, '\0', sizeof(current_dir));
    strcpy(current_dir, openfile_list[fd].dir);
    printf("��Ŀ��Ϊ��%8s��չ��Ϊ:%-6s·��Ϊ:%s�޸�ʱ��Ϊ��%d��%d��%d��%dʱ%d��%d��\n",  openfile_list[curdir].open_fcb.filename,openfile_list[curdir].open_fcb.exname,openfile_list[curdir].dir
            ,openfile_list[curdir].open_fcb.time_info.tm_year+1900,openfile_list[curdir].open_fcb.time_info.tm_mon+1,openfile_list[curdir].open_fcb.time_info.tm_mday,openfile_list[curdir].open_fcb.time_info.tm_hour,openfile_list[curdir].open_fcb.time_info.tm_min,openfile_list[curdir].open_fcb.time_info.tm_sec);
}
fcb *find_free_space(int first){
    int i;
    fcb *dir = (fcb *) (myvhard + BLOCK_SIZE * first);
    for (i = 0; i < BLOCK_SIZE / sizeof(fcb); i++, dir++) {
        if (dir->free == EMPTY) {
            return dir;
        }
    }
    fat *fat0 = (fat *) (myvhard + BLOCK_SIZE);
    fat0 = fat0 + first;
    if(fat0->id == END){
        return NULL;
    } else{
        return find_free_space(fat0->id);
    }
}
void init_folder(int first, int second) {
    int i;
    fcb *par = (fcb *) (myvhard + BLOCK_SIZE * first);
    fcb *cur = (fcb *) (myvhard + BLOCK_SIZE * second);

    fcb_set(cur, ".", "dic", DICTORY, second, BLOCK_SIZE, UNEMPTY,0);
    cur++;
    fcb_set(cur, "..", "dic", DICTORY, first, par->length, UNEMPTY,0);
    cur++;
    for (i = 2; i < BLOCK_SIZE / sizeof(fcb); i++, cur++) {
        cur->free = EMPTY;
    }
}
void mkdir_create(char *args,int mode) {
    int first,second,thrid;
    char path[PATH_MAX_LENGTH];
    char parpath[PATH_MAX_LENGTH], dirname[PATH_MAX_LENGTH];
    char *end;
    fcb *dir = NULL;
    fat *fat0 = (fat *) (myvhard + BLOCK_SIZE);
    int i;
    int cnt;
    get_fact_path(path, args);
    end = strrchr(path, '/');
    memset(parpath,'\0', PATH_MAX_LENGTH);
    if (end == NULL) {
        fprintf(stderr, "��Ҫ���������ַ�/\n");
        return;
    } else {
        strncpy(parpath, path, end - path);
        strcpy(dirname, end + 1);
    }
    if (find_fcb(parpath,&cnt) == NULL) {
        fprintf(stderr, "�޷��ҵ�·�� %s\n", parpath);
        return;
    }
    if (find_fcb(path,&cnt) != NULL) {
        fprintf(stderr, "'%s�Ѵ���\n", args);
        return;
    }
    first = find_fcb(parpath,&cnt)->first;
    fat0 = fat0 + first;
    second = fat0->id;
    while (second != END){
        fat0 = (fat *) (myvhard + BLOCK_SIZE);
        fat0 = fat0 + second;
        second = fat0->id;
    }
    dir = find_free_space(first);
    if(dir == NULL){
        second = get_free();
        if(second !=-1){
            fat_allocate(second,1);
            fat0->id = second;
            fcb *cur = (fcb *) (myvhard + BLOCK_SIZE * second);
            for (i = 0; i < BLOCK_SIZE / sizeof(fcb); i++, cur++) {
                cur->free = 0;
            }
            find_fcb(parpath,&cnt)->length += BLOCK_SIZE;
        } else{
            fprintf(stderr, "ϵͳȫ��д��\n");
            return;
        }
        dir = (fcb *) (myvhard + second);
    }
    thrid= get_free();
    if(thrid !=-1){
        fat_allocate(thrid,1);
    } else{
        fprintf(stderr, "ϵͳȫ��д��\n");
        return;
    }
    if(mode == DICTORY){
        fcb_set(dir, dirname, "dic", DICTORY, thrid, BLOCK_SIZE, UNEMPTY,0);
        init_folder(first, thrid);
    } else{
        char *token = strtok(dirname, ".");
        char exname[8];
        char filename[8];
        strncpy(filename, token, 8);
        token = strtok(NULL, ".");
        if (token != NULL) {
            strncpy(exname, token, 4);
        } else {
            strncpy(exname, "dat", 3);
        }
        fcb_set(dir, filename, exname, DATA, thrid, BLOCK_SIZE, UNEMPTY,0);
    }
}
void rmdir_rm(char *args, int mode){
    int i, j;
    fcb *dir;
    fcb *dir_start;
    int first;
    int cnt;
    char path[PATH_MAX_LENGTH];
    get_fact_path(path, args);
    i = (int )strlen(path);
    //���..,.���ܱ�ɾ��
    if(path[i-1] == '.'){
        fprintf(stderr, "û��Ȩ��ɾ��.��..\n");
        return ;
    }
    if (!strcmp(path, "\\")) {
        fprintf(stderr, "û��Ȩ��ɾ����Ŀ¼\n");
        return ;
    }
    dir = find_fcb(path,&cnt);
    if(mode == DICTORY){
        if (dir == NULL) {
            fprintf(stderr, "�Ҳ����ļ���%s\n", path);
            return ;
        }
        if (dir->attribute == DATA) {
            fprintf(stderr, "��ʹ��rmָ��ɾ���ļ�%s\n", path);
            return ;
        }
    } else{
        if (dir == NULL) {
            fprintf(stderr, "�Ҳ����ļ�%s\n", path);
            return ;
        }
        if (dir->attribute == DICTORY) {
            fprintf(stderr, "��ʹ��rmdirָ��ɾ���ļ���%s\n", path);
            return ;
        }
    }
    //�鿴����Ŀ�Ƿ񱻴�
    for (j = 0; j < MAX_OPENFILE; j++) {
        if (openfile_list[j].topenfile == UNASSIGNED) {
            continue;
        }
        if (!strcmp(dir->filename, openfile_list[j].open_fcb.filename) &&
            dir->first == openfile_list[j].open_fcb.first) {
            fprintf(stderr, "���ȹر�%s,��ɾ��\n", path);
            return;
        }
    }
    first = dir->first;
    if(mode == DICTORY){
        dir_start = (fcb *) (myvhard + BLOCK_SIZE * first);
        dir_start++;
        dir_start++;
        if(dir_start->free == UNEMPTY){
            fprintf(stderr, "�ļ���%s�л������ļ�\n", path);
            return;
        }
        dir->free = EMPTY;
        reclaim_space_fat(cnt);
        dir_start = (fcb *) (myvhard + BLOCK_SIZE * first);
        dir_start->free = EMPTY;
        dir_start++;
        dir_start->free = EMPTY;
        reclaim_space(first);
    } else{
        dir->free = EMPTY;
        reclaim_space(first);
    }
}
void reclaim_space(int first){
    fat *fat0 = (fat *) (myvhard + BLOCK_SIZE);
    fat *fat1 = (fat *) (myvhard + BLOCK_SIZE * FAT_NUM + BLOCK_SIZE);
    fat0 = fat0 + first;
    fat1 = fat1 + first;
    int offset;
    while (fat0->id != END) {
        offset = fat0->id - first;
        first = fat0->id;
        fat0->id = FREE;
        fat1->id = FREE;
        fat0 += offset;
        fat1 += offset;
    }
    fat0->id = FREE;
    fat1->id = FREE;

}
void ls(){
    int first = openfile_list[curdir].open_fcb.first;
    fat *fat0 = (fat *) (myvhard + BLOCK_SIZE);
    fcb *dir = (fcb *) (myvhard + BLOCK_SIZE * first);
    int i;
    fat0 = fat0 + first;
    block0 *init_block = (block0 *) myvhard;
    if (openfile_list[curdir].open_fcb.attribute == DATA){
        fprintf(stderr, "'%s'����һ��Ŀ¼\n", openfile_list[curdir].open_fcb.filename);
        return ;
    }
    while (fat0->id != END){
        dir = (fcb *) (myvhard + BLOCK_SIZE * first);
        for (i = 0; i < BLOCK_SIZE / sizeof(fcb); i++, dir++)  {
            if (dir->free == EMPTY) {
                continue;
            }
            printf("��Ŀ��Ϊ��%s,��չ��Ϊ:%s,����Ϊ%c,����Ϊ%ld,�޸�ʱ��Ϊ��%d��%d��%d��%dʱ%d��%d��\n",  dir->filename,dir->exname,dir->attribute,dir->length,
                   dir->time_info.tm_year+1900,dir->time_info.tm_mon+1,dir->time_info.tm_mday,dir->time_info.tm_hour,dir->time_info.tm_min,dir->time_info.tm_sec);
        }
        first = fat0->id;
        fat0 = (fat *) (myvhard + BLOCK_SIZE);
        fat0 = fat0 + first;
    }
    dir = (fcb *) (myvhard + BLOCK_SIZE * first);
    for (i = 0; i < BLOCK_SIZE / sizeof(fcb); i++, dir++)  {
        if (dir->free == EMPTY) {
            continue;
        }
        printf("��Ŀ��Ϊ��%s��չ��Ϊ:%s����Ϊ%c����Ϊ%ld�����̿��޸�ʱ��Ϊ��%d��%d��%d��%dʱ%d��%d��\n",  dir->filename,dir->exname,dir->attribute,dir->length,
                dir->time_info.tm_year+1900,dir->time_info.tm_mon+1,dir->time_info.tm_mday,dir->time_info.tm_hour,dir->time_info.tm_min,dir->time_info.tm_sec);
    }

}
void close(int fd){
    int cur;
    if (openfile_list[fd].topenfile == UNASSIGNED){
        fprintf(stderr, "����ļ��Ѵ��ڹر�״̬\n");
        return;
    }
    if (!strcmp(openfile_list[fd].dir,"\\")){
        fprintf(stderr, "��Ŀ¼�޷��ر�\n");
        return;
    }
    if (fd == curdir){
        curdir = 0;
        strcpy(current_dir, openfile_list[curdir].dir);
    }
    if (openfile_list[fd].fcb_state == 1) {
        fcb_copy(find_fcb(openfile_list[fd].dir,&cur), &openfile_list[fd].open_fcb);
    }
    openfile_list[fd].topenfile = UNASSIGNED;
}
void write(int des_fd,int mode){
    char aa;
    int len = 0;
    int str_len = 0;
    int len_sum;
    int str_start;
    printf("������Ҫд���ֽ���\n");
    scanf("%d",&str_len);
    char* str = NULL;
    int cur;
    if(mode == 2){
        printf("�������дָ���λ��\n");
        scanf("%d",&cur);
    } else{
        cur = openfile_list[des_fd].count;
    }
    if(mode == 1){
        str = malloc(str_len);
        len_sum = str_len;
    } else if(mode == 2){
        if((openfile_list[des_fd].open_fcb.length - cur) > str_len){
            len_sum = openfile_list[des_fd].open_fcb.length - cur + str_len;
            str_start = len_sum - cur - str_len;
        } else{
            len_sum = str_len + cur;
            str_start = len_sum;
        }
        str = malloc(len_sum*2);
    } else{
        len_sum = openfile_list[des_fd].open_fcb.length + str_len;
        str = malloc(len_sum*2);
    }
    if (openfile_list[des_fd].open_fcb.attribute == DICTORY){
        fprintf(stderr, "�޷�д���ļ���\n");
        return ;
    }
    if (openfile_list[des_fd].topenfile == UNASSIGNED){
        fprintf(stderr, "�ļ�δ��\n");
        return ;
    }
    if(mode == 1){
        memset(str, '\0',str_len);
        getchar();
        printf("Ҫ��������\n");
        if((aa=getchar())=='y'){
            for (int i = 0; i < str_len; ++i) {
                str[i] = (i%10)+'0';
            }
            getchar();
        } else{
            getchar();
            while (scanf("%c",&aa)!=EOF){
                str[len] = aa;
                len++;
                if(len>=str_len){
                    break;
                }
            }
        }
    } else if (mode == 2){
        openfile_list[des_fd].open_fcb.len = len_sum;
        openfile_list[des_fd].count = 0;
        memset(str, '\0',len_sum*2);
        do_read(des_fd,cur,str);
        char *input = malloc(str_len*2);
        memset(input, '\0',str_len*2);
        getchar();
        printf("Ҫ��������\n");
        if((aa=getchar())=='y'){
            int i;
            for ( i = 0; i < str_len; ++i) {
                input[i] = (i%10)+'0';
            }
            input[i] = '\0';
            getchar();
        } else{
            getchar();
            while (scanf("%c",&aa)!=EOF){
                input[len] = aa;
                len++;
                if(len>=str_len){
                    break;
                }
            }
        }
        char *input_end = malloc(2*(len_sum-str_start));
        openfile_list[des_fd].count = str_start;
        do_read(des_fd,len_sum-str_start,input_end);
        strcat(str,input);
        free(input);
        strcat(str,input_end);
        free(input_end);
    } else{
        openfile_list[des_fd].count = 0;
        memset(str, '\0',len_sum*2);
        do_read(des_fd,openfile_list[des_fd].open_fcb.len,str);
        openfile_list[des_fd].open_fcb.len = len_sum;
        char *input = malloc(str_len*2);
        memset(input, '\0',str_len*2);
        getchar();
        printf("Ҫ��������\n");
        if((aa=getchar())=='y'){
            int i;
            for ( i = 0; i < str_len; ++i) {
                input[i] = (i%10)+'0';
            }
            input[i] = '\0';
            getchar();
        } else{
            getchar();
            while (scanf("%c",&aa)!=EOF){
                input[len] = aa;
                len++;
                if(len>=str_len){
                    break;
                }
            }
        }
        strcat(str,input);
        free(input);
    }
    do_write(des_fd,str,len_sum,mode);
}
void do_write(int des_fd,char *str, int len,int mode){
    int first = openfile_list[des_fd].open_fcb.first;
    fat *fat0 = (fat *) (myvhard + BLOCK_SIZE);
    fat *fat1 = (fat *) (myvhard + BLOCK_SIZE * FAT_NUM + BLOCK_SIZE);
    reclaim_space(first);
    int cur;
    int block_num = (int)((double)len/BLOCK_SIZE + 0.5);
    int write = get_free();
    first = write;
    if (write == -1){
        fprintf(stderr, "ϵͳ���޿��д��̿�\n");
        fat0 = (fat *) (myvhard + BLOCK_SIZE);
        fat1 = (fat *) (myvhard + BLOCK_SIZE * FAT_NUM + BLOCK_SIZE);
        memcpy(fat0, fat1, FAT_NUM*BLOCK_SIZE);
    }
    memcpy(myvhard+write*(BLOCK_SIZE),str,BLOCK_SIZE);
    fcb *dir = find_fcb(openfile_list[des_fd].dir,&cur);
    dir->first = write;
    int old = write;
    int i = 1;
    fat0 = fat0 + old;
    fat0->id = END;
    block_num--;
    while (i <= block_num){
        write = get_free();
        if (write == -1){
            fprintf(stderr, "ϵͳ���޿��д��̿�\n");
            fat0 = (fat *) (myvhard + BLOCK_SIZE);
            fat1 = (fat *) (myvhard + BLOCK_SIZE * FAT_NUM + BLOCK_SIZE);
            memcpy(fat0, fat1, FAT_NUM*BLOCK_SIZE);
        }
        fat0->id = write;
        fat0 = (fat *) (myvhard + BLOCK_SIZE);
        fat0 = fat0 + write;
        fat0->id = END;
        memcpy(myvhard+write*(BLOCK_SIZE),str+BLOCK_SIZE*i,BLOCK_SIZE);
        i++;
    }
    fat0 = (fat *) (myvhard + BLOCK_SIZE);
    fat1 = (fat *) (myvhard + BLOCK_SIZE * FAT_NUM + BLOCK_SIZE);
    memcpy(fat1, fat0, FAT_NUM*BLOCK_SIZE);
    dir->length = (int)((double)len/BLOCK_SIZE + 0.5)*BLOCK_SIZE;
    dir->len = len;
    openfile_set(des_fd,dir,openfile_list[des_fd].dir,0,UNMODEFIED,ASSIGNED);
}
int do_read(int des_fd,int len,char *text){
    if((openfile_list[des_fd].count + len) > openfile_list[des_fd].open_fcb.len){
        fprintf(stderr, "��ȡ�����ļ��ܳ�\n");
        return -1;
    }
    // �����߼���źͿ���ƫ����
    int block_num = openfile_list[des_fd].count / BLOCK_SIZE;
    int block_offset = openfile_list[des_fd].count % BLOCK_SIZE;
    char buf[BLOCK_SIZE];
    // �õ�������
    fat* fat1 = (fat*)(myvhard + BLOCK_SIZE);
    int cur_fat = openfile_list[des_fd].open_fcb.first;
    fat* fat_ptr = fat1 + openfile_list[des_fd].open_fcb.first;
    for(int i = 0; i < block_num; i++){
        cur_fat = fat_ptr->id;
        fat_ptr = fat1 + fat_ptr->id;
    }
    // ��ʼ��ȡ
    int cnt = 0;
    while(cnt < len){
        unsigned char* pos = (unsigned char*)(myvhard + BLOCK_SIZE*cur_fat);
        for(int i = 0; i < BLOCK_SIZE; ++i){
            buf[i] = pos[i];
        }
        for(; block_offset < BLOCK_SIZE; ++block_offset){
            text[cnt] = buf[block_offset];
            ++cnt;
            openfile_list[des_fd].count++;
            if(cnt == len){
                break;
            }
        }
        if(cnt < len){
            cur_fat = fat_ptr->id;
            fat_ptr = fat1 + fat_ptr->id;
            block_offset = 0;
        }
    }
    text[cnt] = '\0';
    return cnt;
}
void read(int des_fd){
    if(des_fd < 0 || des_fd >= MAX_OPENFILE){
        fprintf(stderr, "�ļ�����������\n");
        return;
    }
    int start;
    printf("�������ļ���ȡ��ʼλ��\n");
    scanf("%d",&start);
    int len;
    printf("�������ȡ����\n");
    scanf("%d",&len);
    char *text = malloc(len*2);
    if (openfile_list[des_fd].open_fcb.attribute == DICTORY){
        fprintf(stderr, "�޷������ļ���\n");
        return ;
    }
    if (openfile_list[des_fd].topenfile == UNASSIGNED){
        fprintf(stderr, "�ļ�δ��\n");
        return ;
    }
    if (len > openfile_list[des_fd].open_fcb.len){
        fprintf(stderr, "���볤�ȴ����ļ��ܳ�\n");
        return ;
    }
    openfile_list[des_fd].count = start;
    int cnt = do_read(des_fd,  len, text);
    if(cnt == -1){
        fprintf(stderr, "��ȡ�ļ�����\n");
        return ;
    }else{
        printf("%s\n", text);
        printf("����ȡ %d B\n", cnt);
        getchar();
    }
}
void my_exitsys(){
    FILE *fp = fopen(PATH, "w");
    fwrite(myvhard, SIZE, 1, fp);
    fclose(fp);
    free(myvhard);
}
void pwd(){
    printf("��ǰ���ļ�/�ļ��б�Ϊ\n");
    int i;
    for (i = 0; i < MAX_OPENFILE; i++) {
        if (openfile_list[i].topenfile == UNASSIGNED) {
            continue;
        } else{
            printf("��%d��,��Ŀ��Ϊ��%s��չ��Ϊ:%s·��Ϊ:%s�޸�ʱ��Ϊ��%d��%d��%d��%dʱ%d��%d��\n",  i,openfile_list[i].open_fcb.filename,openfile_list[i].open_fcb.exname,openfile_list[i].dir
                    ,openfile_list[i].open_fcb.time_info.tm_year+1900,openfile_list[i].open_fcb.time_info.tm_mon+1,openfile_list[i].open_fcb.time_info.tm_mday,openfile_list[i].open_fcb.time_info.tm_hour,openfile_list[i].open_fcb.time_info.tm_min,openfile_list[i].open_fcb.time_info.tm_sec);
        }
    }
}
void printfat(){
    fat* fat1 = (fat*)(myvhard + BLOCK_SIZE);
    int cnt = 0;
    for(int i = 0; i < SIZE / BLOCK_SIZE; ++i){
        printf("%04x ",fat1->id);
        ++fat1;
        ++cnt;
        if(cnt % 16 == 0){
            printf("\n");
            cnt = 0;
        }
    }
    printf("\n");
}
int main(){
    startsys();
    printf("�ļ�ϵͳ��ʼ���ɹ�\n");
    while(1){
        printf("%s> ", current_dir);
        char yes;
        char command[100] = "";
        fgets(command, sizeof(command), stdin);
        command[strlen(command) - 1] = '\0';
        if(strncmp(command, "format", 6) == 0){
           printf("�Ƿ���Ĵ��̴�С,�����³�ʼ������ʧ�ļ����ݡ�\n");
            if((yes=getchar())=='y'){
                format();
                startsys();
            }
        }
        else if(strncmp(command, "cd", 2) == 0){
            char *dirname = command + 3;
            cd_open(dirname,DICTORY);
        }else if(strncmp(command, "mkdir", 5) == 0){
            char *dirname = command + 6;
            mkdir_create(dirname,DICTORY);
        }
        else if(strncmp(command, "rmdir", 5) == 0){
            char *dirname = command + 6;
            rmdir_rm(dirname,DICTORY);
        }
        else if(strncmp(command, "ls", 2) == 0){
            ls();
        }
        else if(strncmp(command, "create", 6) == 0){
            char *dirname = command + 7;
            mkdir_create(dirname,DATA);
        }
        else if(strncmp(command, "rm", 2) == 0){
            char *dirname = command + 3;
            rmdir_rm(dirname,DATA);
        }
        else if(strncmp(command, "open", 4) == 0){
            char *dirname = command + 5;
            cd_open(dirname,DATA);
        }
        else if(strncmp(command, "close", 5) == 0){
            pwd();
            printf("������Ҫ�رյ����\n");
            int dir;
            scanf("%d",&dir);
            getchar();
            close(dir);
        }
        else if(strncmp(command, "write", 5) == 0){
            int mode;
            printf("��ѡ��д��ģʽ(1.�ض�2.����3.׷��)\n");
            scanf("%d",&mode);
            pwd();
            printf("������Ҫд������\n");
            int dir;
            scanf("%d",&dir);
            write(dir,mode);
        }
        else if(strncmp(command, "read", 4) == 0){
            pwd();
            printf("������Ҫ��������\n");
            int dir;
            scanf("%d",&dir);
            read(dir);
        }
        else if(strncmp(command, "printfat", 8) == 0){
            printfat();
        }
        else if(strncmp(command, "exit", 4) == 0){
            my_exitsys();
            return 0;
        }
        else{
            printf("�������\n");
        }
    }
}
