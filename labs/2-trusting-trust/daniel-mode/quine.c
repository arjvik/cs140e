#include <stdlib.h>
#include <stdio.h>
#include <string.h>

char *s[] = {
    "    0",
    "};",
    "",
    "char *escape(char *s) {",
    "    char *out = malloc(strlen(s) * 2 + 1);",
    "    char *cursor = out;",
    "    while (*s) {",
    "        switch (*s) {",
    "            case '\\\\':",
    "            case '\"':",
    "                *cursor++ = '\\\\';",
    "                *cursor++ = *s;",
    "                break;",
    "            case '\\n':",
    "                *cursor++ = '\\\\';",
    "                *cursor++ = 'n';",
    "                break;",
    "            default:",
    "                *cursor++ = *s;",
    "        }",
    "        s++;",
    "    }",
    "    *cursor = 0;",
    "    return out;",
    "}",
    "",
    "int main(char *argv[], int argc) {",
    "    printf(\"#include <stdlib.h>\\n#include <stdio.h>\\n#include <string.h>\\n\\nchar *s[] = {\\n\");",
    "    for (int i = 0; s[i]; i++)",
    "        printf(\"    \\\"%s\\\",\\n\", escape(s[i]));",
    "    for (int i = 0; s[i]; i++)",
    "        printf(\"%s\\n\", s[i]);",
    "}",
    0
};

char *escape(char *s) {
    char *out = malloc(strlen(s) * 2 + 1);
    char *cursor = out;
    while (*s) {
        switch (*s) {
            case '\\':
            case '"':
                *cursor++ = '\\';
                *cursor++ = *s;
                break;
            case '\n':
                *cursor++ = '\\';
                *cursor++ = 'n';
                break;
            default:
                *cursor++ = *s;
        }
        s++;
    }
    *cursor = 0;
    return out;
}

int main(char *argv[], int argc) {
    printf("#include <stdlib.h>\n#include <stdio.h>\n#include <string.h>\n\nchar *s[] = {\n");
    for (int i = 0; s[i]; i++)
        printf("    \"%s\",\n", escape(s[i]));
    for (int i = 0; s[i]; i++)
        printf("%s\n", s[i]);
}
