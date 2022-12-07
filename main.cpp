#include <iostream>
#include "utils.hpp"
#include "HTMLParser.hpp"


int main (int argc, char *argv[]) {

    init_chinese_env();

    ParserShell shell;
    if (argc == 2) {
        shell.SetSource(argv[1]);
    }
    shell.Start();

    return 0;
}
