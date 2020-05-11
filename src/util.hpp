/**
 *  Date: 2020-01-18 12:46:34
 *  LastEditors: Peng Gao
 *  LastEditTime: 2020-02-18 10:32:58
 */

#ifndef _UTILS_HPP_
#define _UTILS_HPP_

#include <algorithm>
#include <list>
#include <memory>
#include <string>
#include <vector>

/**
 * @brief LR(1)的项目/原始文法(包含拓展产生式)产生式
 *        left         - 产生式的左部符号Symbol的index
 *        right        - 产生式右部符号的index列表
 *        is_lr1_item  - 是否是lr1的项
 *        dot_pos      - lr1的项时有效，表示点的位置
 *        pro_index    - 该产生式/项目对应的产生式列表中的index
 */
typedef struct Item {
    static constexpr int Npos = -1; // 非法位置
    int                  left;
    std::vector<int>     right;
    bool                 is_lr1_item;
    int                  dot_pos;
    int                  pro_index;
    Item(int left, const std::vector<int>& right, bool is_lr1_item = false, int dot_pos = Npos, int pro_index = Npos)
        : left(left), right(right), is_lr1_item(is_lr1_item), dot_pos(dot_pos), pro_index(pro_index) {}
    Item(int left, std::vector<int>&& right, bool is_lr1_item = false, int dot_pos = Npos, int pro_index = Npos)
        : left(left), right(std::move(right)), is_lr1_item(is_lr1_item), dot_pos(dot_pos), pro_index(pro_index) {}

    friend bool
    operator==(const Item& a, const Item& b);
} Item;

bool
operator==(const Item& a, const Item& b) {
    return (a.is_lr1_item == b.is_lr1_item && a.dot_pos == b.dot_pos && a.pro_index == b.pro_index && a.left == b.left
            && a.right == b.right);
}

/**
 *  @brief  : 删除string首尾的空字符 : 空格、tab、'\n'、'\r'等
 *  @param  : str  将被trim的字符串
 *  @return : @a %str 被trim后的引用，
 */
inline std::string&
trim(std::string& str) {
    if (str.empty())
        return str;
    // isspace 为空字符时返回非0 这里反转
    auto isNotSpace = [](int ch) { return !isspace(ch); };
    // 删除起始到第一个不是空字符位置的子串
    str.erase(str.begin(), std::find_if(str.begin(), str.end(), isNotSpace));
    // 删除最后一个不是空字符的下一个位置到最后的子串
    str.erase(std::find_if(str.rbegin(), str.rend(), isNotSpace).base(), str.end());
    return str;
}

/**
 * @brief  : 按分割串将字符串分割为若干子串
 * @param  : str 被分割的字符串
 * @param  : splitStr 以此串为分割线
 * @return : 指向若干子串的shared_ptr指针
 */
inline std::shared_ptr<std::vector<std::string>>
split(const std::string& str, const std::string& splitStr) {
    std::shared_ptr<std::vector<std::string>> strs(new std::vector<std::string>);
    auto                                      start = 0U;
    auto                                      end   = str.find(splitStr, start); // 从起始位置查找分割串

    // 如果str中不含分割串，直接插入原str并返回
    if (end == std::string::npos) {
        strs->push_back(str);
        return strs;
    }
    // 循环查找splitStr
    while (end != std::string::npos) {
        // 取当前分割出的子串
        auto sub = str.substr(start, end - start);
        // 删除首尾空字符
        trim(sub);
        // 如果删除后子串不为空再加入 (这里实现为了方便/一般做法应该不管是否为空均加入)
        if (!sub.empty())
            strs->push_back(sub);
        // 修改查找的起始位置
        start = end + splitStr.length();
        end   = str.find(splitStr, start);
    }
    // 当end等于npos推出循环时，最后一部分子串未被加入，需要加入最后剩余的部分
    auto sub = str.substr(start);
    trim(sub);
    if (!sub.empty())
        strs->push_back(sub);

    return strs;
}

#endif // !_UTILS_HPP_