/**
 * @file compiler.cpp
 * @author Aliver (aliver.len@qq.com)
 * @brief
 * @version 0.1
 * @date 2020-04-23
 *
 * @copyright Copyright (c) 2020
 *
 */

#include <iostream>
#include <string>

#include <cstdlib>
#include <cstring>

#include "grammatical_analysis.hpp"
#include "lexical_analysis.hpp"

using namespace std;

void
usage(const char *prompt = nullptr) {
    if (prompt)
        cout << prompt << endl;
    cout << "用法如下：" << endl;
    cout << "1. compiler -x [代码文件路径] -g [文法文件路径]: 分析类C程序代码文件语法" << endl;
    cout << "2. compiler -o -x [代码文件路径] -g [文法文件路径]: 分析类C程序代码文件语法，输出过程" << endl;
}

int
main(int argc, char **argv) {
    bool   show_details = false;
    string code_path    = "./homework/compiling/test/source_code.txt";
    string grammar_path = "./homework/compiling/Grammar.txt";

    if (argc <= 1) {
        usage(nullptr);
        exit(EXIT_SUCCESS);
    }

    for (int i = 1; i < argc; ++i) {
        if (!strcmp(argv[i], "-x")) {
            if (i + 1 < argc) {
                code_path = argv[++i];
            } else {
                usage();
                exit(EXIT_SUCCESS);
            }
        } else if (!strcmp(argv[i], "-g")) {
            if (i + 1 < argc) {
                grammar_path = argv[++i];
            } else {
                usage();
                exit(EXIT_SUCCESS);
            }
        } else if (!strcmp(argv[i], "-o")) {
            show_details = true;
        } else {
            usage();
            exit(EXIT_SUCCESS);
        }
    }


    Lexical lex(code_path);
    lex.scan();
    lex.print();

    ofstream fout("./table.txt", ios::out);

    LR_1 grammar(grammar_path);
    grammar.printTable(cout);

    int error_count = grammar.parse_token(lex.getTokenStream());
    cout << "\ntotal errors found : " << error_count << endl;

    fout.close();

    return 0;
}