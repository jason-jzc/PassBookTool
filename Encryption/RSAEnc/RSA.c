/*
    -------------------------------------------------------------------------
    Author: JiangZongchen
    Email: hello2022_jzc@yeah.net
    Time: 2025.3
    FileName: RSA.c
    -------------------------------------------------------------------------

    RSA加密/解密算法，仅支持英文字符串加密/解密
    默认使用p=57077,q=32327

    python函数与c语言函数对应关系：
    |python函数|                |c语言函数|
    Info()                      printInfo()
    getPublicKey()              getPublicKey()
    getPrivateKey()             getPrivateKey()
    encrypt()                   encrypt()
    decrypt()                   decrypt()
    clean()                     clean()
*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include <wchar.h>
#include <locale.h>

#include <Python.h>

typedef char* string;

// p和q是两个素数，用于生成密钥对
#define p 101
#define q 313

#define QUANTITY 1 // 生成公钥数量
#define D_VALUE 0 // 生成公钥的最小差值

#define CHECK_NUM 4 // 校验数据位检查码

static int64_t N = p * q; // N = p * q 且N为密钥对中共同的值
static int64_t E; // E是公钥数据，用于加密
static int64_t D; // D是私钥数据，用于解密
static int64_t L; // L是生成密钥的过程数据

uint8_t* DataList_Int;
string Data_str;

uint64_t gcd(int64_t a, int64_t b)
{
    /*
    用于计算最大公约数
    */

    while (b != 0)
    {
        int64_t temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}

uint64_t lcm(int64_t a, int64_t b)
{

    /*
    用于计算最小公倍数
    */

    return abs(a * b) / gcd(a,b);
}

uint64_t mod_pow(uint64_t base, uint64_t exp, uint64_t mod) {
    /*
    用于计算a的b次方取模
    */
    uint64_t result = 1;
    base = base % mod;
    while (exp > 0) {
        if (exp % 2 == 1) {  // 如果exp是奇数，将base乘到结果中
            result = (result * base) % mod;
        }
        exp >>= 1;  // 将exp除以2
        base = (base * base) % mod;  // 平方base
    }
    return result;
}

int64_t extended_gcd(int64_t a, int64_t b, int64_t* x, int64_t* y) {
    /*
    用于计算扩展欧几里得算法
    */
    if (a == 0) {
        *x = 0;
        *y = 1;
        return b;
    }
    int64_t x1, y1;
    int64_t gcd = extended_gcd(b % a, a, &x1, &y1);
    *x = y1 - (b / a) * x1;
    *y = x1;
    return gcd;
}

int64_t mod_inverse(int64_t a, int64_t m) {
    /*
    用于计算模反元素
    */
    int64_t x, y;
    int64_t gcd = extended_gcd(a, m, &x, &y);
    if (gcd != 1) {
        return -1; // Modular inverse does not exist
    }
    else {
        return (x % m + m) % m; // Ensure the result is positive
    }
}

uint8_t* str_to_intList(string sourceData,uint64_t len)
{

    /*
    用于将字符串转换为整数列表, 根据ASCII码生成整数列表
    */
    
    // printf("str_to_intList %lu\n",len);
    uint8_t* intList = (uint8_t*)malloc(len * sizeof(uint8_t));

    for (uint64_t i = 0; i < len; i++)
    {
        intList[i] = (uint8_t)sourceData[i];
        // wchar_t wchar = sourceData[i];
        // intList[i] = (uint8_t)wchar;
    }

    return intList;
}

string intList_to_string(uint8_t* sourceData,uint64_t len)
{

    /*
    用于将整数列表转换为字符串, 根据ASCII码还原字符串
    */

    string strdata = (string)malloc((len + 1) * sizeof(char));

    for (uint64_t i = 0; i < len; i++)
    {
        strdata[i] = (char)sourceData[i];
    }

    strdata[len] = '\0';

    return strdata;
}


// For Python
// ================================================================================================================

static PyObject* printInfo(PyObject* self, PyObject* args)
{

    /*
    输出欢迎信息，用于Python调用
    */

    printf("Welcome to use RSA encrypt/decrypt algorithm!");
    Py_RETURN_NONE;  // 使用Py_RETURN_NONE宏来返回Py_None
}

static PyObject* getPublicKey(PyObject* self, PyObject* args)
{

    /*
    生成公钥，根据RSA算法 1 < E < L 且 E与L互质
    为确保生成效率，则仅生成QUANTITY个E，并随机返回一个
    且同时为保证安全，保证这十个生成的公钥差值大于D_VALUE
    */

    srand(time(NULL) ^ getpid()); // 以当前时间为随机种子，确保随机性
    int64_t min = 1 + rand() % (L / 2); // 生成公钥最小值

    int64_t randomNumList_Pub[QUANTITY]; // 生成公钥列表

    int8_t count = 0; // 计数
    for (int64_t i = min; i < L; i++)
    {
        if (count >= QUANTITY)
        {
            // 仅生成QUANTITY个公钥，确保生成效率
            break;
        }
        else if (1 < i && gcd(i,L) == 1){
            randomNumList_Pub[count] = i;
            count++;
            i += D_VALUE;
        }
    }

    int randomNumList_size = sizeof(randomNumList_Pub) / sizeof(randomNumList_Pub[0]);
    E = randomNumList_Pub[rand() % randomNumList_size];

    PyObject* retVal = (PyObject*)Py_BuildValue("l",E);

    return retVal;
}

static PyObject* getPrivateKey(PyObject* self, PyObject* args)
{
    /*
    生成私钥，根据RSA算法 E * D mod L = 1 且 1 < D < L
    函数传入参数为公钥E
    */

    if (!PyArg_ParseTuple(args,"l",&E))
    {
        return NULL;
    }
    
    D = mod_inverse(E,L);

    PyObject* retVal = (PyObject*)Py_BuildValue("l",D);

    return retVal;

}

static PyObject* encrypt(PyObject* self, PyObject* args)
{

    /*
    字符串加密算法, 根据RSA算法 E * D mod L = 1 且 1 < D < L
    */

    string sourceData;

    if (!PyArg_ParseTuple(args, "sl", &sourceData,&E))
    {
        return NULL;
    }

    uint64_t DataLen = strlen(sourceData);
    // printf("DataLen: %lu\n",DataLen);
    PyObject* retVal = (PyObject*)PyList_New(DataLen * 2);

    DataList_Int = str_to_intList(sourceData,DataLen);

    for (uint64_t i = 0; i < DataLen; i++)
    {
        PyList_SetItem(retVal, i * 2, Py_BuildValue("l",mod_pow(DataList_Int[i], E, N)));
        PyList_SetItem(retVal, i * 2 + 1, Py_BuildValue("l",mod_pow(DataList_Int[i] , E, N) >> CHECK_NUM));
    }

    return retVal;
}

static PyObject* decrypt(PyObject* self, PyObject* args)
{
    PyObject* encryptData;
    if (!PyArg_ParseTuple(args, "Ol", &encryptData,&D)) {
        return NULL;
    }

    // 参数类型验证
    if (!PyList_Check(encryptData)) {
        PyErr_SetString(PyExc_TypeError, "encryptData must be a list");
        return NULL;
    }

    Py_ssize_t listSize = PyList_Size(encryptData);
    
    // 验证列表长度有效性
    if (listSize % 2 != 0) {
        PyErr_SetString(PyExc_ValueError, "Encrypted data list length must be even");
        return NULL;
    }
    const Py_ssize_t dataLen = listSize / 2;

    // 分配内存并验证
    free(DataList_Int);  // 释放先前可能存在的内存
    if ((DataList_Int = (uint8_t*)malloc(dataLen * sizeof(uint8_t))) == NULL) {
        PyErr_SetString(PyExc_MemoryError, "Memory allocation failed");
        return NULL;
    }

    // 处理加密数据
    for (Py_ssize_t i = 0; i < listSize; i += 2) {
        // 获取并验证元素
        PyObject* item1 = PyList_GetItem(encryptData, i);
        PyObject* item2 = PyList_GetItem(encryptData, i+1);
        if (!PyLong_Check(item1) || !PyLong_Check(item2)) {
            PyErr_SetString(PyExc_TypeError, "List items must be integers");
            free(DataList_Int);
            return NULL;
        }

        // 验证数据对有效性
        const long encrypted1 = PyLong_AsLong(item1);
        const long encrypted2 = PyLong_AsLong(item2);
        if (encrypted1 >> CHECK_NUM != encrypted2) {
            PyErr_SetString(PyExc_ValueError, "Encrypted pair mismatch");
            free(DataList_Int);
            // Py_RETURN_NONE;
            return NULL;
        }

        // 执行解密计算
        DataList_Int[i/2] = mod_pow(encrypted1, D, N);
    }

    // 构建返回字符串
    string decrypted_str = intList_to_string(DataList_Int, dataLen);
    PyObject* retVal = Py_BuildValue("s", decrypted_str);
    free(decrypted_str);  // 释放临时字符串内存
    
    return retVal;
}

static PyObject* clean(PyObject* self, PyObject* args)
{

    /*
    清理指针数据，保证下次使用时，不会出现内存泄漏
    */

    if (DataList_Int != NULL)
    {
        free(DataList_Int);
    }
    if (Data_str != NULL)
    {
        free(Data_str);
    }

    Py_RETURN_NONE;
}

static PyMethodDef RSAMethods[] = {
    {"Info", printInfo, METH_VARARGS, "Print Hello"},
    {"getPublicKey",getPublicKey,METH_VARARGS,"Get Public Key"},
    {"getPrivateKey",getPrivateKey,METH_VARARGS,"Get Private Key"},
    {"encrypt", encrypt, METH_VARARGS, "Encrypt"},
    {"decrypt", decrypt, METH_VARARGS, "Decrypt"},
    {"clean", clean, METH_VARARGS, "Clean"},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef RSAmodule = {
    PyModuleDef_HEAD_INIT,
    "RSA",
    NULL,
    -1,
    RSAMethods
};

PyMODINIT_FUNC PyInit_RSA(void)
{
    L = lcm(p-1, q-1);
    setlocale(LC_ALL, "en_US.UTF-8");
    return PyModule_Create(&RSAmodule);
}