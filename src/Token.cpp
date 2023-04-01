#include <Token.h>
namespace lpp
{
    bool lppIdentifier::operator==(const lppIdentifier& rhs) const
    {
        return true;
    }

    lppIdentifier::lppIdentifier(std::string&& symbol_name, ValueType _type)
    {
        name = std::move(symbol_name);
        type = _type;
    }
    lppIdentifier::lppIdentifier(std::string& symbol_name, ValueType _type)
    {
        name = symbol_name;
        type = _type;
    }
    lppIdentifier::lppIdentifier(lppIdentifierView symbol, ValueType _type)
    {
        name = symbol.name;
        type = _type;
    }
    lppIdentifierView::lppIdentifierView(Identifier& identifier)
    {
        name = identifier.name;
        type = identifier.type;
    }
    lppIdentifierView::lppIdentifierView(std::string_view Identifier_name)
    {
        name = Identifier_name;
        type = ValueType::Invalid;
    }
    void TokenVisitor::operator()(const Enter& var)
    {
        output_stream << "EOF";
    }
    void TokenVisitor::operator()(const KeyWord& var)
    {
        output_stream << "Keyword: " << static_cast<short>(var);
    }
    void TokenVisitor::operator()(const Identifier& var)
    {
        output_stream << "identifier: " << var.name;
    }
    void TokenVisitor::operator()(const Digit& var)
    {
        output_stream << "number: " << std::visit(DigitVisitor(), var);
    }
    void TokenVisitor::operator()(const Punchation& var)
    {
        output_stream << "Punchation: " << static_cast<short>(var);
    }
    double DigitVisitor::operator()(auto num)
    {
        return num;
    }
    lppToken::lppToken(lppToken& _token)
    {
        token = _token.token;
        col = _token.col;
        line = _token.line;
    }
    lppToken& lppToken::operator=(lppToken && rvalue)
    {
        this->col = rvalue.col;
        this->line = rvalue.line;
        this->token = std::move(rvalue.token);
        return *this;
    }

    lppToken& lppToken::operator=(lppToken & rvalue)
    {
        token = rvalue.token;
        col = rvalue.col;
        line = rvalue.line;
        return *this;
    }

    std::partial_ordering lppToken::operator<=>(const lppToken& rhs) const
    {
        switch (static_cast<TokenType>(rhs.token->index()))
        {
        case TokenType::NONE: case TokenType::SYMBOL:
            return this->token.value().index() == rhs.token.value().index() ? std::partial_ordering::equivalent : std::partial_ordering::unordered;
            break;
        case TokenType::PUNCHATION:
            if (this->token->index() == rhs.token->index())
            {
                if (std::get<Punchation>(this->token.value()) > std::get<Punchation>(rhs.token.value()))
                {
                    return std::partial_ordering::greater;
                }
                else if (std::get<Punchation>(this->token.value()) > std::get<Punchation>(rhs.token.value())) return std::partial_ordering::equivalent;
                else return std::partial_ordering::less;
            }
            else return std::partial_ordering::unordered;
        case TokenType::DIGIT:
            if (this->token.value().index() == rhs.token->index())
            {
                double lvalue, rvalue;
                lvalue = std::visit(DigitVisitor(), std::get<Digit>(this->token.value()));
                rvalue = std::visit(DigitVisitor(), std::get<Digit>(rhs.token.value()));
                if (lvalue > rvalue) return std::partial_ordering::greater;
                else if(lvalue == rvalue) return std::partial_ordering::equivalent;
                else return std::partial_ordering::less;
            }
            else return std::partial_ordering::unordered;
        case TokenType::KEYWORD:
            if (this->token->index() == rhs.token->index())
            {
                if (std::get<KeyWord>(*this->token) > std::get<KeyWord>(*rhs.token)) return std::partial_ordering::greater;
                else if (std::get<KeyWord>(*this->token) > std::get<KeyWord>(*rhs.token)) return std::partial_ordering::equivalent;
                else return std::partial_ordering::less;
            }
            else return std::partial_ordering::unordered;
        default:
            break;
        }
        return std::partial_ordering::unordered;
    }

    lppToken::lppToken(lppToken&& _token)
    {
        this->col = _token.col;
        this->line = _token.line;
        this->token = std::move(_token.token);
    }
    TokenVisitor::TokenVisitor(std::ostream& _ostream) : output_stream(_ostream)
    {

    }
    lppToken::lppToken(TokenObj _token, size_t _line, size_t _col)
    {
        token = std::move(_token);
        line = _line;
        col = _col;
    }
    double DigitVisitor::operator()(std::monostate num)
    {
        return NAN;
    }
    std::ostream& operator<<(std::ostream &out_stream, const lppToken &token)
    {
        out_stream << "token:";
        std::visit(TokenVisitor(out_stream), token.token.value());
        return out_stream << " line:" << token.line << " column:" << token.col << "\n";
    }
    size_t TokenObjHash::operator()(const TokenObj& _token) const
    {
        size_t result = 0;
        switch (static_cast<TokenType>(_token->index()))
        {
        case TokenType::DIGIT: case TokenType::SYMBOL:
            result = -2;
            break;
        case TokenType::PUNCHATION:
            result = static_cast<size_t>(std::get<Punchation>(_token.value()));
            break;
        case TokenType::KEYWORD:
            result = static_cast<size_t>(std::get<KeyWord>(*_token)) + 256;
            break;
        default:
            result = -1;
            break;
        }
        return result;
    }
}