#include "lexer.h"
#include <fstream>
#include <iostream>

int main(int argc, char* argv[]) {
    std::istream* input = &std::cin;
    std::ifstream file;

    if (argc > 1) {
        file.open(argv[1]);
        if (!file.is_open()) {
            std::cerr << "Error: cannot open file '" << argv[1] << "'" << std::endl;
            return 1;
        }
        input = &file;
    }

    Lexer lexer(*input);
    while (lexer.hasNext()) {
        Token tok = lexer.nextToken();
        std::cout << "[" << tokenTypeName(tok.type);
        if (!tok.lexeme.empty()) {
            std::cout << ":" << tok.lexeme;
        }
        std::cout << "]" << std::endl;
    }

    return 0;
}
