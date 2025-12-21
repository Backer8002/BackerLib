//Source https://stackoverflow.com/questions/4842424/list-of-ansi-color-escape-sequences

#ifndef ANSIESCAPESEQUENCES_H
#define ANSIESCAPESEQUENCES_H

#define ANSI_RESET_ATTRIBUTE "\033[0m"
#define ANSI_TEXT_BOLD "\033[1m"
#define ANSI_TEXT_UNDERLINE "\033[4m"
#define ANSI_BLINKING "\033[5m"
#define ANSI_BLINKING_STOP "\033[25m"
#define ANSI_TEXT_BLACK "\033[30m"
#define ANSI_TEXT_RED "\033[31m"
#define ANSI_TEXT_GREEN "\033[32m"
#define ANSI_TEXT_YELLOW "\033[33m"
#define ANSI_TEXT_BLUE "\033[34m"
#define ANSI_TEXT_MAGENTA "\033[35m"
#define ANSI_TEXT_CYAN "\033[36m"
#define ANSI_TEXT_WHITE "\033[37m"
#define ANSI_TEXT_RGB(r,g,b) "\033[38;2;"#r";"#g";"#b"m"
#define ANSI_BACKGROUD_RGB(r,g,b) "\033[48;2;"#r";"#g";"#b"m"


#endif // ANSIESCAPESEQUENCES_H
