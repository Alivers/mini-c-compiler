#ifndef _SEMANTIC_ANALYSIS_HPP_
#define _SEMANTIC_ANALYSIS_HPP_

#include <string>
#include <vector>

#include "./lexical_analysis.hpp"
#include "./grammatical_analysis.hpp"

/**
 * @brief ������������з��ŵľ�����Ϣ
 */
struct SymbolAttribute {
    token_t token;          /* ���ű�ʶ */
    value_t value;          /* ���ŵľ���ֵ */
    int     table_index;    /* ����������table��index */
    int     in_table_index; /* ����������table�ڲ���index */
    SymbolAttribute(const token_t& token = "", const value_t& value = "", const int table_idx = -1, const int in_table_idx = -1)
        : token(token), value(value), table_index(table_idx), in_table_index(in_table_idx) {}
};
/**
 * @brief ������������б�ʾ���ľ�����Ϣ
 */
struct IdentifierInfo {
    /**
     * @brief ����˵������int��float��void
     * @brief ��ʶ����𣺺�������������ʱ����������
     */
    enum SpecifierType { Int, Float, Void };
    enum IdentifierType { Function, Variable, TempVar, ConstVal };
    IdentifierType id_type; /* ��ʶ����� */
    SpecifierType  sp_type; /* ��(��)������/������������ */
    std::string    id_name; /* ��ʶ����/����ֵ */

    int parameter_num;        /* ������������ */
    int function_entry;       /* ������ڵ�ַ(��Ԫʽ�ı��) */
    int function_table_index; /* �����ĺ������ű�����������ķ��ű��б��е����� */
};

/**
 * @brief ���ű���
 */
class SymbolTable {
public:
    /**
     * @brief ���ű�ö�����Ͷ���
     */
    enum SymbolTableType {
        GlobalTable,   /* ȫ�ֱ� */
        FunctionTable, /* ������ */
        BlockTable,    /* �鼶�� */
        TempTable      /* ��ʱ�� */
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
            pos = -1; /* �Ѵ��� ���ʧ�� */
        }
        return pos;
    }

    IdentifierInfo& operator[](int pos) {
        return table_[pos];
    }

private:
    SymbolTableType             table_type_; /* ������ */
    std::vector<IdentifierInfo> table_;      /* �����б� */
    std::string                 table_name_; /* ���� */
};

/**
 * @brief ��Ԫʽ����
 */
struct Quadruple {
    int         label;   /* ��Ԫʽ�ı�� */
    std::string operate; /* �������� */
    std::string arg_1;   /* ���� 1 */
    std::string arg_2;   /* ���� 2 */
    std::string result;  /* ��� */
};

class Semantic {
public:
    static constexpr int Npos = -1;
    Semantic() {
        /* ����ȫ�ַ��ű� */
        tables_.push_back(SymbolTable(SymbolTable::GlobalTable, "global table"));
        /* ��ǰ������Ϊȫ�������� */
        current_table_stack_.push_back(0);

        /* ������ʱ��������һ������ */
        tables_.push_back(SymbolTable(SymbolTable::TempTable, "temp variable table"));

        /* �� 1 ��ʼ������Ԫʽ��ţ�0������ (j, -, -, main_address) */
        next_label_num_ = 1;
        /* main��������÷Ƿ� */
        main_label_ = Npos;
        /* ��ʼ������Ϊ0����ʾ����Ҫ���� */
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
    std::vector<SymbolAttribute> symbol_list_;         /* ����������̵ķ������� */
    std::vector<SymbolTable>     tables_;              /* �������з��ű����� */
    std::vector<int>             current_table_stack_; /* ��ǰ�������Ӧ�ķ��ű� ����ջ */

    int next_label_num_; /* ��һ����Ԫʽ�ı�� */

    std::vector<Quadruple> quadruples_;         /* ���ɵ���Ԫʽ */
    int                    backpatching_level_; /* ������ */
    std::vector<int>       backpatching_list_;  /* �����б� */

    int main_label_; /* main ������Ӧ����Ԫʽ��� */
};

bool Semantic::SemanticAnalysis(const Item &production) {
    return true;
}

#endif // !_SEMANTIC_ANALYSIS_HPP_
