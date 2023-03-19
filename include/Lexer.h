#ifndef LEXER_H
#define LEXER_H

#include <Common.h>
#include <limits>
#include <float.h>
#include <ostream>
#include <istream>
#include <stack>
#include <sstream>
#include <optional>

namespace lpp
{
    class Lexer final
    {
    public:
        using Node = struct DFANode
        {
            explicit DFANode();
            DFANode *next[26];
            TokenObj token;
        };

        std::vector<Token> tokenize();

        /// @brief save current Lexer parameter from stream.
        /// @param output_stream specify target stream to store current Lexer parameter. 
        /// @return if save success will return true otherwhise will return false.
        bool save(std::ostream& output_stream);

        /// @brief load current Lexer parameter from stream.
        /// @param output_stream specify target stream to load current Lexer parameter. 
        /// @return if save success will return true otherwhise will return false.
        bool load(std::istream& input_stream);
        explicit Lexer();
        Punchation parse_mul_punc();
        void init_keyword(const std::vector<std::pair<std::string_view, TokenObj>> &key_words);
        void init_keyword(const std::pair<std::string_view, TokenObj> token_pair);
        [[nodiscard]] Token get_next_token();
        void set_source(std::istream* _source_file);
    private:
        [[nodiscard]] static double atod(const char* _Str, size_t* _Idx = nullptr);
        [[nodiscard]] static long long atoll(const char* _Str, size_t* _Idx = nullptr, int _Base = 10);
        [[nodiscard]] static size_t atoull(const char* _Str, size_t* _Idx = nullptr, int _Base = 10);
        bool skip_blank();
        Node *root;
        std::string linebuf;
        size_t line, column;
        std::istream* source_file;
        std::allocator<Node> allocator;   
    };
};

#endif