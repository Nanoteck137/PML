#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#include <vector>
#include <unordered_map>

enum TokenType
{
    Token_Unknown,

    Token_Identifier,
    Token_String,
    Token_Number,

    Token_GreaterThen,
    Token_LessThen,

    Token_Plus,
    Token_Minus,
    Token_Asterisk,

    Token_ForwardSlash,
    Token_BackwordSlash,

    Token_EndOfStream,
};

struct Token
{
    TokenType type;

    const char* start;
    const char* end;
};

class Tokenizer
{
private:
    const char* m_Str;

    Token m_CurrentToken;

public:
    Tokenizer(const char* str);
    ~Tokenizer();

    Token GetNextToken();
};

Tokenizer::Tokenizer(const char* str) : m_Str(str) {}

Tokenizer::~Tokenizer() {}

Token Tokenizer::GetNextToken()
{
    while (isspace(*m_Str))
    {
        m_Str++;
    }

    Token result = {};
    result.type = Token_Unknown;

    const char* start = m_Str;
    char c = *m_Str++;

    result.start = start;
    result.end = m_Str;

    switch (c)
    {
    // clang-format off
    case '\0': result.type = Token_EndOfStream; break;
    case '>': result.type = Token_GreaterThen; break;
    case '<': result.type = Token_LessThen; break;
    case '+': result.type = Token_Plus; break;
    case '-': result.type = Token_Minus; break;
    case '*': result.type = Token_Asterisk; break;
    case '/': result.type = Token_ForwardSlash; break;
    case '\\': result.type = Token_BackwordSlash; break;
        // clang-format on
    default:
        if (isalpha(c) || c == '_')
        {
            while (isalnum(*m_Str) || *m_Str == '_')
            {
                m_Str++;
            }

            result.type = Token_Identifier;
            result.start = start;
            result.end = m_Str;
        }
    }

    m_CurrentToken = result;
    return result;
}

enum TagType
{
    Tag_pml,
    Tag_function,
    Tag_call
};

struct Tag
{
    TagType type;
    std::vector<Tag> children;
};

Tag ParseTag(Tokenizer* tokenizer) {}

char* ReadFile(const char* filename)
{
    FILE* file = fopen(filename, "rt");

    fseek(file, 0, SEEK_END);
    int length = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* buffer = (char*)malloc(length * sizeof(char) + 1);
    buffer[length] = 0;

    fread(buffer, 1, length, file);

    fclose(file);

    return buffer;
}

int main(int argc, char** argv)
{
    char* content = ReadFile("test.pml");

    Tokenizer tokenizer(content);

    free(content);

    return 0;
}
