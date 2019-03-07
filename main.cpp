#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#include <vector>
#include <unordered_map>

struct Intern
{
    int length;
    const char* str;
};

std::vector<Intern> interns;

const char* InternString(const char* start, const char* end)
{
    int length = end - start;
    for (int i = 0; i < interns.size(); i++)
    {
        if (interns[i].length == length &&
            strncmp(start, interns[i].str, length) == 0)
        {
            return interns[i].str;
        }
    }

    char* newString = (char*)malloc(length * sizeof(char) + 1);
    memcpy(newString, start, length);
    newString[length] = 0;

    Intern intern;
    intern.length = length;
    intern.str = newString;

    interns.push_back(intern);

    return newString;
}

const char* InternString(const char* str)
{
    return InternString(str, str + strlen(str));
}

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

    union
    {
        const char* ident;
    };
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
    Token GetCurrentToken();
    void ExpectToken(TokenType type);
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
        if (c == '\"')
        {
        }
        else if (isalpha(c) || c == '_')
        {
            while (isalnum(*m_Str) || *m_Str == '_')
            {
                m_Str++;
            }

            result.type = Token_Identifier;
            result.ident = InternString(start, m_Str);
        }
    }

    m_CurrentToken = result;
    return result;
}

Token Tokenizer::GetCurrentToken() { return m_CurrentToken; }

void Tokenizer::ExpectToken(TokenType type)
{
    if (m_CurrentToken.type != type)
    {
        // TODO(patrik): Pretty printing pls
        printf("Unexpected token\n");
        exit(-1);
    }
    else
    {
        GetNextToken();
    }
}

/*enum TagType
{
    Tag_pml,
    Tag_function,
    Tag_call,

    Tag_custom,
};*/

#define TAG(name) const char* tag_name_##name

TAG(pml);
TAG(function);
TAG(call);

#define INIT_TAG(name) tag_name_##name = InternString(#name)

void InitTags()
{
    INIT_TAG(pml);
    INIT_TAG(function);
    INIT_TAG(call);
}

#undef TAG
#undef INIT_TAG

struct Attribute
{
    const char* name;
    const char* value;
};

struct Tag
{
    const char* name;
    std::vector<Attribute> attributes;
    std::vector<Tag*> children;
};

Tag* ParseTag(Tokenizer* tokenizer)
{
    Tag* tag = 0;

    tokenizer->ExpectToken(Token_LessThen);

    if (tokenizer->GetCurrentToken().type == Token_Identifier)
    {
        const char* name = tokenizer->GetCurrentToken().ident;

        tag = new Tag();
        tag->name = name;

        tokenizer->GetNextToken();
    }

    tokenizer->ExpectToken(Token_GreaterThen);

    return tag;
}

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
    InitTags();

    char* content = ReadFile("test.pml");

    Tokenizer tokenizer(content);

    tokenizer.GetNextToken();

    Tag* tag = ParseTag(&tokenizer);

    free(content);

    return 0;
}
