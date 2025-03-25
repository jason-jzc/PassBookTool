/*
    -------------------------------------------------------------------------
    Author: JiangZongchen
    Email: hello2022_jzc@yeah.net
    Time: 2025.3
    FileName: RSA.c
    -------------------------------------------------------------------------

    该文件为密码本文件操作的Python模块, 使用C语言编写
    该模块会将密钥, 用户邮箱地址等数据进行加密打包, 并保存为一个二进制文件
    同时提供邮箱地址防篡改检查功能, 防止恶意修改密码包

    生成二进制文件数据格式:
        (Byte 0~4) 文件类型数据, 共占用5个字节
        (Byte 5) 邮箱长度数据, 共占用1个字节
        (Byte 6~519) 加密后的邮箱数据(最大255字节), 和邮箱校验数据(最大255字节), 共占用514字节(空余4字节)
        (Byte 520~529) RSA私钥数据(最大8字节), 共占用10个字节(空余2字节)
        (Byte 530~531) 密码本中数据个数, 共占用2个字节, 单个密码本最多存储65535个账户密码
        (Byte >=533) 密码本实际数据, 单个密码本最大占用空间为63.8G
            (Byte 0~3) 密码名称长度DataNameLen(Bytes), 共占用4个字节
            (Byte 4~DataNameLen+3) 密码名称数据,共占用DataNameLen个字节, 最大占用4294967295字节(3.9GB)
            (Byte 3+DataNameLen~DataNameLen+7) 账户数据长度AccountDataLen(账户位数*2), 共占用4个字节, 账户最大位数可达到2147483647位
            (Byte DataNameLen+8~DataNameLen+7+AccountDataLen*8) 账户数据, 共占用AccountDataLen*8个字节, 最大占用34359738360字节(31.9GB)
            (Byte DataNameLen+AccountDataLen*8+8~DataNameLen+AccountDataLen*8+12) 密码数据长度, PasswordDataLen(密码位数*2), 共占用4个字节, 密码最大位数可达到2147483647位
            (Byte DataNameLen+AccountDataLen*8+13~DataNameLen+AccountDataLen*8+PasswordDataLen*8+12) 密码数据, 共占用PasswordDataLen*8个字节, 最大占用34359738360字节(31.9GB)
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <json-c/json.h>
#include <locale.h>

#include <Python.h>

// JSON数据中字段名, 请注意需要与python中的相同
#define EMAIL_JSON_NAME "email"
#define KEY_JSON_NAME "key"
#define DATA_JSON_NAME "data"
#define ACCOUNT_JSON_NAME "account"
#define PASSWORD_JSON_NAME "password"

// 文件数据位置, 请在修改前确认位置是否符合数据大小
#define EMAIL_POS 5
#define KEY_POS 520
#define DATA_POS 530

// 用于确定文件类型/文件版本讯息
#define FILE_TYPE 0x0000000000

// 进行按位运算时候的操作码
#define AND_DATA 0x11
#define OR_DATA 0x22
#define XOR_DATA 0x33

// 字符串类型
typedef char* string;

// 声明结构体, 用于存储单个密码数据
typedef struct
{
    string PassName;
    uint64_t* Account;
    uint64_t* Password;
    size_t Account_len;
    size_t Password_len;
} PassData;

// 声明结构体, 用于存储所有密码数据以及密码数据量
typedef struct 
{
    size_t data_count;
    PassData* data; 
} PassBook;

// 对字符串进行按位操作的函数 ========================================
string string_negation(string source)
{
    /*
    该函数用于对字符串中数据进行按位翻转
    */
    string retVal = (string)malloc((strlen(source) + 1) * sizeof(char));
    for (size_t i = 0; i < strlen(source); i++)
    {
        retVal[i] = ~source[i];
    }
    retVal[strlen(source)] = '\0';
    return retVal;
}

string string_and(string source)
{
    /*
    该函数用于对字符串中数据进行按位与操作
    */
    string retVal = (string)malloc((strlen(source) + 1) * sizeof(char));

    for (size_t i = 0; i < strlen(source); i++)
    {
        retVal[i] = source[i] & AND_DATA;
    }
    retVal[strlen(source)] = '\0';

    return retVal;
}

string string_or(string source)
{
    /*
    该函数用于对字符串中数据进行按位或操作
    */
    string retVal = (string)malloc((strlen(source) + 1) * sizeof(char));

    for (size_t i = 0; i < strlen(source); i++)
    {
        retVal[i] = source[i] | OR_DATA;
    }
    retVal[strlen(source)] = '\0';

    return retVal;
}

string string_xor(string source)
{
    /*
    该函数用于对字符串中数据进行按位异或操作
    */
    string retVal = (string)malloc((strlen(source) + 1) * sizeof(char));

    for (size_t i = 0; i < strlen(source); i++)
    {
        retVal[i] = source[i] ^ XOR_DATA;
    }

    retVal[strlen(source)] = '\0';

    return retVal;
}
// ================================================================

// 对uint64_t数组进行按位操作的函数 ==================================
uint64_t* uint64_pointer_negation(uint64_t* source, uint32_t len)
{
    /*
    该函数用于对uint64_t数组中数据进行按位非操作
    */
    uint64_t* retVal = (uint64_t*)malloc(sizeof(uint64_t) * len);

    for (uint32_t i = 0; i < len; i++)
    {
        retVal[i] = ~source[i];
    }

    return retVal;
}

uint64_t* uint64_pointer_and(uint64_t* source, uint32_t len)
{
    /*
    该函数用于对uint64_t数组中数据进行按位与操作
    */
    uint64_t* retVal = (uint64_t*)malloc(sizeof(uint64_t) * len);

    for (uint32_t i = 0; i < len; i++)
    {
        retVal[i] = source[i] & AND_DATA;
    }

    return retVal;
}

uint64_t* uint64_pointer_or(uint64_t* source, uint32_t len)
{
    /*
    该函数用于对uint64_t数组中数据进行按位或操作
    */
    uint64_t* retVal = (uint64_t*)malloc(sizeof(uint64_t) * len);

    for (uint32_t i = 0; i < len; i++)
    {
        retVal[i] = source[i] | OR_DATA;
    }

    return retVal;
}

uint64_t* uint64_pointer_xor(uint64_t* source, uint32_t len)
{
    /*
    该函数用于对uint64_t数组中数据进行按位异或操作
    */
    uint64_t* retVal = (uint64_t*)malloc(sizeof(uint64_t) * len);

    for (uint32_t i = 0; i < len; i++)
    {
        retVal[i] = source[i] ^ XOR_DATA;
    }

    return retVal;
}
// ================================================================


// 解析JSON数据函数 ================================================
string From_json_get_email(const string JsonData)
{
    /*
    该函数用于从JSON数据中解析出邮箱地址, 并返回字符串
    */

    struct json_object* parsed_json; // 创建一个JSON解析器
    parsed_json = json_tokener_parse(JsonData); // 解析JSON数据

    struct json_object* email; // 创建一个邮箱变量
    json_object_object_get_ex(parsed_json,EMAIL_JSON_NAME,&email); // 获取邮箱变量
    string retVal = strdup(json_object_get_string(email)); // 获取邮箱
    json_object_put(parsed_json); // 释放内存

    return retVal;
}

uint64_t From_json_get_key(const string JsonData)
{

    /*
    该函数用于从JSON数据中解析出密钥, 并返回字符串
    */

    struct json_object* parsed_json; // 创建一个JSON解析器
    parsed_json = json_tokener_parse(JsonData); // 解析JSON数据

    struct json_object* key; // 创建一个密钥变量
    json_object_object_get_ex(parsed_json,KEY_JSON_NAME,&key); // 获取密钥变量
    uint64_t retVal = json_object_get_uint64(key); // 获取密钥

    json_object_put(parsed_json); // 释放内存

    return retVal;
}

PassBook From_json_get_PasswordList(const string JsonData)
{
    /*
    该函数用于从JSON数据中解析出密码名称, 账号, 密码等数据, 并返回一个包含所有密码数据的结构体数组
    */
    struct json_object* parsed_json; // 创建一个JSON解析器
    parsed_json = json_tokener_parse(JsonData); // 解析JSON数据

    struct json_object* json_data; // 创建一个JSON数据变量
    json_object_object_get_ex(parsed_json,DATA_JSON_NAME,&json_data); // 获取JSON数据变量

    size_t data_count = json_object_object_length(json_data); // 获取JSON数据数组长度
    PassData* PassDataList = (PassData*)malloc(data_count * sizeof(PassData)); // 创建一个包含所有密码数据的结构体数组

    PassBook retVal; // 创建一个包含所有密码数据以及密码数据个数的结构体
    retVal.data_count = data_count; // 将密码数据个数保存到结构体中

    uint64_t count = 0;
    json_object_object_foreach(json_data,key,val)
    {
        struct json_object* account_json; // 创建一个账号变量
        struct json_object* password_json; // 创建一个密码变量

        json_object_object_get_ex(val,ACCOUNT_JSON_NAME,&account_json); // 获取账号变量
        json_object_object_get_ex(val,PASSWORD_JSON_NAME,&password_json); // 获取密码变量

        PassDataList[count].Account_len = json_object_array_length(account_json); // 获取账号数组长度
        PassDataList[count].Account = (uint64_t*)malloc(PassDataList[count].Account_len * sizeof(uint64_t)); // 申请内存
        
        PassDataList[count].Password_len = json_object_array_length(password_json); // 获取密码数组长度
        PassDataList[count].Password = (uint64_t*)malloc(PassDataList[count].Password_len * sizeof(uint64_t)); // 申请内存
    
        PassDataList[count].PassName = strdup(key); // 将获取的Key名称保存到PassData结构体中
        for (uint16_t i = 0; i < PassDataList[count].Account_len; i++)
        {
            /*
            将JSON数组中的加密后的账户数据保存到PassData结构体中
            */
            PassDataList[count].Account[i] = json_object_get_uint64(json_object_array_get_idx(account_json,i));
        }
        for (uint16_t i = 0; i < PassDataList[count].Password_len; i++)
        {
            /*
            将JSON数组中的加密后的密码数据保存到PassData结构体中
            */
            PassDataList[count].Password[i] = json_object_get_uint64(json_object_array_get_idx(password_json,i));
        }
        count++;
    }
    retVal.data = PassDataList;

    json_object_put(parsed_json);

    return retVal;
}

void FreePassBook(PassBook* book) {

    /*
    该函数用于释放PassBook结构体中的内存
    */

    for (size_t i = 0; i < book->data_count; i++) {
        free(book->data[i].PassName);  // 释放复制的字符串
        free(book->data[i].Account);
        free(book->data[i].Password);
    }
    free(book->data);
}
// ================================================================


// 读取JSON文件函数 =================================================
string read_json_file(string FilePath)
{

    /*
    该函数用于读取JSON文件, 并返回一个字符串, 该字符串为JSON的所有数据
    */

    FILE *file = fopen(FilePath, "r"); // 打开文件
    if (!file)
    {
        PyErr_SetString(PyExc_FileNotFoundError, "File not found");
        return NULL;
    }

    fseek(file,0,SEEK_END); // 获取文件大小
    long file_size = ftell(file);
    fseek(file,0,SEEK_SET);
    
    string file_data = (string)malloc(file_size + 1); // 申请内存
    if (!file_data)
    {
        fclose(file);
        PyErr_SetString(PyExc_MemoryError, "Memory error");
        return NULL;
    }

    fread(file_data, 1, file_size, file); // 读取文件数据
    file_data[file_size] = '\0'; // 结束字符串
    fclose(file); // 关闭文件

    return file_data;
}
// ================================================================


// 写入文件函数 ======================================================
int write_email(string Email, FILE* file)
{
    /*
    该函数用于将邮箱长度, 邮箱地址和邮箱校验数据写入二进制文件, 并返回一个执行状态
    执行状态:
        0: 成功
        -1: 写入邮箱长度失败
        -2: 写入邮箱地址失败
        -3: 写入邮箱校验数据失败 -> 暂未开发
    邮箱部分二进制格式:
        邮箱长度(1 Byte)
        邮箱地址(max 255 Bytes)
        邮箱校验数据(max 255 Bytes) -> 暂未开发
    */

    // string encryptEmail = string_negation(Email);
    // string checkEmail = string_or(Email);

    uint8_t Email_len = strlen(Email); // 获取邮箱长度
    string Email_encrypt = string_xor(string_negation(Email));

    fseek(file,EMAIL_POS,SEEK_SET); // 将文件指针移动到对应位置
    if (fwrite(&Email_len, sizeof(uint8_t), 1, file) != 1) // 写入邮箱长度
    {
        return -1;
    }

    fseek(file,EMAIL_POS + 1,SEEK_SET);
    if (fwrite(Email_encrypt, sizeof(char), Email_len, file) != Email_len) // 写入邮箱地址
    {
        return -2;
    }

    return 0;
}

int write_key(uint64_t Key, FILE* file)
{
    /*
    该函数用于将密钥写入二进制文件, 并返回一个执行状态
    密钥默认写入KEY_POS位置
    执行状态:
        0: 成功
        -1: 写入密钥失败
    */

    uint64_t Key_encrypt = (~Key) ^ XOR_DATA;

    fseek(file,KEY_POS,SEEK_SET);
    if (fwrite(&Key_encrypt, sizeof(uint64_t), 1, file) != 1) // 写入密钥
    {
        return -1;
    }

    return 0;
}

int write_data(PassBook book, FILE* file)
{
    /*
    该函数用于将PassBook结构体中的数据写入二进制文件, 并返回一个执行状态
    执行状态:
        0: 成功
        -1: 写入数据数量失败
        -2: 写入数据名称长度失败
        -3: 写入数据名称失败
        -4: 写入账户长度失败
        -5: 写入账户失败
        -6: 写入密码长度失败
        -7: 写入密码失败
    */

    uint16_t data_count = (uint16_t)book.data_count; // 获取数据数量
    uint16_t data_count_encrypt = (~data_count) ^ XOR_DATA; // 加密数据数量数据

    fseek(file,DATA_POS,SEEK_SET); // 将文件指针移动到文件结尾
    if (fwrite(&data_count_encrypt, sizeof(uint16_t), 1, file) != 1) // 将数据数量写入文件
    {
        return -1;
    }

    PassData* PassDataList = book.data; // 获取PassBook结构体中的数据
    for (uint16_t i = 0; i < data_count; i++)
    {
        string dataName = PassDataList[i].PassName; // 获取数据名称
        string dataName_encrypt = string_xor(string_negation(dataName)); // 加密数据名称

        uint32_t dataNameLen = (uint32_t)strlen(dataName); // 获取数据名称长度
        // printf("dataNameLen: %d\n", dataNameLen);
        uint32_t dataNameLen_encrypt = (~dataNameLen) ^ XOR_DATA; // 加密数据名称长度

        fseek(file, 0, SEEK_END); // 将文件指针移动到文件结尾
        if (fwrite(&dataNameLen_encrypt, sizeof(uint32_t), 1, file) != 1) // 写入数据名称长度
        {
            return -2;
        }

        fseek(file, 0, SEEK_END); // 将文件指针移动到文件结尾
        if (fwrite(dataName_encrypt, sizeof(char), dataNameLen, file) != dataNameLen) // 写入数据名称
        {
            return -3;
        }
        
        uint32_t account_len = (uint32_t)PassDataList[i].Account_len; // 获取账户长度
        uint32_t account_len_encrypt = (~account_len) ^ XOR_DATA; // 加密账户长度数据

        uint64_t* account = PassDataList[i].Account; // 获取账户数据
        uint64_t* account_encrypt = uint64_pointer_xor(uint64_pointer_negation(account,account_len),account_len); // 加密账户数据

        fseek(file, 0, SEEK_END); // 将文件指针移动到文件结尾
        if (fwrite(&account_len_encrypt, sizeof(uint32_t), 1, file) != 1) // 写入账户长度
        {
            return -4;
        }

        fseek(file, 0, SEEK_END); // 将文件指针移动到文件结尾
        if (fwrite(account_encrypt, sizeof(uint64_t), account_len, file) != account_len) // 写入账户数据
        {
            return -5;
        }

        uint32_t password_len = (uint32_t)PassDataList[i].Password_len; // 获取密码长度
        uint32_t password_len_encrypt = (~password_len) ^ XOR_DATA; // 加密密码长度数据

        uint64_t* password = PassDataList[i].Password; // 获取密码数据
        uint64_t* password_encrypt = uint64_pointer_xor(uint64_pointer_negation(password,password_len),password_len); // 加密密码数据

        fseek(file, 0, SEEK_END); // 将文件指针移动到文件结尾
        if (fwrite(&password_len_encrypt, sizeof(uint32_t), 1, file) != 1) // 写入密码长度
        {
            return -6;
        }
        
        fseek(file, 0, SEEK_END); // 将文件指针移动到文件结尾
        if (fwrite(password_encrypt, sizeof(uint64_t), password_len, file) != password_len) // 写入密码数据
        {
            return -7;
        }
    }
    return 0;
}
// =================================================================

// 读取文件数据 ====================================================
string read_email(FILE* file)
{
    /*
    该函数用于从二进制文件中获取邮箱地址并进行验证(验证部分暂未开发)
    */
    uint8_t Email_len; // 邮箱长度

    fseek(file,EMAIL_POS,SEEK_SET);
    if (fread(&Email_len, sizeof(uint8_t), 1, file) != 1) // 读取邮箱长度
    {
        return NULL;
    }

    string Email_encrypt = (string)malloc(Email_len + 1); // 邮箱地址

    fseek(file,EMAIL_POS + 1,SEEK_SET);
    if (fread(Email_encrypt, sizeof(char), Email_len, file) != Email_len) // 读取邮箱地址
    {
        free(Email_encrypt);
        return NULL;
    }

    Email_encrypt[Email_len] = '\0';
    string Email = string_negation(string_xor(Email_encrypt));
    free(Email_encrypt);

    return Email;
}

uint64_t read_key(FILE* file)
{
    /*
    该函数用于从二进制文件中获取密钥并进行验证
    */

    uint64_t Key_encrypt; // 密钥

    fseek(file,KEY_POS,SEEK_SET);
    if (fread(&Key_encrypt, sizeof(uint64_t), 1, file) != 1) // 读取密钥
    {
       return 0;
    }

    uint64_t key = ~(Key_encrypt ^ XOR_DATA);

    return key;
}

PassBook read_data(FILE* file)
{
    /*
    该函数用于从二进制文件中获取数据并进行验证(验证部分暂未开发)
    */

    PassBook book; // PassBook结构体, 用于返回数据

    uint16_t data_count_encrypt; // 数据数量(原始数据)
    uint16_t data_count; // 数据数量(解密后数据)
    
    fseek(file,DATA_POS,SEEK_SET); // 移动文件指针
    fread(&data_count_encrypt, sizeof(uint16_t), 1, file); // 读取数据数量

    data_count = ~(data_count_encrypt ^ XOR_DATA); // 解密数据数量
    book.data_count = data_count; // 设置数据数量

    PassData* PassDataList = (PassData*)malloc(data_count * sizeof(PassData)); // 申请PassData结构体数组

    long NowSeek = DATA_POS + sizeof(uint16_t); // 当前指针位置

    for (uint16_t i = 0; i < data_count; i++)
    {
        // NowSeek += sizeof(uint16_t);

        uint32_t dataNameLen_encrypt; // 数据名称长度(原始数据)
        uint32_t dataNameLen; // 数据名称长度(解密后数据)

        fseek(file, NowSeek, SEEK_SET);
        fread(&dataNameLen_encrypt, sizeof(uint32_t), 1, file);

        dataNameLen = ~(dataNameLen_encrypt ^ XOR_DATA); // 解密数据名称长度

        NowSeek += sizeof(uint32_t); // 更新当前指针位置变量

        string passname_encrypt = (string)malloc(dataNameLen + 1); // 数据名称(原始数据)
        string passname = (string)malloc(dataNameLen + 1); // 数据名称(解密后数据)

        fseek(file, NowSeek, SEEK_SET); // 设置当前指针位置
        fread(passname_encrypt, sizeof(char), dataNameLen, file); // 读取数据名称

        passname_encrypt[dataNameLen] = '\0';
        passname = string_negation(string_xor(passname_encrypt)); // 解密数据名称
        PassDataList[i].PassName = passname; // 设置数据名称

        NowSeek += dataNameLen * sizeof(char); // 更新当前指针位置变量

        uint32_t account_len_encrypt; // 账户长度(原始数据)
        uint32_t account_len; // 账户长度(解密后数据)

        fseek(file, NowSeek, SEEK_SET); // 设置当前指针位置
        fread(&account_len_encrypt, sizeof(uint32_t), 1, file); // 读取账户长度

        account_len = ~(account_len_encrypt ^ XOR_DATA); // 解密账户长度
        PassDataList[i].Account_len = account_len; // 设置账户长度

        NowSeek += sizeof(uint32_t); // 更新当前指针位置变量

        uint64_t* account_encrypt = (uint64_t*)malloc(account_len * sizeof(uint64_t)); // 账户数据(原始数据)
        uint64_t* account = (uint64_t*)malloc(account_len * sizeof(uint64_t)); // 账户数据(解密后数据)

        fseek(file, NowSeek, SEEK_SET); // 设置当前指针位置
        fread(account_encrypt, sizeof(uint64_t), account_len, file); // 读取账户数据

        account = uint64_pointer_negation(uint64_pointer_xor(account_encrypt, account_len), account_len); // 解密账户数据
        PassDataList[i].Account = account; // 设置账户数据

        NowSeek += account_len * sizeof(uint64_t); // 更新当前指针位置变量
        
        uint32_t password_len_encrypt; // 密码长度(原始数据)
        uint32_t password_len; // 密码长度(解密后数据)

        fseek(file, NowSeek, SEEK_SET); // 设置当前指针位置
        fread(&password_len_encrypt, sizeof(uint32_t), 1, file); // 读取密码长度

        password_len = ~(password_len_encrypt ^ XOR_DATA); // 解密密码长度
        PassDataList[i].Password_len = password_len; // 设置密码长度

        NowSeek += sizeof(uint32_t); // 更新当前指针位置变量

        uint64_t* password_encrypt = (uint64_t*)malloc(password_len * sizeof(uint64_t)); // 密码数据(原始数据)
        uint64_t* password = (uint64_t*)malloc(password_len * sizeof(uint64_t)); // 密码数据(解密后数据)

        fseek(file, NowSeek, SEEK_SET); // 设置当前指针位置
        fread(password_encrypt, sizeof(uint64_t), password_len, file); // 读取密码数据

        password = uint64_pointer_negation(uint64_pointer_xor(password_encrypt, password_len), password_len); // 解密密码数据
        PassDataList[i].Password = password; // 设置密码数据

        NowSeek += password_len * sizeof(uint64_t); // 更新当前指针位置变量
    }

    book.data = PassDataList; // 设置数据

    return book;
}
// ===============================================================================


// For Python
// ================================================================================

static PyObject* WriteFile(PyObject* self, PyObject* args)
{
    string InputFilePath; // 输入JSON文件路径
    string OutputFilePath; // 输出二进制文件路径

    if (!PyArg_ParseTuple(args, "ss", &InputFilePath, &OutputFilePath))
    {
        return NULL;
    }

    string jsonData = read_json_file(InputFilePath); // 获取JSON数据
    string email = From_json_get_email(jsonData); // 解析JSON数据, 获取邮箱地址
    uint64_t key = From_json_get_key(jsonData); // 解析JSON数据, 获取密钥
    PassBook Data = From_json_get_PasswordList(jsonData); // 解析JSON数据, 获取账户密码等数据

    FILE* file;
    int code = 0; // 获取错误代码

    file = fopen(OutputFilePath, "wb+"); // 打开文件
    if (file == NULL)
    {
        PyErr_SetString(PyExc_RuntimeError, "FileOperation: File not found! ");
        return NULL;
    }

    code += write_email(email, file); // 写入邮箱地址
    code += write_key(key, file); // 写入密钥
    code += write_data(Data,file); // 写入账户密码等数据
    fclose(file); // 关闭文件
    FreePassBook(&Data); // 释放内存

    code += remove(InputFilePath); // 删除JSON临时文件，保证数据安全

    if (code != 0)
    {
        PyErr_SetString(PyExc_RuntimeError, "FileOperation: WriteFile failed! ");
        return NULL;
    }

    Py_RETURN_NONE;
}

static PyObject* GetData(PyObject* self, PyObject* args)
{
    /*
    该函数用于读取二进制文件, 获取账户密码等数据, 并返回一个Python字典
    */

    string FilePath; // 文件路径

    if (!PyArg_ParseTuple(args, "s", &FilePath))
    {
        return NULL;
    }

    FILE* file;
    // int code = 0;

    file = fopen(FilePath, "rb"); // 打开文件
    if (file == NULL)
    {
        PyErr_SetString(PyExc_RuntimeError, "FileOperation: File not found! ");
        return NULL;
    }

    PassBook book = read_data(file); // 获取数据
    fclose(file); // 关闭文件

    PyObject* retVal = PyDict_New(); // 创建一个字典, 用于返回数据

    for (size_t i = 0; i < book.data_count; i++)
    {
        PyObject* PassData = PyDict_New(); // 创建一个字典, 用于存储一个账户密码等数据

        PyObject* account = PyList_New(book.data[i].Account_len); // 创建一个列表, 用于存储一个账户
        for (size_t j = 0; j < book.data[i].Account_len; j++)
        {
            PyList_SetItem(account, j, PyLong_FromLong(book.data[i].Account[j]));
        }
        PyDict_SetItemString(PassData, ACCOUNT_JSON_NAME, account); // 添加账户数据到字典中

        PyObject* password = PyList_New(book.data[i].Password_len); // 创建一个列表, 用于存储一个密码
        for (size_t j = 0; j < book.data[i].Password_len; j++)
        {
            PyList_SetItem(password, j, PyLong_FromLong(book.data[i].Password[j]));
        }
        PyDict_SetItemString(PassData, PASSWORD_JSON_NAME, password); // 添加密码数据到字典中
        
        PyDict_SetItemString(retVal, book.data[i].PassName, PassData); // 添加账户密码等数据到字典中
    }
    
    FreePassBook(&book); // 释放内存

    return retVal;
}

static PyObject* GetKey(PyObject* self, PyObject* args)
{
    /*
    该函数用于获取密钥
    */

    string FilePath; // 文件路径

    if (!PyArg_ParseTuple(args, "s", &FilePath))
    {
        return NULL;
    }

    FILE* file;

    file = fopen(FilePath, "rb"); // 打开文件
    if (file == NULL)
    {
        PyErr_SetString(PyExc_RuntimeError, "FileOperation: File not found! ");
        return NULL;
    }

    uint64_t key = read_key(file); // 获取密钥

    fclose(file); // 关闭文件

    PyObject* retVal = PyLong_FromLong(key); // 创建一个整数, 用于返回密钥

    return retVal;
}

static PyObject* CheckEmail(PyObject* self, PyObject* args)
{
    /*
    该函数用于检查邮箱地址是否正确
    */

    string input_email;
    string FilePath;

    string real_email;

    if (!PyArg_ParseTuple(args, "ss", &input_email, &FilePath))
    {
        return NULL;
    }

    FILE* file = fopen(FilePath, "rb");

    real_email = read_email(file);

    fclose(file);

    PyObject* retVal;
    if (strcmp(input_email, real_email) == 0)
    {
        retVal = Py_BuildValue("s",real_email);
    }
    else
    {
        retVal = Py_None;
        Py_INCREF(Py_None);
    }

    free(real_email);

    return retVal;
}

static PyMethodDef FileOperationMethods[] = {
    {"WriteFile", WriteFile, METH_VARARGS, "WriteFile"},
    {"GetData", GetData, METH_VARARGS, "GetData"},
    {"GetKey", GetKey, METH_VARARGS, "GetKey"},
    {"CheckEmail", CheckEmail, METH_VARARGS, "CheckEmail"},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef FileOperationmodule = {
    PyModuleDef_HEAD_INIT,
    "FileOperation",
    NULL,
    -1,
    FileOperationMethods
};

PyMODINIT_FUNC PyInit_FileOperation(void)
{
    // setlocale(LC_ALL, "zh_CN.UTF-8");
    return PyModule_Create(&FileOperationmodule);
}