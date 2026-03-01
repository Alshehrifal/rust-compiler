#include "lexer.h"
#include "parser.h"
#include <cstring>
#include <fstream>
#include <iostream>

static std::vector<Token> lexAll(std::istream& input) {
    Lexer lexer(input);
    std::vector<Token> tokens;
    while (lexer.hasNext()) {
        tokens.push_back(lexer.nextToken());
    }
    return tokens;
}

static void printTokens(const std::vector<Token>& tokens) {
    for (const auto& tok : tokens) {
        std::cout << "[" << tokenTypeName(tok.type);
        if (!tok.lexeme.empty()) {
            std::cout << ":" << tok.lexeme;
        }
        std::cout << "]" << std::endl;
    }
}

static void printAST(const ProgramNode& program) {
    std::cout << "Program: " << program.functions.size() << " function(s)" << std::endl;
    for (const auto& fn : program.functions) {
        std::cout << "  fn " << fn->name << "(";
        for (size_t i = 0; i < fn->params.size(); i++) {
            if (i > 0) std::cout << ", ";
            std::cout << fn->params[i].name << ": " << fn->params[i].typeName;
        }
        std::cout << ")";
        if (!fn->returnType.empty()) {
            std::cout << " -> " << fn->returnType;
        }
        std::cout << " [" << fn->body->statements.size() << " statement(s)]" << std::endl;
    }
}

int main(int argc, char* argv[]) {
    bool parseMode = false;
    const char* filename = nullptr;

    for (int i = 1; i < argc; i++) {
        if (std::strcmp(argv[i], "--parse") == 0) {
            parseMode = true;
        } else {
            filename = argv[i];
        }
    }

    std::istream* input = &std::cin;
    std::ifstream file;

    if (filename) {
        file.open(filename);
        if (!file.is_open()) {
            std::cerr << "Error: cannot open file '" << filename << "'" << std::endl;
            return 1;
        }
        input = &file;
    }

    auto tokens = lexAll(*input);

    if (parseMode) {
        try {
            Parser parser(tokens);
            auto program = parser.parse();
            printAST(*program);
        } catch (const ParseError& e) {
            std::cerr << e.what() << std::endl;
            return 1;
        }
    } else {
        printTokens(tokens);
    }

    return 0;
}
