#ifndef _SEMANTIC_ANALYSIS_HPP_
#define _SEMANTIC_ANALYSIS_HPP_

#include <string>
#include <vector>

#include "./lexical_analysis.hpp"
#include "./grammatical_analysis.hpp"

/**
 * @brief 语义分析过程中符号的具体信息
 */
struct SymbolAttribute {
    token_t token;          /* 符号标识 */
    value_t value;          /* 符号的具体值 */
    int     table_index;    /* 符号所处的table的index */
    int     in_table_index; /* 符号所处的table内部的index */
    SymbolAttribute(const token_t& token = "", const value_t& value = "", const int table_idx = -1, const int in_table_idx = -1)
        : token(token), value(value), table_index(table_idx), in_table_index(in_table_idx) {}
};
/**
 * @brief 语义分析过程中表示符的具体信息
 */
struct IdentifierInfo {
    /**
     * @brief 类型说明符：int、float、void
     * @brief 标识符类别：函数、变量、临时变量、常量
     */
    enum SpecifierType { Int, Float, Void };
    enum IdentifierType { Function, Variable, TempVar, ConstVal };
    IdentifierType id_type; /* 标识符类别 */
    SpecifierType  sp_type; /* 变(常)量类型/函数返回类型 */
    std::string    id_name; /* 标识符名/常量值 */

    int parameter_num;        /* 函数参数个数 */
    int function_entry;       /* 函数入口地址(四元式的标号) */
    int function_table_index; /* 函数的函数符号表在整个程序的符号表列表中的索引 */
};

/**
 * @brief 符号表定义
 */
class SymbolTable {
public:
    /**
     * @brief 符号表枚举类型定义
     */
    enum SymbolTableType {
        GlobalTable,   /* 全局表 */
        FunctionTable, /* 函数表 */
        BlockTable,    /* 块级表 */
        TempTable      /* 临时表 */
    };

public:
    SymbolTable(const SymbolTableType type, std::string&& name) : table_type_(type), table_name_(std::move(name)) {}

    SymbolTableType
    table_type(void) {
        return this->table_type_;
    }
    std::string
    table_name(void) {
        return this->table_name_;
    }
    std::vector<IdentifierInfo>&
    table(void) {
        return this->table_;
    }

    int
    FindSymbol(const IdentifierInfo& id) {
        int len = static_cast<int>(table_.size());
        for (int i = 0; i < len; ++i) {
            if (table_[i].id_name == id.id_name) {
                return i;
            }
        }
        return -1;
    }

    int
    AddSymbol(const IdentifierInfo& id) {
        int pos = FindSymbol(id);
        if (pos == -1) {
            table_.push_back(id);
            pos = static_cast<int>(table_.size() - 1);
        } else {
            pos = -1; /* 已存在 添加失败 */
        }
        return pos;
    }

    IdentifierInfo& operator[](int pos) {
        return table_[pos];
    }

private:
    SymbolTableType             table_type_; /* 表类型 */
    std::vector<IdentifierInfo> table_;      /* 符号列表 */
    std::string                 table_name_; /* 表名 */
};

/**
 * @brief 四元式定义
 */
struct Quadruple {
    int         label;   /* 四元式的标号 */
    std::string operate; /* 操作类型 */
    std::string arg_1;   /* 参数 1 */
    std::string arg_2;   /* 参数 2 */
    std::string result;  /* 结果 */
};

class Semantic {
public:
    static constexpr int Npos = -1;
    Semantic() {
        /* 创建全局符号表 */
        tables_.push_back(SymbolTable(SymbolTable::GlobalTable, "global table"));
        /* 当前作用域为全局作用域 */
        current_table_stack_.push_back(0);

        /* 所有临时变量存在一个表中 */
        tables_.push_back(SymbolTable(SymbolTable::TempTable, "temp variable table"));

        /* 从 1 开始生成四元式标号；0号用于 (j, -, -, main_address) */
        next_label_num_ = 1;
        /* main函数标号置非法 */
        main_label_ = Npos;
        /* 初始回填层次为0，表示不需要回填 */
        backpatching_level_ = 0;

    }

    int
    GetNextLabelNum() {
        return next_label_num_++;
    }

    int
    PeekNextLabelNum() {
        return next_label_num_;
    }

    bool
    AddSymbolToList(const SymbolAttribute& symbol) {
        symbol_list_.push_back(symbol);
        return true;
    }

    bool
    SemanticAnalysis(const Item& production);

private:
    std::vector<SymbolAttribute> symbol_list_;         /* 语义分析过程的符号数组 */
    std::vector<SymbolTable>     tables_;              /* 程序所有符号表数组 */
    std::vector<int>             current_table_stack_; /* 当前作用域对应的符号表 索引栈 */

    int next_label_num_; /* 下一个四元式的标号 */

    std::vector<Quadruple> quadruples_;         /* 生成的四元式 */
    int                    backpatching_level_; /* 回填层次 */
    std::vector<int>       backpatching_list_;  /* 回填列表 */

    int main_label_; /* main 函数对应的四元式标号 */
};

bool Semantic::SemanticAnalysis(const Item &production) {
    return true;
}

#endif // !_SEMANTIC_ANALYSIS_HPP_
