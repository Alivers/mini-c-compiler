/**
 * @file grammatical_analysis.hpp
 * @author Aliver (aliver.len@qq.com)
 * @brief
 * @version 0.1
 * @date 2020-05-08
 *
 * @copyright Copyright (c) 2020
 *
 */

#ifndef _GRAMMATICAL_ANALYSIS_HPP_
#define _GRAMMATICAL_ANALYSIS_HPP_

/*!
 * 语法分析
 */

#include <fstream>
#include <iomanip>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <stack>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "lexical_analysis.hpp"
#include "semantic_analysis.hpp"
#include "util.hpp"
/**
 * @brief 符号类型 空串/终结符/非终结符/终止符号(实际并不存在)
 *        id                - 符号的字符串标识(唯一)
 *        type              - 符号类型
 *        can_reach_empty   - 非终结符是否能经过若干步推导得到空串
 *        first_set         - 终结符/非终结符的first集合
 *        follow_set        - 非终结符的fllow集合
 */
typedef struct Symbol {
    using collection_t = std::set<int>;
    enum Type { Epsilon, Terminal, NonTerminal, EndToken };
    std::string id;
    Type        type;
    // bool         can_reach_empty;
    collection_t first_set;
    collection_t follow_set;

    Symbol(const std::string id, Symbol::Type type) : id(id), type(type) {}
} Symbol;

/**
 * @brief 文法类；包含产生式集合
 */
class Grammar {
public:
    std::vector<Symbol> symbols;          /* 所有文法符号 */
    std::set<int>       terminals;        /* 终结符集合(存储终结符在symbols数组中的position) */
    std::set<int>       non_terminals;    /* 非终结符集合(非终结符在symbols数组中的position) */
    std::vector<Item>   productions;      /* 产生式集合 */
    int                 start_production; /* 起始产生式：S->Program在productions数组中的位置 */

    static constexpr int   Npos        = -1;                      // 非法位置
    static constexpr char* EmptyStr    = (char* const) "@";       // Epsilon
    static constexpr char* SplitStr    = (char* const) " | ";     // 产生式右部分隔符
    static constexpr char* ProToken    = (char* const) "->";      // 产生式左右部分隔符
    static constexpr char* EndToken    = (char* const) "#";       // 尾token 终止符号
    static constexpr char* StartToken  = (char* const) "Program"; // 文法起始符号
    static constexpr char* ExtendStart = (char* const) "S";       // 扩展文法起始符号

    int
    get_symbol_index_by_id(const std::string& id) {
        int length = symbols.size();
        for (int i = 0; i < length; ++i) {
            if (id == symbols[i].id) {
                return i;
            }
        }
        return Grammar::Npos;
    }

public:
    explicit Grammar(const std::string& grammar_path) {
        readProductions(grammar_path);

        // for (auto &non : non_terminals) {
        //     symbols[non].can_reach_empty = canDeriveEmpty(non);
        // }

        getFirstOfTerminal();
        getFirstOfNonterminal();
        // getFollowOfNonTerminal();
    }

    bool
    isNonTerminal(int symbol_index) {
        if (symbol_index < 0 || symbol_index >= static_cast<int>(symbols.size()))
            return false;
        return symbols[symbol_index].type == Symbol::NonTerminal;
    }

    bool
    isTerminal(int symbol_index) {
        if (symbol_index < 0 || symbol_index >= static_cast<int>(symbols.size()))
            return false;
        return (symbols[symbol_index].type == Symbol::Terminal || symbols[symbol_index].type == Symbol::EndToken);
    }

    bool
    isEpsilon(int symbol_index) {
        if (symbol_index < 0 || symbol_index >= static_cast<int>(symbols.size()))
            return false;
        return (symbols[symbol_index].type == Symbol::Epsilon);
    }

    bool
    isEndToken(int symbol_index) {
        if (symbol_index < 0 || symbol_index >= static_cast<int>(symbols.size()))
            return false;
        return (symbols[symbol_index].type == Symbol::EndToken);
    }

    std::set<int>
    getFirstOfProduction(const std::vector<int>& right) {
        std::set<int> FirstSet;
        if (right.empty())
            return FirstSet;
        auto it = right.begin();

        // 若是终结符或空串 加入后返回
        if (isTerminal(*it) || symbols[*it].type == Symbol::Epsilon) {
            FirstSet.insert(*it);
            return FirstSet;
        }
        /* flag用于判断最终空串是否要加入到first集合 */
        bool flag = true;
        for (; it != right.end(); ++it) {
            // 初次进循环一定不是终结符
            // 若是终结符，加入后直接退出——表示该右部不可能产生空串
            if (isTerminal(*it)) {
                mergeSetExceptEmpty(FirstSet, symbols[*it].first_set);
                flag = false;
                break;
            }
            // 如是非终结符 合并first集合
            mergeSetExceptEmpty(FirstSet, symbols[*it].first_set);
            // 若当前非终结符可推导出空串，继续循环，否则退出
            flag = flag && symbols[*it].first_set.count(get_symbol_index_by_id(EmptyStr));
            if (!flag)
                break;
        }
        // 若该右部经过若干步推导可产生空串，First集合中加入空串
        if (flag && it == right.end()) {
            FirstSet.insert(get_symbol_index_by_id(EmptyStr));
        }
        return FirstSet;
    }

private:
    void
    readProductions(const std::string& file) {
        std::ifstream grammerIn(file, std::ios::in);
        if (!grammerIn.is_open()) {
            return;
        }

        /* 添加 '#' 终止符号和 epsilon空串 */
        symbols.push_back(Symbol(EndToken, Symbol::EndToken));
        terminals.insert(symbols.size() - 1); /* '#'认为是终结符 */
        symbols.push_back(Symbol(EmptyStr, Symbol::Epsilon));

        std::string tmp;
        while (std::getline(grammerIn, tmp, '\n')) {
            // 忽略空行和注释
            if (trim(tmp).empty() || tmp[0] == '#') {
                continue;
            }

            // 将产生式分割为左部和右部
            std::string left, right;
            auto        strs_p = split(tmp, ProToken);
            if (strs_p->size() == 2) {
                left  = std::move(strs_p->front());
                right = std::move(strs_p->back());
            }
            // 分隔多个产生式
            auto rightSecs_p = split(right, SplitStr);
            if (left == "%token") {
                /* 先插入所有终结符 */
                for (auto& str : *rightSecs_p) {
                    symbols.push_back(Symbol(str, Symbol::Terminal));
                    terminals.insert(symbols.size() - 1);
                }
            } else {
                int left_index = get_symbol_index_by_id(left);
                if (left_index == Npos) {
                    symbols.push_back(Symbol(left, Symbol::NonTerminal));
                    left_index = symbols.size() - 1;
                    non_terminals.insert(left_index);
                }
                for (auto& str : *rightSecs_p) {
                    // 将单一产生式分隔为基本单元
                    auto             basicUnit_p = split(str, " ");
                    std::vector<int> right_index;
                    for (auto& right_unit : *basicUnit_p) {
                        int right_unit_index = get_symbol_index_by_id(right_unit);
                        if (right_unit_index == Npos) {
                            /* 如果不存在 一定为非终结符 插入 */
                            symbols.push_back(Symbol(right_unit, Symbol::NonTerminal));
                            right_unit_index = symbols.size() - 1;
                            non_terminals.insert(right_unit_index);
                        }
                        right_index.push_back(right_unit_index);
                    }
                    productions.push_back(Item(left_index, std::move(right_index)));
                    if (symbols[left_index].id == ExtendStart) {
                        start_production = productions.size() - 1;
                    }
                }
            }
        }
        grammerIn.close();
    }

    bool
    mergeSetExceptEmpty(std::set<int>& des, const std::set<int>& src) {
        if (&des == &src)
            return false;
        int  epsilon_index = get_symbol_index_by_id(EmptyStr);
        bool desExisted    = des.find(epsilon_index) != des.end();
        // bool srcExisted = src.find(EmptyStr) != src.end();
        auto beforeInsert = des.size();
        if (desExisted) {
            des.insert(src.begin(), src.end());
        } else {
            /* 如果des中不存在空串 则删除src中可能存在的空串 */
            des.insert(src.begin(), src.end());
            des.erase(epsilon_index);
        }
        return beforeInsert < des.size();
    }

    bool
    mergeSet(std::set<int>& des, const std::set<int>& src) {
        if (&des == &src)
            return false;
        auto beforeInsert = des.size();
        des.insert(src.begin(), src.end());
        return beforeInsert < des.size();
    }

    void
    getFirstOfTerminal() {
        // 终结符的First集合为自身
        for (auto& ter : terminals) {
            symbols[ter].first_set.insert(ter);
        }
    }

    void
    getFirstOfNonterminal() {
        // 标记 直到所有集合不发生变化
        bool changed;
        while (true) {
            changed = false;
            // 遍历所有非终结符
            for (auto& nonTerminal : non_terminals) {

                for (auto& production : productions) {
                    if (production.left != nonTerminal)
                        continue;
                    // 找到对应产生式，遍历产生式右部
                    auto it = production.right.begin();

                    // 是终结符直接加入first集合并退出——改产生式不能继续使当前非终结符的First集合扩大
                    if (isTerminal(*it) || symbols[*it].type == Symbol::Epsilon) {
                        // 短路运算  不能交换位置
                        changed = symbols[nonTerminal].first_set.insert(*it).second || changed;
                        continue;
                    }
                    // 右部以非终结符开始
                    bool flag = true; // 可推导出空串的标记
                    for (; it != production.right.end(); ++it) {
                        // 如果是终结符，停止迭代
                        if (isTerminal(*it)) {
                            changed = mergeSetExceptEmpty(symbols[nonTerminal].first_set, symbols[*it].first_set) || changed;
                            flag    = false;
                            break;
                        }

                        changed = mergeSetExceptEmpty(symbols[nonTerminal].first_set, symbols[*it].first_set) || changed;
                        // 若该非终结符可推导出空串，则继续迭代
                        flag = flag && symbols[*it].first_set.count(get_symbol_index_by_id(EmptyStr));

                        // 否则直接结束当前产生式的处理
                        if (!flag)
                            break;
                    }
                    // 如果该产生式的所有右部均为非终结符且均可推导出空串，则将空串加入First集合
                    if (flag && it == production.right.end()) {
                        changed = symbols[nonTerminal].first_set.insert(get_symbol_index_by_id(EmptyStr)).second || changed;
                    }
                }
            }
            if (!changed)
                break;
        }
    }

    void
    getFollowOfNonTerminal() {
        // 初始化开始符号
        int start_index = get_symbol_index_by_id(ExtendStart);
        assert(start_index != Npos);

        symbols[start_index].follow_set.insert(get_symbol_index_by_id(EndToken));

        bool changed;
        while (true) {
            changed = false;
            // 遍历所有非终结符
            for (auto& non : non_terminals) {
                // 对每一个非终结符，遍历所有产生式
                for (auto& production : productions) {
                    // 每个产生式 遍历产生式右部 查找当前非终结符的出现位置
                    for (auto it = production.right.begin(); it != production.right.end(); ++it) {
                        if ((*it) != non)
                            continue;
                        // non在产生式中的后缀
                        std::vector<int> suffix(it + 1, production.right.end());
                        std::set<int>    suffixFirst(getFirstOfProduction(suffix));

                        // 若存在 B->aA 将Follow(B)加入Follow(A)
                        // 若存在 B->aAb 且 First(b)中含Empty，则将Follow(B)加入Follow(A)
                        if (suffix.empty() || suffixFirst.find(get_symbol_index_by_id(EmptyStr)) != suffixFirst.end()) {
                            changed = mergeSet(symbols[non].follow_set, symbols[production.left].follow_set) || changed;
                        }
                        // 若存在 B->aAb 将 First(b)-Empty 加入Follow(A)
                        if (!suffix.empty()) {
                            changed = mergeSetExceptEmpty(symbols[non].follow_set, suffixFirst) || changed;
                        }
                    }
                }
            }
            if (!changed)
                break;
        }
    }

    /*     bool
        canDeriveEmpty(int symbol_index) {
            if (isTerminal(symbol_index))
                return false;
            else if (isNonTerminal(symbol_index)) {

                for (auto & production : productions) {
                    // 找到非终结符对应的产生式
                    if (symbol_index == production.left) {

                        // 存在直接产生空串的产生式
                        if (symbols[production.right.front()].type ==
       Symbol::Epsilon) { return true;
                        }
                        // 否则：对当前产生式的所有右部(终结符和非终结符组成)
                        // 若当前产生式的右部全部可多步推导出空串，返回true
                        // 只要有一个符号无法推导出空串，返回false
                        else {
                            bool flag = true;
                            for (auto ch : production.right) {
                                flag = flag && canDeriveEmpty(ch);
                                if (!flag)
                                    break;
                            }
                            // 两种情况：1. 由break退出循环 2. 迭代完成退出循环
                            return flag;
                        }
                    }
                }
            } else {
                return false;
            }
            return true;
        } */
};

/**
 * @brief LR(1)文法计算项集族时使用的闭包类型
 */
typedef struct Closure {
    using item_index_t   = int;
    using symbol_index_t = int;
    /* LR(1) 项 */
    typedef struct Lr1Item {
        item_index_t   lr_item;   /* lr项目(带点的产生式) */
        symbol_index_t la_symbol; /* 向前看符号 */
        bool
        operator==(const Lr1Item& b) {
            return (this->lr_item == b.lr_item && this->la_symbol == b.la_symbol);
        }
    } Lr1Item;

    std::vector<Lr1Item> item_closure; /* 该闭包中LR(1)项的集合 */

    bool
    search(const Closure::Lr1Item& lr1_item) {
        for (auto& item : item_closure) {
            if (item == lr1_item) {
                return true;
            }
        }
        return false;
    }
    bool
    operator==(const Closure& b) {
        if (this->item_closure.size() != b.item_closure.size()) {
            return false;
        }
        int count = 0;
        for (auto& tmp : this->item_closure) {
            for (auto& btmp : b.item_closure) {
                if (tmp == btmp) {
                    ++count;
                    break;
                }
            }
        }
        return count == static_cast<int>(this->item_closure.size());
    }
} Closure;

/**
 * @brief LR(1) 文法，继承Grammar
 */
class LR_1 : public Grammar {
public:
    /* 分析过程中的动作枚举定义 */
    typedef enum Action {
        ShiftIn, // 移入
        Reduce,  // 归约
        Accept,  // 接受
        Error
    } Action;
    /* 具体的action信息 */
    typedef struct ActionInfo {
        Action action; // 对应动作
        int    info;   // 归约产生式或转移状态
    } ActionInfo;

private:
    std::vector<Item>    lr_items;     /* LR(0) 项 */
    std::vector<Closure> item_cluster; /* 项集族 */
    /**
     * 记录转移信息的临时表
     * 表示某个状态(Closure)下遇到某个符号转移到的下一个状态
     * 三个 int 的含义依次为 ：
     *     当前Closure在item_cluster中的index
     *     当前符号在symbols中的index
     *     转移到的Closure在item_cluster中的index
     */
    std::map<std::pair<int, int>, int> goto_tmp;

    /**
     * GOTO[i, A] = j;
     * goto中只用到Action Error(表示未定义)和ShiftIn(表示转移);
     * ACTION[i, A] = "移入/规约/接受";
     */
    std::map<std::pair<int, int>, ActionInfo> goto_table;
    std::map<std::pair<int, int>, ActionInfo> action_table;

public:
    /* 语义分析器 */
    Semantic semantic;

    // 生成LR Item项
    void
    generateLrItems() {
        /* 这里的 A->ε 产生式依旧生成两个项目：A->·ε和A->ε·  后续做特殊处理 */
        for (int i = 0; i < static_cast<int>(productions.size()); ++i) {
            for (int dot = 0; dot <= static_cast<int>(productions[i].right.size()); ++dot) {
                lr_items.push_back(productions[i]);
                lr_items.back().is_lr1_item = true;
                lr_items.back().dot_pos     = dot;
                lr_items.back().pro_index   = i;
            }
        }
    }

    int
    get_lr_items_index_by_item(const Item& item) {
        for (int i = 0; i < static_cast<int>(lr_items.size()); ++i) {
            if (item == lr_items[i]) {
                return i;
            }
        }
        return Npos;
    }

    // 计算LR(1)项集簇
    void
    getItems() {
        /* 判断是否是已经存在的闭包 */
        auto isExistedClosure = [this](const Closure& clo) -> int {
            for (int i = 0; i < static_cast<int>(item_cluster.size()); ++i) {
                if (item_cluster[i] == clo) {
                    return i;
                }
            }
            return Grammar::Npos;
        };
        /* 初始化 item_cluster Closure({S' → ·S, $]}) */
        Item initial_item(
            get_symbol_index_by_id(ExtendStart), { get_symbol_index_by_id(StartToken) }, true, 0, start_production);
        Closure initial_closure;
        initial_closure.item_closure.push_back({ get_lr_items_index_by_item(initial_item), get_symbol_index_by_id(EndToken) });

        item_cluster.push_back(std::move(closure(initial_closure)));
        /* item_cluster中的每个项 */
        for (int i = 0; i < static_cast<int>(item_cluster.size()); ++i) {
            /* 所有文法符号 X */
            for (int s = 0; s < static_cast<int>(symbols.size()); ++s) {
                /* 文法符号：终结符或非终结符 */
                if (symbols[s].type != Symbol::Terminal && symbols[s].type != Symbol::NonTerminal) {
                    continue;
                }
                /* 计算 Goto(I,X) */
                auto transfer = gotoState(item_cluster[i], s);
                /* 为空跳过 */
                if (transfer.item_closure.empty()) {
                    continue;
                }
                /* 已经存在 记录转移状态即可 */
                int existed_index = isExistedClosure(transfer);
                if (existed_index != Grammar::Npos) {
                    goto_tmp[{ i, s }] = existed_index;
                    continue;
                }
                /* 不存在也不为空 加入进item_cluster并记录转移状态 */
                item_cluster.push_back(std::move(transfer));
                /* 记录closure之间的转移关系 */
                goto_tmp[{ i, s }] = item_cluster.size() - 1;
            }
        }
    }

    // 计算GOTO状态转移
    Closure
    gotoState(const Closure& I, int X) {
        Closure J;
        /* X必须是终结符或非终结符 */
        if (!isTerminal(X) && !isNonTerminal(X)) {
            return J;
        }
        for (auto& lr1_item : I.item_closure) {
            /* 对I中的每个 [A->α·Xβ, a] */
            auto& lr0_item = lr_items[lr1_item.lr_item];
            /* dot之后没有文法符号 继续遍历 */
            if (lr0_item.dot_pos >= static_cast<int>(lr0_item.right.size())) {
                continue;
            }
            if (lr0_item.right[lr0_item.dot_pos] != X) {
                continue;
            }
            auto tmp = lr0_item;
            ++tmp.dot_pos;
            J.item_closure.push_back({ get_lr_items_index_by_item(tmp), lr1_item.la_symbol });
        }
        return closure(J);
    }

    // 计算closure闭包
    Closure&
    closure(Closure& I) {
        for (int i = 0; i < static_cast<int>(I.item_closure.size()); ++i) {
            /* 对每个lr1项：[A -> α·Bβ, a] */
            const auto& lr1_item = I.item_closure[i];          /* [A -> α·Bβ, a] */
            const auto& lr0_item = lr_items[lr1_item.lr_item]; /* A -> α·Bβ */
            /* '·'在最后一个位置 其后继没有非终结符 */
            if (lr0_item.dot_pos >= static_cast<int>(lr0_item.right.size())) {
                continue;
            }
            const auto& B = lr0_item.right[lr0_item.dot_pos];
            /* '·'之后的符号为终结符 */
            if (isTerminal(B)) {
                continue;
            }
            if (isEpsilon(B)) {
                /* 如果B是ε，则当前项为 A->·ε */
                /* 为了不在ε上引出转移边，直接将项变为：A->ε· */
                auto tmp = lr0_item;
                ++tmp.dot_pos;
                I.item_closure[i].lr_item = get_lr_items_index_by_item(tmp);
                continue;
            }
            std::vector<int> beta_a(lr0_item.right.begin() + lr0_item.dot_pos + 1, lr0_item.right.end());
            beta_a.push_back(lr1_item.la_symbol);
            auto first_of_beta_a = getFirstOfProduction(beta_a);
            /* 对每个 B -> ·γ 的lr0项 */
            for (int i = 0; i < static_cast<int>(lr_items.size()); ++i) {
                if (lr_items[i].left != B) {
                    continue;
                } else {
                    /* 如果是 B->ε 则将 B->ε·项加入(为了不在ε上引出转移边) */
                    bool is_epsilon = isEpsilon(lr_items[i].right.front());
                    /* 如果是ε产生式但dot不在尾部 继续遍历 */
                    if (is_epsilon && lr_items[i].dot_pos != static_cast<int>(lr_items[i].right.size())) {
                        continue;
                    }
                    /* 如果不是ε产生式且dot不在起始位置 继续遍历 */
                    if (!is_epsilon && lr_items[i].dot_pos != 0) {
                        continue;
                    }
                }

                /* 将 [B -> ·γ, b] 加入到 I 中 */
                /* 注意：1. 这里的b可能是'#'
                        2. 如果是 B->ε 产生式，会将 [B -> ε·, b] 加入到 I 中
                 */
                for (auto& b : first_of_beta_a) {
                    if (!isEpsilon(b)) {
                        if (!I.search({ i, b })) {
                            /* ! debug */
                            // bool is_epsilon = isEpsilon(lr_items[i].right.front());
                            // if (is_epsilon) {
                            //     std::cout << symbols[lr_items[i].left].id << " -> " <<
                            //     symbols[lr_items[i].right.front()].id
                            //     << " dot -> " << lr_items[i].dot_pos << std::endl;
                            // }
                            /* ! end debug */
                            I.item_closure.push_back({ i, b });
                        }
                    }
                }
            }
        }
        return I;
    }

    void
    bulidTable() {
        for (int cluster_idx = 0; cluster_idx < static_cast<int>(item_cluster.size()); ++cluster_idx) {
            for (int lr_item_idx = 0; lr_item_idx < static_cast<int>(lr_items.size()); ++lr_item_idx) {
                for (auto& ter : terminals) {
                    /* 如果lr1项不在当前闭包中 继续遍历 */
                    if (!item_cluster[cluster_idx].search({ lr_item_idx, ter })) {
                        continue;
                    }
                    const auto& lr0_item    = lr_items[lr_item_idx];
                    int         pro_index   = lr0_item.pro_index;
                    int         pro_left    = lr0_item.left;
                    int         pro_dot_pos = lr0_item.dot_pos;
                    int         la_symbol   = ter;
                    if (pro_dot_pos >= static_cast<int>(lr0_item.right.size())) {
                        if (symbols[pro_left].id != ExtendStart) {
                            /* ! debug */
                            // bool is_epsilon = isEpsilon(lr0_item.right.front());
                            // if (is_epsilon) {
                            //     std::cout << "table : " << symbols[lr0_item.left].id << "
                            //     -> " << symbols[lr0_item.right.front()].id
                            //               << " dot -> " << lr0_item.dot_pos << " la = " <<
                            //               symbols[la_symbol].id << std::endl;
                            // }
                            /* ! end debug */

                            action_table[{ cluster_idx, la_symbol }] = { Action::Reduce, pro_index };
                        } else {
                            int end_index                            = get_symbol_index_by_id(EndToken);
                            action_table[{ cluster_idx, end_index }] = { Action::Accept, -1 };
                        }
                    } else {
                        int item_after_dot = lr0_item.right[pro_dot_pos];
                        if (!isTerminal(item_after_dot)) {
                            continue;
                        }
                        auto iter = goto_tmp.find({ cluster_idx, item_after_dot });
                        if (iter != goto_tmp.end()) {
                            action_table[{ cluster_idx, item_after_dot }] = { Action::ShiftIn, iter->second };
                        }
                    }
                }
                for (auto& non_ter : non_terminals) {
                    auto iter = goto_tmp.find({ cluster_idx, non_ter });
                    if (iter != goto_tmp.end()) {
                        goto_table[{ cluster_idx, non_ter }] = { Action::ShiftIn, iter->second };
                    }
                }
            }
        }
    }

    void
    raise_error(const Token& token, std::ostream& os = std::cout) {
        os << std::endl << "Error found near : " << token.value << " [row = " << token.row << "]" << std::endl;
    }

public:
    std::pair<int, int>
    parse_token(std::vector<Token>&& token_stream, std::ostream& os = std::cout) {
        token_stream.push_back({ EndToken, EndToken, static_cast<unsigned>(-1) });
        /* first -> state; second -> symbol */
        std::vector<std::pair<int, int>> symbol_stack;

        int g_error_count = 0, s_error_count = 0;

        semantic.AddSymbolToList(SymbolAttribute(StartToken));

        int step = 0;
        os << "步骤 \t 符号栈 \t 产生式 " << std::endl;

        /* 栈初始化 */
        symbol_stack.push_back({ 0, get_symbol_index_by_id(EndToken) });

        os << ++step << " \t ";
        for (auto& p : symbol_stack) {
            os << "(" << p.first << "," << symbols[p.second].id << ")";
        }
        os << " \t " << std::endl;

        for (int i = 0; i < static_cast<int>(token_stream.size()); ++i) {

            int cur_state = symbol_stack.back().first;

            int  token_idx   = get_symbol_index_by_id(token_stream[i].token);
            auto action_iter = action_table.find({ cur_state, token_idx });
            if (action_iter == action_table.end()) {
                raise_error(token_stream[i]);
                do {
                    symbol_stack.pop_back();
                } while (action_table.find({ symbol_stack.back().first, token_idx }) == action_table.end());
                --i;
                ++g_error_count;
            } else {
                auto action_info = action_iter->second;
                switch (action_info.action) {
                    case Action::ShiftIn:
                        symbol_stack.push_back({ action_info.info, token_idx });

                        os << ++step << " \t ";
                        for (auto& p : symbol_stack) {
                            os << "(" << p.first << "," << symbols[p.second].id << ")";
                        }
                        os << " \t " << std::endl;

                        semantic.AddSymbolToList(
                            SymbolAttribute(token_stream[i].token, token_stream[i].value, token_stream[i].row));
                        break;
                    case Action::Reduce: {
                        auto& production = productions[action_info.info];
                        /* 非空串需要出栈 空串由于右部为空
                         * 不需要出栈(直接push空串对应产生式左部非终结符即可) */
                        if (!isEpsilon(production.right.front())) {
                            auto count = production.right.size();
                            while (count--) {
                                symbol_stack.pop_back();
                            }
                        }
                        auto goto_iter = goto_table.find({ symbol_stack.back().first, production.left });
                        if (goto_iter == goto_table.end()) {
                            raise_error(token_stream[i]);
                            do {
                                symbol_stack.pop_back();
                            } while (goto_table.find({ symbol_stack.back().first, token_idx }) == goto_table.end());
                            --i;
                            ++g_error_count;
                        } else {
                            symbol_stack.push_back({ goto_iter->second.info, production.left });
                            --i;
                            std::string              pro_left = symbols[production.left].id;
                            std::vector<std::string> pro_right;
                            for (auto& r : production.right) {
                                pro_right.push_back(symbols[r].id);
                            }
                            if (!semantic.Analysis(pro_left, pro_right)) {
                                /* todo : error of semantic analysis */
                                ++s_error_count;
                            }

                            os << ++step << " \t ";
                            for (auto& p : symbol_stack) {
                                os << "(" << p.first << "," << symbols[p.second].id << ")";
                            }
                            os << " \t ";
                            os << pro_left << "->";
                            for (auto& r : pro_right) {
                                os << r << " ";
                            }
                            os << std::endl;
                        }
                    } break;
                    case Action::Accept:
                        return { g_error_count, s_error_count };
                    default:
                        /* Todo : error */
                        return { g_error_count, s_error_count };
                        break;
                }
            }
        }

        return { g_error_count, s_error_count };
    }
    void
    printTable(std::ostream& out = std::cout) {
        const int   state_width  = 6;
        const int   action_width = 8;
        const int   goto_width   = 14;
        const char* err_msg      = " ";

        out << std::setw(state_width) << " 状态 " << std::setw(terminals.size() * action_width) << "ACTION"
            << std::setw((non_terminals.size() - 1) * goto_width) << "GOTO" << std::endl;

        out << std::setw(state_width) << " ";
        for (auto& ter : terminals) {
            out << std::setw(action_width) << symbols[ter].id;
        }
        for (auto& non_ter : non_terminals) {
            if (symbols[non_ter].id == ExtendStart) {
                continue;
            }
            out << std::setw(goto_width) << symbols[non_ter].id;
        }
        out << std::endl;

        for (int i = 0; i < static_cast<int>(item_cluster.size()); ++i) {
            out << std::setw(state_width) << i;
            for (auto& ter : terminals) {
                auto iter = action_table.find({ i, ter });
                if (iter == action_table.end()) {
                    out << std::setw(action_width) << err_msg;
                } else {
                    std::string out_msg;
                    if (iter->second.action == Action::Accept) {
                        out_msg += "acc";
                    } else if (iter->second.action == Action::Reduce) {
                        out_msg += "r" + std::to_string(iter->second.info);
                    } else if (iter->second.action == Action::ShiftIn) {
                        out_msg += "s" + std::to_string(iter->second.info);
                    }
                    out << std::setw(action_width) << out_msg;
                }
            }

            for (auto& non_ter : non_terminals) {
                if (symbols[non_ter].id == ExtendStart) {
                    continue;
                }
                auto iter = goto_table.find({ i, non_ter });
                if (iter == goto_table.end()) {
                    out << std::setw(goto_width) << err_msg;
                } else {
                    out << std::setw(goto_width) << std::to_string(iter->second.info);
                }
            }
            out << std::endl;
        }
        out << std::endl;
    }

public:
    explicit LR_1(const std::string& grammar_path) : Grammar(grammar_path) {
        generateLrItems();
        getItems();
        bulidTable();
    }
};

#endif