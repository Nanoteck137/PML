#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#include <vector>
#include <unordered_map>

void Error(const char* msg)
{
    printf("%s\n", msg);
    __debugbreak();
    getchar();
    exit(-1);
};

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

    Token_Equal,

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
        const char* str;
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
    Token PeekToken();
    Token GetCurrentToken();
    void ExpectToken(TokenType type);
};

Tokenizer::Tokenizer(const char* str) : m_Str(str) {}

Tokenizer::~Tokenizer() {}

bool IsWhitespace(char c)
{
    return (c == ' ' || c == '\t' || c == '\n' || c == '\r');
}

bool IsAlpha(char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

Token Tokenizer::GetNextToken()
{
    while (IsWhitespace(*m_Str))
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
	case '=': result.type = Token_Equal; break;
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
            while (*m_Str != '\"')
            {
                m_Str++;
            }

            result.type = Token_String;
            // NOTE(patrik): Do we want to intern here????
            result.str = InternString(start + 1, m_Str);
            m_Str += 1;
        }
        else if (IsAlpha(c) || c == '_')
        {
            while (isalnum(*m_Str) || *m_Str == '_')
            {
                m_Str++;
            }

            result.type = Token_Identifier;
            result.ident = InternString(start, m_Str);
        }
        else
        {
            Error("Unexpected token");
        }
    }

    m_CurrentToken = result;
    return result;
}

Token Tokenizer::PeekToken()
{
    const char* current = m_Str;
    Token currentToken = m_CurrentToken;
    Token result = GetNextToken();
    m_CurrentToken = currentToken;
    m_Str = current;

    return result;
}

Token Tokenizer::GetCurrentToken() { return m_CurrentToken; }

void Tokenizer::ExpectToken(TokenType type)
{
    if (m_CurrentToken.type != type)
    {
        // TODO(patrik): Pretty printing pls
        Error("Unexpected token");
    }
    else
    {
        GetNextToken();
    }
}

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

class Tag
{
private:
    const char* m_Name;
    std::vector<Attribute> m_Attributes;
    std::vector<Tag*> m_Children;

public:
    Tag(const char* name);
    ~Tag();

    void AddChild(Tag* child);

    void AddAttribute(Attribute attribute);

    inline const char* GetName() const { return m_Name; }
};

Tag::Tag(const char* name) : m_Name(name) {}

Tag::~Tag()
{
    for (int i = 0; i < m_Children.size(); i++)
    {
        delete m_Children[i];
    }
}

void Tag::AddChild(Tag* child) { m_Children.push_back(child); }

void Tag::AddAttribute(Attribute attribute)
{
    m_Attributes.push_back(attribute);
}

Attribute ParseAttribute(Tokenizer* tokenizer)
{
    Attribute result = {};
    if (tokenizer->GetCurrentToken().type == Token_Identifier)
    {
        const char* name = tokenizer->GetCurrentToken().ident;
        tokenizer->GetNextToken();

        tokenizer->ExpectToken(Token_Equal);

        // NOTE(patrik): Support more types like numbers
        if (tokenizer->GetCurrentToken().type == Token_String)
        {
            const char* str = tokenizer->GetCurrentToken().str;
            result.name = name;
            result.value = str;

            tokenizer->GetNextToken();
        }
    }
    else
    {
        Error("Expected token identifier");
    }

    return result;
}

Tag* ParseTag(Tokenizer* tokenizer)
{
    Tag* tag = nullptr;

    if (tokenizer->GetCurrentToken().type == Token_LessThen)
    {
        if (tokenizer->PeekToken().type == Token_ForwardSlash)
        {
            return nullptr;
        }

        tokenizer->GetNextToken();
    }
    else
    {
        Error("Expected Less then");
    }

    if (tokenizer->GetCurrentToken().type == Token_Identifier)
    {
        const char* name = tokenizer->GetCurrentToken().ident;

        tag = new Tag(name);

        tokenizer->GetNextToken();
    }
    else
    {
        Error("Expected Identifier");
    }

    while (tokenizer->GetCurrentToken().type != Token_GreaterThen &&
           tokenizer->GetCurrentToken().type != Token_ForwardSlash)
    {
        Attribute attribute = ParseAttribute(tokenizer);
        tag->AddAttribute(attribute);
    }

    if (tokenizer->GetCurrentToken().type != Token_ForwardSlash)
    {
        tokenizer->ExpectToken(Token_GreaterThen);

        Tag* child = ParseTag(tokenizer);
        while (child != nullptr)
        {
            tag->AddChild(child);
            child = ParseTag(tokenizer);
        }

        tokenizer->ExpectToken(Token_LessThen);
        tokenizer->ExpectToken(Token_ForwardSlash);

        if (tokenizer->GetCurrentToken().type == Token_Identifier)
        {
            if (tokenizer->GetCurrentToken().ident != tag->GetName())
            {
                Error("Tag missmatch");
            }

            tokenizer->GetNextToken();
        }
        else
        {
            Error("Expected identifier");
        }

        tokenizer->ExpectToken(Token_GreaterThen);
    }
    else
    {
        tokenizer->GetNextToken();
        tokenizer->ExpectToken(Token_GreaterThen);
    }

    return tag;
}

char* ReadFile(const char* filename)
{
    FILE* file = fopen(filename, "rt");

    fseek(file, 0, SEEK_END);
    int length = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* buffer = new char[length];
    memset(buffer, 0, length);

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

    Tag* root = ParseTag(&tokenizer);

    delete root;
    delete[] content;

    getchar();

    return 0;
}
