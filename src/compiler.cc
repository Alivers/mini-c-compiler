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
usage(const char* prompt = nullptr) {
    if (prompt)
        cout << prompt << endl;
    cout << "用法如下：" << endl;
    cout << "    ./compiler -x [源文件路径] -g [文法文件路径]: 分析类C程序代码文件语法" << endl;
    cout << "例：" << endl;
    cout << "    ./compiler -x source.txt -g grammar.txt" << endl;
    cout << "    对当前目录下的 source.txt 进行分析处理，文法参考 grammar.txt" << endl;
}

int
main(int argc, char** argv) {
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
        } else {
            usage();
            exit(EXIT_SUCCESS);
        }
    }

    ofstream lex_tokens("./Lex_token_stream.txt", ios::out);
    ofstream lr1_table("./Lr1_table.txt", ios::out);
    ofstream lr1_process("./Lr1_process.txt", ios::out);
    ofstream intermediate("./inter_code.txt", ios::out);

    Lexical lex(code_path);
    lex.scan();
    lex.print(lex_tokens);

    LR_1 grammar(grammar_path);
    grammar.printTable(lr1_table);

    auto error_count = grammar.parse_token(lex.getTokenStream(), lr1_process);
    if (error_count.first) {
        cout << "\n 语法分析共发现 " << error_count.first << "处错误！" << endl;
    } else {
        cout << "\n 语法分析完成，未发现语法错误。" << endl;
    }

    if (error_count.second) {
        cout << "\n 语义分析共发现 " << error_count.second << "处错误！" << endl;
    } else {
        cout << "\n 语义分析完成，未发现语义错误。" << endl;
    }

    grammar.semantic.PrintQuadruple(intermediate);
    cout << "\n 中间代码生成完成。" << endl;

    lex_tokens.close();
    lr1_table.close();
    lr1_process.close();
    intermediate.close();

    cout << "\n 程序执行结束。" << endl;
    cout << "\t 词法分析生成的单词流已输出至当前目录下的 "
         << "Lex_token_stream.txt 文件中。" << endl;
    cout << "\t 语法分析生成的LR(1)文法的分析表已输出至当前目录下的 "
         << "Lr1_table.txt 文件中。" << endl;
    cout << "\t 语法分析生成的LR(1)文法的分析过程已输出至当前目录下的 "
         << "Lr1_process.txt 文件中。" << endl;
    cout << "\t 程序的中间代码已输出至当前目录下的 "
         << "inter_code.txt 文件中。" << endl;
    return 0;
}