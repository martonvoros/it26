#ifndef IT_DIALOG_H
#define IT_DIALOG_H

#include "common.h"

typedef void (*ITDialogCallback)(bool confirmed, void* userdata);

void it_dialog_alert(const char* title, const char* message);
void it_dialog_confirm(const char* title, const char* message, ITDialogCallback cb, void* userdata);
bool it_dialog_is_active(void);
void it_dialog_close(bool confirmed);
void it_dialog_render(void);

#endif /* IT_DIALOG_H */
