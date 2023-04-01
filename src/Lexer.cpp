#include <Lexer.h>
namespace lpp
{
    Lexer::Lexer() : line(0), column(0), source_file(nullptr)
    {
        root = allocator.allocate(1);
        new (root) Node();
    }

    // use DFA will be better, wrong choice.
    Punchation Lexer::parse_mul_punc()
    {
        size_t token_size = 0;
        Punchation punc;
        switch (linebuf[column + token_size])
        {
        case '-':
        {
            token_size++;
            switch (linebuf[column + token_size])
            {
            case '>':
                punc = Punchation::POINTERTO;
                column += 2;
                break;
            case '=':
                punc = Punchation::SUBASSING;
                column += 2;
            default:
                punc = Punchation::SUB;
                column++;
                break;
            }
            
            break;
        }
        case ':':
        {
            token_size++;
            if (linebuf[column + token_size] == ':')
            {
                column += 2;
                punc = Punchation::SCOUP;
            }
            else
            {
                column++;
                punc = Punchation::COLON;
            }
            break;
        }
        case '&':
        {
            token_size++;
            switch (linebuf[column + token_size])
            {
            case '&':
                column += 2;
                punc = Punchation::AND;
                break;
            case '=':
                column += 2;
                punc = Punchation::ADDASSIGN;
                break;
            default:
                column++;
                punc = Punchation::BITAND;
                break;
            }
            break;
        }
        case '|':
        {
            token_size++;
            switch (linebuf[column + token_size])
            {
            case '|':
                column += 2;
                punc = Punchation::OR;
                break;
            case '=':
                column += 2;
                punc = Punchation::ORASSIGN;
                break;
            default:
                column++;
                punc = Punchation::BITOR;
                break;
            }
            break;
        }
        case '*':
        {
            token_size++;
            if (linebuf[column + token_size] == '=')
            {
                column += 2;
                punc = Punchation::MULASSIGN;
            }
            else
            {
                column++;
                punc = Punchation::MUL;
            }
            break;
        }
        case '>':
        {
            token_size++;
            switch (linebuf[column + token_size])
            {
            case '>':
                token_size++;
                if (linebuf[column + token_size] == '=')
                {
                    punc = Punchation::RSHIFTASSIGN;
                    column +=3;
                }
                else
                {
                    punc = Punchation::RSHIFT;
                    column += 2;
                }
                break;
            case '=':
                column += 2;
                punc = Punchation::LARGEEQ;
                break;
            default:
                column++;
                punc = Punchation::LARGETHAN;
                break;
            }
            break;
        }
        case '<':
        {
            token_size++;
            switch (linebuf[column + token_size])
            {
            case '>':
                token_size++;
                if (linebuf[column + token_size] == '=')
                {
                    punc = Punchation::LSHIFTASSIGN;
                    column +=3;
                }
                else
                {
                    punc = Punchation::LSHIFT;
                    column += 2;
                }
                break;
            case '=':
                column += 2;
                punc = Punchation::LESSEQ;
                break;
            default:
                column++;
                punc = Punchation::LESSTHAN;
                break;
            }
            break;
        }case '+':
        {
            token_size++;
            if (linebuf[column + token_size] == '=')
            {
                punc = Punchation::ADDASSIGN;
                column += 2;
            }
            else
            {
                punc = Punchation::ADD;
                column++;
            }
            break;
        }
        case '/':
        {
            token_size++;
            if (linebuf[column + token_size] == '=')
            {
                punc = Punchation::DIVASSIGN;
                column += 2;
            }
            else 
            {
                punc = Punchation::DIV;
                column++;
            }
            break;
        }
        case '=':
        {
            token_size++;
            if (linebuf[column + token_size] == '=')
            {
                punc = Punchation::EQU;
                column += 2;
            }
            else 
            {
                punc = Punchation::ASSIGN;
                column++;
            }
            break;
        }
        case '%':
        {
            token_size++;
            if (linebuf[column + token_size] == '=')
            {
                punc = Punchation::MODASSIGN;
                column += 2;
            }
            else 
            {
                punc = Punchation::MOD;
                column++;
            }
            break;
        }
        case '^':
        {
            token_size++;
            if (linebuf[column + token_size] == '=')
            {
                punc = Punchation::XORASSIGN;
                column += 2;
            }
            else 
            {
                punc = Punchation::BITXOR;
                column++;
            }
            break;
        }
        default:
            punc = static_cast<Punchation>(linebuf[column]);
            column++;
            break;
        }
        return punc;
    }
    void Lexer::init_keyword(const std::vector<std::pair<std::string_view, TokenObj>> &key_words)
    {
        for(auto& key_word : key_words)
        {
            init_keyword(key_word);
        }
    }
    void Lexer::init_keyword(const std::pair<std::string_view, TokenObj> token_pair)
    {
        Node* current = root;
        for(size_t var = 0; var < token_pair.first.size(); var++)
        {
            if (!islower(token_pair.first[var])) throw std::logic_error("only alpha can be insert into Tril");
            if (!current->next[token_pair.first[var] - 'a'])
            {
                current->next[token_pair.first[var] - 'a'] = allocator.allocate(1);
                new (current->next[token_pair.first[var] - 'a']) Node();
            }
            current = current->next[token_pair.first[var] - 'a'];
        }
        if (TokenType::NONE == static_cast<TokenType>(current->token->index())) current->token = token_pair.second;
        else throw std::logic_error(std::string("current token ") + std::string(token_pair.first.data(), token_pair.first.size()) + std::string(" has already existed."));
    }
    long long Lexer::atoll(const char *_Str, size_t *_Idx, int _Base)
    {
        int& _Errno_ref  = errno; // Nonzero cost, pay it once
        char* _Eptr;
        _Errno_ref           = 0;
        const long long _Ans = strtoll(_Str, &_Eptr, _Base);

        if (_Str == _Eptr) {
            throw std::invalid_argument("invalid stoll argument");
        }

        if (_Idx) {
            *_Idx = static_cast<size_t>(_Eptr - _Str);
        }

        if (_Errno_ref == ERANGE) {
            throw std::out_of_range("stoll argument out of range");
        }
        return _Ans;
    }
    size_t Lexer::atoull(const char *_Str, size_t *_Idx, int _Base)
    {
        int& _Errno_ref  = errno; // Nonzero cost, pay it once
        char* _Eptr;
        _Errno_ref           = 0;
        const size_t _Ans = strtoull(_Str, &_Eptr, _Base);

        if (_Str == _Eptr) {
            throw std::invalid_argument("invalid stoll argument");
        }

        if (_Idx) {
            *_Idx = static_cast<size_t>(_Eptr - _Str);
        }

        if (_Errno_ref == ERANGE) {
            throw std::out_of_range("stoll argument out of range");
        }
        return _Ans;
    }
    Token Lexer::get_next_token()
    {
        Node* current = root;
        size_t token_size = 0;
        if (skip_blank()) 
        {
            if (!source_file->eof() || linebuf.size())
            {
                return Token(Enter(), line, column);
            }
            else return Token(std::nullopt, line, column);
        }
        if (ispunct(linebuf[column + token_size]) && linebuf[column + token_size] != '_')
        {
            Punchation punc = parse_mul_punc();
            return Token(punc, line, column);
        }
        if (isdigit(linebuf[column + token_size])) 
        {
            Digit digit;
            double fvalue;
            size_t shuffix_size = 0, uvalue;
            long long value;
            try
            {
                value = atoll(linebuf.data() + column, &token_size, 0);
            }
            catch (const std::out_of_range& e)
            {
                switch (linebuf[column + token_size])
                {
                case 'i':
                {
                    throw std::out_of_range("integer literal is too large");
                    break;
                }
                case 'u':
                {
                    uvalue = atoull(linebuf.data() + column, &token_size, 0);
                    goto unsigned_process;
                    break;
                }
                case 'f':
                {
                    goto float_process;
                    break;
                }
                }
            }
            catch (const std::exception& e)
            {
                throw std::exception(e);
            }
            
            
            switch (linebuf[column + token_size])
            {
            case '.':
            {
        float_process:
                fvalue = atod(linebuf.data() + column, &token_size);
                switch (linebuf[column + token_size])
                {
                case 'f':
                {
                    int size = std::stoi(linebuf.data() + column + token_size + 1, &shuffix_size);
                    switch (size)
                    {
                    case 32:
                    {
                        if (fvalue < FLT_MIN || fvalue > FLT_MAX) throw std::out_of_range("literal out of range for `f32`");
                        digit = static_cast<float>(fvalue);
                        break;
                    }
                    case 64:
                    {
                        digit = fvalue;
                        break;
                    }
                    default:
                        goto unsupport_fsuffix;
                        break;
                    }
                    break;
                }
                case ',': case ' ': case ';': case '\n': case '\t':
                    digit = fvalue;
                    break;
                default:
                    goto unsupport_fsuffix;
                    break;
                }
                break;
            }
            case 'i':
            {
                size_t shuffix_size;
                int size = std::stoi(linebuf.data() + column + token_size + 1, &shuffix_size);
                switch (size)
                {
                case 8:
                {
                    if (value > CHAR_MAX || value < CHAR_MIN) throw std::out_of_range("literal out of range for `i8`");
                    digit = static_cast<char>(value);
                    break;
                }
                case 16:
                {
                    if (value > SHRT_MAX || value < SHRT_MIN) throw std::out_of_range("literal out of range for `i16`");
                    digit = static_cast<short>(value);
                    break;
                }
                case 32:
                {
                    if (value > LONG_MAX || value < LONG_MIN) throw std::out_of_range("literal out of range for `i32`");
                    digit = static_cast<long>(value);
                    break;
                }
                case 64:
                {
                    digit = static_cast<long long>(value);
                    break;
                }
                default:
                    goto unsupport_isuffix;
                    break;
                }
                break;
            }
            case 'u':
            {
                uvalue = static_cast<size_t>(value);
    unsigned_process:
                size_t shuffix_size;
                int size = std::stoi(linebuf.data() + column + token_size + 1, &shuffix_size);
                switch (size)
                {
                case 8:
                {
                    if (uvalue > UCHAR_MAX) throw std::out_of_range("literal out of range for `u8`"); 
                    digit = static_cast<unsigned char>(uvalue);
                    break;
                }
                case 16:
                {
                    if (uvalue > USHRT_MAX) throw std::out_of_range("literal out of range for `u16`"); 
                    digit = static_cast<unsigned short>(uvalue);
                    break;
                }
                case 32:
                {
                    if (uvalue > ULLONG_MAX) throw std::out_of_range("literal out of range for `u32`");
                    digit = static_cast<unsigned long>(uvalue);
                    break;
                }
                case 64:
                {
                    digit = static_cast<size_t>(uvalue);
                    break;
                }
                default:
                    goto unsupport_isuffix;
                    break;
                }
                break;
            }
            case 'f':
            {
                int size = std::stoi(linebuf.data() + column + token_size + 1, &shuffix_size);
                switch (size)
                {
                case 32:
                {
                    digit = static_cast<float>(value);
                    break;
                }
                case 64:
                {
                    digit = static_cast<double>(value);
                    break;
                }
                default:
                    goto unsupport_fsuffix;
                    break;
                }
                break;
            }
            case ',': case ' ': case ';': case '\\': case '\t': case ')': case ']':
            {
                if(value < LONG_MAX && value > LONG_MIN) digit = static_cast<long>(value);
                else throw std::out_of_range("literal out of range for `i32`");
                break;
            }
            default:
                throw std::runtime_error("unexcept suffix");
                break;
            }
            {
                Token&& token = Token(digit, line, column);
                column += shuffix_size ?  token_size + shuffix_size + 1 : token_size;
                return token;
            }
            
unsupport_fsuffix:
            {
                std::string suffix;
                while (isalnum(linebuf[column + token_size]))
                {
                    suffix.push_back(linebuf[column + token_size]);
                    token_size++;
                }
                throw std::invalid_argument(std::string("invalid suffix `") + suffix + std::string("` for float literal"));
            }
unsupport_isuffix:
            {
                std::string suffix;
                while (isalnum(linebuf[column + token_size]))
                {
                    suffix.push_back(linebuf[column + token_size]);
                    token_size++;
                }
                throw std::invalid_argument(std::string("invalid suffix `") + suffix + std::string("` for integer literal"));
            }
        }
        while (column + token_size < linebuf.size())
        {
            if ((!islower(linebuf[column + token_size])) || current == nullptr)
            {
                while(true)
                {
                    if((column + token_size < linebuf.size() && !isspace(linebuf[column + token_size]) && !ispunct(linebuf[column + token_size])) || linebuf[column + token_size] == '_')
                    {
                        token_size++;
                    }
                    else if (current == nullptr || static_cast<TokenType>(current->token.value().index()) == TokenType::NONE)
                    {
                        Token&& token = Token(Identifier(linebuf.substr(column, token_size)), line, column);
                        column += token_size;
                        return token;
                    }
                    else
                    {
                        Token&& token = Token(current->token, line, column);
                        column += token_size;
                        return token;
                    }
                }
            }
            current = current->next[linebuf[column + token_size] - 'a'];
            token_size++;
        }
        if (token_size)
        {
            if (static_cast<TokenType>(current->token.value().index()) == TokenType::NONE)
            {
                Token&& token = Token(Identifier(linebuf.substr(column, token_size)), line, column);
                column += token_size;
                return token;
            }
            else
            {
                Token&& token = Token(current->token, line, column);
                column += token_size;
                return token;
            }
        }
        else
        {
            return Token(TokenObj(), line, column);
        } 
    }
    double Lexer::atod(const char* _Str, size_t* _Idx)
    {
        int& _Errno_ref  = errno; // Nonzero cost, pay it once
        char* _Eptr;
        _Errno_ref       = 0;
        const double _Ans = strtod(_Str, &_Eptr);

        if (_Str == _Eptr) {
            throw std::invalid_argument("invalid stoll argument");
        }

        if (_Idx) {
            *_Idx = static_cast<size_t>(_Eptr - _Str);
        }

        if (_Errno_ref == ERANGE) {
            throw std::out_of_range("stof argument out of range");
        }

        return _Ans;
    }
    void Lexer::set_source(std::istream* _source_file)
    {
        source_file = _source_file;
        column = 0;
        line = 1;
        std::getline(*source_file, linebuf);
    }
    bool Lexer::skip_blank()
    {
        while(linebuf.size() <= column || (linebuf[column] == ' ' || linebuf[column] == '\t'))
        {
            column++;
            while(linebuf.size() > column && (linebuf[column] == ' ' || linebuf[column] == '\t'))
            {
                column++;
            }
            if (linebuf.size() <= column)
            {
                line++;
                column = 0;
                if (!std::getline(*source_file, linebuf))
                {
                    linebuf.clear();
                }
                return true;
            }
        }
        return false;
    }
    std::vector<Token> Lexer::tokenize()
    {
        Token token;
        std::vector<Token> tokens;
        do
        {
            token = get_next_token();
            tokens.push_back(std::move(token));
        }
        while (TokenType::NONE != static_cast<TokenType>(token.token.value().index()));
        return std::move(tokens);
    }
    bool Lexer::save(std::ostream &output_stream)
    {
        std::stack<std::pair<Node*, std::string>> DFSStack;
        DFSStack.emplace(std::pair<Node*, std::string>(root, ""));
        std::pair<Node*, std::string> op;
        if (output_stream.bad()) return false;
        try
        {
            while (!DFSStack.empty())
            {
                op = DFSStack.top();
                DFSStack.pop();
                if (TokenType::KEYWORD == static_cast<TokenType>(op.first->token.value().index()))
                {
                    if (op.second.size() > 255) throw std::out_of_range("keyword length should less than 256.");
                    output_stream << op.second << ' ' << static_cast<short>(std::get<KeyWord>(op.first->token.value()));
                    output_stream << '\n';
                }
                for (char c = 0; c < 26; c++)
                {
                    if(op.first->next[c])
                    {
                        DFSStack.emplace(std::pair<Node*, std::string>(op.first->next[c], op.second + static_cast<char>('a' + c)));
                    }
                }
            }
        }
        catch (const std::exception& e)
        {
            return false;
        }
        return true;
    }
    bool Lexer::load(std::istream &input_stream)
    {
        if (input_stream.good()) 
        {
            short kw_type;
            std::string kw_name;
            while (input_stream >> kw_name >> kw_type)
            {
                init_keyword(std::make_pair<std::string_view, TokenObj>(kw_name, static_cast<KeyWord>(kw_type)));
            }
            return true;
        }
        else return false;
    }
    Lexer::DFANode::DFANode()
    {
        memset(this, 0, sizeof(Lexer::DFANode));
        token = std::monostate();
    }
}