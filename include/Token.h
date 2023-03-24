#ifndef TOKEN_H
#define TOKEN_H
#include <variant>
#include <string>
#include <iostream>
#include <limits>
#include <math.h>
#include <llvm/IR/Value.h>
#include <sstream>
#include <optional>
namespace lpp
{
    struct lppToken;
    using IdentifierView = struct lppIdentifierView;
    using ValueType = enum class lppValueType : unsigned char
    {
        Invalid, BOOL, i8, u8, i16, u16, i32, u32, i64, u64, f32, f64, UserDefine
    };
    using Identifier = struct lppIdentifier
    {
        lppIdentifier(std::string&& symbol_name, ValueType type = ValueType::Invalid);
        lppIdentifier(std::string& symbol_name, ValueType type = ValueType::Invalid);
        lppIdentifier(lppIdentifierView symbol, ValueType _type = ValueType::Invalid);
        lppIdentifier() = default;
        std::string name;
        ValueType type;
        bool operator==(const lppIdentifier& rhs) const;
    };
    enum class TokenType : unsigned char
    {
        NONE, KEYWORD, SYMBOL, DIGIT, PUNCHATION, NUM
    };
    enum class KeyWord : unsigned char;
    using Enter = std::monostate;
    using Digit = std::variant<std::monostate, bool, char, unsigned char, short, unsigned short, long, unsigned long, long long, unsigned long long, float, double>;
    //using Digit = llvm::Type*;
    using Punchation = enum class lppPunchation : unsigned char
    {
        COMMA = ',',          // 逗号。
        COLON = ':',
        SEMICOLON = ';',      // 分号。
        LPARENTHESES = '(',   // 左小括号。
        RPARENTHESES = ')',   // 右小括号。
        LBRACKETS = '[',      // 左中括号。
        RBRACKETS = ']',      // 右中括号。
        LBRACES = '{',        // 左大括号。
        RBRACES = '}',        // 右大括号。
        POSITIVE = '+',
        ADD = '+',            // 加。
        NEGATIVE = '-',
        SUB = '-',
        MUL = '*',
        DEREF = '*',
        REF = '&',
        DIV = '/',
        ASSIGN = '=',        
        LESSTHAN = '<',          // 22 小于。
        LARGETHAN = '>',
        BITAND = '&',
        GETADDR = '&',
        BITOR = '|',
        BITXOR = '^',
        NOT = '!',
        DOT = '.',
        CONDIFIR = '?',
        CONDISEC = ':',
        SLASH = '\\',
        MOD = '%',
        ADDASSIGN = 128,
        SUBASSING,
        MULASSIGN,
        DIVASSIGN,
        XORASSIGN,
        ANDASSIGN,
        ORASSIGN,
        MODASSIGN,
        LSHIFTASSIGN,
        RSHIFTASSIGN,
        BITANDASSIGN,
        BITXORASSIGN,
        BITORASSIGN,
        EQU,
        NOTEQU,
        LSHIFT,
        RSHIFT,
        LESSEQ,
        LARGEEQ,
        POW,
        AND,
        OR,
        SCOUP,
        POINTERTO          // 23 结束。
    };
    struct lppIdentifierView
    {
        lppIdentifierView() = default;
        lppIdentifierView(std::string_view Identifier_name);
        lppIdentifierView(Identifier& identifier);
        std::string_view name;
        ValueType type;
    };
    class DigitVisitor
    {
    public:
        DigitVisitor() = default;
        double operator()(auto num);
        double operator()(std::monostate num);
    };
    
    using TokenObj = std::optional<std::variant<Enter, KeyWord, Identifier, Digit, Punchation>>;
    class TokenVisitor
    {
    public:
        TokenVisitor(std::ostream& _ostream = std::cout);
        void operator()(const Enter& var);
        void operator()(const KeyWord& var);
        void operator()(const Identifier& var);
        void operator()(const Digit& var);
        void operator()(const Punchation& var);
    private:
        std::ostream& output_stream;
    };  

    using Token = struct lppToken
    {
    public:
        TokenObj token;
        size_t line, col;
        lppToken() = default;
        lppToken(lppToken& _token);
        lppToken& operator=(lppToken && rvalue);
        lppToken(lppToken&& _token);
        lppToken(TokenObj _token, size_t _line, size_t _col);
        std::partial_ordering operator<=>(const lppToken& rhs) const;
    private:
        friend std::ostream & operator<<(std::ostream &out_stream, const lppToken &token);
    };
    enum class KeyWord : unsigned char
    {
        IF, ELSE, WHILE, SWICH, CASE, STRUCT, CLASS, MUT, LET, IMPL, RETURN, BREAK, CONTINUE, GOTO, USE, EXTERN, TRY, CATCH, UNION, AS, FN, PUB, TRUE, FALSE, SIZEOF, NUM
    };

    class TokenObjHash
    {
    public:
        size_t operator()(const TokenObj& _token) const;
    };
};


#endif