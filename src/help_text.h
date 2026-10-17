#ifndef IT_HELP_TEXT_H
#define IT_HELP_TEXT_H

typedef struct {
    const char* tab_title;
    const char* section_title;
    const char* content;
} ITHelpSection;

int it_help_get_section_count(void);
const ITHelpSection* it_help_get_section(int index);

#endif /* IT_HELP_TEXT_H */
