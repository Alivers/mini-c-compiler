#ifndef _SEMANTIC_ANALYSIS_HPP_
#define _SEMANTIC_ANALYSIS_HPP_

#include <string>
#include <vector>

#include "./grammatical_analysis.hpp"
#include "./lexical_analysis.hpp"
#include "./util.hpp"

/**
 * @brief 语义分析过程中符号的具体信息
 */
struct SymbolAttribute {
    token_t token;          /* 符号标识 */
    value_t value;          /* 符号的具体值 */
    int     row;            /* 所在行号 */
    int     table_index;    /* 符号所处的table的index */
    int     in_table_index; /* 符号所处的table内部的index */
    SymbolAttribute(const token_t& token        = "",
                    const value_t& value        = "",
                    const int      row          = -1,
                    const int      table_idx    = -1,
                    const int      in_table_idx = -1)
        : token(token), value(value), row(row), table_index(table_idx), in_table_index(in_table_idx) {}
};
/**
 * @brief 语义分析过程中标识符的具体信息
 */
struct IdentifierInfo {
    /**
     * @brief 类型说明符：int、float、void
     * @brief 标识符类别：函数、变量、临时变量、常量
     */
    using SpecifierType = std::string;
    enum IdentifierType { Function, Variable, TempVar, ConstVar, ReturnVar };
    IdentifierType id_type; /* 标识符类别 */
    SpecifierType  sp_type; /* 变(常)量类型/函数返回类型 */
    std::string    id_name; /* 标识符名/常量值 */

    int parameter_num;        /* 函数参数个数 */
    int function_entry;       /* 函数入口地址(四元式的标号) */
    int function_table_index; /* 函数的函数符号表在整个程序的符号表列表中的索引 */

    IdentifierInfo() = default;
    IdentifierInfo(const IdentifierType id_type,
                   const SpecifierType& sp_type        = "",
                   const std::string&   id_name        = "",
                   const int            parameter_num  = 0,
                   const int            function_entry = -1,
                   const int            fun_table_idx  = -1)
        : id_type(id_type),
          sp_type(sp_type),
          id_name(id_name),
          parameter_num(parameter_num),
          function_entry(function_entry),
          function_table_index(fun_table_idx) {}
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
    SymbolTable(const SymbolTableType type, const std::string& name) : table_type_(type), table_name_(std::move(name)) {}

    SymbolTableType
    table_type(void) const {
        return this->table_type_;
    }
    std::string
    table_name(void) const {
        return this->table_name_;
    }
    std::vector<IdentifierInfo>&
    table(void) {
        return this->table_;
    }

    int
    FindSymbol(const std::string& id_name) const {
        int len = static_cast<int>(table_.size());
        for (int i = 0; i < len; ++i) {
            if (table_[i].id_name == id_name) {
                return i;
            }
        }
        return -1;
    }

    int
    AddSymbol(const IdentifierInfo& id) {
        int pos = FindSymbol(id.id_name);
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
    Quadruple(const int label, const std::string& ope, const std::string& arg1, const std::string& arg2, const std::string& res)
        : label(label), operate(ope), arg_1(arg1), arg_2(arg2), result(res) {}
};

/**
 * @brief 语义分析器
 */
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
        /* 临时变量计数 */
        temp_var_count = 0;
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

    std::string
    GetNewTmpVar() {
        return "T" + std::to_string(temp_var_count++);
    }

    void PrintQuadruple(std::ostream& os) {
        os << "label : operate, arg1, arg2, result" << std::endl;
        for (auto &qua : quadruples_) {
            os << qua.label << " : " << qua.operate << ", " << qua.arg_1 << ", " << qua.arg_2 << ", " << qua.result << std::endl;
        }
    }

    bool
    Analysis(const std::string& pro_left, const std::vector<std::string>& pro_right);

    void
    PrintQuadruple() const {}

private:
    std::vector<SymbolAttribute> symbol_list_;         /* 语义分析过程的符号数组 */
    std::vector<SymbolTable>     tables_;              /* 程序所有符号表数组 */
    std::vector<int>             current_table_stack_; /* 当前作用域对应的符号表 索引栈 */

    int next_label_num_; /* 下一个四元式的标号 */

    int temp_var_count; /* 临时变量计数 */

    std::vector<Quadruple> quadruples_;         /* 生成的四元式 */
    int                    backpatching_level_; /* 回填层次 */
    std::vector<int>       backpatching_list_;  /* 回填列表 */

    int main_label_; /* main 函数对应的四元式标号 */
};

bool
Semantic::Analysis(const std::string& pro_left, const std::vector<std::string>& pro_right) {
    if ("Program" == pro_left) {
        /* Program -> ExtDefList */
        if (Semantic::Npos == main_label_) {
            std::cerr << "语义错误 : 未定义 main 函数" << std::endl;
            return false;
        }
        PrintQuadruple();
        if ("@" != pro_right[0]) {
            int count = static_cast<int>(pro_right.size());
            while (count--) {
                this->symbol_list_.pop_back();
            }
        }
        this->symbol_list_.push_back(SymbolAttribute(pro_left));
    } else if ("ExtDef" == pro_left && "<ID>" == pro_right[1]) {
        /* ExtDef -> Specifier <ID> ; */
        int         list_length = static_cast<int>(symbol_list_.size());
        const auto& specifier   = symbol_list_[list_length - 3];
        const auto& identifier  = symbol_list_[list_length - 2];

        bool existed = false;
        for (int scope_layer = current_table_stack_.size() - 1; scope_layer >= 0; --scope_layer) {
            const auto& table = tables_[current_table_stack_[scope_layer]];
            if (table.FindSymbol(identifier.value) != -1) {
                existed = true;
                break;
            }
        }

        if (existed) {
            std::cerr << "语义错误 : 第 " << identifier.row << " 行，变量 " << identifier.value << " 重定义" << std::endl;
            return false;
        }

        IdentifierInfo variable;
        variable.id_name = identifier.value;
        variable.id_type = IdentifierInfo::Variable;
        variable.sp_type = IdentifierInfo::SpecifierType(specifier.value);

        tables_[current_table_stack_.back()].AddSymbol(variable);

        int count = static_cast<int>(pro_right.size());
        while (count--) {
            this->symbol_list_.pop_back();
        }
        this->symbol_list_.push_back(SymbolAttribute(pro_left, identifier.value, identifier.row));

    } else if ("Specifier" == pro_left) {
        /* Specifier -> void | int | float */
        int         list_length = static_cast<int>(symbol_list_.size());
        const auto& specifier   = symbol_list_[list_length - 1];
        int         count       = static_cast<int>(pro_right.size());
        while (count--) {
            this->symbol_list_.pop_back();
        }
        this->symbol_list_.push_back(SymbolAttribute(pro_left, specifier.value, specifier.row));
    } else if ("CreateFunTable_m" == pro_left) {
        /* CreateFunTable_m -> @ */
        /* 此时 symbol_list_ 的最后一个符号为 函数名
           FunDec -> <ID> CreateFunTable_m ( VarList )
           首先判断函数名是否重定义
         */
        int         list_length = static_cast<int>(symbol_list_.size());
        const auto& identifier  = symbol_list_[list_length - 1];
        const auto& specifier   = symbol_list_[list_length - 2];
        if (tables_[0].FindSymbol(identifier.value) != -1) {
            std::cerr << "语义错误 : 第 " << identifier.row << " 行，函数 " << identifier.value << " 重定义" << std::endl;
            return false;
        }
        /* 创建新的函数表 */
        tables_.push_back(SymbolTable(SymbolTable::FunctionTable, identifier.value));
        /* 在全局符号表中创建函数符号项 */
        tables_[0].AddSymbol(
            IdentifierInfo(IdentifierInfo::Function, specifier.value, identifier.value, 0, 0, tables_.size() - 1));
        /* 进入新的函数作用域 */
        current_table_stack_.push_back(tables_.size() - 1);

        IdentifierInfo return_val;
        return_val.id_type = IdentifierInfo::ReturnVar;
        return_val.id_name = tables_.back().table_name() + "_ret_val";
        return_val.sp_type = specifier.value;

        /* 记录main函数 */
        if (identifier.value == "main") {
            main_label_ = PeekNextLabelNum();
        }
        quadruples_.push_back(Quadruple(GetNextLabelNum(), identifier.value, "-", "-", "-"));
        /* 向函数表中加入返回变量 */
        tables_[current_table_stack_.back()].AddSymbol(return_val);
        /* 右部为空串 不需要pop */
        this->symbol_list_.push_back(SymbolAttribute(pro_left, identifier.value, identifier.row));
    } else if ("ExitFunTable_m" == pro_left) {
        /* ExitFunTable_m -> @ */
        /* 函数结束 退出作用域 */
        current_table_stack_.pop_back();
        /* 右部为空串 不需要pop */
        this->symbol_list_.push_back(SymbolAttribute(pro_left));
    } else if ("ParamDec" == pro_left) {
        /* ParamDec -> Specifier <ID> */
        int         list_length = static_cast<int>(symbol_list_.size());
        const auto& identifier  = symbol_list_[list_length - 1];
        const auto& specifier   = symbol_list_[list_length - 2];
        /* 获取当前函数表 */
        auto& function_table = tables_[current_table_stack_.back()];
        /* 获取当前函数在全局符号中的索引 */
        int   table_pos       = tables_[0].FindSymbol(function_table.table_name());
        auto& function_symbol = tables_[0][table_pos];

        if (-1 != function_table.FindSymbol(identifier.value)) {
            std::cerr << "语义错误 : 第 " << identifier.row << " 行，函数参数 " << identifier.value << " 重定义" << std::endl;
            return false;
        }
        /* 函数表中加入形参变量 */
        int new_var_pos = function_table.AddSymbol(IdentifierInfo(IdentifierInfo::Variable, specifier.value, identifier.value));
        /* 函数形参个数增加 */
        ++function_symbol.parameter_num;

        int count = static_cast<int>(pro_right.size());
        while (count--) {
            this->symbol_list_.pop_back();
        }
        this->symbol_list_.push_back(
            SymbolAttribute(pro_left, identifier.value, identifier.row, current_table_stack_.back(), new_var_pos));
    } else if ("Block" == pro_left) {
        /* Block -> Block_m { DefList StmtList } */
        int count = static_cast<int>(pro_right.size());
        while (count--) {
            this->symbol_list_.pop_back();
        }
        this->symbol_list_.push_back(SymbolAttribute(pro_left, std::to_string(PeekNextLabelNum())));
    } else if ("Stmt" == pro_left && "return" == pro_right[0]) {
        /* Stmt -> return Exp ; */
        int         list_length = static_cast<int>(symbol_list_.size());
        const auto& ret_exp     = symbol_list_[list_length - 2];
        auto&       fun_table   = tables_[current_table_stack_.back()];

        SymbolAttribute symbol_attr;
        if (!ret_exp.value.empty()) {
            const auto& fun_name = fun_table.table_name();
            std::string result   = fun_table[0].id_name;
            std::string arg_1    = ret_exp.value;
            quadruples_.push_back(Quadruple(GetNextLabelNum(), ":=", arg_1, "-", result));
            symbol_attr.value = ret_exp.value;
        }
        symbol_attr.token = pro_left;

        quadruples_.push_back(Quadruple(GetNextLabelNum(), "return", "-", "-", fun_table.table_name()));

        int count = static_cast<int>(pro_right.size());
        while (count--) {
            this->symbol_list_.pop_back();
        }
        this->symbol_list_.push_back(symbol_attr);
    } else if ("IfStmt_m1" == pro_left) {
        /* IfStmt_m1 -> @ */
        ++backpatching_level_;
        symbol_list_.push_back(SymbolAttribute(pro_left, std::to_string(PeekNextLabelNum())));
    } else if ("IfStmt_m2" == pro_left) {
        /* IfStmt_m2 -> @ */
        int         list_len = static_cast<int>(symbol_list_.size());
        const auto& if_exp   = symbol_list_[list_len - 2];

        /* 待回填四元式 : 假出口 */
        quadruples_.push_back(Quadruple(GetNextLabelNum(), "j=", if_exp.value, "0", ""));
        backpatching_list_.push_back(quadruples_.size() - 1);
        /* 待回填四元式 : 真出口 */
        quadruples_.push_back(Quadruple(GetNextLabelNum(), "j", "-", "-", ""));
        backpatching_list_.push_back(quadruples_.size() - 1);

        symbol_list_.push_back(SymbolAttribute(pro_left, std::to_string(PeekNextLabelNum())));
    } else if ("IfNext" == pro_left && "IfStmt_next" == pro_right[0]) {
        /* IfNext -> IfStmt_next else Block */
        int         list_len  = static_cast<int>(symbol_list_.size());
        const auto& if_stmt_n = symbol_list_[list_len - 3];

        int count = static_cast<int>(pro_right.size());
        while (count--) {
            this->symbol_list_.pop_back();
        }
        this->symbol_list_.push_back(SymbolAttribute(pro_left, if_stmt_n.value));
    } else if ("IfStmt_next" == pro_left) {
        /* IfStmt_next -> @ */
        /* If 的跳出语句(else 之前) */
        quadruples_.push_back(Quadruple(GetNextLabelNum(), "j", "-", "-", ""));
        backpatching_list_.push_back(quadruples_.size() - 1);

        symbol_list_.push_back(SymbolAttribute(pro_left, std::to_string(PeekNextLabelNum())));
    } else if ("IfStmt" == pro_left) {
        /* IfStmt -> if IfStmt_m1 ( Exp ) IfStmt_m2 Block IfNext */
        int         list_len = static_cast<int>(symbol_list_.size());
        const auto& if_m2    = symbol_list_[list_len - 3];
        const auto& if_next  = symbol_list_[list_len - 1];

        if (if_next.value.empty()) {
            /* 只有 if  */
            /* 真出口 */
            int pos = backpatching_list_.back();
            backpatching_list_.pop_back();
            quadruples_[pos].result = if_m2.value;
            /* 假出口 */
            pos = backpatching_list_.back();
            backpatching_list_.pop_back();
            quadruples_[pos].result = std::to_string(PeekNextLabelNum());
        } else {
            /* if - else */
            /* if 块出口 */
            int pos = backpatching_list_.back();
            backpatching_list_.pop_back();
            quadruples_[pos].result = std::to_string(PeekNextLabelNum());
            /* if 真出口 */
            pos = backpatching_list_.back();
            backpatching_list_.pop_back();
            quadruples_[pos].result = if_m2.value;
            /* if 假出口 */
            pos = backpatching_list_.back();
            backpatching_list_.pop_back();
            quadruples_[pos].result = if_next.value;
        }
        --backpatching_level_;

        int count = static_cast<int>(pro_right.size());
        while (count--) {
            this->symbol_list_.pop_back();
        }
        this->symbol_list_.push_back(SymbolAttribute(pro_left));
    } else if ("WhileStmt_m1" == pro_left) {
        /* WhileStmt_m1 -> @ */
        ++backpatching_level_;
        this->symbol_list_.push_back(SymbolAttribute(pro_left, std::to_string(PeekNextLabelNum())));
    } else if ("WhileStmt_m2" == pro_left) {
        /* WhileStmt_m2 -> @ */
        int         list_len  = static_cast<int>(symbol_list_.size());
        const auto& while_exp = symbol_list_[list_len - 2];

        /* 待回填四元式 : 假出口 */
        quadruples_.push_back(Quadruple(GetNextLabelNum(), "j=", while_exp.value, "0", ""));
        backpatching_list_.push_back(quadruples_.size() - 1);
        /* 待回填四元式 : 真出口 */
        quadruples_.push_back(Quadruple(GetNextLabelNum(), "j", "-", "-", ""));
        backpatching_list_.push_back(quadruples_.size() - 1);

        this->symbol_list_.push_back(SymbolAttribute(pro_left, std::to_string(PeekNextLabelNum())));
    } else if ("WhileStmt" == pro_left) {
        /* WhileStmt -> while WhileStmt_m1 ( Exp ) WhileStmt_m2 Block */
        int         list_len = static_cast<int>(symbol_list_.size());
        const auto& while_m1 = symbol_list_[list_len - 6];
        const auto& while_m2 = symbol_list_[list_len - 2];

        /* 无条件跳转到 while 的条件判断语句处 */
        quadruples_.push_back(Quadruple(GetNextLabelNum(), "j", "-", "-", while_m1.value));

        /* 回填 : 真出口 */
        int pos = backpatching_list_.back();
        backpatching_list_.pop_back();
        quadruples_[pos].result = while_m2.value;
        /* 回填 : 假出口 */
        pos = backpatching_list_.back();
        backpatching_list_.pop_back();
        quadruples_[pos].result = std::to_string(PeekNextLabelNum());

        --backpatching_level_;

        int count = static_cast<int>(pro_right.size());
        while (count--) {
            this->symbol_list_.pop_back();
        }
        this->symbol_list_.push_back(SymbolAttribute(pro_left));
    } else if ("Dec" == pro_left && (pro_right.size() <= 1)) {
        /* Dec -> <ID> */
        int         list_len      = static_cast<int>(symbol_list_.size());
        const auto& identifier    = symbol_list_.back();
        const auto& specifier     = symbol_list_[list_len - 2];
        auto&       current_table = tables_[current_table_stack_.back()];

        if (-1 != current_table.FindSymbol(identifier.value)) {
            std::cerr << "语义错误 : 第 " << identifier.row << " 行，变量 " << identifier.value << " 重定义" << std::endl;
            return false;
        }

        current_table.AddSymbol(IdentifierInfo(IdentifierInfo::Variable, specifier.value, identifier.value));

        int count = static_cast<int>(pro_right.size());
        while (count--) {
            this->symbol_list_.pop_back();
        }
        this->symbol_list_.push_back(SymbolAttribute(pro_left, identifier.value));
    } else if ("Dec" == pro_left && (pro_right.size() <= 1)) {
        /* Dec -> <ID> = Exp */
        int         list_len      = static_cast<int>(symbol_list_.size());
        const auto& identifier    = symbol_list_[list_len - 3];
        const auto& specifier     = symbol_list_[list_len - 4];
        auto&       current_table = tables_[current_table_stack_.back()];

        if (-1 != current_table.FindSymbol(identifier.value)) {
            std::cerr << "语义错误 : 第 " << identifier.row << " 行，变量 " << identifier.value << " 重定义" << std::endl;
            return false;
        }

        current_table.AddSymbol(IdentifierInfo(IdentifierInfo::Variable, specifier.value, identifier.value));

        int count = static_cast<int>(pro_right.size());
        while (count--) {
            this->symbol_list_.pop_back();
        }
        this->symbol_list_.push_back(SymbolAttribute(pro_left, identifier.value));
    } else if ("Aritop" == pro_left) {
        /* Aritop -> + | - | * | / */
        const auto& op = symbol_list_.back();

        int count = static_cast<int>(pro_right.size());
        while (count--) {
            this->symbol_list_.pop_back();
        }
        this->symbol_list_.push_back(SymbolAttribute(pro_left, op.value));
    } else if ("Assignop" == pro_left) {
        /* Assignop -> = | += | -= | *= | /= */
        const auto& op = symbol_list_.back();

        int count = static_cast<int>(pro_right.size());
        while (count--) {
            this->symbol_list_.pop_back();
        }
        this->symbol_list_.push_back(SymbolAttribute(pro_left, op.value));
    } else if ("Relop" == pro_left) {
        /* Relop -> > | < | >= | <= | == | != */
        const auto& op = symbol_list_.back();

        int count = static_cast<int>(pro_right.size());
        while (count--) {
            this->symbol_list_.pop_back();
        }
        this->symbol_list_.push_back(SymbolAttribute(pro_left, op.value));
    } else if ("CallFunCheck" == pro_left) {
        /* CallFunCheck -> @ */
        int         list_len = static_cast<int>(symbol_list_.size());
        const auto& fun_id   = symbol_list_[list_len - 2];

        int fun_id_pos = tables_[0].FindSymbol(fun_id.value);
        if (-1 == fun_id_pos) {
            std::cerr << "语义错误 : 第 " << fun_id.row << " 行，调用函数 " << fun_id.value << " 未定义" << std::endl;
            return false;
        }
        if (tables_[0][fun_id_pos].id_type != IdentifierInfo::Function) {
            std::cerr << "语义错误 : 第 " << fun_id.row << " 行，调用函数 " << fun_id.value << " 未定义" << std::endl;
            return false;
        }
        symbol_list_.push_back(SymbolAttribute(pro_left, "", -1, 0, fun_id_pos));
    } else if ("Args" == pro_left && pro_right[0] == "@") {
        /* Args -> @ */
        /* 这里 value = 0 表示该产生式产生 0 个函数实参 */
        this->symbol_list_.push_back(SymbolAttribute(pro_left, "0"));
    } else if ("Args" == pro_left && pro_right.back() == "Exp") {
        /* Args -> Exp */
        const auto& exp = symbol_list_.back();
        quadruples_.push_back(Quadruple(GetNextLabelNum(), "param", exp.value, "-", "-"));
        int count = static_cast<int>(pro_right.size());
        while (count--) {
            this->symbol_list_.pop_back();
        }
        this->symbol_list_.push_back(SymbolAttribute(pro_left, "1"));
    } else if ("Args" == pro_left) {
        /* Args -> Exp , Args */
        int list_len = static_cast<int>(symbol_list_.size());
        const auto& exp = symbol_list_[list_len - 3];
        quadruples_.push_back(Quadruple(GetNextLabelNum(), "param", exp.value, "-", "-"));
        int aru_num = std::stoi(symbol_list_.back().value) + 1;
        int count   = static_cast<int>(pro_right.size());
        while (count--) {
            this->symbol_list_.pop_back();
        }
        this->symbol_list_.push_back(SymbolAttribute(pro_left, std::to_string(aru_num)));
    } else if ("Exp" == pro_left && "<ID>" == pro_right[0] && pro_right.back() != "<ID>" && pro_right.back() != "Exp") {
        /* Exp -> <ID> ( CallFunCheck Args ) */
        int         list_len   = static_cast<int>(symbol_list_.size());
        const auto& identifier = symbol_list_[list_len - 5];
        const auto& args       = symbol_list_[list_len - 2];
        const auto& check      = symbol_list_[list_len - 3];

        int para_num = tables_[check.table_index][check.in_table_index].parameter_num;
        if (para_num > std::stoi(args.value)) {
            std::cerr << "语义错误 : 第 " << identifier.row << " 行, 调用函数" << identifier.value << ", 所给参数过少"
                      << std::endl;
            return false;
        } else if (para_num < std::stoi(args.value)) {
            std::cerr << "语义错误 : 第 " << identifier.row << " 行, 调用函数" << identifier.value << ", 所给参数过多"
                      << std::endl;
            return false;
        }
        /* 生成函数调用四元式 */
        std::string new_tmp_var = GetNewTmpVar();
        quadruples_.push_back(Quadruple(GetNextLabelNum(), "call", identifier.value, "-", new_tmp_var));

        int count = static_cast<int>(pro_right.size());
        while (count--) {
            this->symbol_list_.pop_back();
        }
        /* 新的exp的value为临时变量名 */
        this->symbol_list_.push_back(SymbolAttribute(pro_left, new_tmp_var));
    } else if ("Exp" == pro_left && "<ID>" == pro_right[0] && "<ID>" != pro_right.back()) {
        /* Exp -> <ID> Assignop Exp */
        int         list_len = static_cast<int>(symbol_list_.size());
        const auto& id       = symbol_list_[list_len - 3];
        const auto& sub_exp  = symbol_list_.back();
        const auto& op       = symbol_list_[list_len - 2];

        if (op.value.size() == 1) {
            quadruples_.push_back(Quadruple(GetNextLabelNum(), ":" + op.value, sub_exp.value, "-", id.value));
        } else {
            quadruples_.push_back(Quadruple(GetNextLabelNum(), op.value, id.value, sub_exp.value, id.value));
        }


        int count = static_cast<int>(pro_right.size());
        while (count--) {
            this->symbol_list_.pop_back();
        }
        /* 新的exp的value为临时变量名 */
        this->symbol_list_.push_back(SymbolAttribute(pro_left, id.value));
    } else if ("Exp" == pro_left && "<ID>" == pro_right[0]) {
        /* Exp -> <ID> */
        const auto& id = symbol_list_.back();
        /* todo : whether the <ID> was defined */
        int count = static_cast<int>(pro_right.size());
        while (count--) {
            this->symbol_list_.pop_back();
        }
        this->symbol_list_.push_back(SymbolAttribute(pro_left, id.value));
    } else if ("Exp" == pro_left && ("<INT>" == pro_right[0] || "<FLOAT>" == pro_right[0])) {
        /* Exp -> <INT> | <FLOAT> */
        const auto& const_val = symbol_list_.back();

        int count = static_cast<int>(pro_right.size());
        while (count--) {
            this->symbol_list_.pop_back();
        }
        this->symbol_list_.push_back(SymbolAttribute(pro_left, const_val.value));
    } else if ("Exp" == pro_left) {
        int         list_len = static_cast<int>(symbol_list_.size());
        std::string new_exp_val;
        if ("(" == pro_right[0] && pro_right.size() == 3u) {
            /* Exp -> ( Exp ) */
            const auto& sub_exp = symbol_list_[list_len - 2];
            new_exp_val = sub_exp.value;
        } else if (pro_right[1] == "Relop") {
            /* Exp -> Exp Relop Exp */
            const auto& sub_exp1 = symbol_list_[list_len - 3];
            const auto& op       = symbol_list_[list_len - 2];
            const auto& sub_exp2 = symbol_list_[list_len - 1];
            int next_label_num = GetNextLabelNum();
            std::string new_tmp_var = GetNewTmpVar();
            quadruples_.push_back(Quadruple(next_label_num, "j" + op.value, sub_exp1.value, sub_exp2.value, std::to_string(next_label_num + 3)));
            quadruples_.push_back(Quadruple(GetNextLabelNum(), ":=", "0", "-", new_tmp_var));
            quadruples_.push_back(Quadruple(GetNextLabelNum(), "j", "-", "-", std::to_string(next_label_num + 4)));
            quadruples_.push_back(Quadruple(GetNextLabelNum(), ":=", "1", "-", new_tmp_var));

            new_exp_val = new_tmp_var;
        } else if (pro_right[1] == "Aritop") {
            /* Exp -> Exp Aritop Exp */
            const auto& sub_exp1= symbol_list_[list_len - 3];
            const auto& op = symbol_list_[list_len - 2];
            const auto& sub_exp2 = symbol_list_[list_len - 1];
            std::string new_tmp_var = GetNewTmpVar();
            quadruples_.push_back(Quadruple(GetNextLabelNum(), op.value, sub_exp1.value, sub_exp2.value, new_tmp_var));

            new_exp_val = new_tmp_var;
        }
        int count = static_cast<int>(pro_right.size());
        while (count--) {
            this->symbol_list_.pop_back();
        }
        this->symbol_list_.push_back(SymbolAttribute(pro_left, new_exp_val));
    } else {
        /* ExtDefList -> ExtDef ExtDefList | @ */
        /* ExtDef -> Specifier FunDec Block ExitFunTable_m */
        /* FunDec -> <ID> CreateFunTable_m ( VarList ) */
        /* VarList -> ParamDec , VarList | ParamDec | @ */
        /* Block_m -> @ */
        /* StmtList -> Stmt StmtList | @ */
        /* Stmt -> IfSttmt | WhileStmt | Exp ; */
        /* IfNext -> @ */
        /* DefList -> Def DefList | @ */
        /* Def -> Specifier Dec ; */
        if (pro_right[0] != "@") {
            int count = static_cast<int>(pro_right.size());
            while (count--) {
                this->symbol_list_.pop_back();
            }
        }
        this->symbol_list_.push_back(SymbolAttribute(pro_left));
    }
    return true;
}

#endif // !_SEMANTIC_ANALYSIS_HPP_
