#include "tf_card_probe.h"

#include <stdio.h>
#include <string.h>

#include "ff.h"
#include "./sdio/bsp_sdio_sdcard.h"

#define TF_PROBE_ROOT_PATH          "0:/tf_card"
#define TF_PROBE_FALLBACK_PATH      "0:"
#define TF_PROBE_MAX_ROOT_ENTRIES   24U
#define TF_PROBE_MAX_DATASET_SCAN   1400U
#define TF_PROBE_MAX_SAMPLE_FILES   3U
#define TF_PROBE_LFN_SIZE           256U
#define TF_PROBE_PATH_SIZE          96U

extern SD_CardInfo SDCardInfo;

static FATFS g_tf_fatfs;

static const char *TFCardProbe_Name(FILINFO *info)
{
#if _USE_LFN
    if ((info->lfname != 0) && (info->lfname[0] != '\0')) {
        return info->lfname;
    }
#endif
    return info->fname;
}

static char TFCardProbe_ToLower(char ch)
{
    if ((ch >= 'A') && (ch <= 'Z')) {
        return (char)(ch - 'A' + 'a');
    }
    return ch;
}

static unsigned char TFCardProbe_HasSuffix(const char *name, const char *suffix)
{
    unsigned int name_len;
    unsigned int suffix_len;
    unsigned int i;

    if ((name == 0) || (suffix == 0)) {
        return 0U;
    }

    name_len = (unsigned int)strlen(name);
    suffix_len = (unsigned int)strlen(suffix);
    if ((suffix_len == 0U) || (name_len < suffix_len)) {
        return 0U;
    }

    for (i = 0U; i < suffix_len; i++) {
        if (TFCardProbe_ToLower(name[name_len - suffix_len + i]) !=
            TFCardProbe_ToLower(suffix[i])) {
            return 0U;
        }
    }
    return 1U;
}

static unsigned char TFCardProbe_IsImageName(const char *name)
{
    if (TFCardProbe_HasSuffix(name, ".bmp") != 0U) {
        return 1U;
    }
    if (TFCardProbe_HasSuffix(name, ".png") != 0U) {
        return 1U;
    }
    if (TFCardProbe_HasSuffix(name, ".jpg") != 0U) {
        return 1U;
    }
    if (TFCardProbe_HasSuffix(name, ".jpeg") != 0U) {
        return 1U;
    }
    return 0U;
}

static void TFCardProbe_PrintEntry(const char *root_path, FILINFO *info)
{
    const char *name;
    const char *type;

    name = TFCardProbe_Name(info);
    if ((name == 0) || (name[0] == '\0')) {
        return;
    }
    type = ((info->fattrib & AM_DIR) != 0U) ? "dir" : "file";
    printf("TF_ENTRY,path=%s,name=%s,type=%s,size=%lu\r\n",
           root_path,
           name,
           type,
           (unsigned long)info->fsize);
}

static void TFCardProbe_PrintRootEntries(const char *root_path)
{
    DIR dir;
    FILINFO info;
    FRESULT result;
    unsigned int count;
    char lfn[TF_PROBE_LFN_SIZE];

    result = f_opendir(&dir, root_path);
    printf("TF_STATUS,state=list_root,path=%s,result=%u\r\n", root_path, (unsigned int)result);
    if (result != FR_OK) {
        return;
    }

    count = 0U;
    while (count < TF_PROBE_MAX_ROOT_ENTRIES) {
        memset(&info, 0, sizeof(info));
#if _USE_LFN
        info.lfname = lfn;
        info.lfsize = sizeof(lfn);
        lfn[0] = '\0';
#endif
        result = f_readdir(&dir, &info);
        if ((result != FR_OK) || (info.fname[0] == '\0')) {
            break;
        }
        TFCardProbe_PrintEntry(root_path, &info);
        count++;
    }

    f_closedir(&dir);
    printf("TF_STATUS,state=list_root_done,path=%s,entries=%u,result=%u\r\n",
           root_path,
           count,
           (unsigned int)result);
}

static void TFCardProbe_PrintDataset(const char *path)
{
    DIR dir;
    FILINFO info;
    FRESULT result;
    unsigned int entries;
    unsigned int dirs;
    unsigned int images;
    unsigned int labels;
    unsigned int samples;
    unsigned int truncated;
    const char *name;
    char lfn[TF_PROBE_LFN_SIZE];

    entries = 0U;
    dirs = 0U;
    images = 0U;
    labels = 0U;
    samples = 0U;
    truncated = 0U;

    result = f_opendir(&dir, path);
    if (result != FR_OK) {
        printf("TF_DIR,path=%s,result=%u,entries=0,dirs=0,images=0,labels=0,scanned=0,truncated=0\r\n",
               path,
               (unsigned int)result);
        return;
    }

    while (entries < TF_PROBE_MAX_DATASET_SCAN) {
        memset(&info, 0, sizeof(info));
#if _USE_LFN
        info.lfname = lfn;
        info.lfsize = sizeof(lfn);
        lfn[0] = '\0';
#endif
        result = f_readdir(&dir, &info);
        if ((result != FR_OK) || (info.fname[0] == '\0')) {
            break;
        }

        name = TFCardProbe_Name(&info);
        entries++;
        if ((info.fattrib & AM_DIR) != 0U) {
            dirs++;
            continue;
        }

        if (TFCardProbe_IsImageName(name) != 0U) {
            images++;
            if (samples < TF_PROBE_MAX_SAMPLE_FILES) {
                printf("TF_FILE,path=%s,name=%s,kind=image,size=%lu\r\n",
                       path,
                       name,
                       (unsigned long)info.fsize);
                samples++;
            }
        } else if ((strcmp(name, "label.txt") == 0) ||
                   (strcmp(name, "labels.txt") == 0) ||
                   TFCardProbe_HasSuffix(name, ".csv") != 0U) {
            labels++;
            if (samples < TF_PROBE_MAX_SAMPLE_FILES) {
                printf("TF_FILE,path=%s,name=%s,kind=label,size=%lu\r\n",
                       path,
                       name,
                       (unsigned long)info.fsize);
                samples++;
            }
        }
    }

    if (entries >= TF_PROBE_MAX_DATASET_SCAN) {
        truncated = 1U;
    }
    f_closedir(&dir);

    printf("TF_DIR,path=%s,result=%u,entries=%u,dirs=%u,images=%u,labels=%u,scanned=%u,truncated=%u\r\n",
           path,
           (unsigned int)result,
           entries,
           dirs,
           images,
           labels,
           entries,
           truncated);
}

static void TFCardProbe_PrintDatasets(const char *root_path)
{
    static const char *datasets[] = {
        "mnist",
        "personal",
        "ui_collected",
        "emnist_letters",
        "external_usps",
        "chinese"
    };
    unsigned int i;
    char path[TF_PROBE_PATH_SIZE];

    for (i = 0U; i < (sizeof(datasets) / sizeof(datasets[0])); i++) {
        sprintf(path, "%s/%s", root_path, datasets[i]);
        TFCardProbe_PrintDataset(path);
    }
}

void TFCardProbe_Run(void)
{
    DIR root_dir;
    FRESULT result;
    FRESULT root_result;
    const char *root_path;
    unsigned long capacity_mb;

    printf("TF_STATUS,state=start,driver=sdio_fatfs,path=%s\r\n", TF_PROBE_ROOT_PATH);
    printf("TF_STATUS,state=mount_begin,path=0:\r\n");
    result = f_mount(&g_tf_fatfs, "0:", 1);
    if (result != FR_OK) {
        printf("TF_STATUS,state=mount_failed,result=%u\r\n", (unsigned int)result);
        return;
    }

    capacity_mb = (unsigned long)(SDCardInfo.CardCapacity / 1048576U);
    printf("TF_STATUS,state=mounted,result=0,capacity_mb=%lu,block_size=%lu\r\n",
           capacity_mb,
           (unsigned long)SDCardInfo.CardBlockSize);

    root_path = TF_PROBE_ROOT_PATH;
    root_result = f_opendir(&root_dir, root_path);
    if (root_result != FR_OK) {
        root_path = TF_PROBE_FALLBACK_PATH;
        printf("TF_STATUS,state=tf_card_dir_missing,path=%s,result=%u,fallback=%s\r\n",
               TF_PROBE_ROOT_PATH,
               (unsigned int)root_result,
               root_path);
    } else {
        f_closedir(&root_dir);
    }

    TFCardProbe_PrintRootEntries(root_path);
    TFCardProbe_PrintDatasets(root_path);

    f_mount(0, "0:", 1);
    printf("TF_STATUS,state=done,path=%s\r\n", root_path);
}
