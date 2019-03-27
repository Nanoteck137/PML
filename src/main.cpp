#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#include <vector>
#include <unordered_map>

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>

void Error(const char* msg)
{
    printf("%s\n", msg);
    //__debugbreak();
    __builtin_trap();
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

#define TAG_EQ(tag, name) if (tag->GetName() == tag_name_##name)

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
    Tag* GetChild(int index);

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

Tag* Tag::GetChild(int index) { return m_Children[index]; }

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

    char* buffer = new char[length + 1];
    memset(buffer, 0, length + 1);

    fread(buffer, 1, length, file);

    fclose(file);

    return buffer;
}

class Context
{
private:
    llvm::LLVMContext m_Context;
    llvm::Module* m_Module;
    llvm::IRBuilder<>* m_Builder;

    std::unordered_map<std::string, llvm::Type*> m_Types;

public:
    Context();
    ~Context();

    llvm::Type* GetType(const std::string& name) { return m_Types[name]; }

    llvm::LLVMContext& GetLLVMContext() { return m_Context; }
    llvm::Module* GetModule() const { return m_Module; }
    llvm::IRBuilder<>& GetBuilder() const { return *m_Builder; }
};

Context::Context()
{
    m_Module = new llvm::Module("wooh", m_Context);

    m_Types["int8"] = llvm::IntegerType::get(m_Context, 8);
    m_Types["int16"] = llvm::IntegerType::get(m_Context, 16);
    m_Types["int32"] = llvm::IntegerType::get(m_Context, 32);
    m_Types["int64"] = llvm::IntegerType::get(m_Context, 64);

    m_Builder = new llvm::IRBuilder<>(m_Context);
}
Context::~Context() { delete m_Module; }

void GenerateCode(Context* context, Tag* tag)
{
    TAG_EQ(tag, pml) { GenerateCode(context, tag->GetChild(0)); }

    TAG_EQ(tag, function)
    {
        llvm::FunctionType* type =
            llvm::FunctionType::get(context->GetType("int32"), false);

        llvm::Function* function = llvm::Function::Create(
            type, llvm::GlobalValue::LinkageTypes::ExternalLinkage, "main",
            context->GetModule());

        llvm::BasicBlock* block =
            llvm::BasicBlock::Create(context->GetLLVMContext(), "", function);

        context->GetBuilder().SetInsertPoint(block);
    }
}

int main(int argc, char** argv)
{
    InitTags();

    Context* context = new Context();

    std::vector<llvm::Type*> funcParams = {
        llvm::PointerType::get(context->GetType("int8"), 0)};

    llvm::FunctionType* type =
        llvm::FunctionType::get(context->GetType("int32"), funcParams, true);

    llvm::Function* printfFunc = llvm::Function::Create(
        type, llvm::GlobalValue::LinkageTypes::ExternalLinkage, "printf",
        context->GetModule());

    char* content = ReadFile("test.pml");

    Tokenizer tokenizer(content);

    tokenizer.GetNextToken();

    Tag* root = ParseTag(&tokenizer);
    GenerateCode(context, root);

    llvm::Value* test =
        context->GetBuilder().CreateGlobalStringPtr("Hello World\n");

    std::vector<llvm::Value*> callParams = {test};
    context->GetBuilder().CreateCall(printfFunc, callParams);

    context->GetBuilder().CreateRet(
        llvm::ConstantInt::get(context->GetType("int32"), 0));

    context->GetModule()->print(llvm::outs(), nullptr);

    delete root;
    delete[] content;

    delete context;

    getchar();

    return 0;
}
