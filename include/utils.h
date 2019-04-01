#ifndef _ELEVENMPV_UTILS_H_
#define _ELEVENMPV_UTILS_H_

void Utils_SetMax(int *set, int value, int max);
void Utils_SetMin(int *set, int value, int min);
int Utils_ReadControls(void);
int Utils_InitAppUtil(void);
int Utils_TermAppUtil(void);
int Utils_GetEnterButton(void);
int Utils_GetCancelButton(void);
int Utils_Alphasort(const void *p1, const void *p2);
char *Utils_Basename(const char *filename);

#endif
