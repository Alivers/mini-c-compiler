/*!
 * Date: 2020-01-18 13:04:23
 * LastEditors: Peng Gao
 * LastEditTime: 2020-01-20 19:21:14
 */
#ifndef _LEXICAL_ANALYSIS_HPP_
#define _LEXICAL_ANALYSIS_HPP_

/*!
 * 词法分析
 */

#include <fstream>
#include <list>
#include <set>
#include <string>
#include <vector>

#include <iomanip>
#include <iostream>

typedef std::string token_t; // 符号类型
typedef std::string value_t; // 值类型(标识符名称/常量值等)
typedef unsigned    row_t;   // 行号类型

const std::set<token_t> Keyword    = { "void", "int", "float", "if", "else", "while", "return" };
const std::set<token_t> Separator  = { ",", ";", "(", ")", "{", "}" };
const std::set<token_t> Operator   = { "+",  "-",  "*", "/", "=", "+=", "-=", "*=", "/=",
                                     "&&", "||", "!", ">", "<", ">=", "<=", "==", "!=" };
const token_t           Identifier = "<ID>";
const token_t           ConstInt   = "<INT>";
const token_t           ConstFloat = "<FLOAT>";

const std::set<token_t> All_Tokens = std::move([]() {
    std::set<token_t> tmp;
    tmp.insert(Keyword.begin(), Keyword.end());
    tmp.insert(Separator.begin(), Separator.end());
    tmp.insert(Operator.begin(), Operator.end());
    tmp.insert(Identifier);
    tmp.insert(ConstInt);
    tmp.insert(ConstFloat);
    return tmp;
}());

typedef struct Token {
    token_t token; // 对应单词符号
    value_t value; // 该符号的具体值
    row_t   row;   // 所在代码行
} Token;

class Lexical {
private:
    std::list<Token> token_stream;
    std::ifstream    fin;

public:
    Lexical() = delete;
    Lexical(const std::string &code_path) {
        this->fin = std::ifstream(code_path);
    }
    ~Lexical() {
        this->fin.close();
    }
    void
    scan();
    void
    print(std::ostream &out = std::cout);
    std::vector<Token>
    getTokenStream() {
        return std::vector<Token>{ token_stream.begin(), token_stream.end() };
    }
};

void
Lexical::print(std::ostream &out) {
    out << std::setw(16) << "token type";
    out << std::setw(16) << "token value";
    out << std::setw(8) << "row" << std::endl;
    for (const auto &token : this->token_stream) {
        out << std::setw(16) << token.token;
        out << std::setw(16) << token.value;
        out << std::setw(8) << token.row << std::endl;
    }
}

void
Lexical::scan() {
    this->fin.seekg(std::ios::beg);

    row_t   line = 1;
    char    tmp;
    token_t buf;

    // eof返回true当且仅当尝试读取文件流最后一个字符的后一个位置时被置位
    while (!this->fin.eof()) {
        // 此时尝试get读取字符之前 可能已经到文件尾 但eofbit未被置位
        // get之后 eofbit会被置位
        tmp = char(this->fin.get());
        if (this->fin.eof())
            break;

        if (isspace(tmp)) {
            if (tmp == '\n')
                ++line;
            continue;
        }
        buf = tmp;

        // 关键字或标识符
        if (isalpha(tmp)) {
            while (true) {
                tmp = char(this->fin.get());
                if (this->fin.eof())
                    break;
                if (isalpha(tmp) || isdigit(tmp))
                    buf += tmp;
                else
                    break;
            }
            this->fin.seekg(-1, std::ios::cur);
            // 关键字
            if (Keyword.find(buf) != Keyword.cend())
                this->token_stream.push_back({ buf, buf, line });
            // 标识符
            else
                this->token_stream.push_back({ Identifier, buf, line });
        }
        // 常量数值
        else if (isdigit(tmp)) {
            bool isfloat = false;
            while (true) {
                tmp = char(this->fin.get());
                if (this->fin.eof())
                    break;
                if (isdigit(tmp))
                    buf += tmp;
                else {
                    isfloat = (tmp == '.'); // 是浮点数
                    if (isfloat)
                        buf += tmp;
                    break;
                }
            }
            while (isfloat) {
                tmp = char(this->fin.get());
                if (this->fin.eof())
                    break;
                if (isdigit(tmp))
                    buf += tmp;
                else
                    break;
            }
            this->fin.seekg(-1, std::ios::cur);
            // 浮点数 (暂时只实现 . 小数点形式)
            if (isfloat)
                this->token_stream.push_back({ ConstFloat, buf, line });
            // 整数
            else
                this->token_stream.push_back({ ConstInt, buf, line });
        }
        // 分隔符
        else if (Separator.find(buf) != Separator.cend()) {
            this->token_stream.push_back({ buf, buf, line });
        }
        // 运算符
        else if (Operator.find(buf) != Operator.cend()) {
            tmp = char(this->fin.peek());
            if (this->fin.eof())
                break;
            if (buf == "/") {
                // 行注释
                if (tmp == '/') {
                    this->fin.seekg(1, std::ios::cur);
                    while ((tmp = char(this->fin.get())) != '\n' && this->fin.good())
                        ;
                    if (tmp == '\n')
                        ++line;
                }
                // 块注释
                else if (tmp == '*') {
                    char pre = '\0';
                    this->fin.seekg(1, std::ios::cur);
                    while (((tmp = char(this->fin.get())) != '/' || pre != '*') && this->fin.good()) {
                        pre = tmp;
                        if (pre == '\n')
                            ++line;
                    }
                }
                // '/=' 运算符
                else if (tmp == '=') {
                    buf += '=';
                    this->fin.seekg(1, std::ios::cur);
                    this->token_stream.push_back({ buf, buf, line });
                }
                // '/' 运算符
                else
                    this->token_stream.push_back({ buf, buf, line });
            }
            // 其他双符号长度运算符
            else if (Operator.find(buf + tmp) != Operator.cend()) {
                this->fin.seekg(1, std::ios::cur);
                this->token_stream.push_back({ buf + tmp, buf + tmp, line });
            }
            // 其他单符号长度运算符
            else
                this->token_stream.push_back({ buf, buf, line });
        } else {
            std::cout << "第 " << line << " 行，无法识别的单词符号 : " << (int)buf[0] << std::endl;
        }
    }
}

#endif // !_LEXICAL_ANALYSIS_HPP_
